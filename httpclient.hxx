#ifndef _HTTPCLIENT_HXX
#define _HTTPCLIENT_HXX

#include <cstddef>
#include <cstring>
#include <vector>
#include <string>
#include <map>

#include <wolfssl/options.h>
#include <wolfssl/ssl.h>

enum HTTP_METHOD
{
    HTTP_GET = 1,
    HTTP_POST,
    HTTP_HEAD,
    HTTP_PUT,
    HTTP_DELETE,
    HTTP_PATCH,
    HTTP_OPTIONS,
    HTTP_TRACE
};

enum HTTP_STATUS
{
    HTTP_SUCCESS,
    HTTP_ERROR_BAD_HOST,
    HTTP_ERROR_BAD_URL,
    HTTP_ERROR_NO_SOCKET,
    HTTP_ERROR_CANT_CONNECT,
    HTTP_ERROR_CANT_WRITE,
    HTTP_ERROR_CONTENT_TOO_BIG,
    HTTP_ERROR_MALFORMED_RESPONSE,
    HTTP_ERROR_SSL,
};

enum CONTENT_TYPE
{
    CONTENT_TYPE_UNKNOWN,
    CONTENT_TYPE_TEXT,
    CONTENT_TYPE_HTML,
    CONTENT_TYPE_JSON,
    CONTENT_TYPE_XML,
    CONTENT_TYPE_JAVASCRIPT,
    CONTENT_TYPE_IMAGE_JPEG,
    CONTENT_TYPE_IMAGE_PNG,
    CONTENT_TYPE_CSS
};

#define USER_AGENT "bot_core/0.1"
#define MAX_ENTITY_LENGTH 131072

#define GET_FORMAT "%s %s HTTP/1.1\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n"
#define POST_FORMAT "%s %s HTTP/1.1\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
#define HEAD_FORMAT "%s %s HTTP/1.1\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n"
#define PUT_FORMAT "%s %s HTTP/1.1\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
#define DELETE_FORMAT "%s %s HTTP/1.1\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n"
#define PATCH_FORMAT "%s %s HTTP/1.1\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\nContent-Type: %s\r\nContent-Length: %zu\r\n"
#define OPTIONS_FORMAT "%s %s HTTP/1.1\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n"
#define TRACE_FORMAT "%s %s HTTP/1.1\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n"

#ifndef PACKED
#if defined(_MSC_VER)
#define PACKED
#else
#define PACKED __attribute__((packed))
#endif
#endif

#ifdef _WIN32
#pragma pack(push, 1)
#endif

typedef struct
{
    unsigned short port PACKED;
    int rtype PACKED;
    char host[2048] PACKED;
    char file[1024] PACKED;
    char data[16384] PACKED;
    char referer[2048] PACKED;
} PACKED HTTP_REQUEST;

#ifdef _WIN32
#pragma pack(pop)
#endif

#ifdef _WIN32
#pragma pack(push, 1)
#endif

typedef struct
{
    char header[1024] PACKED;
    char response[MAX_ENTITY_LENGTH] PACKED;
    unsigned int header_len PACKED;
    unsigned long response_len PACKED;
    unsigned int response_code PACKED;
    unsigned int content_type PACKED;
} PACKED HTTP_RESPONSE;

#ifdef _WIN32
#pragma pack(pop)
#endif

#ifdef _WIN32
#pragma pack(push, 1)
#endif

// Structure to represent a POST parameter
struct PostParameter
{
    std::string name;
    std::string value;
    std::string filename;    // For file uploads
    std::string contentType; // For file uploads
    bool isFile = false;
};

class PACKED CHttpClient
{
private:
    int m_iSocket;
    HTTP_REQUEST m_Request;
    HTTP_RESPONSE m_Response;
    int m_iError;
    char m_szBindAddress[256];
    int m_iHasBindAddress;

    WOLFSSL_CTX *m_ssl_ctx;
    WOLFSSL *m_ssl;
    WOLFSSL_METHOD *method;
    bool tls_protocol;

    std::map<std::string, std::string> m_customHeaders;

    bool Connect(const char *szHost, int iPort, const char *szBindAddress = nullptr);
    void CloseConnection();
    bool Send(const char *szData);
    bool Send(const char *szData, size_t len); // Added for sending binary data
    int Recv(char *szBuffer, int iBufferSize);

    void InitRequest(int iType, const char *szURL, const char *szPostData, const char *szReferer);
    void HandleEntity();
    void Process();

    std::string prepareUrlEncodedData(const std::vector<PostParameter> &params);
    std::string prepareMultipartData(const std::vector<PostParameter> &params, std::string &boundary);

    bool sendDataRequest(int iType, const std::string &postData, const char *szURL, const char *szReferer, const char *contentType = nullptr);

    bool sendDataRequest(int iType, const std::vector<PostParameter> &params, const char *szURL, const char *szReferer);

    void addHeaders(std::string &request_head);

public:
    void AddCustomHeader(const std::string &name, const std::string &value)
    {
        m_customHeaders[name] = value;
    }

    void ClearCustomHeaders()
    {
        m_customHeaders.clear();
    }

    int ProcessURL(int iType, const char *szURL, const char *szData, const char *szReferer);
    int ProcessURL(int iType, const char *szURL, const std::vector<PostParameter> &params, const char *szReferer);
    bool GetHeaderValue(const char *szHeaderName, char *szReturnBuffer, size_t iBufSize);
    int GetResponseCode() const { return m_Response.response_code; }
    int GetContentType() const { return m_Response.content_type; }
    const char *GetResponseHeaders() const { return m_Response.header; }
    const char *GetDocument() const { return m_Response.response; }
    int GetDocumentLength() const { return m_Response.response_len; }

    CHttpClient(const char *szBindAddress);
    ~CHttpClient();
};

#ifdef _WIN32
#pragma pack(pop)
#endif

#endif