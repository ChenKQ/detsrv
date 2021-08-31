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

/**
 * @brief Bounding Box of an object 
 */
typedef struct _BBox
{
    int idx;    // the ith object in the detection result
    std::string name; // class id
    double prob; // the confidence
    int minx; 
    int maxx; 
    int miny; 
    int maxy;
} BBox;

/**
 * @brief Detection Result
 */ 
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

/**
 * @brief Builder: 提供接口类与实现类的默认创建实例的方法:CreateInstance()
 * 该类模板供Factory进行默认调用，一般不直接调用，针对特殊类可以对该类模板进行特化
 * 
 * Usage:
 * @code {Builder}
 * class A;
 * class B: public A;
 * using XXXBuilder = Builder<A, B>;
 * @endcode
 * 
 */ 
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
    static_assert(std::is_default_constructible<IMPLEMENT>::value, 
                  "the implementation is not default constructable");
    INTERFACE* p = new IMPLEMENT();
    return std::shared_ptr<INTERFACE>(p);
}

/**
 * @brief Factory: 工厂类模板，根据字符串创建接口类的实例
 * 
 * Usage:
 * @code {.Factory-define}
 * 1- 新建接口类和实现：
 * class IA;
 * class B;
 * using FactoryIA = Factory<IA>;
 * Factory<IA>::REGISTER<B>("b");
 * 
 * 2- 使用：
 * shared_ptr<IA> binstance = FactoryIA::CreateInstance("b");
 * @endcode
 **/
template<typename INTERFACE>
struct Factory
{
    using CreateFunc = std::shared_ptr<INTERFACE> (void);  

    /**
     * @brief REGISTER 将实现类注册到接口类的工厂中
     * 
     * @tparam IMPLEMENT 实现类的类型
     * @param name ：注册名称
     */
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

/**
 * @brief 输入接口类
 * 
 * */
class IInput
{
public:
    virtual bool open(void* params) = 0;
    virtual void close() = 0;
    virtual bool read(cv::Mat& outImage) = 0;
    virtual bool isOpen() const = 0;

    virtual ~IInput() = default;
}; // IInput
/**
 * @brief 设置IInput接口类工厂的别名并进行显示实例化
 * 
 */
using InputFactory = Factory<IInput>;

/**
 * @brief 输出接口类
 * 
 */
class IOutput
{
public:
    virtual bool open(void* params) = 0;
    virtual void close() = 0;
    virtual bool write(cv::Mat& image) = 0;
    virtual bool isOpen() const = 0;

    virtual ~IOutput() = default;
}; // IOutput
/**
 * @brief 设置IOutput接口类工厂的别名并进行显示实例化
 * 
 */
using OutputFactory = Factory<IOutput>;

// class IPostProcess
// {
// public:
//     virtual bool process(cv::Mat& img, DetectionResult& result) = 0;

//     virtual ~IPostProcess() = default;
// }; 

} // namespce detsvr

#endif