#include "detcore/plugincore.h"
// #include "detcore/utils.h"
#include <iostream>
#include <cassert>

extern "C"
{
#include "dlfcn.h"
}

namespace detsvr
{

DynamicLoader::DynamicLoader(const char* filename, int flags) : 
                                            m_filename(filename), 
                                            m_flags(flags),
                                            m_handle(nullptr)
{ }

DynamicLoader::~DynamicLoader()
{
    close();
}

void DynamicLoader::open()
{
    // Logger& logger = detsvr::Logger::CreateInstance();

    m_handle = dlopen(m_filename.c_str(), m_flags);
    if(0 == m_handle)
    {
        m_handle==nullptr;
        const char* error= dlerror(); // clear any existing error
        std::string errorInfo = std::string{"cannot open the file: "} + m_filename + ":\n" + error;
        std::cerr << errorInfo << '\n';
        // logger.Log(errorInfo);
        throw std::runtime_error(errorInfo);
    }
    std:: cout << "loaded the file: " << m_filename << '\n';
    // logger.Log(std::string{"loaded the file: "} + m_filename );
    dlerror(); // clear any existing error
}

void DynamicLoader::close()
{
    if(nullptr != m_handle)
    {
        dlclose(m_handle);
    }
}

void* DynamicLoader::loopup(const char* symbol)
{
    if(m_handle == nullptr)
    {
        return nullptr;
    }

    // Logger& logger = detsvr::Logger::CreateInstance();

    void* ptr = nullptr;
    ptr = dlsym(m_handle, symbol);

    if(0 == ptr)
    {
        const char* error = dlerror(); // clear any existing errors
        std::string errorInfo = std::string{"cannot find the method: "} + symbol + ":\n" + error;
        std::cerr << errorInfo << '\n';
        // logger.Log(errorInfo);
        throw std::runtime_error(errorInfo);
    }
    std::cout << std::string{"found method: "} + symbol + '\n';
    // logger.Log(std::string{"found method: "} + symbol);
    dlerror(); // clear any existing error
    return ptr;
}

}  // end namespace detsvr