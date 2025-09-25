from conan import ConanFile
from conan.tools.cmake import CMakeToolchain


class ExampleRecipe(ConanFile):
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeDeps", "VirtualBuildEnv"

    def requirements(self):
        #self.requires("opencv/4.12.0")
        pass

    def generate(self):
        tc = CMakeToolchain(self, generator="Ninja")
        tc.generate()