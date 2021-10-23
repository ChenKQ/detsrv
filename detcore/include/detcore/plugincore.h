#ifndef DETSVR_PLUGIN_CORE_H
#define DETSVR_PLUGIN_CORE_H

#include <dlfcn.h>
#include <memory>

namespace detsvr
{

class DynamicLoader final
{
public:
    DynamicLoader(const char* filename, int flags = RTLD_NOW);
    DynamicLoader(const DynamicLoader& ) = delete;
    ~DynamicLoader();

    void open();
    void close();
    void* loopup(const char* symbol);

private:
    const std::string m_filename;
    const int m_flags;
    void* m_handle;

};

template <typename T>
class PluginFactory
{
public:
    PluginFactory() = default;
    std::shared_ptr<T> CreateInstance(const char* filename);

private:
    std::shared_ptr<DynamicLoader> pLoader;
};

template<typename T>
std::shared_ptr<T> PluginFactory<T>::CreateInstance(const char* filename)
{
    DynamicLoader* ptmp = new DynamicLoader(filename);
    pLoader.reset(ptmp);
    pLoader->open();
    std::string methodName = "createInstance";
    void* p_factory = pLoader->loopup(methodName.c_str());
    using func = void * (*) ();
    func f = reinterpret_cast<func>(p_factory);
    void* pobj = f();
    T* pobjT = static_cast<T*>(pobj);
    std::shared_ptr<T> spobj {pobjT};
    return spobj;
}


}

#endif