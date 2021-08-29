#ifndef DETSVR_VIDEO_READER_H
#define DETSVR_VIDEO_READER_H

#include "detsvr/detsvr.h"

#include <memory>
#include <string>
#include <map>
#include <functional>
#include <opencv2/opencv.hpp>

namespace detsvr
{

// class IVideoReader
// {
// // public:
// //     using CreateFunc = std::shared_ptr<IVideoReader> (void);
// public:
//     virtual bool open(const std::string& uri) = 0;
//     virtual void close() = 0;
//     virtual bool read(cv::Mat& outImage) = 0;
//     virtual bool isOpen() const = 0;

//     virtual ~IVideoReader() = default;
// }; // IVideoReader


class OpenCVReader : public IInput
{
public:
    OpenCVReader() = default;
    ~OpenCVReader() override = default;
    OpenCVReader(const OpenCVReader& other) = delete; // nocopy
    OpenCVReader& operator= (const OpenCVReader& rhs) = delete; // no assnignment    virtual bool open(const std::string& uri) = 0;
    
    virtual bool open(void* uri) =0;
    void close() override { cap.release(); }
    bool read(cv::Mat& outImage) override;
    bool isOpen() const override {return cap.isOpened(); }

protected:
    cv::VideoCapture cap;
}; // OpenCVReader

class RtspReader final: public OpenCVReader
{
public:
    RtspReader() = default;
    ~RtspReader() override { close(); }
    
    bool open(void* uri) override;
}; // RtspReader


class CSICameraReader final : public OpenCVReader
{
public:
    CSICameraReader() = default;
    ~CSICameraReader() override { close(); }

    bool open(void* uri) override;
}; // CSICameraReader

} // namespace detsvr

#endif