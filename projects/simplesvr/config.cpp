#include "config.h"
#include <string>
#include <fstream>
#include <sstream>

namespace detsvr
{

Config& Config::GetInstance()
{
    static Config cfg;
    return cfg;
}

} // end namespace detsvr