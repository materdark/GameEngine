#include "VulkanBase/VulkanAssimpModel/VulkanAssimpModel.h"

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;
BoneTransformsUBO boneTransformsUBO;

void loadModel(const std::string& path, VkPhysicalDevice& physicalDevice, const glm::mat4& parentTransform)
{
    Assimp::Importer importer;

    // 加载OBJ文件，并自动处理相关的MTL文件
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    // 检查文件是否成功加载
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        GE_CORE_ERROR("ERROR::ASSIMP::{}", importer.GetErrorString());
        return;
    }

    GE_CORE_INFO("Model loaded successfully!");

    // 处理模型数据
    processNode(scene->mRootNode, scene, physicalDevice, parentTransform);
}


void processNode(aiNode* node, const aiScene* scene, VkPhysicalDevice& physicalDevice, const glm::mat4& parentTransform) {
    // 获取当前节点的变换矩阵
    glm::mat4 nodeTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    // 计算当前节点的全局变换矩阵
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    // 处理每个节点中的所有网格
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, physicalDevice, globalTransform);

        // 在这里使用 globalTransform 进行变换相关的操作
    }

    // 递归处理子节点
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, physicalDevice, globalTransform);
    }
}



void processMesh(aiMesh* mesh, const aiScene* scene, VkPhysicalDevice& physicalDevice, const glm::mat4& globalTransform) {
    // 处理网格中的顶点
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        // 纹理坐标
        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        // 顶点颜色（如果有多个颜色通道，这里只处理第一个）
        if (mesh->mColors[0]) {
            vertex.color = glm::vec3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b);
        }
        else {
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f); // 默认白色
        }

        // 切线和副切线
        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = glm::vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 0.0f);
            vertex.bitangent = glm::vec4(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z, 0.0f);
        }

        // 初始化骨骼权重和索引
        vertex.joint0 = glm::vec4(0.0f);
        vertex.weight0 = glm::vec4(0.0f);

        // 使用全局变换矩阵变换顶点位置
        vertex.pos = glm::vec3(globalTransform * glm::vec4(vertex.pos, 1.0f));

        vertices.push_back(vertex);
    }

    // 在这里将顶点和索引数据传递给Vulkan的缓冲区...
    vks::VulkanDevice vulkanDevice(physicalDevice);
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
        aiBone* bone = mesh->mBones[boneIndex];
        int boneID = boneIndex; // 骨骼索引
        std::map<std::string, glm::mat4> boneTransforms;

        // 提取骨骼变换矩阵
        aiMatrix4x4 transform = bone->mOffsetMatrix;
        glm::mat4 boneTransform = glm::transpose(glm::make_mat4(&transform.a1));
        // 存入 UBO
        boneTransformsUBO.boneTransforms[boneIndex] = boneTransform;

        // 处理骨骼权重
        for (unsigned int weightIndex = 0; weightIndex < bone->mNumWeights; weightIndex++) {
            aiVertexWeight weight = bone->mWeights[weightIndex];
            unsigned int vertexID = weight.mVertexId;
            float boneWeight = weight.mWeight;

            for (int i = 0; i < 4; i++) {
                if (vertices[vertexID].weight0[i] == 0.0f) {
                    vertices[vertexID].joint0[i] = boneID;
                    vertices[vertexID].weight0[i] = boneWeight;
                    break;
                }
            }
        }
    }

    VkDeviceSize uboSize = sizeof(BoneTransformsUBO);
    VkBuffer uboBuffer;
    VkDeviceMemory uboMemory;

    //result 创建失败就会收到一个错误信息,而不是vk_success
    VkResult result = vulkanDevice.createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, // 使用 UBO 标志
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uboSize,
        &uboBuffer,
        &uboMemory,
        &boneTransformsUBO  // 将骨骼变换矩阵数据传递给 UBO
    );

    if (result != VK_SUCCESS) {
        GE_CORE_ERROR("Failed to create bone transforms uniform buffer!");
    }
    else {
        GE_CORE_INFO("Success to create bone transforms uniform buffer!");
    }

    // 处理网格中的面（索引）
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // 处理材质
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        processMaterial(material); // 处理材质
    }


     result = vulkanDevice.createBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBufferSize,
        &vertexBuffer,
        &vertexBufferMemory,
        vertices.data()  // 将顶点数据传递给缓冲区
    );

    if (result != VK_SUCCESS) {
        GE_CORE_ERROR("Failed to create vertex buffer!");
    }
    else {
        GE_CORE_INFO("Success to create vertex buffer!");
    }

    VkDeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    result = vulkanDevice.createBuffer(
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT, // 使用索引缓冲区标志
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBufferSize,
        &indexBuffer,
        &indexBufferMemory,
        indices.data()  // 将索引数据传递给缓冲区
    );

    if (result != VK_SUCCESS) {
        GE_CORE_ERROR("Failed to create index buffer!");
    }
    else {
        GE_CORE_INFO("Success to create index buffer!");
    }
}


