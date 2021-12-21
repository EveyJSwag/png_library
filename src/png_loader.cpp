#include "png_loader.h"


#define BUILD_COLOR(r, g, b, bit_depth)     b | (g << bit_depth) | (r << (bit_depth * 2))
#define RED_MASK                            0xFF0000
#define GREEN_MASK                          0x00FF00
#define BLUE_MASK                           0x0000FF
#define GET_RED(full_color, bit_depth)      ((RED_MASK & full_color) >> bit_depth *2)
#define GET_GREEN(full_color, bit_depth)    ((GREEN_MASK & full_color) >> bit_depth)
#define GET_BLUE(full_color, bit_depth)     ((BLUE_MASK & full_color))

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
        for (int width_byte_index = 0; width_byte_index < info.image_width * 3; width_byte_index+=3)
        {
            png_colors.push_back(BUILD_COLOR(png_row[width_byte_index], png_row[width_byte_index+1], png_row[width_byte_index+2], info.bit_depth));
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
        for (int width_byte_index = 0; width_byte_index < info.image_width * 3; width_byte_index++)
        {
            raw_png_bytes.push_back(png_row[width_byte_index]);
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
    std::cout << return_vector.size() << std::endl;
    std::cout << raw_png_bytes.size() << std::endl;
    return return_vector;
}

void png_loader::write_to_png(const char* png_file_path)
{
    unsigned int bad_number = 0x00101090;
    std::vector<unsigned int> new_png_colors;
    for (int i = 0; i < png_colors.size(); i++)
    {
        if (png_colors[i] == bad_number)
        {
            new_png_colors.push_back(0x00FFFFFF);
        }
        else
        {
            new_png_colors.push_back(png_colors[i]);
        }
    }

    std::vector<unsigned char> new_byte_vector = convert_color_to_bytes(new_png_colors);

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
            sizeof(png_byte) * info.image_width * 3);

        for (int k = 0; k < sizeof(png_byte) * info.image_width * 3; k++)
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
