#ifndef DETSVR_DETSVR_H
#define DETSVR_DETSVR_H

#include <map>
#include <string>
#include <vector>
#include <memory>
#include <functional>

namespace cv
{
    class Mat;
} // namespace cv


namespace detsvr
{
typedef struct _BBox
{
    int idx; 
    std::string name;
    double prob; 
    int minx;
    int maxx;
    int miny;
    int maxy;
} BBox;

typedef struct _DetectionResult
{
    std::string img_tag; // source
    std::string img_time; // "YYYY-MM-DD hh:mm:ss"
    int img_height; // pixel
    int img_width; // pixel
    int pre_time; // ms
    int inf_time; // ms

    std::vector<BBox> list; // detected objects
} DetectionResult;

class IInput
{
public:
    virtual bool open(void* params) = 0;
    virtual void close() = 0;
    virtual bool read(cv::Mat& outImage) = 0;
    virtual bool isOpen() const = 0;

    virtual ~IInput() = default;
}; // IInput

class IOutput
{
public:
    virtual bool open(void* params) = 0;
    virtual void close() = 0;
    virtual bool write(cv::Mat& image) = 0;
    virtual bool isOpen() const = 0;

    virtual ~IOutput() = default;
}; // IOutput

class IPostProcess
{
public:
    virtual bool process(cv::Mat& img, DetectionResult& result) = 0;

    virtual ~IPostProcess() = default;
}; 

template<typename INTERFACE, typename IMPLEMENT>
struct Builder
{
    static_assert(std::is_base_of<INTERFACE, IMPLEMENT>::value, 
                  "the relationship is not inheritance");

    static std::shared_ptr<INTERFACE> CreateInstance();
}; // Builder

template<typename INTERFACE, typename IMPLEMENT>
std::shared_ptr<INTERFACE> Builder<INTERFACE, IMPLEMENT>::CreateInstance()
{
    INTERFACE* p = new IMPLEMENT();
    return std::shared_ptr<INTERFACE>(p);
}

template<typename INTERFACE>
struct Factory
{
    using CreateFunc = std::shared_ptr<INTERFACE> (void);  

    template<typename IMPLEMENT>
    static void REGISTER(const std::string& name);
 
    static std::shared_ptr<INTERFACE> CreateInstance(const std::string& name);

private:
    static std::map<std::string, std::function<CreateFunc>> repository;
    
}; // Factory

template<typename INTERFACE>
template <typename IMPLEMENT>
void Factory<INTERFACE>::REGISTER(const std::string& name)
{
    if(repository.find(name)!=repository.end())
    {
        return;
    }
    repository[name] = Builder<INTERFACE, IMPLEMENT>::CreateInstance;
}

template<typename INTERFACE>
std::shared_ptr<INTERFACE> 
Factory<INTERFACE>::CreateInstance(const std::string& name)
{
    if(repository.find(name)==repository.end())
    {
        return {nullptr};
    }
    auto& func = repository.at(name);
    return func();
}

} // namespce detsvr

#endif