// The embedded guide and command catalog (cli-guide-spec, cli-output-spec §4).
//
// Compiled into the binary as string constants: an agent that lands on a
// machine with this binary and no network can still learn the tool. Never
// fetched at runtime.
#pragma once

inline const char* TOOL = "boilerplate-cli-ui-cpp";
inline const char* VERSION = "1.0.0";

inline const char* GUIDE_JSON = R"JSON({
"boilerplate-cli-ui-cpp":"A C++ CLI with an embedded web UI, compiled to one binary.",
"version":"1.0.0",
"one_liner":"Starts a cpp-httplib server that serves a Vue 3 dashboard at / and a JSON API at /api/*, from one binary with the UI compiled in as generated headers — no assets to deploy alongside it.",
"model":{
"binary":"one executable; generate-headers.sh turns each ui/ file into a C++ string constant that the binary carries.",
"server":"cpp-httplib, bound to an explicit host:port — never 0.0.0.0 by default.",
"daemon":"fork/setsid to detach, with /_health as the source of truth for liveness.",
"contract":"agent-first: data on stdout, context on stderr, semantic exit codes, typed errors, an embedded guide."
},
"loop":[
"./build.sh — regenerate the UI headers and compile with CMake",
"./build/boilerplate-cli-ui-cpp serve — foreground on 127.0.0.1:8080",
"open http://127.0.0.1:8080/ for the UI, or curl /api/status for JSON",
"./build/boilerplate-cli-ui-cpp daemon start — background it instead",
"./build/boilerplate-cli-ui-cpp daemon stop — stop it"
],
"concepts":{
"embedded UI":"generate-headers.sh converts ui/ files into include/ui/*.h string constants. Edit the files, rerun build.sh.",
"loopback default":"serve binds 127.0.0.1 unless --host says otherwise. Binding the whole network is deliberate.",
"shutdown token":"off-loopback, POST /_shutdown requires X-Shutdown-Token matching $SHUTDOWN_TOKEN, or it answers 403 and keeps running.",
"exit codes":"0 ok, 80-89 input, 90-99 state, 100-109 external, 110-119 internal. The code equals .error.code in the body."
},
"commands":{"server":[
"boilerplate-cli-ui-cpp serve [--host H] [--port N]",
"boilerplate-cli-ui-cpp daemon start [--port N]",
"boilerplate-cli-ui-cpp daemon stop [--port N]",
"boilerplate-cli-ui-cpp daemon status [--port N]"
],"introspection":[
"boilerplate-cli-ui-cpp guide [--human]",
"boilerplate-cli-ui-cpp help-json",
"boilerplate-cli-ui-cpp version [--json]"
]},
"examples":[
{"goal":"serve the UI on a custom port","do":["./build/boilerplate-cli-ui-cpp serve --port 3000"]},
{"goal":"background it and confirm it is up","do":["./build/boilerplate-cli-ui-cpp daemon start --port 3000","./build/boilerplate-cli-ui-cpp daemon status --port 3000"]},
{"goal":"expose it on the LAN with a kill switch that needs a token","do":["SHUTDOWN_TOKEN=s3cret ./build/boilerplate-cli-ui-cpp serve --host 0.0.0.0 --port 8080"]}
],
"gotchas":[
"The UI is compiled in as generated headers: editing ui/ does nothing until you rerun build.sh.",
"serve binds 127.0.0.1 by default. If you expected it on the LAN, pass --host 0.0.0.0 — and then set SHUTDOWN_TOKEN, or /_shutdown answers 403 to everyone.",
"daemon start is idempotent: called twice it reports the running instance instead of racing a second process onto the port.",
"daemon stop against a stopped daemon exits 0 — a no-op success, not an error.",
"Startup lines go to stderr. An agent parsing stdout sees only data."
],
"see_also":["https://cli-specs.intrane.fr"]})JSON";

