# Bot Core
#### A platform for writing simple bot scripts in Lua

---

[![CMake on multiple platforms](https://github.com/borz7zy/bot_core/actions/workflows/cmake-multi-platform.yml/badge.svg)](https://github.com/borz7zy/bot_core/actions/workflows/cmake-multi-platform.yml)

---

The **Bot Core** project aims to streamline the creation of bots using the lua language. The core of the platform supports plugins, allowing developers to create custom plugins tailored to their needs.

### Official Plugins
- Coming soon.

---

### Download
You can download the release version for the OS you need (from the existing ones downloaded in the release, respectively) in the [releases](https://github.com/borz7zy/bot_core/releases) section.

You can also find the latest builds in the project's [Actions](https://github.com/borz7zy/bot_core/actions/workflows/cmake-multi-platform.yml)
If there is nothing for your platform anywhere, you can build it yourself using the instructions below.

---

### Clone the Repository
```bash
git clone https://github.com/borz7zy/bot_core.git
cd bot_core
```

### Pre-Build Steps
```bash
git submodule init
git submodule update --init --recursive
```

#### Compiling the wolfssl library (required for tls connections in the https client)
At the moment only PowerShell script is available. On MacOS and Linux you can install powershell, in the next updates I will add bash files.

```PowerShell
./prebuild-libs.ps1
```

By default, the Release version of wolfssl is built. If required, specify via the flag -buildtype "Debug".

There are also the following flags:
- -ccomp "compiler": specify the required compiler for C
- -cppcomp "compiler": specify the required compiler for C++
- -arch "architecture": specify the architecture for which the library should be compiled

### Build project
```
mkdir build && cd build
```

For Debug mode:
```
cmake -DCMAKE_BUILD_TYPE=Debug ..
```

OR Release
```
cmake -DCMAKE_BUILD_TYPE=Release ..
```

### Build Instructions
**Windows:**
```powershell
cmake --build .
```

**Linux/MacOS:**
```bash
make
```

---

### Third-Party Libraries Used in the Project
- [lua/lua](https://github.com/lua/lua)
- [wolfSSL/wolfssl](https://github.com/wolfSSL/wolfssl)
