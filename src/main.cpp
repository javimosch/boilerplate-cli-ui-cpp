// boilerplate-cli-ui-cpp — a C++ CLI with an embedded web UI.
//
// The command surface follows the agent-first CLI specs
// (https://cli-specs.intrane.fr):
//   cli-output-spec  data on stdout, context on stderr, exit codes 80-119,
//                    typed errors, help-json
//   cli-guide-spec   `guide`, embedded in the binary
//   cli-daemon-spec  `serve --host --port`, /_health, /_shutdown,
//                    `daemon start|stop|status`

#include <chrono>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>

#include <httplib.h>

#include "guide.h"

// Include embedded UI files
#include "ui/index_html.h"
#include "ui/app_js.h"
#include "ui/styles_css.h"
#include "ui/components/AppLayout_js.h"
#include "ui/components/Sidebar_js.h"
#include "ui/components/StatusCard_js.h"
#include "ui/views/Dashboard_js.h"
#include "ui/views/Settings_js.h"

// Semantic exit codes (cli-output-spec §2).
enum : int {
    EXIT_MISSING_ARG = 80,
    EXIT_UNKNOWN_COMMAND = 85,
    EXIT_PRECONDITION = 90,
    EXIT_EXTERNAL = 100,
    EXIT_INTERNAL = 110,
};

static const char* PID_FILE = "/tmp/boilerplate-cli-ui-cpp.pid";
static const char* LOG_FILE = "/tmp/boilerplate-cli-ui-cpp.log";
static const int DEFAULT_PORT = 8080;
static const char* DEFAULT_HOST = "127.0.0.1";

// The host the server actually bound. /_shutdown is token-gated whenever this
// is not loopback (cli-daemon-spec §3).
static std::string bound_host = DEFAULT_HOST;

static auto start_time = std::chrono::steady_clock::now();

// ─── Output helpers ─────────────────────────────────────────────

/// Emits a typed error on stdout and exits with the matching code. The exit
/// status and .error.code are the same number by construction (§2, §3).
[[noreturn]] static void die(int code, const std::string& type,
                             const std::string& message,
                             const std::string& suggestion) {
    const bool recoverable = code >= 100 && code <= 109;
    std::cout << R"({"ok":false,"error":{"code":)" << code
              << R"(,"type":")" << type
              << R"(","message":")" << message
              << R"(","recoverable":)" << (recoverable ? "true" : "false")
              << R"(,"suggestions":[")" << suggestion << R"("]}})" << std::endl;
    std::exit(code);
}

static std::string format_uptime() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();

    long hours = elapsed / 3600;
    long minutes = (elapsed % 3600) / 60;
    long seconds = elapsed % 60;

    if (hours > 0)
        return std::to_string(hours) + "h" + std::to_string(minutes) + "m" + std::to_string(seconds) + "s";
    if (minutes > 0)
        return std::to_string(minutes) + "m" + std::to_string(seconds) + "s";
    return std::to_string(seconds) + "s";
}

/// Help is context, not the answer to a query, so it goes to stderr and stdout
/// stays clean for data (cli-output-spec §1).
static void print_help() {
    std::cerr << "boilerplate-cli-ui-cpp - C++ CLI with an embedded web UI\n\n"
              << "Usage:\n"
              << "  boilerplate-cli-ui-cpp <command> [options]\n\n"
              << "Commands:\n"
              << "  serve [--host H] [--port N]   run the HTTP server in the foreground\n"
              << "  daemon start [--port N]       start it in the background\n"
              << "  daemon stop [--port N]        stop the background server\n"
              << "  daemon status [--port N]      report background server status\n"
              << "  guide [--human]               the embedded operator guide\n"
              << "  help-json                     machine-readable command catalog\n"
              << "  version [--json]              show version information\n"
              << "  help                          show this help message\n\n"
              << "Endpoints:\n"
              << "  GET  /            Web UI\n"
              << "  GET  /api/status  Server status (JSON)\n"
              << "  GET  /_health     Liveness: {ok,service,pid}\n"
              << "  POST /_shutdown   Stop the server (token-gated off-loopback)\n\n"
              << "Exit codes: 0 ok, 80-89 input, 90-99 state, 100-109 external, 110-119 internal"
              << std::endl;
}

