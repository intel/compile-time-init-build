from conans import ConanFile

class CibConan(ConanFile):
    requires = ["catch2/3.1.0"]
    generators = ["CMakeDeps", "CMakeToolchain"]
    settings = "os", "compiler", "build_type", "arch"



