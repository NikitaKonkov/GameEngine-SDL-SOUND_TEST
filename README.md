# C++ Game Engine with SDL3 and Vulkan

![License](https://img.shields.io/badge/license-MIT-blue.svg)
![C++](https://img.shields.io/badge/C++-17-blue.svg?logo=c%2B%2B)
![SDL](https://img.shields.io/badge/SDL3-3.2.10-brightgreen.svg)
![Vulkan](https://img.shields.io/badge/Vulkan-1.4.309-red.svg?logo=vulkan)
![Status](https://img.shields.io/badge/status-%20prototype-yellow.svg)
![Build](https://img.shields.io/badge/build-cmake-success.svg)

A modern C++ game engine built with SDL3 and Vulkan, providing a flexible foundation for game development with optimized performance. Designed to leverage modern hardware capabilities while maintaining an intuitive API for game developers.

<br>
<br>

## ✨ Features

- 🖼️ **SDL3 Integration** - Window management, input handling, and platform abstraction
- 🚀 **Vulkan Rendering Backend** - Modern graphics API for optimal performance
- ⚙️ **Advanced Input System** - Flexible keyboard/mouse/gamepad handling with action mapping
- 🔊 **Basic Audio Engine** - Simple sound playback functionality
- 🎹 **Piano System** - Interactive keyboard piano with customizable notes and sustain mode
- 📁 **Settings Management** - File-based configuration with automatic persistence
- 💻 **CPU Optimization** - Optional AVX2/AVX512 instruction set utilization
- 🛠️ **Cross-Platform** - CMake build system for Windows and Linux (coming soon)

## 📋 Requirements

- **C++17 compatible compiler** (MSVC 19.20+, GCC 8+, or Clang 8+)
- **CMake 3.16+**
- **SDL3** development libraries
- **Vulkan SDK 1.4.309.0+**

## 🔧 Building the Project

### Prerequisites

1. Install [CMake](https://cmake.org/download/)
2. Install [Vulkan SDK](https://vulkan.lunarg.com/sdk/home)
3. Set up SDL3 development libraries

### Quick Build Instructions

#### Windows CMD
Use the provided batch script:
```
build.bat                 # Build with AVX2 (default)
build.bat --avx512        # Build with AVX512
build.bat --clean         # Clean build with AVX2
build.bat --clean --avx512  # Clean build with AVX512
```

#### Linux (Coming Soon)
Use the provided shell script:
```
./build.sh                # Build with AVX2 (default)
./build.sh --avx512       # Build with AVX512
./build.sh --clean        # Clean build with AVX2
./build.sh --clean --avx512  # Clean build with AVX512
```

### Manual Build Instructions

1. Clone the repository
   ```
   git clone https://github.com/yourusername/GameEngine-SDL-VULKAN-CPP.git
   cd GameEngine-SDL-VULKAN-CPP
   ```

2. Create a build directory
   ```
   mkdir build
   cd build
   ```

3. Configure the project
   ```
   cmake ..
   ```

4. Build the project
   ```
   cmake --build .
   ```

### Build Options

| Option | Description | Default |
|--------|-------------|---------|
| `USE_AVX512` | Enable AVX512 instruction set | `OFF` |
| `PRODUCTION_BUILD` | Configure for production release | `OFF` |
| `BUILD_TESTS` | Build test suite | `OFF` |

Example:
```
cmake -DUSE_AVX512=ON -DPRODUCTION_BUILD=ON ..
```

## 📂 Project Structure

```
/
├── Engine/                     # Core engine code
│   ├── include/                # Header files
│   │   ├── app/               # Main application components
│   │   ├── assets/            # Asset management
│   │   │   └── piano/         # Piano system
│   │   ├── audio/             # Audio system 
│   │   ├── inputs/            # Input handling system
│   │   ├── renderer/          # Rendering system
│   │   └── settings/          # Configuration system
│   ├── src/                    # Source files
│   │   ├── app/               # Application implementation
│   │   ├── assets/            # Assets implementation
│   │   │   └── piano/         # Piano system implementation
│   │   ├── audio/             # Audio implementation
│   │   ├── inputs/            # Input system implementation
│   │   ├── renderer/          # Renderer implementation
│   │   └── settings/          # Settings implementation
│   └── resources/              # Game resources and configurations
│       ├── shaders/           # GLSL shaders
│       ├── sounds/            # Audio files
│       └── settings.txt       # Default settings file
└── CMakeLists.txt             # Main build system
```

## ⚙️ Configuration

The engine uses a settings system that manages configuration through a text file. Default settings include:

| Setting | Description | Default Value |
|---------|-------------|---------------|
| screenWidth | Display width | 1280 |
| screenHeight | Display height | 720 |
| fullscreen | Fullscreen mode | false |
| vsync | Vertical sync enabled | true |
| maxFPS | Frame rate limit | 60 |
| masterVolume | Main volume level | 1.0 |

Settings are automatically saved to and loaded from `resources/settings.txt`.

## 🔑 Key Features Implementation

### Input System

The engine offers a flexible input system that supports both direct key status queries and action mapping:

```cpp
// Check if a specific key is pressed
if (Keyboard::Input.isKeyPressed(SDLK_SPACE)) {
    // Jump action
}

// Use the action mapping system
Keyboard::Input.mapAction("Jump", SDLK_SPACE, SDLK_W);
if (Keyboard::Input.isActionPressed("Jump")) {
    // Jump action
}
```

### Piano System

The engine includes an interactive piano system that maps keyboard keys to musical notes:

```cpp
// Initialize the piano system
InitializePiano();

// Play a specific note for 1 second
gPiano->playNote("C4", 1000);

// Toggle sustain mode (longer note duration)
gPiano->toggleSustainMode();

// Default keyboard mapping:
// Q -> C4, W -> D4, E -> E4, R -> F4, T -> G4
// Z -> A4, U -> B4, I -> C5, O -> D5, P -> E5
```

The piano system is integrated into the main engine loop and automatically handles key events.

### Graphics Pipeline

The rendering system uses a modern Vulkan pipeline with:

- Shader hot-reloading for rapid development
- Swap chain management with window resize handling
- Command buffer optimization
- Basic triangle rendering foundation

## 📝 License

[MIT License](LICENSE) - Feel free to use, modify, and distribute this code for personal and commercial projects.