project = Project()

project:CreateBinary("test_threadkit"):AddDependencies(
    project:CreateDependency()
        :AddSourceFiles("*.cpp")
        :AddFlags({"-Wall", "-Werror", "-Wextra"})
        :AddStaticLibraries("..", "threadkit_static"))

return project
