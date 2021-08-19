#include "plugincore.h"
#include "dlfcn.h"
#include "utils.h"
#include <iostream>
#include <cassert>

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
    Logger& logger = detsvr::Logger::CreateInstance();

    m_handle = dlopen(m_filename.c_str(), m_flags);
    if(0 == m_handle)
    {
        m_handle==nullptr;
        std::string errorInfo = std::string{"cannot open the file: "} + m_filename;
        logger.Log(errorInfo);
        throw std::runtime_error(errorInfo);
    }
    logger.Log(std::string{"loaded the file: "} + m_filename );
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

    void* ptr = nullptr;
    ptr = dlsym(m_handle, symbol);

    if(0 == ptr)
    {
        dlerror(); // clear any existing errors
    }
    return ptr;
}

std::shared_ptr<IDetect> PluginCore::CreateDetector(const char* filename)
{
    Logger& logger = detsvr::Logger::CreateInstance();

    DynamicLoader loader(filename);
    loader.open();
    std::string methodName = "createInstance";
    void* p_factory = loader.loopup(methodName.c_str());
    if(p_factory == nullptr)
    {
        std::string errorInfo = std::string{"cannot find the method: "} + methodName;
        logger.Log(errorInfo);
        throw std::runtime_error(errorInfo);
    }
    logger.Log(std::string{"found method: "} + methodName + " in " + filename);
    using func = std::shared_ptr<IDetect> (*) ();
    func f = reinterpret_cast<func>(p_factory);
    return f();
}

}