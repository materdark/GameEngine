project(GarbagesEngine )

# 选项
option(USE_SPDLOG "Use spdlog library" ON)
option(USE_CATCH2 "Use Catch2 testing framework" ON)
option(USE_SDL "Use SDL library" ON)
option(USE_JSON "Use JSON library" ON)
option(USE_VULKAN "Use Vulkan" ON)
option(USE_VMA "Use Vulkan Memory Allocator" ON)
option(USE_GLM "Use OpenGL Mathematics" ON)
option(USE_ASSIMP "Use Assimp library" ON)
option(USE_DEAR_IMGUI "Use Dear ImGui library" ON)
option(USE_KTX "Use Ktx library" ON)
option(USE_D2D_WSI "Build the project using Direct to Display swapchain" OFF)
option(USE_DIRECTFB_WSI "Build the project using DirectFB swapchain" OFF)
option(USE_WAYLAND_WSI "Build the project using Wayland swapchain" OFF)
option(USE_HEADLESS "Build the project using headless extension swapchain" OFF)
option(USE_RELATIVE_ASSET_PATH "Load assets (shaders, models, textures) from a fixed path relative to the binar" OFF)
option(FORCE_VALIDATION "Forces validation on for all samples at compile time (prefer using the -v / --validation command line arguments)" OFF)


#添加子项目
add_subdirectory(VulkanBase)

add_subdirectory(VulkanExamples)

#定义一个名为RESOURCE_INSTALL_DIR的变量，并将其设置为空字符串。该变量用于指定资源安装的路径。
set(RESOURCE_INSTALL_DIR "" CACHE PATH "Path to install resources to (leave empty for running uninstalled)")

# •	-DNOMINMAX宏定义用于禁止Windows平台上的min和max宏定义，以避免与标准库中的std::min和std::max函数冲突。
# •	-D_USE_MATH_DEFINES宏定义用于启用一些数学常量和函数的定义，例如M_PI和sin()，这些定义通常在<cmath>头文件中。
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNOMINMAX -D_USE_MATH_DEFINES")

# -Wno-switch-enum选项用于禁用Clang编译器在使用switch语句时对枚举类型的警告。这个选项告诉编译器不要发出关于枚举类型switch语句中的警告信息。
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-switch-enum")
endif()


#生成预处理宏,防止生成不安全的,过期的C++函数
add_definitions(-D_CRT_SECURE_NO_WARNINGS)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
file(GLOB SOURCE *.cpp )

# 编译时强制进行验证
if (FORCE_VALIDATION)
	add_definitions(-DFORCE_VALIDATION)
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Use FindVulkan module added with CMAKE 3.7
if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()

IF(UNIX AND NOT APPLE)
	set(LINUX TRUE)
ENDIF()

if (NOT CMAKE_VERSION VERSION_LESS 3.7.0)
	message(STATUS "Using module to find Vulkan")
	find_package(Vulkan)
endif()

IF (NOT Vulkan_FOUND)
	message(FATAL_ERROR "Could not find Vulkan library!")
ELSE()
	message(STATUS ${Vulkan_LIBRARY})
ENDIF()

# 确定编译器范围
IF(MSVC)
	SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /EHsc")
ELSEIF(APPLE)
	IF(CMAKE_GENERATOR MATCHES "Makefiles" OR CMAKE_GENERATOR MATCHES "Ninja")
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -ObjC++")
	ELSE() # SRS - use objective-c++ language flag vs. ObjC++ for Xcode builds
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fobjc-arc -xobjective-c++")
	ENDIF()
ENDIF(MSVC)

