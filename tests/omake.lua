project = CreateProject()

project:CreateBinary("test_threadkit"):AddDependencies(
    project:CreateDependency()
        :AddSourceFiles("*.cpp")
        :AddFlags("-Wall", "-Werror", "-Wextra")
        :AddStaticLibrary("..", "threadkit_static"))

return project
