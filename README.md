# antartar

## development setup

Tested with
- `conan 1.52`
- `Visual Studio 17 2022` with `v17.3.4` devenv
- `powershell`

### Setup BKM

Best known method at the moment:
```powershell
conan config init
conan config install ./conan

mkdir build && cd build

conan install ..  -pr:h default -pr:b default -s compiler.cppstd=20 -s build_type=Debug --build=missing -s vulkan-validationlayers:build_type=Release

cmd "/K" '.\generators\conanrun.bat && powershell'
cmd "/K" '.\generators\conanbuild.bat && powershell'

devenv ..

# currently to build shaders it's required to run build via conan
conan build ..
```
