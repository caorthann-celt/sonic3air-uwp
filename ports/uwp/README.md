# Sonic 3 A.I.R. UWP Host

This folder contains the UWP package host for Sonic 3 A.I.R.

The package specific files live here so the shared game code can stay in the normal upstream tree and the UWP boundary stays easy to spot in diffs.

## What is included

- UWP app manifest and package assets
- SDL WinRT startup entry
- bridge into the shared engine entry point
- local dependency bundle for SDL, Mesa and GLAD compatibility glue
- CMake project that builds the runtime from source and deploys the required UWP files

The repo is meant to be self contained for this port. A fresh checkout should not need any extra local runtime drop just to configure and build.

## Build configurations

The UWP CMake only exposes:

- `Debug`
- `Release`

That keeps the generated Visual Studio solution cleaner and avoids extra configs we are not using for the port.

## Quick start

From this folder:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Debug
```

For a release package build:

```powershell
cmake --build build --config Release
```

The dependency bundle already lives under `third_party/uwp-deps`, and the GLAD compatibility source already lives under `third_party/glad-compat`, so another developer should be able to clone the repo, run CMake and build without chasing down extra setup.

If you prefer presets:

```powershell
cmake --preset uwp-debug
cmake --build --preset uwp-debug
```

## Notes

- The port builds the shared Sonic 3 A.I.R. code directly. There is no separate local `sonic3air.dll` runtime step anymore.
- Shared code changes for the UWP port should stay behind clear `PLATFORM_UWP` checks wherever practical so rebases against upstream stay manageable.
