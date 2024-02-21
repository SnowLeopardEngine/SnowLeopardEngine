#include <iostream>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure
#include <glm/glm.hpp>
#include <ostream>
#include <vector>

struct Vertex {
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct Texture {
    std::string Type;
    std::string Path;
};

std::vector<Vertex> Vertices;
std::vector<unsigned int> Indices;
std::vector<Texture> Textures;

std::vector<Texture> TexturesLoader;
std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type, std::string typeName);

void DoTheSceneProcessing(aiNode *node, const aiScene* scene)
{
    // TODO
    std::cout<<"******************************************************************************"<<std::endl;
    std::cout<<"node->mNumMeshes:         "<<node->mNumMeshes<<std::endl;
    for(unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]]; 
        //m_Meshes.push_back(processMesh(mesh, scene));
        std::cout<<"mesh->mName:         "<<mesh->mName.C_Str()<<std::endl;
        //vertices
        std::cout<<"mesh->mNumVertices:         "<<mesh->mNumVertices<<std::endl;
        for(unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 v;
            v.x = mesh->mVertices[i].x;
            v.y = mesh->mVertices[i].y;
            v.z = mesh->mVertices[i].z; 
            vertex.Position = v;
            if (mesh->HasNormals())
            {
                v.x = mesh->mNormals[i].x;
                v.y = mesh->mNormals[i].y;
                v.z = mesh->mNormals[i].z;
                vertex.Normal = v;
            }
            if(mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                vec.x = mesh->mTextureCoords[0][i].x; 
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
                v.x = mesh->mTangents[i].x;
                v.y = mesh->mTangents[i].y;
                v.z = mesh->mTangents[i].z;
                vertex.Tangent = v;
                v.x = mesh->mBitangents[i].x;
                v.y = mesh->mBitangents[i].y;
                v.z = mesh->mBitangents[i].z;
                vertex.Bitangent = v;
            }
            else
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            Vertices.push_back(vertex);
        }

        //indices
        std::cout<<"mesh->mNumFaces:         "<<mesh->mNumFaces<<std::endl;
        for(unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for(unsigned int j = 0; j < face.mNumIndices; j++)
            Indices.push_back(face.mIndices[j]);        
        }

        //textures
        std::cout<<"mesh->mMaterialIndex:         "<<mesh->mMaterialIndex<<std::endl;
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        // 1. diffuse maps
        std::vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "diffuse");
        Textures.insert(Textures.end(), diffuseMaps.begin(), diffuseMaps.end());
        std::cout<<"diffuseMaps number:         "<<material->GetTextureCount(aiTextureType_DIFFUSE)<<std::endl;
        // 2. specular maps
        std::vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "specular");
        Textures.insert(Textures.end(), specularMaps.begin(), specularMaps.end());
        std::cout<<"specularMaps number:         "<<material->GetTextureCount(aiTextureType_SPECULAR)<<std::endl;
        // 3. normal maps
        std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "normal");
        Textures.insert(Textures.end(), normalMaps.begin(), normalMaps.end());
        std::cout<<"normalMaps number:         "<<material->GetTextureCount(aiTextureType_HEIGHT)<<std::endl;
        // 4. height maps
        std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "height");
        Textures.insert(Textures.end(), heightMaps.begin(), heightMaps.end());
        std::cout<<"heightMaps number:         "<<material->GetTextureCount(aiTextureType_AMBIENT)<<std::endl;
    }
    for(unsigned int i = 0; i < node->mNumChildren; i++)
    {
        DoTheSceneProcessing(node->mChildren[i], scene);
    }
}

std::vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, std::string typeName)
{
    std::vector<Texture> textures;
    for(unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        bool skip = false;
        for(auto & j : TexturesLoader)
        {
            if(std::strcmp(j.Path.data(), str.C_Str()) == 0)
            {
                textures.push_back(j);
                skip = true; 
                break;
            }
        }
        if(!skip)
        {
            Texture t;
            t.Type = typeName;
            t.Path = str.C_Str();
            textures.push_back(t);
            TexturesLoader.push_back(t);
        }
    }
    return textures;
}

// https://assimp-docs.readthedocs.io/en/latest/usage/use_the_lib.html#access-by-c-class-interface
bool DoTheImportThing(const std::string& pFile)
{
    // Create an instance of the Importer class
    Assimp::Importer importer;

    // And have it read the given file with some example postprocessing
    // Usually - if speed is not the most important aspect for you - you'll
    // probably to request more postprocessing than we do in this example.
    const aiScene* scene = importer.ReadFile(pFile,
                                             aiProcess_CalcTangentSpace | aiProcess_Triangulate |
                                                 aiProcess_JoinIdenticalVertices | aiProcess_SortByPType);

    // If the import failed, report it
    if (nullptr == scene)
    {
        std::cerr << importer.GetErrorString() << std::endl;
        return false;
    }

    // Now we can access the file's contents.
    DoTheSceneProcessing(scene->mRootNode, scene);
    // We're done. Everything will be cleaned up by the importer destructor
    return true;
}

int main()
{
    if (!DoTheImportThing("assets/Walking.fbx"))
    {
        std::cerr << "Failed to import fbx file" << std::endl;
        return 1;
    }
    std::cout<<"******************************************************************************"<<std::endl;
    std::cout<<"Vertices.size:         "<<Vertices.size()<<std::endl;
    std::cout<<"Textures.size:         "<<Textures.size()<<std::endl;


    return 0;
}