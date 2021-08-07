#include "plugincore.h"
#include "dlfcn.h"

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
    m_handle = dlopen(m_filename.c_str(), m_flags);
    if(0 == m_handle)
    {
        m_handle = nullptr;
    }
    dlerror(); // clear any existing error
}

void DynamicLoader::close()
{
    if(nullptr == m_handle)
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
    DynamicLoader loader(filename);
    loader.open();
    void* p_factory = loader.loopup("createInstance");
    if(p_factory == nullptr)
    {
        return {nullptr};
    }

    using func = std::shared_ptr<IDetect> (*) ();
    func f = reinterpret_cast<func>(p_factory);
    return f();
}

}