// ─── Flags ──────────────────────────────────────────────────────

static bool has_flag(const std::vector<std::string>& args, const std::string& name) {
    for (const auto& a : args)
        if (a == name) return true;
    return false;
}

/// Reads --name value or --name=value.
static std::string flag_value(const std::vector<std::string>& args,
                              const std::string& name,
                              const std::string& def = "") {
    const std::string prefix = name + "=";
    for (size_t i = 0; i < args.size(); ++i) {
        if (args[i] == name && i + 1 < args.size()) return args[i + 1];
        if (args[i].rfind(prefix, 0) == 0) return args[i].substr(prefix.size());
    }
    return def;
}

static std::string env_or(const char* key, const std::string& def) {
    const char* v = std::getenv(key);
    return (v && *v) ? std::string(v) : def;
}

/// The host default MUST be loopback (cli-daemon-spec §1): serving the whole
/// network is a deliberate act, never something that happens because nobody
/// passed a flag.
static std::string resolve_host(const std::vector<std::string>& args) {
    std::string h = flag_value(args, "--host", flag_value(args, "-host"));
    return h.empty() ? env_or("HOST", DEFAULT_HOST) : h;
}

static int resolve_port(const std::vector<std::string>& args) {
    std::string p = flag_value(args, "--port", flag_value(args, "-port", flag_value(args, "-p")));
    if (p.empty()) p = env_or("PORT", std::to_string(DEFAULT_PORT));
    try {
        return std::stoi(p);
    } catch (...) {
        die(EXIT_MISSING_ARG, "bad_flag_value",
            "--port must be a number, got " + p,
            "boilerplate-cli-ui-cpp serve --port 8080");
    }
}

// ─── Daemon lifecycle (cli-daemon-spec §4) ──────────────────────
//
// /_health is the source of truth for liveness, not the pid file, which goes
// stale when a process dies without cleaning up. Every subcommand is
// idempotent. cpp-httplib ships a client, so the probe adds no dependency.

static bool probe_health(int port) {
    httplib::Client cli("127.0.0.1", port);
    cli.set_connection_timeout(0, 500000);  // 500ms
    cli.set_read_timeout(1, 0);
    auto res = cli.Get("/_health");
    return res && res->status == 200;
}

