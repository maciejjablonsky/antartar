from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout, CMake

class AntartarConanFile(ConanFile):
    name = "antartar"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeToolchain", "CMakeDeps"  
    
    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def layout(self):
        cmake_layout(self)
    
    def build_requirements(self):
        self.build_requires("shaderc/2021.1")
        self.build_requires("cmake/[>=3.24]")

    def requirements(self):
        self.requires("vulkan-loader/1.3.224.0")
        self.requires("fmt/9.1.0")
        self.requires("glfw/3.3.8")
        self.requires("ms-gsl/4.0.0")
        self.requires("range-v3/0.12.0")

    def build(self):
        cmake = CMake(self)
        if self.should_configure:
            cmake.configure()
        if self.should_build:
            cmake.build()
    