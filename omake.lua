project = CreateProject()

dep = project:CreateDependency()
    :AddSourceFiles("*.cpp")
    :AddFlags("-Wall", "-Werror", "-Wextra", "-fPIC")
    :AddSysLibraries("pthread")

project:CreateStaticLibrary("threadkit_static"):AddDependencies(dep)
project:CreateSharedLibrary("threadkit_shared"):AddDependencies(dep)

return project
