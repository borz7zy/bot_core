#ifndef _ASYNCHTTP_HXX
#define _ASYNCHTTP_HXX

#include <map>
#include <atomic>
#include <thread>
#include <functional>
#include "httpclient.hxx"
#include <memory>
#include <future>

class AsyncHttp
{
public:
    AsyncHttp();
    ~AsyncHttp();

    int StartRequest(int method, const std::string &url, const std::vector<PostParameter> &params = {}, const std::string &referer = "");
    bool IsRequestComplete(int id);
    std::pair<int, std::unique_ptr<HTTP_RESPONSE>> GetRequestResult(int id);
    void SetBindAddress(const char *bindAddress);

private:
    std::atomic<int> nextId;
    std::map<int, std::future<std::pair<int, std::unique_ptr<HTTP_RESPONSE>>>> futures;
    std::map<int, bool> completedRequests;
    std::mutex mtx;
    char m_szBindAddress[256];
    bool m_iHasBindAddress = false;

    std::pair<int, std::unique_ptr<HTTP_RESPONSE>> HandleRequest(int id, int method, const std::string &url, const std::vector<PostParameter> &params, const std::string &referer);
};

#endif