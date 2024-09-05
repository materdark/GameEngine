#include "VulkanBase/VulkanAssimpModel/VulkanAssimpModel.h"

std::vector<Vertex> vertices;
std::vector<uint32_t> indices;
BoneTransformsUBO boneTransformsUBO;

void loadModel(const std::string& path, VkPhysicalDevice& physicalDevice, const glm::mat4& parentTransform)
{
    Assimp::Importer importer;

    // ����OBJ�ļ������Զ�������ص�MTL�ļ�
    const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

    // ����ļ��Ƿ�ɹ�����
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        GE_CORE_ERROR("ERROR::ASSIMP::{}", importer.GetErrorString());
        return;
    }

    GE_CORE_INFO("Model loaded successfully!");

    // ����ģ������
    processNode(scene->mRootNode, scene, physicalDevice, parentTransform);
}


void processNode(aiNode* node, const aiScene* scene, VkPhysicalDevice& physicalDevice, const glm::mat4& parentTransform) {
    // ��ȡ��ǰ�ڵ�ı任����
    glm::mat4 nodeTransform = glm::transpose(glm::make_mat4(&node->mTransformation.a1));

    // ���㵱ǰ�ڵ��ȫ�ֱ任����
    glm::mat4 globalTransform = parentTransform * nodeTransform;

    // ����ÿ���ڵ��е���������
    for (unsigned int i = 0; i < node->mNumMeshes; i++) {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        processMesh(mesh, scene, physicalDevice, globalTransform);

        // ������ʹ�� globalTransform ���б任��صĲ���
    }

    // �ݹ鴦���ӽڵ�
    for (unsigned int i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, physicalDevice, globalTransform);
    }
}



