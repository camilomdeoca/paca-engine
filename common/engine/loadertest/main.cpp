#include "utils/Assert.hpp"
#include <ResourceFileFormats.hpp>
#include <utils/Log.hpp>
#include <engine/Loader.hpp>

std::string toString(const glm::mat4 &mat)
{
    return std::format(
        "\t{:>7.3f} {:>7.3f} {:>7.3f} {:>7.3f}\n"
        "\t{:>7.3f} {:>7.3f} {:>7.3f} {:>7.3f}\n"
        "\t{:>7.3f} {:>7.3f} {:>7.3f} {:>7.3f}\n"
        "\t{:>7.3f} {:>7.3f} {:>7.3f} {:>7.3f}",
        mat[0].x, mat[0].y, mat[0].x, mat[0].w,
        mat[1].x, mat[1].y, mat[1].x, mat[1].w,
        mat[2].x, mat[2].y, mat[2].x, mat[2].w,
        mat[3].x, mat[3].y, mat[3].x, mat[3].w);
}

int main (int argc, char *argv[]) {
    const char *path = "assets/meshes/man/man.gltf";
    INFO("Importing {}", path);
    INFO("CWD: {}", getenv("PWD"));

    {
        //std::ifstream ifs(path);
        //std::string line;
        //while (std::getline(ifs, line)) {
        //    INFO("{}", line);
        //}
    }

    auto mesh = engine::loaders::load<paca::fileformats::AnimatedMesh>(path);

    if (!mesh) return 1;

    for (size_t i = 0; i < mesh->vertices.size() && i < 1000; i++)
    {
        auto &vertex = *(paca::fileformats::AnimatedMesh::Vertex*)(&mesh->vertices[i]);
        INFO("Vertex {}", i);
        INFO("\tv {} {} {}", vertex.position.x, vertex.position.y, vertex.position.z);
        INFO("\tn {} {} {}", vertex.normal.x, vertex.normal.y, vertex.normal.z);
        INFO("\tt {} {} {}", vertex.tangent.x, vertex.tangent.y, vertex.tangent.z);
        INFO("\tu {} {}", vertex.texture.x, vertex.texture.y);
        INFO("\ti {} {} {} {}", vertex.boneIDs.x, vertex.boneIDs.y, vertex.boneIDs.z, vertex.boneIDs.w);
        INFO("\tw {} {} {} {}", vertex.boneWeights.x, vertex.boneWeights.y, vertex.boneWeights.z, vertex.boneWeights.w);
    }

    INFO("Nº bones: {} - {}", mesh->skeleton.bones.size(), mesh->skeleton.boneNames.size());
    ASSERT(mesh->skeleton.bones.size() == mesh->skeleton.boneNames.size());
    for (size_t i = 0; i < mesh->skeleton.bones.size(); i++)
    {
        paca::fileformats::Bone bone = mesh->skeleton.bones[i];
        std::string boneName = mesh->skeleton.boneNames[i];

        INFO("{}: {} -> {}", i, boneName, bone.parentID);
        INFO("\n{}", toString(bone.offsetMatrix));
    }

    return 0;
}
