# Logger

If you use this as a standalone, you'll need [ETL](https://github.com/ETLCPP/etl).
You can add it with conan, or as a submodule, etc.

## Build

If you're using CLion, you need to add in CMake options (File -> Settings -> Build, Execution, Deployment -> CMake ->
CMake Options) this `-DCMAKE_TOOLCHAIN_FILE=cmake-build-debug/build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release`.

If you just cmake from cli, just add the same flags in your command.


### Conan

To build, you need to follow these steps:
- First run `conan profile detect --force`: Generates default profile detecting GCC. However, for this project, you need to set up
  the correct architecture. Find where `conan` sets up profiles (probably `~/.conan2/profiles`), run `cp default arm`
  in that folder, and edit the `arm` file. You need to change the `arch` entry to `arch=armv7`.
- Then run `conan install . --output-folder=cmake-build-debug --build=missing -pr arm`. If you're using CLion and don't see `cmake-build-debug`, you have to `Reload CMake project` to have it generated.
  After you've run `conan install...` you can `Reload CMake project` and build as per usual.
- Make sure you followed the steps under the `Build` section

<details>
<summary>Getting conan</summary>

You can install [conan](https://conan.io/) following the instructions from
[here](https://docs.conan.io/2/installation.html).:
</details>
