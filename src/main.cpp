#include <iostream>
#include <string>
#include <chrono>
#include <httplib.h>

// Include embedded UI files
#include "ui/index_html.h"
#include "ui/app_js.h"
#include "ui/styles_css.h"
#include "ui/components/AppLayout_js.h"
#include "ui/components/Sidebar_js.h"
#include "ui/components/StatusCard_js.h"
#include "ui/views/Dashboard_js.h"
#include "ui/views/Settings_js.h"

// Start time for uptime calculation
auto start_time = std::chrono::steady_clock::now();

// Get content type from file extension
std::string get_content_type(const std::string& path) {
    if (path.find(".html") != std::string::npos) return "text/html";
    if (path.find(".js") != std::string::npos) return "application/javascript";
    if (path.find(".css") != std::string::npos) return "text/css";
    if (path.find(".json") != std::string::npos) return "application/json";
    if (path.find(".png") != std::string::npos) return "image/png";
    if (path.find(".jpg") != std::string::npos || path.find(".jpeg") != std::string::npos) return "image/jpeg";
    if (path.find(".svg") != std::string::npos) return "image/svg+xml";
    return "application/octet-stream";
}

// Format uptime string
std::string format_uptime() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
    
    int hours = elapsed / 3600;
    int minutes = (elapsed % 3600) / 60;
    int seconds = elapsed % 60;
    
    if (hours > 0) {
        return std::to_string(hours) + "h" + std::to_string(minutes) + "m" + std::to_string(seconds) + "s";
    } else if (minutes > 0) {
        return std::to_string(minutes) + "m" + std::to_string(seconds) + "s";
    } else {
        return std::to_string(seconds) + "s";
    }
}

void print_help() {
    std::cout << "boilerplate-cli-ui-cpp - C++ CLI with embedded web UI" << std::endl;
    std::cout << std::endl;
    std::cout << "Usage:" << std::endl;
    std::cout << "  boilerplate-cli-ui-cpp <command> [options]" << std::endl;
    std::cout << std::endl;
    std::cout << "Commands:" << std::endl;
    std::cout << "  start       Start HTTP server with web UI" << std::endl;
    std::cout << "  version     Show version information" << std::endl;
    std::cout << "  help        Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Start Options:" << std::endl;
    std::cout << "  -p, --port <PORT>  Port for HTTP server (default 8080)" << std::endl;
    std::cout << std::endl;
    std::cout << "API Endpoints:" << std::endl;
    std::cout << "  GET /            Web UI" << std::endl;
    std::cout << "  GET /api/status  Server status (JSON)" << std::endl;
    std::cout << "  GET /api/health  Health check (JSON)" << std::endl;
}

int main(int argc, char* argv[]) {
    int port = 8080;
    
    // Parse arguments
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "version" || arg == "--version") {
            std::cout << "boilerplate-cli-ui-cpp v1.0.0" << std::endl;
            return 0;
        }
        
        if (arg == "help" || arg == "--help" || arg == "-h") {
            print_help();
            return 0;
        }
        
        if ((arg == "-p" || arg == "--port") && i + 1 < argc) {
            port = std::stoi(argv[++i]);
        }
    }
    
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
    
    // Components
    svr.Get("/js/components/AppLayout.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(AppLayout_JS, "application/javascript");
    });
    
    svr.Get("/js/components/Sidebar.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(Sidebar_JS, "application/javascript");
    });
    
    svr.Get("/js/components/StatusCard.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(StatusCard_JS, "application/javascript");
    });
    
    // Views
    svr.Get("/js/views/Dashboard.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(Dashboard_JS, "application/javascript");
    });
    
    svr.Get("/js/views/Settings.js", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(Settings_JS, "application/javascript");
    });
    
    // ─── API Endpoints ──────────────────────────────────────────
    svr.Get("/api/status", [&port](const httplib::Request&, httplib::Response& res) {
        std::string uptime = format_uptime();
        std::string json = R"({"status":"running","port":)" + std::to_string(port) + 
                          R"(,"uptime":")" + uptime + 
                          R"(","version":"1.0.0"})";
        res.set_content(json, "application/json");
    });
    
    svr.Get("/api/health", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(R"({"status":"healthy","version":"1.0.0"})", "application/json");
    });
    
    // ─── Start Server ───────────────────────────────────────────
    std::cout << "Server starting on http://localhost:" << port << std::endl;
    std::cout << "UI available at http://localhost:" << port << "/" << std::endl;
    std::cout << "API available at http://localhost:" << port << "/api/status" << std::endl;
    std::cout << "Press Ctrl+C to stop" << std::endl;
    
    svr.listen("0.0.0.0", port);
    
    return 0;
}
