# SnowLeopardEngine

<h4 align="center">
  Yet another game engine group project.
</h4>

<p align="center">
    <a href="https://github.com/SnowLeopardEngine/SnowLeopardEngine/actions" alt="CI-Windows">
        <img src="https://img.shields.io/github/actions/workflow/status/SnowLeopardEngine/SnowLeopardEngine/BuildWindows.yml?branch=master&label=CI-Windows&logo=github" /></a>
    <a href="https://github.com/SnowLeopardEngine/SnowLeopardEngine/actions" alt="CI-Linux">
        <img src="https://img.shields.io/github/actions/workflow/status/SnowLeopardEngine/SnowLeopardEngine/BuildLinux.yml?branch=master&label=CI-Linux&logo=github" /></a>
    <a href="https://github.com/SnowLeopardEngine/SnowLeopardEngine/actions" alt="CI-MacOS">
        <img src="https://img.shields.io/github/actions/workflow/status/SnowLeopardEngine/SnowLeopardEngine/BuildMacOS.yml?branch=master&label=CI-MacOS&logo=github" /></a>
    <a href="https://github.com/SnowLeopardEngine/SnowLeopardEngine/issues" alt="GitHub Issues">
        <img src="https://img.shields.io/github/issues/SnowLeopardEngine/SnowLeopardEngine">
    </a>
    <a href="https://github.com/SnowLeopardEngine/SnowLeopardEngine/blob/master/LICENSE" alt="GitHub">
        <img src="https://img.shields.io/github/license/SnowLeopardEngine/SnowLeopardEngine">
    </a>
</p>

This project is a group project of COMP5530M(23/24 Group Project, 37449), University of Leeds.

Group Members:

| Name         | Responsibility                                                       |
| ------------ | -------------------------------------------------------------------- |
| Kexuan Zhang | Leader. Architecture, Core Systems, Rendering, Editor, Audio, Report |
| Ziyu Min     | Associate Leader. Rendering, Shaders, Report                         |
| Jubiao Lin   | Physics, In-Game GUI, Poster                                         |
| Simiao Wang  | Physics, Poster, Showcase Video                                      |
| Ruofan He    | GamePlay (Path-Finding)                                              |
| Haodong Lin  | Animation                                                            |
| Yanni Ma     | Editor                                                               |

## Features
TODO

## Examples
TODO

## Game

## Prerequisites

### Windows

- [XMake](https://github.com/xmake-io/xmake)
- Visual Studio 2019 (or above) with C++ Desktop Development Environment
- Git 2.1 (or above)
- .NET 8 SDK

> Avoid long-path limit:
> Execute with system manager permission (Powershell):
> ```powershell
> New-ItemProperty -Path "HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem" ` -Name "LongPathsEnabled" -Value 1 -PropertyType DWORD -Force
> git config --system core.longpaths true
> ```

Install .NET 8 SDK through winget:

```bat
winget install Microsoft.DotNet.SDK.8
```

### macOS

- [XMake](https://github.com/xmake-io/xmake)
- XCode 13 (or above)
- Git 2.1 (or above)
- .NET 8 SDK

To install XMake on macOS, you can use HomeBrew:

```bash
brew install xmake
```

If you don't have HomeBrew, you can also install it through bash command:

```bash
bash <(curl -fsSL https://xmake.io/shget.text)
```

To install .NET 8 SDK on macOS, please download the installer [here](https://dotnet.microsoft.com/zh-cn/download/dotnet/8.0).

### Linux

- [XMake](https://github.com/xmake-io/xmake)
- GCC / Clang
- Git 2.1 (or above)
- .NET 8 SDK

#### Ubuntu / Raspberry Pi OS / Other Debian Systems

To install XMake, simply use curl:

```bash
bash <(curl -fsSL https://xmake.io/shget.text)
```

Install other packages:

```bash
sudo apt-get install build-essential cmake git clang libx11-dev libxrandr-dev libxrender-dev libglvnd-dev libxinerama-dev libxcursor-dev libxi-dev
```

To install .NET 8 SDK on Linux, please download the installer [here](https://dotnet.microsoft.com/zh-cn/download/dotnet/8.0), or you can install from package managers.

#### Other Linux Systems

TODO

### Build Instructions

#### Clone Repo

```powershell
git clone git@github.com:SnowLeopardEngine/SnowLeopardEngine.git --recursive
```

#### Build with XMake

**Windows**

```powershell
cd SnowLeopardEngine
./build.bat
```

**macOS / Linux**

```bash
cd SnowLeopardEngine
./build.sh
```

#### Run Examples, for example, `SnowLeopardEditor`

```powershell
xmake run SnowLeopardEditor
```

## 3rd Party libraries

## Reference List

## License
This project is licensed under the [MIT](./LICENSE) license.
