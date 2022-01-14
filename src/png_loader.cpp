#include "png_loader.h"


#define BUILD_COLOR(r, g, b, bit_depth)                 b | (g << bit_depth) | (r << (bit_depth * 2))
#define BUILD_COLOR_ALPHA(r, g, b, a, bit_depth)        a | (b << bit_depth) | (g << (bit_depth * 2)) | (r << (bit_depth * 3))
#define RED_MASK                            0xFF0000
#define GREEN_MASK                          0x00FF00
#define BLUE_MASK                           0x0000FF
#define RED_MASK_A                          0xFF000000
#define GREEN_MASK_A                        0x00FF0000
#define BLUE_MASK_A                         0x0000FF00
#define ALPHA_MASK_A                        0x000000FF


#define GET_RED_ALPHA(full_color, bit_depth)      ((RED_MASK_A   & full_color) >> bit_depth * 3)
#define GET_GREEN_ALPHA(full_color, bit_depth)    ((GREEN_MASK_A & full_color) >> bit_depth * 2)
#define GET_BLUE_ALPHA(full_color, bit_depth)     ((BLUE_MASK_A  & full_color) >> bit_depth)
#define GET_ALPHA(full_color, bit_depth)          ((ALPHA_MASK_A & full_color))

#define GET_RED(full_color, bit_depth)            ((RED_MASK   & full_color) >> bit_depth * 2)
#define GET_GREEN(full_color, bit_depth)          ((GREEN_MASK & full_color) >> bit_depth)
#define GET_BLUE(full_color, bit_depth)           ((BLUE_MASK  & full_color))

png_loader::png_loader(const char* png_file_path)
{
    png_file_ptr = std::make_unique<FILE*>(fopen(png_file_path, "rb"));

    if (!(*png_file_ptr.get()))
        throw png_loader_exception(__LINE__, __FILE__, "Failed to open png");


    png_struct_ptr = std::make_unique<png_structp>(
        png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL));

    if (!(*png_struct_ptr.get()))
        throw png_loader_exception(__LINE__, __FILE__, "Failed to create png read struct");


    png_info_ptr = std::make_unique<png_infop>(
        png_create_info_struct(*(png_struct_ptr.get())));

    if (!(*png_info_ptr.get()))
        throw png_loader_exception(__LINE__, __FILE__, "Failed to create png info struct");


    load_png();
    read_png_bytes();
    
    if (bytes_per_pixel == 4)
        read_png_colors_alpha();
    else if (bytes_per_pixel == 3)
        read_png_colors();

}

void png_loader::read_png_colors()
{
    png_bytepp png_rows;
    png_rows = png_get_rows(*png_struct_ptr.get(), *png_info_ptr.get());
    size_t total_bytes = png_get_rowbytes(*png_struct_ptr.get(), *png_info_ptr.get());

    for (int height_byte_index = 0; height_byte_index < info.image_height; height_byte_index++)
    {
        png_bytep png_row = png_rows[height_byte_index];
        for (int width_byte_index = 0; width_byte_index < info.image_width * bytes_per_pixel; width_byte_index+=3)
        {
            png_colors.push_back(BUILD_COLOR(png_row[width_byte_index], png_row[width_byte_index+1], png_row[width_byte_index+2], info.bit_depth));
        }
    } 
}

void png_loader::read_png_colors_alpha()
{
    png_bytepp png_rows;
    png_rows = png_get_rows(*png_struct_ptr.get(), *png_info_ptr.get());
    size_t total_bytes = png_get_rowbytes(*png_struct_ptr.get(), *png_info_ptr.get());

    for (int height_byte_index = 0; height_byte_index < info.image_height; height_byte_index++)
    {
        png_bytep png_row = png_rows[height_byte_index];
        for (int width_byte_index = 0; width_byte_index < info.image_width * bytes_per_pixel; width_byte_index+=4)
        {
            png_colors.push_back(BUILD_COLOR_ALPHA(png_row[width_byte_index], png_row[width_byte_index+1], png_row[width_byte_index+2], png_row[width_byte_index+3], info.bit_depth));
        }
    } 
}

void png_loader::read_png_bytes()
{
    png_bytepp png_rows;
    png_rows = png_get_rows(*png_struct_ptr.get(), *png_info_ptr.get());
    size_t total_bytes = png_get_rowbytes(*png_struct_ptr.get(), *png_info_ptr.get());

    for (int height_byte_index = 0; height_byte_index < info.image_height; height_byte_index++)
    {
        png_bytep png_row = png_rows[height_byte_index];
        for (int width_byte_index = 0; width_byte_index < info.image_width * bytes_per_pixel; width_byte_index++)
        {
            raw_png_bytes.push_back(png_row[width_byte_index]);
        }
    }
}

