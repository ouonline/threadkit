project = CreateProject()

target = project:CreateLibrary("threadpool", STATIC | SHARED)
target:AddSourceFiles("*.cpp")
target:AddSysLibraries("pthread")

return project
