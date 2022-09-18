from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout, CMake
from conans.tools import environment_append

class AntartarConanFile(ConanFile):
    name = "antartar"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeToolchain", "CMakeDeps"  
    
    def generate(self):
        tc = CMakeToolchain(self)
    
        tc.preprocessor_definitions["ANTARTAR_IS_DEBUG"] = 1 if self.settings.build_type == "Debug" else 0
        tc.preprocessor_definitions["ANTARTAR_IS_RELEASE"] = 1 if self.settings.build_type == "Release" else 0
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def layout(self):
        cmake_layout(self)
    
    def build_requirements(self):
        self.build_requires("shaderc/2021.1")
        self.build_requires("cmake/[>=3.24]")

    def requirements(self):
        self.requires("vulkan-loader/1.3.216.0")
        self.requires("fmt/9.1.0")
        self.requires("glfw/3.3.8")
        self.requires("ms-gsl/4.0.0")
        self.requires("range-v3/0.12.0")
        self.requires("vulkan-validationlayers/1.3.216.0")

    def _vulkan_layers(self):
        layers = {}
        if self.settings.build_type == "Debug":
            layers["VK_INSTANCE_LAYERS"] = "VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation"
        return layers


    def build(self):
        with environment_append(self._vulkan_layers()):
            cmake = CMake(self)
            if self.should_configure:
                cmake.configure()
            if self.should_build:
                cmake.build()
    