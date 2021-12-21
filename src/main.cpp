#include "png_loader.h"
#include <iostream>

int main()
{

    try
    {
        png_loader* png_ldr = new png_loader("ryu_sheet.png");
        png_ldr->write_to_png("poop.png");
    }
    catch (png_loader::png_loader_exception& exec)
    {
        std::cout << exec.get_exception_details() << std::endl;
    }

}
