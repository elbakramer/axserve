# Introduction

AxServe is a server program that provides interfaces of COM or OCX components through a gRPC server.

## Motivation

There are many ways to integrate COM or OCX components, such as creating native apps or using other libraries or frameworks. But they all had their own strengths and weaknesses.

Options that I had considered so far were:

Library | Module | Based On | Maintainer | Language
-- | -- | -- | -- | --
[Win32](https://learn.microsoft.com/en-us/windows/win32/) |   |   | Microsoft | C/C++
[MFC](https://learn.microsoft.com/en-us/cpp/mfc/mfc-desktop-applications?view=msvc-170) |   |   | Microsoft | C++
[Qt5](https://doc.qt.io/qt-5/) | [ActiveQt](https://doc.qt.io/qt-5/activeqt-index.html) | Win32 | Qt Group | C++
[Qt6](https://doc.qt.io/qt-6/) | [ActiveQt](https://doc.qt.io/qt-6/activeqt-index.html) | Win32 | Qt Group | C++
[pywin32](https://github.com/mhammond/pywin32) | win32com.client | Win32 | Mark   Hammond | Python
[pywin32](https://github.com/mhammond/pywin32) | pywin.mfc | MFC | Mark   Hammond | Python
[PyQt5](https://www.riverbankcomputing.com/software/pyqt/) | PyQt5.QAxContainer | Qt5 | Riverbank Computing | Python
[PyQt6](https://www.riverbankcomputing.com/software/pyqt/) | PyQt6.QAxContainer | Qt6 | Riverbank Computing | Python
PySide2 | PySide2.QtAxContainer | Qt5 | Qt Group | Python
PySide6 | PySide6.QtAxContainer | Qt6 | Qt Group | Python

And I had considered some Pros & Cons of them:

- Requires build step
    - (-) C/C++ Language options require build step
    - (+) Python Language options do not require built step
- Complexity in dependency resolution and installation
    - (-) Python Language options may have no proper python version that supports all requirements like target architecture (x86/x64) and version support (3.x)
    - (-) And Python Language options may require multiple python installations with some IPC (Inter-Process Communication) technique as workaround
    - (+) C/C++ Language options only require single installation for each target architecture once built
    - (-) But C/C++ Language options may have hard time in managing it's dependencies compared to python when it comes to installation and building
- Size of dependencies
    - (+) Microsoft Maintainer options require no further dependencies
    - (-) Other libraries and frameworks based options may require large amount of dependencies which might complicate things
- Ease of use, amount of learning materials and references, quality of documentation
    - (+) Qt based Python bindings options are easy to use, have lots of materials and references
    - (+) Qt based options have good documentation and sufficient references
    - (-) Other options may have less materials, might be harder to use
    - (+) Python Language options are generally considered as easy compared to C++
- Possibility of being deprecated or obsolete
    - (-) Qt5 Based options will become deprecated/obsolete over time in favor of Qt6
    - (+) Qt6 based options are latest version in Qt
- 32bit architecture support
    - (+) Qt5 Based options support 32bit architecture on windows naturally with prebuilt binaries ([link](https://doc.qt.io/qt-5/windows.html))
    - (-) Qt6 Based options do not provide prebuilt binaries for 32bit architecture on windows ([link](https://doc.qt.io/qt-6/windows.html))
- Applicability of acquired knowledge across domains
    - (+) Qt based options can leverage learned skills to create another applications for platforms other than windows
    - (-) Other options may be too platform specific
- License
    - (-) PyQt Based options require GPLv3 license, unless commercial license is used
    - (+) Qt Group Maintainer options (QtX and PySideX) require LGPLv3 license
    - (+) Pywin32 based options generally considered as PSF-2.0 ([issue](https://github.com/mhammond/pywin32/issues/1646))
    - (+) Microsoft Maintainer options have less license implications

My personal goal was to use 32bit COM/OCX feature in python. So based on the analysis and my goal, my final decision was:

- Not to bring the COM/OCX part of dependency to python
- But make the dependency loose by supporting that using IPC technique with some libraries like gRPC
- Learn Qt6 and use that for development
- Build Qt6 for 32bit architecture support on my own
- Build single server executable and use that in python
- More specificaly, run the server executable using `subprocess` and connect to that using `grpcio`

And here is the outcome of the server part.

# Usage

## Server

### GUI

1. Run the executable by double clicking
2. Type the required information
    - CLSID required to instantiate an Active-X or COM object.
    - Address URI for gRPC server to bind.
3. Press start button to start server

![axserve](./axserve.png)

### Console

If built with console support, give required options to run the server as cli argument:

```
.\axserve-x86-console-debug.exe --clsid="{A1574A0D-6BFA-4BD7-9020-DED88711818D}" --address="localhost:8080" --no-gui
```

That `--no-gui` option makes the application run without GUI components. This can be useful for cases when embedding this executable in other things. FYI, technically it's not a pure non-gui application but just tries to hide or not to show the windows created internally.

The GUI version also accepts the same cli arguments. But note that it cannot print any messages since there is no console attached. FYI, the GUI version uses message boxes for that instead when needed (like printing errors).

## Client

Just started working on a Python client.

Check the following codes for more information, until the documentation is added:

- Python client implementation [axserve.py](./src/python/axserve/axserve.py)
- Example usage of it [example_client.py](./src/python/example_client.py) 
- Proto file for gRPC service definition [active.proto](./src/cpp/axserve/internal/server/proto/active.proto)

# Building

## Install Tools for Building Project

### Install MSVC BuildTools

https://learn.microsoft.com/en-us/visualstudio/install/use-command-line-parameters-to-install-visual-studio?view=vs-2022

```
vs_buildtools.exe 
    --add "Microsoft.VisualStudio.Workload.VCTools"
    --add "Microsoft.VisualStudio.Component.VC.Tools.x86.x64"
    --add "Microsoft.VisualStudio.Component.VC.CMake.Project"
    --add "Microsoft.VisualStudio.Component.VC.ATL"
    --add "Microsoft.VisualStudio.Component.VC.ATLMFC"
    --includeRecommended
    --passive
    --norestart
```

### Install Chocolatey

https://chocolatey.org/install

```
Set-ExecutionPolicy Bypass -Scope Process -Force; [System.Net.ServicePointManager]::SecurityProtocol = [System.Net.ServicePointManager]::SecurityProtocol -bor 3072; iex ((New-Object System.Net.WebClient).DownloadString('https://community.chocolatey.org/install.ps1'))
```

### Install NASM

https://www.nasm.us/

```
choco install nasm
```

### Install Python

https://www.python.org/

```
choco install python311
```

### Install CMake

https://cmake.org/

```
choco install cmake
```

### Install Ninja

https://ninja-build.org/

```
choco install ninja
```

## Build Project

### List Configure Presets

```
cmake --list-presets
```

### Configure

```
cmake --preset x86-win32-debug .
```

### Build

```
cmake --build .\build\x86-win32-debug --parallel
```
