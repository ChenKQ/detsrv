#include "../src/config.h"

int main(int argc, const char* argv[])
{
    detsvr::Config& cfg = detsvr::Config::GetInstance();
    cfg.dump("config.txt");
    return 0;
}