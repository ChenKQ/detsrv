#ifndef DETSVR_PLUGIN_CORE_H
#define DETSVR_PLUGIN_CORE_H

// #include "detcore/detection.h"
#include <dlfcn.h>
#include <memory>

namespace detsvr
{

class IDetect;

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

class PluginFactory
{
public:
    PluginFactory() = default;
    std::shared_ptr<::detsvr::IDetect> CreateDetector(const char* filename);

private:
    std::shared_ptr<DynamicLoader> pLoader;
};

}

#endif