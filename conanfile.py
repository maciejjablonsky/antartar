from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout, CMake
from conan.tools.env import Environment, VirtualRunEnv, VirtualBuildEnv
import os


class AntartarConanFile(ConanFile):
    name = "antartar"
    settings = "os", "arch", "compiler", "build_type"
    generators = "CMakeToolchain", "CMakeDeps"

    @property
    def _is_debug(self):
        return self.settings.build_type == "Debug"

    def generate(self):
        tc = CMakeToolchain(self)

        tc.preprocessor_definitions["ANTARTAR_IS_DEBUG"] = (
            1 if self.settings.build_type == "Debug" else 0
        )
        tc.preprocessor_definitions["ANTARTAR_IS_RELEASE"] = (
            1 if self.settings.build_type == "Release" else 0
        )

        shaders_path = os.path.normpath(
            os.path.join(self.build_folder, "shaders")
        ).replace("\\", "\\\\\\\\")
        tc.preprocessor_definitions["ANTARTAR_SHADERS_DIRECTORY"] = f'"{shaders_path}"'
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

        # this creates custom script with custom env variables
        vk_run_env = Environment()
        if self.settings.build_type == "Debug":
            vk_run_env.define(
                "VK_INSTANCE_LAYERS",
                "VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation",
            )
        vk_run_envvars = vk_run_env.vars(
            self, scope="run"
        )  # scopes the env variables to run environment
        vk_run_envvars.save_script("vk_run_env")  # saves a file

        run_env = VirtualRunEnv(self)
        run_env.generate()  # creates a launcher script which sets up dependencies runenv and the above custom run script

        build_env = VirtualBuildEnv(self)
        build_env.generate()

        self._update_cmake_presets()

    def _load_json_from_file(self, path):
        import json

        with open(path, "r") as file:
            return json.load(file)

    def _save_json_to_file(self, path, content):
        import json

        with open(path, "w") as file:
            file.write(json.dumps(content, indent=4))

    def _add_env_variable_to_cmake_presets(self, cmake_presets_path, name, value):
        cmake_presets = self._load_json_from_file(cmake_presets_path)
        for preset_type in ["configurePresets", "buildPresets"]:
            for preset in cmake_presets[preset_type]:
                new_variable = {name: value}
                if "environment" in preset:
                    env = preset["environment"]
                    env = {**env, **new_variable}
                    preset["environment"] = env
                else:
                    preset["environment"] = new_variable
        self._save_json_to_file(cmake_presets_path, cmake_presets)

    def _add_cmake_executable_to_cmake_presets(self, cmake_presets_path):
        cmake_presets = self._load_json_from_file(cmake_presets_path)

        cmake_path = self.deps_env_info["cmake"].path[0]
        self.output.info(
            f'Setting "cmakeExecutable" in {cmake_presets_path} to "{cmake_path}"'
        )
        for configuration_preset in cmake_presets["configurePresets"]:
            configuration_preset["cmakeExecutable"] = os.path.join(
                cmake_path, "cmake.exe"
            )
        self._save_json_to_file(cmake_presets_path, cmake_presets)

    def _update_cmake_presets(self):
        cmake_presets_path = os.path.join(
            self.build_folder, "generators", "CMakePresets.json"
        )
        assert os.path.exists(cmake_presets_path)

        self._add_cmake_executable_to_cmake_presets(cmake_presets_path)

        if self._is_debug:
            self._add_env_variable_to_cmake_presets(
                cmake_presets_path,
                name="VK_INSTANCE_LAYERS",
                value="VK_LAYER_LUNARG_api_dump;VK_LAYER_KHRONOS_validation",
            )
            validation_layers_path = os.path.normpath(
                self.deps_cpp_info["vulkan-validationlayers"].bin_paths[0]
            )
            self._add_env_variable_to_cmake_presets(
                cmake_presets_path,
                name="VK_LAYER_PATH",
                value=validation_layers_path,
            )

    def layout(self):
        cmake_layout(self)

    def build_requirements(self):
        self.build_requires("cmake/[>=3.24]")

    def requirements(self):
        self.requires("fmt/9.1.0")
        self.requires("glfw/3.3.8")
        self.requires("ms-gsl/4.0.0")
        self.requires("range-v3/0.12.0")
        self.requires("vulkan-loader/1.3.231.1")
        self.requires("spirv-tools/1.3.231.1")
        if self.settings.build_type == "Debug":
            self.requires("vulkan-validationlayers/1.3.231.1")
        self.requires("shaderc/2021.1")

    def _build_shaders(self):
        # lame but enough way to deal with compilation
        shaders_directory = os.path.normpath(
            os.path.join(self.source_folder, "shaders")
        )
        self.output.info(f"Compiling shaders from directory {shaders_directory}")
        shaders_build_directory = os.path.join(self.build_folder, "shaders")
        self.output.info(f"Creating shaders output folder at {shaders_build_directory}")
        if not os.path.exists(shaders_build_directory):
            os.makedirs(shaders_build_directory)

        for root, directories, files in os.walk(shaders_directory):
            for file in [
                file
                for file in files
                if file.endswith(".vert") or file.endswith(".frag")
            ]:
                shader_path = os.path.normpath(os.path.join(root, file))
                self.output.info(f"Compiling {shader_path}")
                output_path = os.path.join(shaders_build_directory, f"{file}.spv")
                self.run(f"glslc {shader_path} -o {output_path}", run_environment=True)

    def build(self):
        cmake = CMake(self)
        if self.should_configure:
            cmake.configure()
        if self.should_build:
            self._build_shaders()
            cmake.build()
