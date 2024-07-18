#pragma once
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <glm/glm.hpp>
#include <array>
#include <vulkan/vulkan.h>
#include <iostream>
#include <VulkanBase/VulkanBuffer.h>
#include <VulkanBase/VulkanDevice.h>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include<spdlog/spdlog.h>

// 动画数据结构
template<typename T>
struct KeyFrame {
    double time;
    T value;
};

struct BoneAnimation {
    std::string name;
    std::vector<KeyFrame<glm::vec3>> positionKeys;
    std::vector<KeyFrame<glm::quat>> rotationKeys;
    std::vector<KeyFrame<glm::vec3>> scaleKeys;
};

struct AnimationData {
    std::string name;
    double duration;
    double ticksPerSecond;
    std::vector<BoneAnimation> channels;
};


void loadModel(const std::string& path, VkPhysicalDevice& physicalDevice, const glm::mat4& parentTransform = glm::mat4(1.0f));
void processNode(aiNode* node, const aiScene* scene, VkPhysicalDevice& physicalDevice, const glm::mat4& parentTransform);
void processMesh(aiMesh* mesh, const aiScene* scene, VkPhysicalDevice& physicalDevice, const glm::mat4& parentTransform);
void processMaterial(aiMaterial* material);
void processAnimations(const aiScene* scene, std::vector<AnimationData>& animations);
void processLightsAndCameras(const aiScene* scene);
struct Vertex {

    glm::vec3 pos;       // 顶点位置
    glm::vec3 normal;    // 法线
    glm::vec2 texCoord;  // 纹理坐标
    glm::vec3 color;     // 颜色（可选）
    glm::vec2 uv;        // UV坐标（可选）
    glm::vec4 joint0;    // 骨骼权重（可选）
    glm::vec4 weight0;   // 骨骼索引权重（可选）
    glm::vec4 tangent;   // 切线和副切线（可选）
    glm::vec4 bitangent;  // 切线和副切线（可选）
    // 如果使用Vulkan，你可能还需要定义顶点输入绑定描述符等

        // 获取顶点绑定描述符
    static VkVertexInputBindingDescription getBindingDescription() {
        VkVertexInputBindingDescription bindingDescription = {};
        bindingDescription.binding = 0;
        bindingDescription.stride = sizeof(Vertex);
        bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return bindingDescription;
    }

    // 获取顶点属性描述符
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions() {
        std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions = {};

        // 位置属性
        attributeDescriptions[0].binding = 0;
        attributeDescriptions[0].location = 0;
        attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[0].offset = offsetof(Vertex, pos);

        // 法线属性
        attributeDescriptions[1].binding = 0;
        attributeDescriptions[1].location = 1;
        attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributeDescriptions[1].offset = offsetof(Vertex, normal);

        // 纹理坐标属性
        attributeDescriptions[2].binding = 0;
        attributeDescriptions[2].location = 2;
        attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
        attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

        return attributeDescriptions;
    }
};
struct BoneTransformsUBO {
    glm::mat4 boneTransforms[100];
};