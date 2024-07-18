
file(GLOB BASE_SRC "*.cpp" "*.hpp" "*.h" "../vendor/imgui/*.cpp")
file(GLOB BASE_HEADERS  "*.hpp" "*.h" )

#包含项目头文件
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/Assimp)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/Catch2)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/glm)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/imgui)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/Json)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/ktx/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/SDL)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/spdlog/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/tinygltf)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/vulkan/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/VulkanMemoryAllocator)

# •	-DNOMINMAX宏定义用于禁止Windows平台上的min和max宏定义，以避免与标准库中的std::min和std::max函数冲突。
# •	-D_USE_MATH_DEFINES宏定义用于启用一些数学常量和函数的定义，例如M_PI和sin()，这些定义通常在<cmath>头文件中。
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# 设置资产与着色器目录
if(RESOURCE_INSTALL_DIR)
	add_definitions(-DVK_EXAMPLE_ASSETS_DIR=\"${RESOURCE_INSTALL_DIR}/assets/\")
	add_definitions(-DVK_EXAMPLE_SHADERS_DIR=\"${RESOURCE_INSTALL_DIR}/shaders/\")
	install(DIRECTORY assets/ DESTINATION ${RESOURCE_INSTALL_DIR}/assets/)
	install(DIRECTORY shaders/ DESTINATION ${RESOURCE_INSTALL_DIR}/shaders/)
else()
	if(NOT USE_RELATIVE_ASSET_PATH)
		add_definitions(-DVK_EXAMPLE_ASSETS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/assets\")
		add_definitions(-DVK_EXAMPLE_SHADERS_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders/\")
	endif()
endif()

set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/ktx)
set(KTX_SOURCES
    ${KTX_DIR}/lib/texture.c
    ${KTX_DIR}/lib/hashlist.c
    ${KTX_DIR}/lib/checkheader.c
    ${KTX_DIR}/lib/swap.c
    ${KTX_DIR}/lib/memstream.c
    ${KTX_DIR}/lib/filestream.c)

add_library(VulkanBase STATIC ${BASE_SRC} ${KTX_SOURCES})
if(WIN32)
    target_link_libraries(VulkanBase ${Vulkan_LIBRARY} ${WINLIBS})
 else(WIN32)
    target_link_libraries(VulkanBase ${Vulkan_LIBRARY} ${XCB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
endif(WIN32)