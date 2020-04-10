project = CreateProject()

dep = project:CreateDependency()
dep:AddSourceFiles("*.cpp")
dep:AddFlags("-Wall", "-Werror", "-Wextra", "-fPIC")
dep:AddStaticLibrary("..", "threadpool_static")

target = project:CreateBinary("test_threadpool")
target:AddDependencies(dep)

return project
