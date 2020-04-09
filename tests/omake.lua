project = CreateProject()

target = project:CreateBinary("test_threadpool")
target:AddSourceFiles("*.cpp")
target:AddFlags("-Wall", "-Werror", "-Wextra", "-fPIC")
target:AddStaticLibrary("..", "threadpool_static")

return project