void png_loader::change_png_background(
    unsigned int old_background_pixel,
    unsigned int new_background_pixel)
{
    std::vector<unsigned int> new_png_colors;
    for (int i = 0; i < png_colors.size(); i++)
    {
        if (png_colors[i] == old_background_pixel)
        {
            png_colors[i] = new_background_pixel;
        }
    }
}

void png_loader::load_png()
{
    png_init_io(*png_struct_ptr.get(), *png_file_ptr.get());
    png_read_png(*png_struct_ptr.get(), *png_info_ptr.get(), 0, 0);
    png_get_IHDR(
        *png_struct_ptr.get(), 
        *png_info_ptr.get(), 
        &(info.image_width), 
        &(info.image_height), 
        (int*)(&info.bit_depth), 
        &(info.color_type),
        &(info.interlace_method),
        &(info.compression_method), 
        &(info.filter_method));

    if (info.color_type != 2 && info.color_type != PNG_COLOR_TYPE_RGBA)
        throw png_loader_exception(__LINE__,__FILE__, "Other color types are not supported");

    if ((int)info.bit_depth != 8)
        throw png_loader_exception(__LINE__,__FILE__, "Only bit depth of 8 is supported");

    if (info.color_type == 2) 
        bytes_per_pixel = 3;

    if (info.color_type == PNG_COLOR_TYPE_RGBA) 
        bytes_per_pixel = 4;

}

const char* png_loader::png_loader_exception::what() const noexcept
{
    return "png_loader exception";
}

std::vector<unsigned char> png_loader::convert_color_to_bytes(
    std::vector<unsigned int> color_vector)
{
    std::vector<unsigned char> return_vector;
    for (int i = 0; i < color_vector.size(); i++)
    {
        return_vector.push_back(GET_RED(color_vector[i], info.bit_depth));
        return_vector.push_back(GET_GREEN(color_vector[i], info.bit_depth));
        return_vector.push_back(GET_BLUE(color_vector[i], info.bit_depth));
    }
    return return_vector;
}

const unsigned char* png_loader::get_raw_png_bytes()
{
    return raw_png_bytes.data();
}

std::vector<unsigned char> png_loader::get_png_bytes()
{
    return raw_png_bytes;
}

std::vector<unsigned int> png_loader::get_png_colors()
{
    return png_colors;
}

void png_loader::write_to_png(const char* png_file_path)
{
    std::vector<unsigned char> new_byte_vector = convert_color_to_bytes(png_colors);

    png_file_write_ptr = std::make_unique<FILE*>(fopen(png_file_path, "wb"));
    if (!(*png_file_write_ptr.get()))
        throw png_loader_exception(__LINE__, __FILE__, "Failed to create png");

    png_write_struct_ptr = std::make_unique<png_structp>(
        png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL));

    if (!(*png_write_struct_ptr.get()))
        throw png_loader_exception(__LINE__, __FILE__, "Failed to png write struct");


    png_write_info_ptr = std::make_unique<png_infop>(
        png_create_info_struct(*(png_write_struct_ptr.get())));

    if (!(*png_write_info_ptr.get()))
        throw png_loader_exception(__LINE__, __FILE__, "Failed to create png info struct");

    
    png_set_IHDR(
        *(png_write_struct_ptr.get()), 
        *(png_write_info_ptr.get()),
        info.image_width, 
        info.image_height, 
        info.bit_depth, 
        info.color_type, 
        info.interlace_method, 
        info.compression_method, 
        info.filter_method);

    const unsigned char* raw_byte_data = new_byte_vector.data();
    
    png_bytepp png_byte_rows = (png_bytepp)png_malloc(
        *(png_write_struct_ptr.get()), 
        info.image_height * sizeof(png_byte*)) ;
    
    for (int j = 0; j < info.image_height; j++)
    {
        png_bytep png_byte_row = (png_bytep)png_malloc(
            *(png_write_struct_ptr.get()),   
            sizeof(png_byte) * info.image_width * bytes_per_pixel);

        for (int k = 0; k < sizeof(png_byte) * info.image_width * bytes_per_pixel; k++)
        {
             png_byte_row[k] = *raw_byte_data;
             raw_byte_data++;
        }
        png_byte_rows[j] = png_byte_row;
    }

    png_init_io(*png_write_struct_ptr.get(), *png_file_write_ptr.get());
    png_set_rows(
        *png_write_struct_ptr.get(), 
        *png_write_info_ptr.get(), 
        png_byte_rows);

    png_write_png(
        *png_write_struct_ptr.get(), 
        *png_write_info_ptr.get(),
        PNG_FORMAT_GRAY,
        0);
}


png_loader::png_loader_exception::png_loader_exception(
    int line, 
    const char* file, 
    std::string reason)
{
    std::stringstream exception_stream;
    exception_stream << "png_loader exception thrown" << std::endl <<
        "\t" << file << ":" << line << std::endl <<
        "\t" << reason;

    exception_details = exception_stream.str();
}

png_loader::png_info_t png_loader::get_png_info()
{
    return info;
}
