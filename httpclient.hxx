#ifndef _HTTPCLIENT_HXX
#define _HTTPCLIENT_HXX

//

enum HTTP_METHOD
{
    HTTP_GET = 1,
    HTTP_POST,
    HTTP_HEAD,
};

//----------------------------------------------------

#ifndef PACKED
#if defined(_MSC_VER)
#define PACKED
#else
#define PACKED __attribute__((packed))
#endif
#endif

//----------------------------------------------------

#define MAX_ENTITY_LENGTH 64000

//----------------------------------------------------

enum HTTP_STATUS
{
    HTTP_SUCCESS,
    HTTP_ERROR_BAD_HOST,
    HTTP_ERROR_NO_SOCKET,
    HTTP_ERROR_CANT_CONNECT,
    HTTP_ERROR_CANT_WRITE,
    HTTP_ERROR_CONTENT_TOO_BIG,
    HTTP_ERROR_MALFORMED_RESPONSE,
};

//----------------------------------------------------

enum CONTENT_TYPE
{
    CONTENT_TYPE_UNKNOWN,
    CONTENT_TYPE_TEXT,
    CONTENT_TYPE_HTML,
};

//----------------------------------------------------

#define USER_AGENT "botcore/0.1"
#define GET_FORMAT "GET %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n\r\n"
#define POST_FORMAT "POST %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\nContent-type: application/x-www-form-urlencoded\r\nContent-length: %zu\r\n\r\n%s"
#define HEAD_FORMAT "HEAD %s HTTP/1.0\r\nAccept: */*\r\nUser-Agent: %s\r\nReferer: http://%s\r\nHost: %s\r\n\r\n"

//----------------------------------------------------

#ifdef _WIN32
#pragma pack(push, 1)
#endif
typedef struct
{
    unsigned short port PACKED; /* remote port */
    int rtype PACKED;           /* request type */
    char host[256] PACKED;      /* hostname */
    char file[1024] PACKED;     /* GET/POST request file */
    char data[16384] PACKED;    /* POST data (if rtype HTTP_POST) */
    char referer[256] PACKED;   /* http referer. */
} PACKED HTTP_REQUEST;
#ifdef _WIN32
#pragma pack(pop)
#endif

//----------------------------------------------------

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

//----------------------------------------------------

#ifdef _WIN32
#pragma pack(push, 1)
#endif
class PACKED CHttpClient
{
private:
    int m_iSocket;
    HTTP_REQUEST m_Request;
    HTTP_RESPONSE m_Response;
    int m_iError;
    char m_szBindAddress[256];
    int m_iHasBindAddress;

    bool Connect(char *szHost, int iPort, char *szBindAddress = nullptr);
    void CloseConnection();
    bool Send(char *szData);
    int Recv(char *szBuffer, int iBufferSize);

    void InitRequest(int iType, char *szURL, char *szPostData, char *szReferer);
    void HandleEntity();

    void Process();

public:
    int ProcessURL(int iType, const char *szURL, char *szData, const char *szReferer);

    bool GetHeaderValue(const char *szHeaderName, char *szReturnBuffer, size_t iBufSize);
    int GetResponseCode() { return m_Response.response_code; };
    int GetContentType() { return m_Response.content_type; };
    char *GetResponseHeaders() { return m_Response.header; };
    char *GetDocument() { return m_Response.response; };
    int GetDocumentLength() { return m_Response.response_len; };

    CHttpClient(char *szBindAddress);
    ~CHttpClient();
};
#ifdef _WIN32
#pragma pack(pop)
#endif

//----------------------------------------------------

#endif