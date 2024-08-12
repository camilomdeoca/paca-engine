#include "ImageLoader.hpp"

#include <cassert>
#include <cstdio>
#include <png.h>
#include <pngconf.h>
#include <vector>

std::vector<unsigned char> ImageLoader::loadPNG(const std::string &filepath, unsigned int &width, unsigned int &height, unsigned int &channels)
{
    printf("Started reading %s.\n", filepath.c_str());
    std::vector<unsigned char> data;

    FILE *fp = fopen(filepath.c_str(), "rb");
    if (fp == nullptr) {
        fprintf(stderr, "Error opening file: %s.\n", filepath.c_str());
        return data;
    }

    png_byte sig[8];
    fread(sig, 1, 8, fp);
    if (!png_check_sig(sig, 8))
        return data; /* bad signature */

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (png_ptr == nullptr) {
        fclose(fp);
        fprintf(stderr, "Error creating png read struct.\n");
        return data;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);

    if (info_ptr == nullptr) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, (png_infopp)nullptr, (png_infopp)nullptr);
        fprintf(stderr, "Error creating png info struct.\n");
        return data;
    }

    if (setjmp(png_jmpbuf(png_ptr)))
    {
        /* Free all of the memory associated with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        /* If we get here, we had a problem reading the file */
        fprintf(stderr, "Error reading file.\n");
        return data;
    }

    png_init_io(png_ptr, fp);
    png_set_sig_bytes(png_ptr, 8);

    png_read_info(png_ptr, info_ptr);

    width = 0;
    height = 0;
    int bit_depth = 0, color_type = -1;

    png_uint_32 retval = png_get_IHDR(png_ptr, info_ptr, &width, &height, &bit_depth, &color_type, nullptr, nullptr, nullptr);
    channels = png_get_channels(png_ptr, info_ptr);

    if (retval != 1) {
        fprintf(stderr, "Error getting IHDR of png.\n");
        return data;
    }

    /* Expand paletted colors into true RGB triplets. */
    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    /* Extract multiple pixels with bit depths of 1, 2 or 4 from a single
    * byte into separate bytes (useful for paletted and grayscale images).
    */
    //png_set_packing(png_ptr);

    // If i wanted to add alpha channel to RGB (without alpha) image
    //if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    //    png_set_tRNS_to_alpha(png_ptr);

    size_t rowbytes = png_get_rowbytes(png_ptr, info_ptr);

    // if rows are 4-byte aligned (default or by calling glPixelStorei(GL_UNPACK_ALIGNMENT, 4))
    // not necesary if rows are 1-byte aligned (called glPixelStorei(GL_UNPACK_ALIGNMENT, 1))
    //if (rowbytes % 4) rowbytes = (rowbytes / 4 + 1) * 4;
    rowbytes = (rowbytes + 3) & ~0x03; // better way of doing it than the line above

    data.resize(height * rowbytes);

    //unsigned char *ptr = data.data() + data.size() - width*channels;
    
if (rowbytes == width*channels) printf("fine\n");
printf("rowbytes %zu, channels %d, width %d\n", rowbytes, channels, width);

    std::vector<unsigned char *> ptrs(height);
    for (unsigned int i = 0; i < height; i++)
        ptrs[i] = &data[(height-1-i) * rowbytes];

    png_read_image(png_ptr, ptrs.data());
    
    //for (size_t i = rowbytes*(height-1); i >= 0; i -= rowbytes) {
    //for (size_t i = 0; i < data.size(); i += rowbytes) {
    //    png_read_row(png_ptr, &ptr[i], nullptr);
    //}

    png_read_end(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    return data;
}
