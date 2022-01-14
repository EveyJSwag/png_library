#include "png_loader.h"
#include <iostream>

int main()
{

    try
    {
        png_loader* png_ldr = new png_loader("ryu_stage_alpha_2.png");
        
    }
    catch (png_loader::png_loader_exception& exec)
    {
        std::cout << exec.get_exception_details() << std::endl;
    }

}
