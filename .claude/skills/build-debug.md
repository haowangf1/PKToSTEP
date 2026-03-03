# build-debug

Configure and build the project in Debug mode using Conan and CMake.

## Behavior

This skill automates the build process following the gen_debug.ps1 workflow:

1. **Check dependencies**: Only run `conan install` if dependencies are missing (conan_toolchain.cmake not found)
2. **Configure CMake**: Generate build files with Visual Studio 2022 generator
3. **Build**: Compile the project in Debug configuration

## Usage

```
/build-debug
```

## Implementation

When invoked:

1. Check if `build/Debug/conan_toolchain.cmake` exists
2. If not found, run: `conan install . -of=build/Debug -s build_type=Debug --build=missing`
3. Run: `cmake -S . -B build/Debug -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=build/Debug/conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW`
4. Run: `cmake --build build/Debug --config Debug`

## Notes

- Uses Visual Studio 17 2022 generator
- Output directory: `build/Debug`
- Executable location: `build/Debug/Debug/PKToSTEP.exe`
- DLLs need to be copied manually to `build/Debug/Debug/` directory after first build
