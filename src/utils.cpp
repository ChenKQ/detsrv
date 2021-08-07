#include "utils.h"

namespace detsvr
{

std::string HttpUtils::dumpHeaders(const httplib::Headers& headers)
{
    std::string s;
    char buf[BUFSIZ];

    for (const auto &x : headers) {
        snprintf(buf, sizeof(buf), "%s: %s\n", x.first.c_str(), x.second.c_str());
        s += buf;
    }
    return s;
}

std::string HttpUtils::dumpMultipartFiles(const httplib::MultipartFormDataMap& files)
{
    std::string s;
    char buf[BUFSIZ];

    s += "--------------------------------\n";

    for (const auto &x : files) {
        const auto &name = x.first;
        const auto &file = x.second;

        snprintf(buf, sizeof(buf), "name: %s\n", name.c_str());
        s += buf;

        snprintf(buf, sizeof(buf), "filename: %s\n", file.filename.c_str());
        s += buf;

        snprintf(buf, sizeof(buf), "content type: %s\n", file.content_type.c_str());
        s += buf;

        snprintf(buf, sizeof(buf), "text length: %zu\n", file.content.size());
        s += buf;

        s += "----------------\n";
    }

    return s;
}

std::string HttpUtils::log(const httplib::Request& req, const httplib::Response& res)
{
    std::string s;
    char buf[BUFSIZ];

    s += "================================\n";

    snprintf(buf, sizeof(buf), "%s %s %s\n", req.method.c_str(),
            req.version.c_str(), req.path.c_str());
    s += buf;

    // std::string query;
    // for (auto it = req.params.begin(); it != req.params.end(); ++it) {
    //     const auto &x = *it;
    //     snprintf(buf, sizeof(buf), "%c%s=%s",
    //             (it == req.params.begin()) ? '?' : '&', x.first.c_str(),
    //             x.second.c_str());
    //     query += buf;
    // }
    // snprintf(buf, sizeof(buf), "%s\n", query.c_str());
    // s += buf;

    s += HttpUtils::dumpHeaders(req.headers);
    s += HttpUtils::dumpMultipartFiles(req.files);

    s += "--------------------------------\n";

    snprintf(buf, sizeof(buf), "%d\n", res.status);
    s += buf;
    s += HttpUtils::dumpHeaders(res.headers);

    return s;
}

void Logger::initialize(const std::string& p_filename,
		const std::string& p_logtitle)
{
    // char str[19];

    // get the time, and convert it to struct tm format
    // time_t a = time(0);
	// struct tm* b = localtime(&a);
    // strftime(str, 19, "-%Y-%m-%d-%H%M%S", b);

    TextLog* p_logger = 
        new TextLog(p_filename + ".log", p_logtitle, true, true);
        // new TextLog(p_filename + std::string(str) , p_logtitle, true, true);
    if (p_logger == nullptr)
    {
        throw std::runtime_error("fail to create logger");
    }
    m_plogger.reset(p_logger);
}

void Logger::Log(const std::string& p_entry)
{
    m_plogger->Log(p_entry);
}

Logger& Logger::CreateInstance()
{
    static Logger logger;
    return logger;
}

} // end namespace detsvr