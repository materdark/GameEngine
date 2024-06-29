project(SandBox)

# 创建可执行文件
add_executable(SandBox "SandBox.cpp")

# 链接 GarbagesEngine 库
target_link_libraries(SandBox PRIVATE GarbagesEngine)

# 添加 GarbagesEngine 的头文件目录
target_include_directories(SandBox PRIVATE ${CMAKE_SOURCE_DIR}/GarbagesEngine/src)

# 将 GarbagesEngine.dll 复制到可执行文件输出目录,防止找不到dll文件
add_custom_command(TARGET SandBox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:GarbagesEngine>
    $<TARGET_FILE_DIR:SandBox>
)
