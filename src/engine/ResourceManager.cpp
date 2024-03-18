#include "ResourceManager.hpp"
#include "opengl/Texture.hpp"
#include <cstdio>
#include <memory>
#include <unordered_map>
#include <utility>

static std::unordered_map<std::string, std::weak_ptr<Texture>> s_textures;

std::shared_ptr<Texture> ResourceManager::getTexture(const std::string &path)
{
    printf("Getting texture %s.\n", path.c_str());
    std::unordered_map<std::string, std::weak_ptr<Texture>>::iterator iter =
        s_textures.find(path);

    if (iter == s_textures.end())
    {
        std::shared_ptr<Texture> texture = std::make_shared<Texture>(path);
        std::weak_ptr<Texture> toInsertPtr = texture;
        s_textures.insert(std::make_pair(path, toInsertPtr));
        return texture;
    }

    std::shared_ptr<Texture> texture = iter->second.lock();
    if (texture)
        return texture;

    s_textures.erase(path);
    texture = std::make_shared<Texture>(path);
    std::weak_ptr<Texture> toInsertPtr = texture;
    s_textures.insert(std::make_pair(path, toInsertPtr));
    return texture;
}
