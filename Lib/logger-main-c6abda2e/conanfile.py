from os.path import join

from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMake, cmake_layout
from conan.tools.files import copy

class LoggerRecipe(ConanFile):
    name = "logger"
    version = "1.0"
    revision_mode = "scm"

    # Optional metadata
    license = "MIT"
    author = "SpaceDot - AcubeSAT, acubesat.obc@spacedot.gr"
    url = "gitlab.com/acubesat/obc/logger"
    description = "A logger library for the AcubeSAT nanosatellite"
    topics = ("satellite", "acubesat", "logger")

    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    # Sources are located in the same place as this recipe, copy them to the recipe
    exports_sources = "CMakeLists.txt", "src/*", "inc/*"
    generators = "CMakeDeps"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["NO_SYSTEM_INCLUDE"] = True
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        copy(self, pattern="*.hpp", src=join(self.source_folder, "inc"), dst=join(self.package_folder, "inc"), keep_path=False)
        cmake = CMake(self)
        cmake.install()

    def package_info(self):

        self.cpp_info.components["log_common"].libs = ["log_common"]
        self.cpp_info.components["log_common"].set_property("cmake_target_name", "log_common")

        self.cpp_info.components["log_common"].libdirs = ["lib"]
        self.cpp_info.components["log_common"].includedirs = ["inc"]

        if not str(self.settings.arch).startswith('arm'):
            self.cpp_info.components["log_x86"].libs = ["log_x86"]
            self.cpp_info.components["log_x86"].set_property("cmake_target_name", "log_x86")

            self.cpp_info.components["log_x86"].libdirs = ["lib"]
            self.cpp_info.components["log_x86"].includedirs = ["inc"]



    def requirements(self):
        self.requires("etl/20.37.2")
