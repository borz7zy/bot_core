#include "asynchttp.hxx"
#include "httpclient.hxx"
#include <mutex>
#include <future>

AsyncHttp::AsyncHttp() : nextId(1)
{
    memset(m_szBindAddress, 0, sizeof(m_szBindAddress));
}

AsyncHttp::~AsyncHttp()
{
    for (auto &[id, future] : futures)
    {
        if (future.valid())
        {
            future.get(); // Get to release resources even if we don't use the result
        }
    }
}
void AsyncHttp::SetBindAddress(const char *bindAddress)
{
    std::lock_guard<std::mutex> lock(mtx);
    m_iHasBindAddress = 0;
    memset(m_szBindAddress, 0, sizeof(m_szBindAddress));
    if (bindAddress)
    {
        m_iHasBindAddress = 1;
        strncpy(m_szBindAddress, bindAddress, sizeof(m_szBindAddress) - 1);
        m_szBindAddress[sizeof(m_szBindAddress) - 1] = '\0';
    }
}
int AsyncHttp::StartRequest(int method, const std::string &url, const std::vector<PostParameter> &params, const std::string &referer)
{
    std::lock_guard<std::mutex> lock(mtx);
    int id = nextId++;

    // Launch async task using std::async
    futures[id] = std::async(std::launch::async, &AsyncHttp::HandleRequest, this, id, method, url, params, referer);
    return id;
}

bool AsyncHttp::IsRequestComplete(int id)
{
    std::lock_guard<std::mutex> lock(mtx);
    return completedRequests.find(id) != completedRequests.end();
}

std::pair<int, std::unique_ptr<HTTP_RESPONSE>> AsyncHttp::GetRequestResult(int id)
{
    std::lock_guard<std::mutex> lock(mtx);

    auto it = futures.find(id);
    if (it == futures.end())
    {
        return {HTTP_ERROR_MALFORMED_RESPONSE, nullptr}; // Request id not found.
    }

    if (it->second.valid())
    {
        completedRequests[id] = true;
        return it->second.get(); // Get the result and release resources
    }
    else
    {
        return {HTTP_ERROR_MALFORMED_RESPONSE, nullptr}; // Request not completed.
    }
}

std::pair<int, std::unique_ptr<HTTP_RESPONSE>> AsyncHttp::HandleRequest(int id, int method, const std::string &url, const std::vector<PostParameter> &params, const std::string &referer)
{
    std::unique_ptr<HTTP_RESPONSE> response = std::make_unique<HTTP_RESPONSE>();
    CHttpClient client(m_iHasBindAddress ? m_szBindAddress : nullptr);
    int status = HTTP_ERROR_MALFORMED_RESPONSE;
    if (params.size() == 0)
    {
        status = client.ProcessURL(method, url.c_str(), nullptr, referer.c_str());
    }
    else
    {
        status = client.ProcessURL(method, url.c_str(), params, referer.c_str());
    }

    if (status == HTTP_SUCCESS)
    {
        // Copy the data from the HTTP client's response
        memcpy(response->header, client.GetResponseHeaders(), sizeof(response->header) - 1);
        response->header[sizeof(response->header) - 1] = '\0';

        memcpy(response->response, client.GetDocument(), sizeof(response->response) - 1);
        response->response[sizeof(response->response) - 1] = '\0';

        response->header_len = client.GetResponseHeaders() ? strlen(client.GetResponseHeaders()) : 0;
        response->response_len = client.GetDocumentLength();
        response->response_code = client.GetResponseCode();
        response->content_type = client.GetContentType();
    }
    return {status, std::move(response)};
}