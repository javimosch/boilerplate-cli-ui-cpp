# boilerplate-cli-ui-cpp

C++ CLI with embedded web UI. Single binary, no runtime dependencies.
Part of [SuperCLI](https://github.com/javimosch/supercli) - build CLI/UI plugins fast for 2026.
<!-- FLEET-TABLE:BEGIN -->

| Stack | Binary | Cold start | Idle RSS | Specs | SDK |
|-------|--------|-----------:|---------:|:-----:|----:|
| [machin + React 18 CDN](https://github.com/javimosch/boilerplate-cli-ui-machin) | 63 KB | 2 ms | 3.1 MB | 28/28 | ~2 MB |
| [machin isomorphic (wasm UI)](https://github.com/javimosch/boilerplate-cli-ui-machin-isomorphic) | 76 KB | 2 ms | 3.1 MB | 28/28 | ~2 MB |
| [Nim + Vue 3](https://github.com/javimosch/boilerplate-cli-ui-nim) | 372 KB | 1 ms | 2.0 MB | 2/21 | ~50 MB |
| **C++ + Vue 3** | **692 KB** | **4 ms** | **7.4 MB** | **28/28** | **~2000 MB** |
| [Zig + Vue 3](https://github.com/javimosch/boilerplate-cli-ui-zig) | 971 KB | 1 ms | 2.0 MB | 28/28 | ~50 MB |
| [Rust + vanilla JS](https://github.com/javimosch/boilerplate-cli-ui-rust) | 1003 KB | 1 ms | 2.5 MB | 28/28 | ~800 MB |
| [V + Vue 3](https://github.com/javimosch/boilerplate-cli-ui-v) | 1.2 MB | 2 ms | 2.5 MB | 5/20 | ~5 MB |
| [Crystal + Vue 3](https://github.com/javimosch/boilerplate-cli-ui-crystal) | 3.1 MB | 3 ms | 5.9 MB | 5/20 | ~50 MB |
| [Go + Vue 3 CDN](https://github.com/javimosch/boilerplate-cli-ui-go-v2-vue) | 5.5 MB | 3 ms | 5.8 MB | 28/28 | ~150 MB |
| [Go + React 18 CDN](https://github.com/javimosch/boilerplate-cli-ui-go-v2-react) | 5.5 MB | 4 ms | 5.6 MB | 28/28 | ~150 MB |
| [Dart + Vue 3](https://github.com/javimosch/boilerplate-cli-ui-dart) | 6.3 MB | 6 ms | 7.2 MB | 2/21 | ~400 MB |
| [Deno + vanilla JS](https://github.com/javimosch/boilerplate-cli-ui-deno) | 76.1 MB | 25 ms | 45.0 MB | 28/28 | ~100 MB |
| [Node.js + vanilla JS](https://github.com/javimosch/boilerplate-cli-ui-node) | 122.8 MB | 62 ms | 53.3 MB | 28/28 | ~500 MB |

Not measured in this run (toolchain unavailable): [Python + React CDN](https://github.com/javimosch/boilerplate-cli-ui-python), [.NET 8 + Vue 3](https://github.com/javimosch/boilerplate-cli-ui-dotnet).

*Binary size, cold start (median of 11 `version` runs) and idle RSS measured on Linux-x86_64 on 2026-09-08. **Specs** is the [cli-spec-conformance](https://github.com/javimosch/cli-spec-conformance) score across cli-output-spec, cli-guide-spec and cli-daemon-spec, measured by running each binary — not claimed. Every row builds the same reference app; a row at 28/28 also implements the same agent-first contract, which is what makes its size comparable to the others. Rows below 28/28 have not been converted yet, so read their sizes as a floor. Regenerate with [boilerplate-cli-ui-fleet](https://github.com/javimosch/boilerplate-cli-ui-fleet); never edit this table by hand.*

<!-- FLEET-TABLE:END -->
## Architecture
```
boilerplate-cli-ui-cpp/
├── src/
│   └── main.cpp           # CLI + HTTP server
├── include/ui/            # Embedded UI as header files
│   ├── index_html.h
│   ├── app_js.h
│   ├── styles_css.h
│   ├── components/
│   └── views/
├── ui/                    # Original UI files
│   ├── index.html
│   ├── app.js
│   ├── css/styles.css
│   └── js/
├── CMakeLists.txt
├── build.sh
└── README.md
## Key Feature: Raw String Literals
Frontend files are **embedded as C++ raw string literals**:
```cpp
// include/ui/app_js.h
const char* APP_JS = R"(
// Main Vue Application
const { createApp } = Vue;
...
)";
**Benefits:**
- Single binary output (no runtime file dependencies)
- Compile-time embedding (no I/O at runtime)
- Same modularity as other versions
**Trade-offs:**
- Must generate header for each UI file
- Must add route for each file manually
- Build requires CMake + cpp-httplib
## Prerequisites
```bash
# Ubuntu/Debian
sudo apt install cmake g++
# macOS
brew install cmake
# Windows
winget install Kitware.CMake
## Build
chmod +x build.sh
./build.sh
Or manually:
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
## Usage
# Start server (foreground)
./build/boilerplate-cli-ui-cpp start
# Start on custom port
./build/boilerplate-cli-ui-cpp start -p 3000
# Show version
./build/boilerplate-cli-ui-cpp version
# Show help
./build/boilerplate-cli-ui-cpp help
## API Endpoints
| Endpoint | Description |
|----------|-------------|
| `GET /` | Web UI |
| `GET /api/status` | Server status (JSON) |
| `GET /api/health` | Health check (JSON) |
## Hashbang Routing
Routes use hashbang URLs:
- `http://localhost:8080/#/dashboard` - Dashboard view
- `http://localhost:8080/#/settings` - Settings view
- `http://localhost:8080/` - Defaults to dashboard
## Frontend Stack
- **Vue 3** (CDN) - Reactive UI with hashbang routing
- **Tailwind CSS** (CDN) - Utility-first styling
- **Lucide Icons** (CDN) - Icon library
## Adding New UI Files
1. Create file in `ui/` directory
2. Run `./generate-headers.sh` to create header
3. Add `#include` and route in `main.cpp`
## Comparison
| Aspect | Go | Rust | .NET | C++ |
|--------|-----|------|------|-----|
| Binary size | ~5MB | ~150MB | ~1.1MB | ~800MB | ~89MB | ~600MB | ~500KB |
| Embed pattern | `//go:embed` | `include_str!` | `EmbeddedResource` | Raw string literals |
| Modularity | ⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ |
| Dev speed | Fast | Medium | Fast | Slow |
| Build system | go build | cargo | dotnet | CMake |
