project = CreateProject()

target = project:CreateLibrary("threadpool")
target:AddSourceFile("*.cpp")
target:AddSystemDynamicLibraries("pthread")

return project
