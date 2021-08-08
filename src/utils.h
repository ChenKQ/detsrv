#ifndef _DETSVR_UTILS_
#define _DETSVR_UTILS_

#include "httplib.h"
#include "minilogger.h"

#include <string>
#include <memory>

namespace detsvr
{

struct HttpUtils
{
    /**
     * @brief dump the heep headers into a string
     * 
     * @param headers 
     * @return std::string 
     */
    static std::string dumpHeaders(const httplib::Headers& headers);

    /**
     * @brief dump the multipart files into a string
     * 
     * @param files 
     * @return std::string 
     */
    static std::string dumpMultipartFiles(const httplib::MultipartFormDataMap& files);

    /**
     * @brief logging every http request and response
     * 
     * @param req 
     * @param res 
     * @return std::string 
     */
    static std::string log(const httplib::Request& req, const httplib::Response& res);
};

class Logger final
{
public:
    ~Logger() = default;

    void initialize(const std::string& p_filename,
		const std::string& p_logtitle);
    void Log(const std::string& p_entry);

    static Logger& CreateInstance();

private:
    Logger() = default;
    std::unique_ptr<TextLog> m_plogger {nullptr};
    
};


struct License
{
    static int QueryMac(std::string& macAddr, const std::string& NICName);
    static std::string GenerateKey(const std::string& macAddr);
    static bool Legal(const std::string& authorizedCode, const std::string& NICName);
};



} // end namespace detsvr

#endif