#ifndef DETSVR_PLUGIN_CORE_H
#define DETSVR_PLUGIN_CORE_H

#include "detcore/detection.h"
#include <dlfcn.h>
#include <memory>

namespace detsvr
{

// class IDetect;

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

struct PluginCore 
{
    static std::shared_ptr<IDetect> CreateDetector(const char* filename);
};

}

#endif