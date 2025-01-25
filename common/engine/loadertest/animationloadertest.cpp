#include <ResourceFileFormats.hpp>
#include <utils/Log.hpp>
#include <engine/Loader.hpp>

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

    auto animation = engine::loaders::load<paca::fileformats::Animation>(path);

    if (!animation) return 1;

    for (size_t i = 0; i < animation->keyframes.size() && i < 10; i++)
    {
        INFO("Bone {} has {} keys", i, animation->keyframes[i].positions.size());
        for (size_t j = 0; j < animation->keyframes[i].positions.size() && j < 10; j++)
        {
            const glm::vec3 &pos = animation->keyframes[i].positions[j].position;
            const glm::quat &rot = animation->keyframes[i].rotations[j].quaternion;
            const glm::quat &siz = animation->keyframes[i].scalings[j].scale;
            INFO("\tPOS: {} {} {}", pos.x, pos.y, pos.z);
            INFO("\tROT: {} {} {} {}", rot.x, rot.y, rot.z, rot.w);
            INFO("\tSIZ: {} {} {}", siz.x, siz.y, siz.z);
        }
    }

    return 0;
}