void processAnimations(const aiScene* scene, std::vector<AnimationData>& animations) {
    for (unsigned int i = 0; i < scene->mNumAnimations; i++) {
        aiAnimation* anim = scene->mAnimations[i];
        AnimationData animation;

        animation.name = anim->mName.C_Str();
        animation.duration = anim->mDuration;
        animation.ticksPerSecond = anim->mTicksPerSecond;

        // 日志输出动画信息
        GE_CORE_INFO("正在处理动画: {}", animation.name);
        GE_CORE_INFO("持续时间: {}, 每秒帧数: {}", animation.duration, animation.ticksPerSecond);

        for (unsigned int j = 0; j < anim->mNumChannels; j++) {
            aiNodeAnim* channel = anim->mChannels[j];
            BoneAnimation boneAnim;
            boneAnim.name = channel->mNodeName.C_Str();

            // 使用 spdlog 记录骨骼动画通道信息
            GE_CORE_INFO("处理通道: {}", boneAnim.name);

            // 提取关键帧数据
            for (unsigned int k = 0; k < channel->mNumPositionKeys; k++) {
                aiVectorKey posKey = channel->mPositionKeys[k];
                boneAnim.positionKeys.push_back({ posKey.mTime, glm::vec3(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z) });
                GE_CORE_INFO("位置关键帧: 时间={}, 值=({}, {}, {})",
                    posKey.mTime, posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
            }
            // 旋转关键帧数据提取
            for (unsigned int k = 0; k < channel->mNumRotationKeys; k++) {
                aiQuatKey rotKey = channel->mRotationKeys[k];
                boneAnim.rotationKeys.push_back({ rotKey.mTime, glm::quat(rotKey.mValue.w, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z) });
                GE_CORE_INFO("    旋转关键帧: 时间={}, 值=({}, {}, {}, {})",
                    rotKey.mTime, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z, rotKey.mValue.w);
            }

            // 缩放关键帧数据提取
            for (unsigned int k = 0; k < channel->mNumScalingKeys; k++) {
                aiVectorKey scaleKey = channel->mScalingKeys[k];
                boneAnim.scaleKeys.push_back({ scaleKey.mTime, glm::vec3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z) });
                GE_CORE_INFO("    缩放关键帧: 时间={}, 值=({}, {}, {})",
                    scaleKey.mTime, scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z);
            }

            animation.channels.push_back(boneAnim);
        }

        animations.push_back(animation);
    }
}


void processMaterial(aiMaterial* material) {
    // 提取透明度
    float opacity = 1.0f;
    if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
        GE_CORE_INFO("Opacity: {}", opacity);
    }
    else {
        GE_CORE_INFO("No transparency information found.");
    }
    // 提取漫反射颜色
    aiColor3D color(0.f, 0.f, 0.f);
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        GE_CORE_INFO("Diffuse color: {}, {}, {}", color.r, color.g, color.b);
    }
    else {
        GE_CORE_INFO("No diffuse color found.");
    }

    // 提取漫反射纹理
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString str;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        GE_CORE_INFO("Diffuse texture path: {}", str.C_Str());
    }
	else {
		GE_CORE_INFO("No diffuse texture found.");
	}

    // 其他材质属性可以在这里提取

     // 提取法线贴图
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
        aiString str;
        material->GetTexture(aiTextureType_NORMALS, 0, &str);
        GE_CORE_INFO("Normal map texture path: {}", str.C_Str());
    }
    else {
        GE_CORE_INFO("No normal map texture found.");
    }

    // 提取光泽度/反射率贴图
    if (material->GetTextureCount(aiTextureType_SPECULAR) > 0) {
        aiString str;
        material->GetTexture(aiTextureType_SPECULAR, 0, &str);
        GE_CORE_INFO("Specular/reflection map texture path: {}", str.C_Str());
    }
    else {
        GE_CORE_INFO("No specular/reflection map texture found.");
    }

}

void processLightsAndCameras(const aiScene* scene) {
    // 检测灯光信息
    if (scene->mNumLights > 0) {
        GE_CORE_INFO("灯光信息已找到！");
        for (unsigned int i = 0; i < scene->mNumLights; i++) {
            aiLight* light = scene->mLights[i];
            // 处理灯光数据
        }
    }
    else {
        GE_CORE_INFO("未找到灯光信息。");
    }

    // 检测相机信息
    if (scene->mNumCameras > 0) {
        GE_CORE_INFO("相机信息已找到！");
        for (unsigned int i = 0; i < scene->mNumCameras; i++) {
            aiCamera* camera = scene->mCameras[i];
            // 处理相机数据
        }
    }
    else {
        GE_CORE_INFO("未找到相机信息。");
    }
}
