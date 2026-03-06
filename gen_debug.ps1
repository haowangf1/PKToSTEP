rm -rf build
conan install . -of=build/Debug -s build_type=Debug --build=missing
cmake -S . -B build/Debug -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=build/Debug/conan_toolchain.cmake -DCMAKE_POLICY_DEFAULT_CMP0091=NEW
cmake --build build/Debug --config Debug