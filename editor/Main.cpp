#include "ModelConverter.hpp"

#include <ResourceFileFormats.hpp>
#include <Serializer.hpp>
#include <YamlSerializer.hpp>

#include <boost/pfr/core_name.hpp>
#include <string>
#include <vector>
#include <fstream>

int main (int argc, char *argv[]) {
    std::vector<std::string> resources;
    while (*(++argv))
    {
        resources.emplace_back(*argv);
    }
    std::string outputfile = resources.back();
    resources.pop_back();

    paca::fileformats::ResourcePack resourcePack;

    for (const std::string &resourceFilename : resources)
    {
        paca::fileformats::ResourcePack resourcesInFile
            = modelToPacaFormat(resourceFilename, resourceFilename.substr(0, resourceFilename.find_last_of(".")));
        for (const paca::fileformats::Model &model : resourcesInFile.models)
            resourcePack.models.emplace_back(model);
        for (const paca::fileformats::Material &material : resourcesInFile.materials)
            resourcePack.materials.emplace_back(material);
        for (const paca::fileformats::Texture &texture : resourcesInFile.textures)
            resourcePack.textures.emplace_back(texture);
        for (const paca::fileformats::Animation &animation : resourcesInFile.animations)
            resourcePack.animations.emplace_back(animation);
    }

    std::ofstream ofs(outputfile, std::ios::binary);
    paca::fileformats::Serializer serializer(ofs);
    serializer(resourcePack);

    paca::fileformats::Scene scene;
    scene.objects.emplace_back();
    scene.objects[0].model = "/home/camilo/Documents/camilo/cosas/paca-tools/temp/plane/plane";
    scene.objects[0].position = glm::vec3(0.0f, 0.0f, 0.0f);
    scene.objects[0].rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    scene.objects[0].scale = glm::vec3(1.0f, 1.0f, 1.0f);
    std::ofstream yamlOfs("pack.yaml");
    paca::fileformats::YamlSerializer yamlSerializer(yamlOfs);
    yamlSerializer(scene);

    return 0;
}
