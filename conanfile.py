from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout, CMake
from conan.tools.env import Environment, VirtualRunEnv

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

        # this creates custom script with custom env variables
        vk_run_env = Environment()
        if self.settings.build_type == "Debug":
            vk_run_env.define("VK_INSTANCE_LAYERS", "VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation")
        vk_run_envvars = vk_run_env.vars(self, scope="run") # scopes the env variables to run environment
        vk_run_envvars.save_script("vk_run_env") # saves a file

        ms = VirtualRunEnv(self) 
        ms.generate() # creates a launcher script which sets up dependencies runenv and the above custom run script

    def layout(self):
        cmake_layout(self)
    
    def build_requirements(self):
        self.build_requires("cmake/[>=3.24]")
        self.build_requires("shaderc/2021.1")

    def requirements(self):
        self.requires("fmt/9.1.0")
        self.requires("glfw/3.3.8")
        self.requires("ms-gsl/4.0.0")
        self.requires("range-v3/0.12.0")
        self.requires("vulkan-loader/1.3.216.0")
        if self.settings.build_type == "Debug":
            self.requires("vulkan-validationlayers/1.3.216.0")


    def build(self):
        cmake = CMake(self)
        if self.should_configure:
            cmake.configure()
        if self.should_build:
            cmake.build()
   