void processMesh(aiMesh* mesh, const aiScene* scene, VkPhysicalDevice& physicalDevice, const glm::mat4& globalTransform) {
    // ���������еĶ���
    for (unsigned int i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;
        vertex.pos = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);
        vertex.normal = glm::vec3(mesh->mNormals[i].x, mesh->mNormals[i].y, mesh->mNormals[i].z);

        // ��������
        if (mesh->mTextureCoords[0]) {
            vertex.texCoord = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
        }
        else {
            vertex.texCoord = glm::vec2(0.0f, 0.0f);
        }

        // ������ɫ������ж����ɫͨ��������ֻ�����һ����
        if (mesh->mColors[0]) {
            vertex.color = glm::vec3(mesh->mColors[0][i].r, mesh->mColors[0][i].g, mesh->mColors[0][i].b);
        }
        else {
            vertex.color = glm::vec3(1.0f, 1.0f, 1.0f); // Ĭ�ϰ�ɫ
        }

        // ���ߺ͸�����
        if (mesh->HasTangentsAndBitangents()) {
            vertex.tangent = glm::vec4(mesh->mTangents[i].x, mesh->mTangents[i].y, mesh->mTangents[i].z, 0.0f);
            vertex.bitangent = glm::vec4(mesh->mBitangents[i].x, mesh->mBitangents[i].y, mesh->mBitangents[i].z, 0.0f);
        }

        // ��ʼ������Ȩ�غ�����
        vertex.joint0 = glm::vec4(0.0f);
        vertex.weight0 = glm::vec4(0.0f);

        // ʹ��ȫ�ֱ任����任����λ��
        vertex.pos = glm::vec3(globalTransform * glm::vec4(vertex.pos, 1.0f));

        vertices.push_back(vertex);
    }

    // �����ｫ������������ݴ��ݸ�Vulkan�Ļ�����...
    vks::VulkanDevice vulkanDevice(physicalDevice);
    VkDeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;

    for (unsigned int boneIndex = 0; boneIndex < mesh->mNumBones; boneIndex++) {
        aiBone* bone = mesh->mBones[boneIndex];
        int boneID = boneIndex; // ��������
        std::map<std::string, glm::mat4> boneTransforms;

        // ��ȡ�����任����
        aiMatrix4x4 transform = bone->mOffsetMatrix;
        glm::mat4 boneTransform = glm::transpose(glm::make_mat4(&transform.a1));
        // ���� UBO
        boneTransformsUBO.boneTransforms[boneIndex] = boneTransform;

        // �������Ȩ��
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

    //result ����ʧ�ܾͻ��յ�һ��������Ϣ,������vk_success
    VkResult result = vulkanDevice.createBuffer(
        VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, // ʹ�� UBO ��־
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        uboSize,
        &uboBuffer,
        &uboMemory,
        &boneTransformsUBO  // �������任�������ݴ��ݸ� UBO
    );

    if (result != VK_SUCCESS) {
        GE_CORE_ERROR("Failed to create bone transforms uniform buffer!");
    }
    else {
        GE_CORE_INFO("Success to create bone transforms uniform buffer!");
    }

    // ���������е��棨������
    for (unsigned int i = 0; i < mesh->mNumFaces; i++) {
        aiFace face = mesh->mFaces[i];
        for (unsigned int j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    // �������
    if (mesh->mMaterialIndex >= 0) {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        processMaterial(material); // �������
    }


     result = vulkanDevice.createBuffer(
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        vertexBufferSize,
        &vertexBuffer,
        &vertexBufferMemory,
        vertices.data()  // ���������ݴ��ݸ�������
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
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT, // ʹ��������������־
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        indexBufferSize,
        &indexBuffer,
        &indexBufferMemory,
        indices.data()  // ���������ݴ��ݸ�������
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

        // ��־���������Ϣ
        GE_CORE_INFO("���ڴ�����: {}", animation.name);
        GE_CORE_INFO("����ʱ��: {}, ÿ��֡��: {}", animation.duration, animation.ticksPerSecond);

        for (unsigned int j = 0; j < anim->mNumChannels; j++) {
            aiNodeAnim* channel = anim->mChannels[j];
            BoneAnimation boneAnim;
            boneAnim.name = channel->mNodeName.C_Str();

            // ʹ�� spdlog ��¼��������ͨ����Ϣ
            GE_CORE_INFO("����ͨ��: {}", boneAnim.name);

            // ��ȡ�ؼ�֡����
            for (unsigned int k = 0; k < channel->mNumPositionKeys; k++) {
                aiVectorKey posKey = channel->mPositionKeys[k];
                boneAnim.positionKeys.push_back({ posKey.mTime, glm::vec3(posKey.mValue.x, posKey.mValue.y, posKey.mValue.z) });
                GE_CORE_INFO("λ�ùؼ�֡: ʱ��={}, ֵ=({}, {}, {})",
                    posKey.mTime, posKey.mValue.x, posKey.mValue.y, posKey.mValue.z);
            }
            // ��ת�ؼ�֡������ȡ
            for (unsigned int k = 0; k < channel->mNumRotationKeys; k++) {
                aiQuatKey rotKey = channel->mRotationKeys[k];
                boneAnim.rotationKeys.push_back({ rotKey.mTime, glm::quat(rotKey.mValue.w, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z) });
                GE_CORE_INFO("    ��ת�ؼ�֡: ʱ��={}, ֵ=({}, {}, {}, {})",
                    rotKey.mTime, rotKey.mValue.x, rotKey.mValue.y, rotKey.mValue.z, rotKey.mValue.w);
            }

            // ���Źؼ�֡������ȡ
            for (unsigned int k = 0; k < channel->mNumScalingKeys; k++) {
                aiVectorKey scaleKey = channel->mScalingKeys[k];
                boneAnim.scaleKeys.push_back({ scaleKey.mTime, glm::vec3(scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z) });
                GE_CORE_INFO("    ���Źؼ�֡: ʱ��={}, ֵ=({}, {}, {})",
                    scaleKey.mTime, scaleKey.mValue.x, scaleKey.mValue.y, scaleKey.mValue.z);
            }

            animation.channels.push_back(boneAnim);
        }

        animations.push_back(animation);
    }
}


void processMaterial(aiMaterial* material) {
    // ��ȡ͸����
    float opacity = 1.0f;
    if (material->Get(AI_MATKEY_OPACITY, opacity) == AI_SUCCESS) {
        GE_CORE_INFO("Opacity: {}", opacity);
    }
    else {
        GE_CORE_INFO("No transparency information found.");
    }
    // ��ȡ��������ɫ
    aiColor3D color(0.f, 0.f, 0.f);
    if (material->Get(AI_MATKEY_COLOR_DIFFUSE, color) == AI_SUCCESS) {
        GE_CORE_INFO("Diffuse color: {}, {}, {}", color.r, color.g, color.b);
    }
    else {
        GE_CORE_INFO("No diffuse color found.");
    }

    // ��ȡ����������
    if (material->GetTextureCount(aiTextureType_DIFFUSE) > 0) {
        aiString str;
        material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
        GE_CORE_INFO("Diffuse texture path: {}", str.C_Str());
    }
	else {
		GE_CORE_INFO("No diffuse texture found.");
	}

    // �����������Կ�����������ȡ

     // ��ȡ������ͼ
    if (material->GetTextureCount(aiTextureType_NORMALS) > 0) {
        aiString str;
        material->GetTexture(aiTextureType_NORMALS, 0, &str);
        GE_CORE_INFO("Normal map texture path: {}", str.C_Str());
    }
    else {
        GE_CORE_INFO("No normal map texture found.");
    }

    // ��ȡ�����/��������ͼ
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
    // ���ƹ���Ϣ
    if (scene->mNumLights > 0) {
        GE_CORE_INFO("�ƹ���Ϣ���ҵ���");
        for (unsigned int i = 0; i < scene->mNumLights; i++) {
            aiLight* light = scene->mLights[i];
            // ����ƹ�����
        }
    }
    else {
        GE_CORE_INFO("δ�ҵ��ƹ���Ϣ��");
    }

    // ��������Ϣ
    if (scene->mNumCameras > 0) {
        GE_CORE_INFO("�����Ϣ���ҵ���");
        for (unsigned int i = 0; i < scene->mNumCameras; i++) {
            aiCamera* camera = scene->mCameras[i];
            // �����������
        }
    }
    else {
        GE_CORE_INFO("δ�ҵ������Ϣ��");
    }
}
