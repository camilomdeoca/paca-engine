#pragma once

#include <string>
#include <vector>

class ImageLoader {
public:
     static std::vector<unsigned char> loadPNG(const std::string &filepath, unsigned int &width, unsigned int &height, unsigned int &channels);
};
