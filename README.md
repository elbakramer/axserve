# Introduction

AxServe is a server program and client interface that provides functionalities of COM or OCX components through a gRPC server.

## Motivation

There are many ways to integrate COM or OCX components, such as creating native apps or using other libraries or frameworks. But they all have their own strengths and weaknesses.

Options that I had considered so far were:

Library | Module | Based On | Maintainer | Language
-- | -- | -- | -- | --
[Win32](https://learn.microsoft.com/en-us/windows/win32/) |   |   | [Microsoft](https://www.microsoft.com/) | C/C++
[MFC](https://learn.microsoft.com/en-us/cpp/mfc/mfc-desktop-applications?view=msvc-170) |   |   | [Microsoft](https://www.microsoft.com/) | C++
[Qt5](https://doc.qt.io/qt-5/) | [ActiveQt](https://doc.qt.io/qt-5/activeqt-index.html) | Win32 | [Qt Group](https://www.qt.io/) | C++
[Qt6](https://doc.qt.io/qt-6/) | [ActiveQt](https://doc.qt.io/qt-6/activeqt-index.html) | Win32 | [Qt Group](https://www.qt.io/) | C++
[pywin32](https://github.com/mhammond/pywin32) | [win32com.client](https://github.com/mhammond/pywin32/tree/main/com/win32com/client) | Win32 | [Mark Hammond](https://github.com/mhammond) | Python
[pywin32](https://github.com/mhammond/pywin32) | [pywin.mfc.activex](https://github.com/mhammond/pywin32/blob/main/Pythonwin/pywin/mfc/activex.py) | MFC | [Mark Hammond](https://github.com/mhammond) | Python
[PyQt5](https://www.riverbankcomputing.com/software/pyqt/) | [PyQt5.QAxContainer](https://www.riverbankcomputing.com/static/Docs/PyQt5/api/qaxcontainer/qaxcontainer-module.html) | Qt5 | [Riverbank Computing](https://www.riverbankcomputing.com/) | Python
[PyQt6](https://www.riverbankcomputing.com/software/pyqt/) | [PyQt6.QAxContainer](https://www.riverbankcomputing.com/static/Docs/PyQt6/api/qaxcontainer/qaxcontainer-module.html) | Qt6 | [Riverbank Computing](https://www.riverbankcomputing.com/) | Python
[PySide2](https://doc.qt.io/qtforpython-5/) | [PySide2.QtAxContainer](https://doc.qt.io/qt-5/activeqt-container.html) | Qt5 | [Qt Group](https://www.qt.io/) | Python
[PySide6](https://doc.qt.io/qtforpython-6/) | [PySide6.QtAxContainer](https://doc.qt.io/qt-6/activeqt-container.html) | Qt6 | [Qt Group](https://www.qt.io/) | Python

And Pros & Cons of each of these options that I had experienced while using some of them were as follows:

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
    - (+) Qt based options can leverage learned skills to create other applications for platforms other than windows
    - (-) Other options may be too platform specific
- License
    - (-) PyQt Based options require GPLv3 license, unless commercial license is used
    - (+) Qt Group Maintainer options (QtX and PySideX) require LGPLv3 license
    - (+) Pywin32 based options generally considered as PSF-2.0 ([issue](https://github.com/mhammond/pywin32/issues/1646))
    - (+) Microsoft Maintainer options have less license implications

My personal goal was to use 32bit COM/OCX feature in python. So based on the analysis and my goal, my final decision was:

- Not to bring the COM/OCX part of dependency to the python side
- But make the dependency loose by supporting the functionality using an IPC technique with some libraries like gRPC
- Learn Qt6 and use that for development
- Build Qt6 for 32bit architecture support on my own
- Build single server executable and use that in python
- More specifically, run the server executable using `subprocess` and connect to that using `grpcio`

And this project is the outcome of those choices.

# Usage

## Server

### GUI

1. Run the executable by double clicking.
2. Type the required information.
    - CLSID required to instantiate an Active-X or COM object.
    - Address URI for gRPC server to bind.
3. Press start button to start server.

![axserve](https://github.com/elbakramer/axserve/blob/main/axserve.png)

### Console

If built with console support, give required options to run the server as cli argument:

```
.\axserve-x86-console-debug.exe --clsid="{A1574A0D-6BFA-4BD7-9020-DED88711818D}" --address="localhost:8080" --no-gui
```

That `--no-gui` option makes the application run without GUI components. This can be useful for cases like embedding this executable in other things. FYI, technically it's not a pure non-gui application but just tries to hide or not to show the windows created internally.

The GUI version also accepts the same cli arguments. But note that it cannot print any messages since there is no console attached. FYI, the GUI version uses message boxes for that instead when needed (like printing errors).

## Client

Just started working on a Python client.

Check the following codes for more information, until relevant documentations are added:

- Example usage of it [example_client.py](https://github.com/elbakramer/axserve/blob/main/src/python/example_client.py)
- Example usages from tests:
  - Normal synchronous API: [test_iexplorer.py](https://github.com/elbakramer/axserve/blob/main/tests/test_iexplorer.py)
  - Asynchronous API under `asyncio` framework: [test_iexplorer_async.py](https://github.com/elbakramer/axserve/blob/main/tests/test_iexplorer_async.py)
- Python client implementation [stub.py](https://github.com/elbakramer/axserve/blob/main/src/python/axserve/client/stub.py)
- Proto file for gRPC service definition [active.proto](https://github.com/elbakramer/axserve/blob/main/src/proto/active.proto)

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

### Install GIt

https://git-scm.com/

```
choco install git.install
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

## Build Project using CMake

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

## Build Project using Hatch and publish

### Build Project for Python client package

```
hatch build
```

## Publish to PyPI

```
hatch publish
```
