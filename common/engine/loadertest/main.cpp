#include <ResourceFileFormats.hpp>
#include <utils/Log.hpp>
#include <engine/Loader.hpp>

#include <fstream>

int main (int argc, char *argv[]) {
    const char *path = "teapot.gltf";
    INFO("Importing {}", path);
    INFO("CWD: {}", getenv("PWD"));

    {
        //std::ifstream ifs(path);
        //std::string line;
        //while (std::getline(ifs, line)) {
        //    INFO("{}", line);
        //}
    }

    auto mesh = engine::loaders::load<paca::fileformats::StaticMesh>(path);

    if (!mesh) return 1;

    for (size_t i = 0;
         i < mesh->vertices.size() && i < 10 * sizeof(paca::fileformats::StaticMesh::Vertex);
         i += sizeof(paca::fileformats::StaticMesh::Vertex))
    {
        auto &vertex = *(paca::fileformats::StaticMesh::Vertex*)(&mesh->vertices[i]);
        INFO("Vertex {}", i);
        INFO("\tv {} {} {}", vertex.position.x, vertex.position.y, vertex.position.z);
        INFO("\tn {} {} {}", vertex.normal.x, vertex.normal.y, vertex.normal.z);
        INFO("\tt {} {} {}", vertex.tangent.x, vertex.tangent.y, vertex.tangent.z);
        INFO("\tu {} {}", vertex.texture.x, vertex.texture.y);
    }

    return 0;
}
