project = CreateProject()

dep = project:CreateDependency()
dep:AddSourceFiles("*.cpp")
dep:AddFlags("-Wall", "-Werror", "-Wextra", "-fPIC")
dep:AddSysLibraries("pthread")

a = project:CreateStaticLibrary("threadpool_static")
a:AddDependencies(dep)

so = project:CreateSharedLibrary("threadpool_shared")
so:AddDependencies(dep)

return project
