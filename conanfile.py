from conan import ConanFile
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout


class AlpineConan(ConanFile):
    name = "alpine"
    version = "0.0.19"
    license = "LGPL-2.1-or-later"
    description = "Decentralized peer-to-peer resource discovery and distributed querying platform"
    settings = "os", "compiler", "build_type", "arch"
    options = {
        "enable_tls": [True, False],
        "build_tests": [True, False],
        "build_benchmarks": [True, False],
    }
    default_options = {
        "enable_tls": False,
        "build_tests": False,
        "build_benchmarks": False,
    }

    def requirements(self):
        self.requires("nlohmann_json/3.11.3")
        self.requires("spdlog/1.14.1")
        self.requires("asio/1.30.2")
        if self.options.enable_tls:
            self.requires("openssl/[>=3.0 <4]")
            self.requires("jwt-cpp/0.7.0")
        if self.options.build_tests:
            self.requires("catch2/3.5.2")
        if self.options.build_benchmarks:
            self.requires("benchmark/1.8.3")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.variables["ALPINE_USE_SYSTEM_DEPS"] = True
        tc.variables["ALPINE_ENABLE_TLS"] = self.options.enable_tls
        tc.variables["ALPINE_BUILD_TESTS"] = self.options.build_tests
        tc.variables["ALPINE_BUILD_BENCHMARKS"] = self.options.build_benchmarks
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