/// Polls every 100ms for up to 5s, rather than sleeping a fixed amount (§4).
static bool wait_for(int port, bool want) {
    for (int i = 0; i < 50; ++i) {
        if (probe_health(port) == want) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    return false;
}

static void write_pid(pid_t pid) {
    std::ofstream f(PID_FILE);
    if (f) f << pid;
}

static pid_t read_pid() {
    std::ifstream f(PID_FILE);
    pid_t pid = 0;
    if (f) f >> pid;
    return pid;
}

static void serve(const std::string& host, int port);

/// Idempotent: an already-healthy port means report it and succeed, rather than
/// racing a second process onto it (§4).
static void daemon_start(const std::string& host, int port) {
    if (probe_health(port)) {
        std::cout << R"({"ok":true,"running":true,"already_running":true,"port":)"
                  << port << "}" << std::endl;
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        die(EXIT_INTERNAL, "spawn_failed", "fork failed",
            "boilerplate-cli-ui-cpp serve --port " + std::to_string(port));
    }

    if (pid == 0) {
        // Child: detach into its own session and redirect the streams to the
        // log, so the daemon outlives the shell that started it.
        setsid();
        int fd = open(LOG_FILE, O_WRONLY | O_CREAT | O_APPEND, 0644);
        if (fd >= 0) {
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            if (fd > STDERR_FILENO) close(fd);
        }
        int null_fd = open("/dev/null", O_RDONLY);
        if (null_fd >= 0) {
            dup2(null_fd, STDIN_FILENO);
            if (null_fd > STDERR_FILENO) close(null_fd);
        }
        write_pid(getpid());
        serve(host, port);
        std::_Exit(0);
    }

    if (!wait_for(port, true)) {
        kill(pid, SIGTERM);
        unlink(PID_FILE);
        die(EXIT_EXTERNAL, "daemon_unhealthy",
            std::string("started but /_health never answered (see ") + LOG_FILE + ")",
            "boilerplate-cli-ui-cpp serve --port " + std::to_string(port));
    }

    std::cout << R"({"ok":true,"running":true,"already_running":false,"pid":)"
              << read_pid() << R"(,"port":)" << port
              << R"(,"log":")" << LOG_FILE << R"("})" << std::endl;
}

/// A no-op success when nothing is running: an agent stopping an
/// already-stopped daemon has got what it asked for (§4).
static void daemon_stop(int port) {
    if (!probe_health(port)) {
        unlink(PID_FILE);
        std::cout << R"({"ok":true,"running":false,"stopped":false,"port":)"
                  << port << "}" << std::endl;
        return;
    }

    httplib::Client cli("127.0.0.1", port);
    cli.set_read_timeout(2, 0);
    httplib::Headers headers;
    const char* token = std::getenv("SHUTDOWN_TOKEN");
    if (token && *token) headers.emplace("X-Shutdown-Token", token);

    auto res = cli.Post("/_shutdown", headers, "", "application/json");
    if (!res) {
        die(EXIT_EXTERNAL, "shutdown_failed", "POST /_shutdown failed",
            "boilerplate-cli-ui-cpp daemon status --port " + std::to_string(port));
    }
    if (res->status != 200) {
        die(EXIT_EXTERNAL, "shutdown_refused",
            "POST /_shutdown returned " + std::to_string(res->status),
            "set SHUTDOWN_TOKEN if the daemon is bound off-loopback");
    }

    wait_for(port, false);
    unlink(PID_FILE);
    std::cout << R"({"ok":true,"running":false,"stopped":true,"port":)"
              << port << "}" << std::endl;
}

/// Status only ever reads — it never carries the shutdown token (§4).
static void daemon_status(int port) {
    if (!probe_health(port)) {
        std::cout << R"({"ok":true,"running":false,"port":)" << port << "}" << std::endl;
        return;
    }
    std::cout << R"({"ok":true,"running":true,"pid":)" << read_pid()
              << R"(,"port":)" << port << R"(,"log":")" << LOG_FILE << R"("})" << std::endl;
}

// ─── Server ─────────────────────────────────────────────────────

static bool is_loopback(const std::string& host) {
    return host == "127.0.0.1" || host == "localhost" || host == "::1";
}

/// Off-loopback, an open shutdown route is a remote kill switch (§3).
static bool shutdown_authorized(const httplib::Request& req) {
    if (is_loopback(bound_host)) return true;
    const char* token = std::getenv("SHUTDOWN_TOKEN");
    if (!token || !*token) return false;
    return req.get_header_value("X-Shutdown-Token") == token;
}

static void serve(const std::string& host, int port) {
    bound_host = host;
    httplib::Server svr;

    // ─── Static UI Files ────────────────────────────────────────
    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(INDEX_HTML, "text/html");
    });
    svr.Get("/js/app.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(APP_JS, "application/javascript");
    });
    svr.Get("/css/styles.css", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(STYLES_CSS, "text/css");
    });
    svr.Get("/js/components/AppLayout.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(AppLayout_JS, "application/javascript");
    });
    svr.Get("/js/components/Sidebar.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(Sidebar_JS, "application/javascript");
    });
    svr.Get("/js/components/StatusCard.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(StatusCard_JS, "application/javascript");
    });
    svr.Get("/js/views/Dashboard.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(Dashboard_JS, "application/javascript");
    });
    svr.Get("/js/views/Settings.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(Settings_JS, "application/javascript");
    });

    // ─── App API ────────────────────────────────────────────────
    svr.Get("/api/status", [port](const httplib::Request&, httplib::Response& res) {
        std::string json = R"({"status":"running","port":)" + std::to_string(port) +
                           R"(,"uptime":")" + format_uptime() +
                           R"(","version":"1.0.0"})";
        res.set_content(json, "application/json");
    });

    // ─── Daemon lifecycle (§2, §3) ──────────────────────────────
    auto health = [port](const httplib::Request&, httplib::Response& res) {
        // Open and cheap: liveness only, no dependency checks.
        std::string json = R"({"ok":true,"service":"boilerplate-cli-ui-cpp","pid":)" +
                           std::to_string(getpid()) + R"(,"port":)" +
                           std::to_string(port) + "}";
        res.set_content(json, "application/json");
    };
    svr.Get("/_health", health);
    svr.Get("/api/health", health);

    svr.Post("/_shutdown", [&svr](const httplib::Request& req, httplib::Response& res) {
        if (!shutdown_authorized(req)) {
            // 403, and the process MUST NOT stop.
            res.status = 403;
            res.set_content(
                R"({"ok":false,"error":{"code":90,"type":"forbidden","message":"X-Shutdown-Token required when bound off-loopback","recoverable":false}})",
                "application/json");
            return;
        }
        // Answer before exiting, so the caller learns the request was accepted.
        res.set_content(R"({"ok":true,"stopping":true})", "application/json");
        std::thread([&svr] {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            unlink(PID_FILE);
            svr.stop();
        }).detach();
    });

    // ─── The guide over HTTP (cli-guide-spec §3) ────────────────
    svr.Get("/guide", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(GUIDE_JSON, "application/json");
    });
    svr.Get("/llms.txt", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(LLMS_TXT, "text/plain; charset=utf-8");
    });

    // Startup lines are context — stderr, never stdout (§1).
    std::cerr << "boilerplate-cli-ui-cpp serving on http://" << host << ":" << port << "/" << std::endl;
    std::cerr << "  API: http://" << host << ":" << port << "/api/status" << std::endl;

    // Bind the requested host, not 0.0.0.0: a server told to serve localhost
    // must not be reachable from the whole network (§1).
    if (!svr.listen(host.c_str(), port)) {
        die(EXIT_PRECONDITION, "port_unavailable",
            "cannot bind " + host + ":" + std::to_string(port),
            "boilerplate-cli-ui-cpp serve --port " + std::to_string(port + 1));
    }
}

