#include "detectionservice.h"
#include "serialization.h"
#include "detcore/serialize.h"
#include "detection.h"
#include "base64.h"
#include "config.h"
#include "detcore/utils.h"
#include "uavship.h"

using namespace httplib;

namespace detsvr
{
DetectionService::DetectionService(Config& cfg, ::detsvr::Logger& logger) : 
                                            m_cfg(cfg), m_logger(logger)
{
    detsvr::IDetect* pdetector = new detsvr::UAVShip();
    m_pdetector = std::shared_ptr<detsvr::IDetect>(pdetector);
}

int DetectionService::initialize()
{
    if(!svr.is_valid())
    {
        m_logger.Log("server has an error...\n");
        return -1;
    }

    svr.Get("/", [](const Request& req, Response& res)
        {
            res.set_content("Detection Server is running...\n","text/plain");
        }
    );

    svr.Post("/json", [&](const Request& req, Response& res)
        {
            const Params& parms = req.params;
            m_logger.Log("params are ==============");
            std::stringstream strm;
            for(auto it = req.params.begin(); it != req.params.end(); ++it)
            {
                const auto& x = *it;
                strm << x.first + ": " + x.second << ',';
            }
            m_logger.Log(strm.str());
            m_logger.Log("params end =============");

            res.set_content("get data: \n" + strm.str() + '\n',"text/plain");
        }
    );

    svr.Post("/dump", [&](const Request& req, Response& res)
        {
            const Params& parms = req.params;
            auto ptr = parms.find("img");
            if(ptr == parms.end())
            {
                throw std::runtime_error("cannot find key \"img\"\n");
            }
            const std::string& rawDataStr = ptr->second;
            const std::string jpegData = base64_decode(rawDataStr, false);
            std::fstream saveFile("dump.jpg", std::ios::binary | std::ios::out);
            if(saveFile.is_open())
            {
                saveFile.write(jpegData.c_str(), jpegData.length());
                saveFile.close();
            }

            std::stringstream strm;
            strm << "saved dump.jpg; size: " << jpegData.length();
            m_logger.Log(strm.str());
            res.set_content(strm.str(), "text/plain");
        }
    );

    svr.Post("/detect", [&](const Request& req, Response& res)
        {
            const Params& parms = req.params;
            auto ptr = parms.find("img");
            if(ptr == parms.end())
            {
                throw std::runtime_error("cannot find key \"img\"\n");
            }
            const std::string& rawDataStr = ptr->second;
            const std::string jpegData = base64_decode(rawDataStr, false);
            
            if(nullptr == m_pdetector)
            {
                throw std::runtime_error("fail to load the detector...");
            }
            DetectionResult result = m_pdetector->detect(jpegData.c_str(), jpegData.length());
            
            std::string str = ISerialize<DetectionResult>::Serialize(result);
            res.set_content(str, "text/plain");
        }
    );

    svr.set_error_handler([](const Request & /*req*/, Response &res) 
        {
            const char *fmt = "<p>Error Status: <span style='color:red;'>%d</span></p>";
            char buf[BUFSIZ];
            snprintf(buf, sizeof(buf), fmt, res.status);
            res.set_content(buf, "text/html");
        }
    );

    svr.set_logger([&](const Request& req, const Response& res)
        {
            m_logger.Log(detsvr::HttpUtils::log(req, res));
            // std::cout << detsvr::HttpUtils::log(req, res);
        }
    );

    if(!svr.set_mount_point("/", m_cfg.svrCfg.baseDir))
    {
        m_logger.Log("The specified base directory doesn't exist...\n");
        return 1;
    }

    return 0;
}

void DetectionService::start()
{
    std::stringstream sstrm;
    sstrm << "The server is running at port " << m_cfg.svrCfg.port << "...\n" ;
    m_logger.Log(sstrm.str());
    svr.listen(m_cfg.svrCfg.listenIP.c_str(), m_cfg.svrCfg.port);
}

}