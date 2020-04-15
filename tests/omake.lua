project = CreateProject()

dep = project:CreateDependency()
dep:AddSourceFiles("*.cpp")
dep:AddFlags("-Wall", "-Werror", "-Wextra")
dep:AddStaticLibrary("..", "threadkit_static")

target = project:CreateBinary("test_threadkit")
target:AddDependencies(dep)

return project
