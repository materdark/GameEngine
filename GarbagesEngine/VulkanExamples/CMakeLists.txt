# Copyright (c) 2016-2024, Sascha Willems
# SPDX-License-Identifier: MIT
# 函数用于构建单个示例
function(buildExample EXAMPLE_NAME)
	SET(EXAMPLE_FOLDER ${CMAKE_CURRENT_SOURCE_DIR}/${EXAMPLE_NAME})
	message(STATUS "Generating project file for example in ${EXAMPLE_FOLDER}")
	# Main
	file(GLOB SOURCE *.cpp ${BASE_HEADERS} ${EXAMPLE_FOLDER}/*.cpp)
	SET(MAIN_CPP ${EXAMPLE_FOLDER}/${EXAMPLE_NAME}.cpp)
	if(EXISTS ${EXAMPLE_FOLDER}/main.cpp)
		SET(MAIN_CPP ${EXAMPLE_FOLDER}/main.cpp)
	ENDIF()
	if(EXISTS ${EXAMPLE_FOLDER}/${EXAMPLE_NAME}.h)
		SET(MAIN_HEADER ${EXAMPLE_FOLDER}/${EXAMPLE_NAME}.h)
	ENDIF()
	# imgui示例需要额外的源文件
	IF(${EXAMPLE_NAME} STREQUAL "imgui")
		file(GLOB ADD_SOURCE "../external/imgui/*.cpp")
		SET(SOURCE ${SOURCE} ${ADD_SOURCE})
	ENDIF()
	# wayland需要额外的源文件
	IF(USE_WAYLAND_WSI)
		SET(SOURCE ${SOURCE} ${CMAKE_BINARY_DIR}/xdg-shell-client-protocol.h ${CMAKE_BINARY_DIR}/xdg-shell-protocol.c)
	ENDIF()
	# 添加着色器文件
	set(SHADER_DIR_GLSL "../shaders/glsl/${EXAMPLE_NAME}")
	file(GLOB SHADERS_GLSL "${SHADER_DIR_GLSL}/*.vert" "${SHADER_DIR_GLSL}/*.frag" "${SHADER_DIR_GLSL}/*.comp" "${SHADER_DIR_GLSL}/*.geom" "${SHADER_DIR_GLSL}/*.tesc" "${SHADER_DIR_GLSL}/*.tese" "${SHADER_DIR_GLSL}/*.mesh" "${SHADER_DIR_GLSL}/*.task" "${SHADER_DIR_GLSL}/*.rgen" "${SHADER_DIR_GLSL}/*.rchit" "${SHADER_DIR_GLSL}/*.rmiss" "${SHADER_DIR_GLSL}/*.rcall" "${SHADER_DIR_GLSL}/*.rahit" "${SHADER_DIR_GLSL}/*.rint" "${SHADER_DIR_GLSL}/*.glsl")
	set(SHADER_DIR_HLSL "../shaders/hlsl/${EXAMPLE_NAME}")
	file(GLOB SHADERS_HLSL "${SHADER_DIR_HLSL}/*.vert" "${SHADER_DIR_HLSL}/*.frag" "${SHADER_DIR_HLSL}/*.comp" "${SHADER_DIR_HLSL}/*.geom" "${SHADER_DIR_HLSL}/*.tesc" "${SHADER_DIR_HLSL}/*.tese" "${SHADER_DIR_HLSL}/*.mesh" "${SHADER_DIR_HLSL}/*.task" "${SHADER_DIR_HLSL}/*.rgen" "${SHADER_DIR_HLSL}/*.rchit" "${SHADER_DIR_HLSL}/*.rmiss" "${SHADER_DIR_HLSL}/*.rcall" "${SHADER_DIR_HLSL}/*.rahit" "${SHADER_DIR_HLSL}/*.rint")
	source_group("Shaders\\GLSL" FILES ${SHADERS_GLSL})
	source_group("Shaders\\HLSL" FILES ${SHADERS_HLSL})
	# 添加可选的自述文件/教程
	file(GLOB README_FILES "${EXAMPLE_FOLDER}/*.md")
	if(WIN32)
		add_executable(${EXAMPLE_NAME} WIN32 ${MAIN_CPP} ${SOURCE} ${MAIN_HEADER} ${SHADERS_GLSL} ${SHADERS_HLSL} ${README_FILES})
		target_link_libraries(${EXAMPLE_NAME} VulkanBase ${Vulkan_LIBRARY} ${WINLIBS})
	else(WIN32)
		add_executable(${EXAMPLE_NAME} ${MAIN_CPP} ${SOURCE} ${MAIN_HEADER} ${SHADERS_GLSL} ${SHADERS_HLSL} ${README_FILES})
		target_link_libraries(${EXAMPLE_NAME} VulkanBase )
	endif(WIN32)

	file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
	set_target_properties(${EXAMPLE_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)
	if(${EXAMPLE_NAME} STREQUAL "texture3d")
		if(APPLE)
			# 使用Macports路径为默认值,因为x86和AppleSillicon上相同,可以在cmake命令行上复制自制程序
			if(NOT OpenMP_omp_LIBRARY AND EXISTS /opt/local/lib/libomp/libomp.dylib)
				set(OpenMP_omp_LIBRARY /opt/local/lib/libomp/libomp.dylib)
			endif()
			if(CMAKE_C_COMPILER_ID MATCHES "Clang\$")
				set(OpenMP_C_FLAGS "-Xclang -fopenmp")
				set(OpenMP_C_LIB_NAMES "omp")
				if(NOT OpenMP_C_INCLUDE_DIR AND EXISTS /opt/local/include/libomp)
					set(OpenMP_C_INCLUDE_DIR /opt/local/include/libomp)
				endif()
			endif()
			if(CMAKE_CXX_COMPILER_ID MATCHES "Clang\$")
				set(OpenMP_CXX_FLAGS "-Xclang -fopenmp")
				set(OpenMP_CXX_LIB_NAMES "omp")
				if(NOT OpenMP_CXX_INCLUDE_DIR AND EXISTS /opt/local/include/libomp)
					set(OpenMP_CXX_INCLUDE_DIR /opt/local/include/libomp)
				endif()
			endif()
		endif()
		find_package(OpenMP)
		if(OpenMP_CXX_FOUND)
			set_target_properties(${EXAMPLE_NAME} PROPERTIES COMPILE_FLAGS ${OpenMP_CXX_FLAGS})
			target_include_directories(${EXAMPLE_NAME} PRIVATE ${OpenMP_CXX_INCLUDE_DIRS})
			target_link_libraries(${EXAMPLE_NAME} ${OpenMP_CXX_LIBRARIES})
		endif()
	endif()

	if(RESOURCE_INSTALL_DIR)
		install(TARGETS ${EXAMPLE_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
	endif()
endfunction(buildExample)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/bin/")

# 构建所有示例
function(buildExamples)
	foreach(EXAMPLE ${EXAMPLES})
		buildExample(${EXAMPLE})
	endforeach(EXAMPLE)
endfunction(buildExamples)

#添加头文件项目
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../VulkanBase)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../vendor/Assimp)
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

set(EXAMPLES
	bloom
	bufferdeviceaddress
	computecloth
	computecullandlod
	computeheadless
	computenbody
	computeparticles
	computeraytracing
	computeshader
	conditionalrender
	conservativeraster
	debugprintf
	debugutils
	deferred
	deferredmultisampling
	deferredshadows
	descriptorbuffer
	descriptorindexing
	descriptorsets
	displacement
	distancefieldfonts
	dynamicrendering
	dynamicstate
	dynamicuniformbuffer	
	gears
	geometryshader
	gltfloading
	gltfscenerendering
	gltfskinning
	graphicspipelinelibrary
	hdr
	hostimagecopy
	imgui
	indirectdraw
	inlineuniformblocks
	inputattachments
	instancing
	meshshader
	multisampling
	multithreading
	multiview
	negativeviewportheight	
	occlusionquery
	offscreen
	oit
	parallaxmapping
	particlesystem
	pbrbasic
	pbribl
	pbrtexture
	pipelines
	pipelinestatistics
	pushconstants
	pushdescriptors
	radialblur
	rayquery
	raytracingbasic
	raytracingcallable
	raytracinggltf
	raytracingintersection
	raytracingreflections
	raytracingpositionfetch
	raytracingsbtdata
	raytracingshadows
	raytracingtextures
	renderheadless
	screenshot
	shaderobjects
	shadowmapping
	shadowmappingomni
	shadowmappingcascade
	specializationconstants
	sphericalenvmapping
	ssao
	stencilbuffer
	subpasses
	terraintessellation
	tessellation
	textoverlay
	texture
	texture3d
	texturearray
	texturecubemap
	texturecubemaparray
	texturemipmapgen
	texturesparseresidency
	timelinesemaphore
	triangle
	variablerateshading
	vertexattributes
	viewportarray
	vulkanscene
)

buildExamples()
