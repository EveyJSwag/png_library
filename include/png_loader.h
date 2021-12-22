 #ifndef PNG_LOADER_H
 #define PNG_LOADER_H

#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory>
#include <exception>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>

class png_loader
{
public:

    png_loader(const char* png_file_path);

    typedef struct png_info_t
    {
        unsigned int image_height;
        unsigned int image_width;
        unsigned char bit_depth;
        int color_type;
        int interlace_method;
        int compression_method;
        int filter_method;
    } png_info_t;

    void write_to_png(const char* png_file_path);

    void change_png_background(
        unsigned int old_background_pixel,
        unsigned int new_background_pixel);

    const unsigned char* get_raw_png_bytes();

    std::vector<unsigned char> get_png_bytes();

    std::vector<unsigned int> get_png_colors();

    png_info_t get_png_info();

    class png_loader_exception : public std::exception
    {
    public:
        png_loader_exception(int line, const char* file, std::string reason);
        virtual const char* what() const noexcept;
        std::string get_exception_details() { return exception_details; }
    private:
        std::string exception_details;
    };

private:
    std::unique_ptr<FILE*> png_file_ptr;
    std::unique_ptr<png_structp> png_struct_ptr;
    std::unique_ptr<png_infop> png_info_ptr;
    png_info_t info;

    std::unique_ptr<FILE*> png_file_write_ptr;
    std::unique_ptr<png_structp> png_write_struct_ptr;
    std::unique_ptr<png_infop> png_write_info_ptr;

    std::vector<unsigned char> raw_png_bytes;
    std::vector<unsigned int>  png_colors;

    std::vector<unsigned char> convert_color_to_bytes(
        std::vector<unsigned int> color_vector);

    void load_png();
    void read_png_bytes();
    void read_png_colors();
};

 #endif /* PNG_LOADER_H */
