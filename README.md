# boilerplate-cli-ui-cpp

C++ CLI with embedded web UI. Single binary, no runtime dependencies.

Part of [SuperCLI](https://github.com/javimosch/supercli) - build CLI/UI plugins fast for 2026.

**Other versions**: [Go+Vue](https://github.com/javimosch/boilerplate-cli-ui-go-v2-vue) | [Rust+Vue](https://github.com/javimosch/boilerplate-cli-ui-rust) | [.NET+Vue](https://github.com/javimosch/boilerplate-cli-ui-dotnet) | [Nim](https://github.com/javimosch/boilerplate-cli-ui-nim) | [Node](https://github.com/javimosch/boilerplate-cli-ui-node) | [Python](https://github.com/javimosch/boilerplate-cli-ui-python)

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
```

## Key Feature: Raw String Literals

Frontend files are **embedded as C++ raw string literals**:

```cpp
// include/ui/app_js.h
const char* APP_JS = R"(
// Main Vue Application
const { createApp } = Vue;
...
)";
```

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
```

## Build

```bash
chmod +x build.sh
./build.sh
```

Or manually:

```bash
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

## Usage

```bash
# Start server (foreground)
./build/boilerplate-cli-ui-cpp start

# Start on custom port
./build/boilerplate-cli-ui-cpp start -p 3000

# Show version
./build/boilerplate-cli-ui-cpp version

# Show help
./build/boilerplate-cli-ui-cpp help
```

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
| Binary size | ~5MB | ~1.1MB | ~89MB | ~500KB |
| Embed pattern | `//go:embed` | `include_str!` | `EmbeddedResource` | Raw string literals |
| Modularity | ⭐⭐⭐ | ⭐⭐ | ⭐⭐⭐ | ⭐⭐ |
| Dev speed | Fast | Medium | Fast | Slow |
| Build system | go build | cargo | dotnet | CMake |
