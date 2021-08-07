#include "service.h"
#include "utils.h"
#include "config.h"

using namespace httplib;

namespace detsvr
{
SimpleService::SimpleService(Logger& logger): m_logger(logger)
{}

int SimpleService::initialize()
{
    if(!svr.is_valid())
    {
        m_logger.Log("server has an error...\n");
        return -1;
    }

    svr.Get("/", [](const Request& req, Response& res)
        {
            res.set_redirect("/hi");
        }
    );

    svr.Get("/hi", [](const Request& req, Response& res)
        {
            res.set_content("Hello World!\n","text/plain");
        }
    );

    svr.Get("/slow", [](const Request& req, Response& res)
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            res.set_content("Slow...\n","text/plain");
        }
    );

    svr.Get("/stop", [&](const Request& req, Response& res)
        {
            svr.stop();
        }
    );

    svr.Post("/multipart",[](const Request& req, Response& res)
        {
            auto body = HttpUtils::dumpHeaders(req.headers) + 
                        HttpUtils::dumpMultipartFiles(req.files);
                
            res.set_content(body, "text/plain");
        }
    );

    svr.Get("/dumpHeader", [](const Request& req, Response& res)
        {
            auto headerStr = detsvr::HttpUtils::dumpHeaders(req.headers);
            res.set_content(headerStr, "text/plain");
        }
    );

    svr.Get("/dumpLog", [](const Request& req, Response& res)
        {
            auto logStr = detsvr::HttpUtils::log(req, res);
            res.set_content(logStr, "text/plain");
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

    Config& cfg = Config::GetInstance();

    if(!svr.set_mount_point("/", cfg.svrCfg.baseDir))
    {
        m_logger.Log("The specified base directory doesn't exist...\n");
        return 1;
    }

    return 0;
}

void SimpleService::start()
{
    Config& cfg = Config::GetInstance();

    std::stringstream sstrm;
    sstrm << "The server is running at port " << cfg.svrCfg.port << "...\n" ;
    m_logger.Log(sstrm.str());
    svr.listen(cfg.svrCfg.listenIP.c_str(), cfg.svrCfg.port);
}

}