#这段代码的作用是根据不同的操作系统和选项来配置编译器标志、链接库和输出目录，以便正确构建vulkanExamples项目。
IF(WIN32)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan-1 vulkan PATHS ${CMAKE_SOURCE_DIR}/libs/vulkan)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WIN32_KHR")
ELSEIF(LINUX)
	IF (NOT Vulkan_FOUND)
		find_library(Vulkan_LIBRARY NAMES vulkan HINTS "$ENV{VULKAN_SDK}/lib" "${CMAKE_SOURCE_DIR}/libs/vulkan" REQUIRED)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE("Using bundled Vulkan library version")
		ENDIF()
	ENDIF()
	find_package(Threads REQUIRED)
	IF(USE_D2D_WSI)
		MESSAGE("Using direct to display extension...")
		add_definitions(-D_DIRECT2DISPLAY)
	ELSEIF(USE_DIRECTFB_WSI)
		find_package(DirectFB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_DIRECTFB_EXT")
		include_directories(${DIRECTFB_INCLUDE_DIR})
	ELSEIF(USE_WAYLAND_WSI)
		find_program(PKG_CONFIG pkg-config)
		if (NOT PKG_CONFIG)
			message(FATAL_ERROR "pkg-config binary not found")
		endif ()
		find_package(Wayland REQUIRED)
		if (NOT WAYLAND_FOUND)
			message(FATAL_ERROR "Wayland development package not found")
		endif ()
		pkg_check_modules(WAYLAND_PROTOCOLS REQUIRED wayland-protocols)
		if (NOT WAYLAND_PROTOCOLS_FOUND)
			message(FATAL_ERROR "Wayland protocols package not found")
		endif ()
		find_program(WAYLAND_SCANNER wayland-scanner)
		if (NOT WAYLAND_SCANNER)
			message(FATAL_ERROR "wayland-scanner binary not found")
		endif ()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_WAYLAND_KHR")
		include_directories(${WAYLAND_INCLUDE_DIR})
		pkg_get_variable(protocol_dir wayland-protocols pkgdatadir)
		execute_process(COMMAND ${WAYLAND_SCANNER} client-header ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h
				COMMAND ${WAYLAND_SCANNER} private-code ${protocol_dir}/stable/xdg-shell/xdg-shell.xml ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
		include_directories(${CMAKE_BINARY_DIR})
	ELSEIF(USE_HEADLESS)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_HEADLESS_EXT")
	ELSE(USE_D2D_WSI)
		find_package(XCB REQUIRED)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_XCB_KHR")
	ENDIF(USE_D2D_WSI)
ELSEIF(APPLE)
	IF (CMAKE_GENERATOR MATCHES "Xcode")
		# Suppress regeneration for Xcode since environment variables will be lost if not set in Xcode locations/custom paths
		set(CMAKE_SUPPRESS_REGENERATION ON)
		set(CMAKE_XCODE_GENERATE_SCHEME ON)
		IF (Vulkan_FOUND)
			# If the Vulkan loader was found and its environment variables are defined, make them available within Xcode schemes
			IF (DEFINED ENV{VK_ADD_LAYER_PATH})
				set(CMAKE_XCODE_SCHEME_ENVIRONMENT "${CMAKE_XCODE_SCHEME_ENVIRONMENT};VK_ADD_LAYER_PATH=$ENV{VK_ADD_LAYER_PATH}")
			ENDIF()
			IF (DEFINED ENV{VK_ICD_FILENAMES})
				set(CMAKE_XCODE_SCHEME_ENVIRONMENT "${CMAKE_XCODE_SCHEME_ENVIRONMENT};VK_ICD_FILENAMES=$ENV{VK_ICD_FILENAMES}")
			ENDIF()
			IF (DEFINED ENV{VK_DRIVER_FILES})
				set(CMAKE_XCODE_SCHEME_ENVIRONMENT "${CMAKE_XCODE_SCHEME_ENVIRONMENT};VK_DRIVER_FILES=$ENV{VK_DRIVER_FILES}")
			ENDIF()
		ENDIF()
	ENDIF()
	IF (NOT Vulkan_FOUND)
		# If the Vulkan loader was not found, then look for libMoltenVK.dylib in the Vulkan SDK and legacy MoltenVK locations
		set(CMAKE_FIND_FRAMEWORK NEVER)
		find_library(Vulkan_LIBRARY NAMES MoltenVK HINTS "$ENV{VULKAN_SDK}/lib" "$ENV{VULKAN_SDK}/dylib/macOS" REQUIRED)
		IF (Vulkan_LIBRARY)
			set(Vulkan_FOUND ON)
			MESSAGE(STATUS "Using MoltenVK Vulkan Portability library")
		ENDIF()
	ENDIF()
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DVK_USE_PLATFORM_METAL_EXT -DVK_EXAMPLE_XCODE_GENERATED")
	# Todo : android?
ENDIF(WIN32)

IF(WIN32)
	# Nothing here (yet)
ELSEIF(APPLE)
	link_libraries(${Vulkan_LIBRARY} "-framework AppKit" "-framework QuartzCore")
ELSE(WIN32)
	link_libraries(${XCB_LIBRARIES} ${Vulkan_LIBRARY} ${Vulkan_LIBRARY} ${DIRECTFB_LIBRARIES} ${WAYLAND_CLIENT_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
ENDIF(WIN32)

# 包含头文件目录
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)


# 添加源文件
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Windows系统动态库生成lib文件命令
set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

# 创建动态库
add_library(GarbagesEngine SHARED ${SOURCES} )

# 目标链接库
target_include_directories(GarbagesEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# spdlog
if(USE_SPDLOG)
    message(STATUS "Adding spdlog as a subdirectory")
    add_subdirectory(vendor/spdlog)
    target_include_directories(GarbagesEngine PUBLIC vendor/spdlog/include)
    target_link_libraries(GarbagesEngine PRIVATE spdlog::spdlog)
    target_compile_definitions(GarbagesEngine PRIVATE USE_SPDLOG)
endif()

# Catch2
if(USE_CATCH2)
    add_subdirectory(vendor/Catch2)
    target_link_libraries(GarbagesEngine PRIVATE Catch2::Catch2)
    target_compile_definitions(GarbagesEngine PRIVATE USE_CATCH2)
endif()

# 使用 SDL
if(USE_SDL)
    if(SDL_STATIC)
        set(BUILD_SHARED_LIBS OFF)
    else()
        set(BUILD_SHARED_LIBS ON)
    endif()

    add_subdirectory(vendor/SDL)

    if(SDL_STATIC)
        target_include_directories(GarbagesEngine PUBLIC vendor/SDL/include vender/SDL/include/SDL3)
        target_link_libraries(GarbagesEngine PRIVATE SDL3-static)
    else()
        target_include_directories(GarbagesEngine PUBLIC vendor/SDL/include vender/SDL/include/SDL3)
        target_link_libraries(GarbagesEngine PRIVATE SDL3::SDL3)
    endif()
endif()

# JSON
if(USE_JSON)
    add_subdirectory(vendor/Json)
    target_include_directories(GarbagesEngine PUBLIC vendor/Json/include)
    target_link_libraries(GarbagesEngine PRIVATE nlohmann_json::nlohmann_json)
    target_compile_definitions(GarbagesEngine PRIVATE USE_JSON)
endif()

# Vulkan
if(USE_VULKAN)
    set(VULKAN_SDK_PATH "${CMAKE_CURRENT_SOURCE_DIR}/vendor/vulkan" CACHE PATH "Path to Vulkan SDK")
    set(VULKAN_INCLUDE_DIR "${VULKAN_SDK_PATH}/Include")
    if(WIN32)
        if(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/Lib")
        else()
            set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/Lib32")
        endif()
    else()
        set(VULKAN_LIB_DIR "${VULKAN_SDK_PATH}/lib")
    endif()
    find_library(VULKAN_LIBRARY NAMES vulkan-1 vulkan PATHS ${VULKAN_LIB_DIR} NO_DEFAULT_PATH)
    if(NOT VULKAN_LIBRARY)
        message(FATAL_ERROR "Could not find Vulkan library")
    endif()
    target_include_directories(GarbagesEngine PUBLIC ${VULKAN_INCLUDE_DIR})
    target_link_libraries(GarbagesEngine PRIVATE ${VULKAN_LIBRARY})
    target_compile_definitions(GarbagesEngine PRIVATE USE_VULKAN)
endif()

# VMA
if(USE_VMA)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/VulkanMemoryAllocator EXCLUDE_FROM_ALL)
    target_link_libraries(GarbagesEngine PRIVATE GPUOpen::VulkanMemoryAllocator)
    target_compile_definitions(GarbagesEngine PRIVATE USE_VMA)
endif()

# GLM
if(USE_GLM)
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/vendor/glm)
    target_link_libraries(GarbagesEngine PRIVATE glm::glm)
    target_compile_definitions(GarbagesEngine PRIVATE USE_GLM)
endif()

# 使用预编译的Assimp库
if(USE_ASSIMP)
    target_include_directories(GarbagesEngine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/vendor/Assimp/include)
    target_link_libraries(GarbagesEngine PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/vendor/Assimp/lib/x64/assimp-vc143-mt.lib)
    target_compile_definitions(GarbagesEngine PRIVATE USE_ASSIMP)

    # 复制DLL文件到输出目录，以便运行时加载
    add_custom_command(TARGET GarbagesEngine POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/assimp/bin/x64/assimp-vc143-mt.dll
        $<TARGET_FILE_DIR:GarbagesEngine>)
endif()

# 使用Dear ImGui
if(USE_DEAR_IMGUI)
    file(GLOB IMGUI_SOURCES
        "vendor/imgui/*.cpp"
    )

    add_library(GE_imgui STATIC ${IMGUI_SOURCES})
    target_include_directories(GE_imgui PUBLIC vendor/imgui)
    target_link_libraries(GarbagesEngine PRIVATE GE_imgui)
    target_compile_definitions(GarbagesEngine PRIVATE USE_DEAR_IMGUI)
endif()

# 使用预编译的KTX库
if(USE_KTX)
    set(KTX_DIR ${CMAKE_CURRENT_SOURCE_DIR} vendor/ktx)
    set(KTX_SOURCES
        ${KTX_DIR}/lib/texture.c
        ${KTX_DIR}/lib/hashlist.c
        ${KTX_DIR}/lib/checkheader.c
        ${KTX_DIR}/lib/swap.c
        ${KTX_DIR}/lib/memstream.c
        ${KTX_DIR}/lib/filestream.c)

    add_library(ktx STATIC ${KTX_SOURCES})
    target_include_directories(ktx PUBLIC ${KTX_DIR}/include)
    target_link_libraries(GarbagesEngine PRIVATE ktx)
    target_compile_definitions(GarbagesEngine PRIVATE USE_KTX)
endif()

# 添加预编译头文件
target_precompile_headers(GarbagesEngine PUBLIC "src/Pch.h")


# 设置宏
# 定义平台相关的宏
if(WIN32)
    set(GE_PLATFORM_WINDOWS 1)
else()
    message(FATAL_ERROR "GarbageEngine only supports Windows!")
endif()

# 定义 'src' 目录的路径
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# 定义动态链接相关的宏
option(GE_DYNAMIC_LINK "Use dynamic linking" ON)
option(GE_BUILD_DLL "Build as DLL" OFF)

if(GE_DYNAMIC_LINK)
    set(GARBAGEENGINE_API "__declspec(dllimport)")
    if(GE_BUILD_DLL)
        set(GE_BUILD_DLL 1)
        set(GARBAGEENGINE_API "__declspec(dllexport)")
    endif()
else()
    set(GARBAGEENGINE_API "")
endif()

# 默认宏定义
set(GE_DEBUG 1)  # 例如，开启调试信息

# 传递宏定义到编译器
target_compile_definitions(GarbagesEngine PUBLIC
    GE_PLATFORM_WINDOWS=${GE_PLATFORM_WINDOWS}
    GE_DYNAMIC_LINK=${GE_DYNAMIC_LINK}
    GE_BUILD_DLL=${GE_BUILD_DLL}
    GE_DEBUG=${GE_DEBUG}
    GE_ROOT="${CMAKE_SOURCE_DIR}"
)

