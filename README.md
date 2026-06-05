# boilerplate-cli-ui-cpp

C++ CLI with embedded web UI. Single binary, no runtime dependencies.
Part of [SuperCLI](https://github.com/javimosch/supercli) - build CLI/UI plugins fast for 2026.
| Stack | Repo | Binary | SDK Size |
|-------|------|--------|----------|
| Go + inline HTML | [boilerplate-cli-ui-go](https://github.com/javimosch/boilerplate-cli-ui-go) | ~5MB | ~150MB |
| Go + Vue 3 CDN | [boilerplate-cli-ui-go-v2-vue](https://github.com/javimosch/boilerplate-cli-ui-go-v2-vue) | ~5MB | ~150MB |
| Go + React 18 CDN | [boilerplate-cli-ui-go-v2-react](https://github.com/javimosch/boilerplate-cli-ui-go-v2-react) | ~5MB | ~150MB |
| Deno + vanilla JS | [boilerplate-cli-ui-deno](https://github.com/javimosch/boilerplate-cli-ui-deno) | ~76MB | ~100MB |
| Node.js + vanilla JS | [boilerplate-cli-ui-node](https://github.com/javimosch/boilerplate-cli-ui-node) | ~123MB | ~500MB+ |
| Python + React CDN | [boilerplate-cli-ui-python](https://github.com/javimosch/boilerplate-cli-ui-python) | ~10MB | ~300MB |
| Rust + vanilla JS | [boilerplate-cli-ui-rust](https://github.com/javimosch/boilerplate-cli-ui-rust) | ~1.1MB | ~800MB |
| .NET 8 + Vue 3 | [boilerplate-cli-ui-dotnet](https://github.com/javimosch/boilerplate-cli-ui-dotnet) | ~89MB | ~600MB |
| **C++ + Vue 3** | **boilerplate-cli-ui-cpp** | **~493KB** |
| Nim + Vue 3 | [boilerplate-cli-ui-nim](https://github.com/javimosch/boilerplate-cli-ui-nim) | ~364KB | ~50MB |
| Zig + Vue 3 | [boilerplate-cli-ui-zig](https://github.com/javimosch/boilerplate-cli-ui-zig) | ~190KB | ~50MB |
| Dart + Vue 3 | [boilerplate-cli-ui-dart](https://github.com/javimosch/boilerplate-cli-ui-dart) | ~6.4MB | ~400MB |
|| V + Vue 3 | [boilerplate-cli-ui-v](https://github.com/javimosch/boilerplate-cli-ui-v) | ~1.2MB | ~5MB |
|| Crystal + Vue 3 | [boilerplate-cli-ui-crystal](https://github.com/javimosch/boilerplate-cli-ui-crystal) | ~3.1MB | ~50MB |
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
