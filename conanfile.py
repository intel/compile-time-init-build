from conans import ConanFile, CMake

class CibConan(ConanFile):
    name = "cib"
    version = "1.0.0"
    requires = ["catch2/3.1.0"]
    generators = ["CMakeDeps", "CMakeToolchain"]
    settings = ["os", "compiler", "build_type", "arch"]

    exports_sources = "include/*", "CMakeLists.txt", "test/*", "benchmark/*"
    no_copy_source = False

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        self.run("cmake --build . {} -t tests".format(cmake.build_config))
        cmake.test()

    def package(self):
        self.copy("*.hpp")

    def package_id(self):
        self.info.clear()


