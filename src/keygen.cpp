#include "utils.h"

#include <iostream>
#include <string>


int main(int argc, char** argv)
{

    if(argc < 2)
    {
        std:: cout << "Usage: ./keygen MAC_ADDR\n";
        return 1;
    }

    if (strlen(argv[1]) != 17 )
    {
        std::cout << "wrong mac address...";
        return 2;
    }

    std::string license = detsvr::License::GenerateKey(std::string{argv[1]});

    std::cout << license << std::endl;
    return 0;

}