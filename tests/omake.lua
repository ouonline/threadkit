project = CreateProject()

target = project:CreateBinary("test_threadpool")
target:AddSourceFiles("*.cpp")
target:AddLibrary("..", "threadpool", STATIC)

return project