inline const char* GUIDE_MARKDOWN = R"MD(# boilerplate-cli-ui-cpp

A C++ CLI with an embedded web UI, compiled to one binary.

## Model

- One executable; the UI ships inside it as generated header constants.
- cpp-httplib, bound to an explicit host:port.
- The daemon forks and detaches; /_health is liveness.
- Agent-first: data on stdout, context on stderr, semantic exit codes.

## Loop

1. `./build.sh`
2. `./build/boilerplate-cli-ui-cpp serve`
3. Open http://127.0.0.1:8080/ or curl /api/status.
4. `./build/boilerplate-cli-ui-cpp daemon start` to background it.
5. `./build/boilerplate-cli-ui-cpp daemon stop` to stop it.

## Commands

- `serve [--host H] [--port N]`
- `daemon start|stop|status [--port N]`
- `guide [--human]`, `help-json`, `version [--json]`

## Gotchas

- The UI is compiled in: rerun `./build.sh` after editing ui/.
- `serve` binds 127.0.0.1 by default; `--host 0.0.0.0` is deliberate.
- Off-loopback, `POST /_shutdown` needs `X-Shutdown-Token` = `$SHUTDOWN_TOKEN`.
- `daemon start` twice is idempotent; `daemon stop` when stopped exits 0.
)MD";

inline const char* LLMS_TXT = R"TXT(# boilerplate-cli-ui-cpp

A C++ CLI with an embedded web UI. One binary.

## Drive it

    boilerplate-cli-ui-cpp serve [--host H] [--port N]
    boilerplate-cli-ui-cpp daemon start|stop|status [--port N]

JSON on stdout, context on stderr, exit 0/80-119.

## Learn it

    boilerplate-cli-ui-cpp guide      # embedded, JSON
    boilerplate-cli-ui-cpp help-json  # command catalog

HTTP: GET /  GET /api/status  GET /_health  POST /_shutdown  GET /guide
)TXT";

inline const char* HELP_JSON = R"JSON({
"version":"1.0","tool":"boilerplate-cli-ui-cpp","tool_version":"1.0.0",
"commands":[
{"name":"serve","summary":"run the HTTP server in the foreground","flags":[
{"name":"--host","summary":"bind address","default":"127.0.0.1","env":"HOST"},
{"name":"--port","summary":"port","default":"8080","env":"PORT"}]},
{"name":"daemon start","summary":"start the server in the background (idempotent)"},
{"name":"daemon stop","summary":"stop the background server (no-op success if stopped)"},
{"name":"daemon status","summary":"report background server status"},
{"name":"guide","summary":"the embedded operator guide","flags":[{"name":"--human","summary":"markdown instead of JSON"}]},
{"name":"help-json","summary":"this machine-readable command catalog"},
{"name":"version","summary":"print the version","flags":[{"name":"--json","summary":"JSON output"}]}
],
"endpoints":[
{"method":"GET","path":"/","summary":"the embedded web UI"},
{"method":"GET","path":"/api/status","summary":"app status JSON"},
{"method":"GET","path":"/_health","summary":"liveness: {ok,service,pid}"},
{"method":"POST","path":"/_shutdown","summary":"stop the server; token-gated off-loopback"},
{"method":"GET","path":"/guide","summary":"the guide over HTTP"},
{"method":"GET","path":"/llms.txt","summary":"the short agent-facing README"}
],
"exit_codes":{
"0":"success",
"80":"missing argument or bad flag value",
"85":"unknown command",
"90":"precondition failed (port unavailable, forbidden)",
"100":"external failure (the daemon did not answer)",
"110":"internal error"
},
"env":[
{"name":"PORT","summary":"default port"},
{"name":"HOST","summary":"default bind address"},
{"name":"SHUTDOWN_TOKEN","summary":"required by POST /_shutdown when bound off-loopback"}
],
"see_also":["boilerplate-cli-ui-cpp guide","https://cli-specs.intrane.fr"]})JSON";
