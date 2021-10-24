#include "detcore/utils.h"
#include <ifaddrs.h>
#include <net/if.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <string>
#include <cstring>
#include <functional>

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

    ::minilogger::TextLog* p_logger = 
        new ::minilogger::TextLog(p_filename + ".log", p_logtitle, true, true);
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

int License::QueryMac(std::string& macAddr, const std::string& NICName)
{
    struct ifreq ifr;
    int sock = 0;
    char mac[32] = "";

    sock = socket(AF_INET,SOCK_STREAM,0);
    if (sock < 0)
    {
        return 1;
    }

    strcpy(ifr.ifr_name, NICName.c_str());
    if(ioctl(sock, SIOCGIFHWADDR, &ifr) < 0)
    {
        return 3;
    }

    int i = 0;
    for(i=0; i< 6; ++i)
    {
        sprintf(mac+3*i, "%02X:", (unsigned char)ifr.ifr_hwaddr.sa_data[i]);
    }
    mac[3*i-1] = 0;
    macAddr = std::string{mac};    

    return 0;
}

std::string License::GenerateKey(const std::string& macAddr)
{
    std::string macUpper {macAddr};
    for(auto& c : macUpper) c = toupper(c);
    
    std::string license = std::string{"AIRCAS:"} + macUpper + std::string{":CYBER"};
    std::hash<char> hash_func;
    std::stringstream hashValueStrm;
    for(auto& c : license) 
    {
        hashValueStrm << hash_func(c);
    }
    return hashValueStrm.str();
}

bool License::Legal(const std::string& authorizedCode, const std::string& NICName)
{
    std::string macAddress {""}; 
    int flag = QueryMac(macAddress, NICName);
    if(flag != 0)
    {
        return false;
    }

    std::string legalLicense = GenerateKey(macAddress);
    if(legalLicense == authorizedCode)
    {
        return true;
    }
    else 
    {
        return false;
    }
}

} // end namespace detsvr