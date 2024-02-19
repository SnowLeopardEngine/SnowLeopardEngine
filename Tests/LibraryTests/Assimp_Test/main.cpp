#include <iostream>

#include <assimp/Importer.hpp>  // C++ importer interface
#include <assimp/postprocess.h> // Post processing flags
#include <assimp/scene.h>       // Output data structure

void DoTheSceneProcessing(const aiScene* scene)
{
    // TODO
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
    DoTheSceneProcessing(scene);

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
    return 0;
}