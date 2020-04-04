project = CreateProject()

target = project:CreateBinary("test_threadpool")
target:AddSourceFile("*.cpp")
target:AddStaticLibrary("..", "threadpool")

return project
