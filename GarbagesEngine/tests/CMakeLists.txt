# 设置测试名称
project(tests)

# 添加测试源文件
file(GLOB TEST_SOURCES "*.cpp")

# 创建测试可执行文件
add_executable(tests "tests.cpp")

# 链接Catch2库
target_link_libraries(tests PRIVATE Catch2::Catch2)

# 添加测试
include(CTest)
include(Catch)
catch_discover_tests(tests)
