#pragma once

#include <ResourceFileFormats.hpp>

/*!
 * @param path The path of the model
 * @param size Outputs the size of the model (in the assetPack) in bytes
 */
paca::fileformats::AssetPack modelToPacaFormat(const std::string &path);
paca::fileformats::AssetPack modelToPacaFormat(const std::string &path, size_t &size);