// ─── Main ───────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    std::vector<std::string> all;
    for (int i = 1; i < argc; ++i) all.emplace_back(argv[i]);

    if (all.empty()) {
        print_help();
        return EXIT_MISSING_ARG;
    }

    const std::string cmd = all[0];
    std::vector<std::string> rest(all.begin() + 1, all.end());

    if (cmd == "help" || cmd == "--help" || cmd == "-h") {
        print_help();
        return 0;
    }

    if (cmd == "version" || cmd == "--version") {
        if (has_flag(rest, "--json")) {
            std::cout << R"({"version":")" << VERSION << R"(","name":")" << TOOL << R"("})" << std::endl;
        } else {
            std::cout << TOOL << " v" << VERSION << std::endl;
        }
        return 0;
    }

    if (cmd == "guide") {
        std::cout << (has_flag(rest, "--human") ? GUIDE_MARKDOWN : GUIDE_JSON) << std::endl;
        return 0;
    }

    if (cmd == "help-json") {
        std::cout << HELP_JSON << std::endl;
        return 0;
    }

    if (cmd == "serve") {
        serve(resolve_host(rest), resolve_port(rest));
        return 0;
    }

    if (cmd == "daemon") {
        if (rest.empty()) {
            die(EXIT_MISSING_ARG, "missing_argument",
                "daemon needs a subcommand: start, stop or status",
                "boilerplate-cli-ui-cpp daemon status");
        }
        const std::string sub = rest[0];
        std::vector<std::string> tail(rest.begin() + 1, rest.end());
        const int port = resolve_port(tail);

        if (sub == "start") daemon_start(resolve_host(tail), port);
        else if (sub == "stop") daemon_stop(port);
        else if (sub == "status") daemon_status(port);
        else die(EXIT_UNKNOWN_COMMAND, "unknown_command",
                 "unknown daemon subcommand " + sub,
                 "boilerplate-cli-ui-cpp daemon status");
        return 0;
    }

    // Back-compat aliases for the pre-spec command names.
    if (cmd == "start") {
        const int port = resolve_port(rest);
        if (has_flag(rest, "-daemon") || has_flag(rest, "--daemon")) {
            daemon_start(resolve_host(rest), port);
        } else {
            serve(resolve_host(rest), port);
        }
        return 0;
    }
    if (cmd == "stop") {
        daemon_stop(resolve_port(rest));
        return 0;
    }
    if (cmd == "status") {
        daemon_status(resolve_port(rest));
        return 0;
    }

    die(EXIT_UNKNOWN_COMMAND, "unknown_command",
        "unknown command " + cmd,
        "boilerplate-cli-ui-cpp help-json");
}
