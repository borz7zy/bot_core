#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cassert>
#include <memory>
#include <sstream>
#include <fstream>
#include <iomanip>

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#endif

#include "httpclient.hxx"
#include "utils.hxx"

//----------------------------------------------------

CHttpClient::CHttpClient(const char *szBindAddress)
{
    memset(&m_Request, 0, sizeof(HTTP_REQUEST));
    memset(&m_Response, 0, sizeof(HTTP_RESPONSE));

    m_iHasBindAddress = 0;
    memset(m_szBindAddress, 0, sizeof(m_szBindAddress));
    if (szBindAddress)
    {
        m_iHasBindAddress = 1;
        strncpy(m_szBindAddress, szBindAddress, sizeof(m_szBindAddress) - 1);
        m_szBindAddress[sizeof(m_szBindAddress) - 1] = '\0';
    }

    m_iError = HTTP_SUCCESS; // Request is successful until otherwise indicated
    m_iSocket = (-1);

    // WolfSSL init
    wolfSSL_Init();
    method = wolfTLSv1_2_client_method();
    if ((m_ssl_ctx = wolfSSL_CTX_new(method)) == NULL)
    {
        printf("Failed to wolfSSL_CTX_new\n");
    }
    assert(m_ssl_ctx != nullptr);

    if (wolfSSL_CTX_set_default_verify_paths(m_ssl_ctx) != SSL_SUCCESS)
    {
        fprintf(stderr, "wolfSSL_CTX_set_default_verify_paths error\n");
        wolfSSL_CTX_free(m_ssl_ctx);
        return;
    }

#ifdef DEBUG
    wolfSSL_CTX_set_verify(m_ssl_ctx, SSL_VERIFY_NONE, NULL);
#endif

    // Winsock init
#ifdef WIN32
    WORD wVersionRequested;
    WSADATA wsaData;
    wVersionRequested = MAKEWORD(2, 2);
    WSAStartup(wVersionRequested, &wsaData);
#endif
}

//----------------------------------------------------

CHttpClient::~CHttpClient()
{

    // WolfSSL cleanup
    if (m_ssl_ctx)
    {
        wolfSSL_CTX_free(m_ssl_ctx);
    }

    // Winsock cleanup
#ifdef WIN32
    WSACleanup();
#endif
}

//----------------------------------------------------

int CHttpClient::ProcessURL(int iType, const char *szURL, const char *szPostData, const char *szReferer)
{
    if (iType != HTTP_GET && iType != HTTP_HEAD && szPostData)
    {
        PostParameter param;
        param.value = szPostData ? szPostData : "";
        return ProcessURL(iType, szURL, {param}, szReferer);
    }
    if (strncmp(szURL, "http://", 7) == 0)
    {
        tls_protocol = false;
        szURL += 7;
    }
    else if (strncmp(szURL, "https://", 8) == 0)
    {
        tls_protocol = true;
        szURL += 8;
    }
    else
    {
        m_iError = HTTP_ERROR_BAD_URL;
        return m_iError;
    }

    InitRequest(iType, szURL, szPostData, szReferer);
    Process();
    return m_iError;
}
int CHttpClient::ProcessURL(int iType, const char *szURL, const std::vector<PostParameter> &params, const char *szReferer)
{

    if (strncmp(szURL, "http://", 7) == 0)
    {
        tls_protocol = false;
        szURL += 7;
    }
    else if (strncmp(szURL, "https://", 8) == 0)
    {
        tls_protocol = true;
        szURL += 8;
    }
    else
    {
        m_iError = HTTP_ERROR_BAD_URL;
        return m_iError;
    }

    if (iType == HTTP_POST || iType == HTTP_PUT || iType == HTTP_PATCH)
    {
        return sendDataRequest(iType, params, szURL, szReferer);
    }

    InitRequest(iType, szURL, nullptr, szReferer);
    Process();
    return m_iError;
}

//----------------------------------------------------

bool CHttpClient::GetHeaderValue(const char *szHeaderName, char *szReturnBuffer, size_t iBufSize)
{
    const char *szHeaderStart = RuntilH::Util_stristr(m_Response.header, szHeaderName);
    if (!szHeaderStart)
    {
        return false;
    }

    szHeaderStart += strlen(szHeaderName) + 1;

    const char *szHeaderEnd = strchr(szHeaderStart, '\n');
    if (!szHeaderEnd)
    {
        szHeaderEnd = m_Response.header + strlen(m_Response.header); // (END OF STRING)
    }

    size_t iCopyLength = szHeaderEnd - szHeaderStart;
    if (iBufSize <= iCopyLength)
    {
        return false;
    }

    memcpy(szReturnBuffer, szHeaderStart, iCopyLength);
    szReturnBuffer[iCopyLength] = '\0';
    return true;
}

//----------------------------------------------------

bool CHttpClient::Connect(const char *szHost, int iPort, const char *szBindAddress)
{
    sockaddr_in sa{}, bind_sa{};
    hostent *hp = gethostbyname(szHost);
    if (!hp)
    {
        m_iError = HTTP_ERROR_BAD_HOST;
        return false;
    }

    // Prepare a socket
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons(static_cast<unsigned short>(iPort));

    if (szBindAddress)
    {
        hostent *bind_hp = gethostbyname(szBindAddress);
        if (!bind_hp)
        {
            m_iError = HTTP_ERROR_BAD_HOST;
            return false;
        }
        memcpy(&bind_sa.sin_addr, bind_hp->h_addr, bind_hp->h_length);
        bind_sa.sin_family = bind_hp->h_addrtype;
        bind_sa.sin_port = 0;
    }

    if ((m_iSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        m_iError = HTTP_ERROR_NO_SOCKET;
        return false;
    }

    if (szBindAddress && bind(m_iSocket, reinterpret_cast<struct sockaddr *>(&bind_sa), sizeof(bind_sa)) < 0)
    {
        m_iError = HTTP_ERROR_CANT_CONNECT;
        return false;
    }

    // Try to connect
    if (connect(m_iSocket, reinterpret_cast<struct sockaddr *>(&sa), sizeof(sa)) < 0)
    {
        CloseConnection();
        m_iError = HTTP_ERROR_CANT_CONNECT;
        return false;
    }

    if (tls_protocol)
    {
        // SSL-session init
        m_ssl = wolfSSL_new(m_ssl_ctx);
        if (m_ssl == NULL)
        {
            fprintf(stderr, "wolfSSL_new error\n");
            return false;
        }
        assert(m_ssl != nullptr);

        // Binding a socket to an SSL session
        wolfSSL_set_fd(m_ssl, m_iSocket);

        // Establishing a TLS connection
        if (wolfSSL_connect(m_ssl) != SSL_SUCCESS)
        {
            int err = wolfSSL_get_error(m_ssl, 0);
            printf("WolfSSL connect error: %d\n", err);
            m_iError = HTTP_ERROR_SSL;
            wolfSSL_free(m_ssl);
            return false;
        }
    }

    return true;
}

//----------------------------------------------------

void CHttpClient::CloseConnection()
{

    if (m_ssl)
    {
        wolfSSL_shutdown(m_ssl);
        wolfSSL_free(m_ssl);
        m_ssl = nullptr;
    }

#ifdef WIN32
    closesocket(m_iSocket);
#else
    close(m_iSocket);
#endif
}

//----------------------------------------------------

bool CHttpClient::Send(const char *szData)
{
    return Send(szData, strlen(szData));
}

bool CHttpClient::Send(const char *szData, size_t len)
{
    if (tls_protocol)
    {
        if (wolfSSL_write(m_ssl, szData, len) < 0)
        {
            m_iError = HTTP_ERROR_CANT_WRITE;
            return false;
        }
    }
    else
    {
        if (send(m_iSocket, szData, len, 0) < 0)
        {
            m_iError = HTTP_ERROR_CANT_WRITE;
            return false;
        }
    }
    return true;
}

//----------------------------------------------------

int CHttpClient::Recv(char *szBuffer, int iBufferSize)
{
    if (tls_protocol)
    {
        return wolfSSL_read(m_ssl, szBuffer, iBufferSize);
    }
    return recv(m_iSocket, szBuffer, iBufferSize, 0);
}

//----------------------------------------------------

void CHttpClient::InitRequest(int iType, const char *szURL, const char *szPostData, const char *szReferer)
{
    char port[64];          // port string
    const char *port_char;  // position of ':' if any
    unsigned int slash_pos; // position of first '/' numeric
    const char *slash_ptr;  //
    char szUseURL[2048];    // in case we have to cat something to it.

    memset(&m_Request, 0, sizeof(HTTP_REQUEST));

    // Set the request type
    m_Request.rtype = iType;

    // Copy the URL to use
    strncpy(szUseURL, szURL, sizeof(szUseURL) - 1);
    szUseURL[sizeof(szUseURL) - 1] = '\0';

    // Copy the referer
    if (szReferer)
    {
        strncpy(m_Request.referer, szReferer, sizeof(m_Request.referer) - 1);
        m_Request.referer[sizeof(m_Request.referer) - 1] = '\0';
    }
    if (iType != HTTP_GET && iType != HTTP_HEAD && szPostData != nullptr)
    {
        strncpy(m_Request.data, szPostData, sizeof(m_Request.data) - 1);
        m_Request.data[sizeof(m_Request.data) - 1] = '\0';
    }

    // Copy hostname from URL
    slash_ptr = strchr(szUseURL, '/');

    if (!slash_ptr)
    {
        strcat(szUseURL, "/");
        slash_ptr = strchr(szUseURL, '/');
    }

    slash_pos = static_cast<unsigned int>(slash_ptr - szUseURL);
    memcpy(m_Request.host, szUseURL, slash_pos);
    m_Request.host[slash_pos] = '\0';

    // Copy the rest of the URL to the file string.
    strcpy(m_Request.file, strchr(szUseURL, '/'));

    if (tls_protocol)
    {
        m_Request.port = 443;
    }
    else
    {
        m_Request.port = 80;
    }
    // Any special port used in the URL?
    if ((port_char = strchr(m_Request.host, ':')) != nullptr)
    {
        strcpy(port, port_char + 1);
        *const_cast<char *>(port_char) = '\0';
        m_Request.port = atoi(port);
    }

    strcpy(m_Request.file, strchr(szUseURL, '/'));
}

//----------------------------------------------------
// Helper function to URL encode a string
static std::string urlEncode(const std::string &str)
{
    std::ostringstream escaped;
    escaped.fill('0');
    escaped << std::hex;

    for (char c : str)
    {
        if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~')
        {
            escaped << c;
            continue;
        }
        escaped << '%' << std::setw(2) << int((unsigned char)c);
    }
    return escaped.str();
}

std::string CHttpClient::prepareUrlEncodedData(const std::vector<PostParameter> &params)
{
    std::string postData;
    for (size_t i = 0; i < params.size(); ++i)
    {
        postData += urlEncode(params[i].name) + "=" + urlEncode(params[i].value);
        if (i < params.size() - 1)
        {
            postData += "&";
        }
    }
    return postData;
}

std::string CHttpClient::prepareMultipartData(const std::vector<PostParameter> &params, std::string &boundary)
{
    std::string multipartData;
    boundary = "---------------------------" + std::to_string(std::rand()); // Generate a simple boundary

    for (const auto &param : params)
    {
        multipartData += "--" + boundary + "\r\n";

        if (param.isFile)
        {
            multipartData += "Content-Disposition: form-data; name=\"" + param.name + "\"; filename=\"" + param.filename + "\"\r\n";
            multipartData += "Content-Type: " + param.contentType + "\r\n\r\n";

            // Read file content
            std::ifstream file(param.filename, std::ios::binary);
            if (file.is_open())
            {
                std::ostringstream fileContentStream;
                fileContentStream << file.rdbuf();
                multipartData += fileContentStream.str();
            }
            else
            {
                fprintf(stderr, "Error: Could not open file for multipart: %s\n", param.filename.c_str());
            }
            file.close();
            multipartData += "\r\n";
        }
        else
        {
            multipartData += "Content-Disposition: form-data; name=\"" + param.name + "\"\r\n\r\n";
            multipartData += param.value + "\r\n";
        }
    }
    multipartData += "--" + boundary + "--\r\n";
    return multipartData;
}

void CHttpClient::addHeaders(std::string &request_head)
{
    for (const auto &header : m_customHeaders)
    {
        request_head += header.first + ": " + header.second + "\r\n";
    }
    request_head += "\r\n";
}

bool CHttpClient::sendDataRequest(int iType, const std::vector<PostParameter> &params, const char *szURL, const char *szReferer)
{
    std::string postData;
    std::string boundary;
    bool isMultipart = false;
    const char *contentType = nullptr;

    for (const auto &param : params)
    {
        if (param.isFile)
        {
            isMultipart = true;
            break;
        }
    }

    if (isMultipart)
    {
        postData = prepareMultipartData(params, boundary);
        contentType = ("multipart/form-data; boundary=" + boundary).c_str();
    }
    else
    {
        postData = prepareUrlEncodedData(params);
        contentType = "application/x-www-form-urlencoded";
    }

    return sendDataRequest(iType, postData, szURL, szReferer, contentType);
}

bool CHttpClient::sendDataRequest(int iType, const std::string &postData, const char *szURL, const char *szReferer, const char *contentType)
{
    int header_len;
    char request_head[16384];
    const char *methodStr;

    InitRequest(iType, szURL, postData.c_str(), szReferer);

    if (!Connect(m_Request.host, m_Request.port, m_iHasBindAddress ? m_szBindAddress : nullptr))
    {
        return false;
    }

    switch (iType)
    {
    case HTTP_POST:
        methodStr = "POST";
        break;
    case HTTP_PUT:
        methodStr = "PUT";
        break;
    case HTTP_PATCH:
        methodStr = "PATCH";
        break;
    default:
        methodStr = "POST";
    }
    header_len = snprintf(request_head, sizeof(request_head), POST_FORMAT,
                          methodStr, m_Request.file, USER_AGENT, m_Request.referer, m_Request.host,
                          contentType ? contentType : "application/x-www-form-urlencoded", postData.length());

    std::string headersStr = request_head;
    addHeaders(headersStr);

    if (!Send(headersStr.c_str()))
        return false;
    if (!Send(postData.c_str(), postData.length()))
        return false;

    HandleEntity();

    return true;
}
//----------------------------------------------------

void CHttpClient::Process()
{
    int header_len;
    char request_head[16384];
    const char *methodStr;
    if (!Connect(m_Request.host, m_Request.port, m_iHasBindAddress ? m_szBindAddress : nullptr))
    {
        return;
    }
    // Build the HTTP Header
    switch (m_Request.rtype)
    {
    case HTTP_GET:
        methodStr = "GET";
        break;
    case HTTP_HEAD:
        methodStr = "HEAD";
        break;
    case HTTP_DELETE:
        methodStr = "DELETE";
        break;
    case HTTP_OPTIONS:
        methodStr = "OPTIONS";
        break;
    case HTTP_TRACE:
        methodStr = "TRACE";
        break;
    default:
        return;
    }
    header_len = snprintf(request_head, sizeof(request_head), GET_FORMAT,
                          methodStr, m_Request.file, USER_AGENT, m_Request.referer, m_Request.host);

    std::string headersStr = request_head;
    addHeaders(headersStr);

    if (!Send(headersStr.c_str()))
        return;

    HandleEntity();
}

//----------------------------------------------------

#define RECV_BUFFER_SIZE 8192

void CHttpClient::HandleEntity()
{
    int bytes_total = 0;
    int bytes_read = 0;
    char buffer[RECV_BUFFER_SIZE];
    char response[MAX_ENTITY_LENGTH];

    char header[8192];
    const char *head_end;
    char *pcontent_buf;
    char content_len_str[256] = {0};

    bool header_got = false;
    bool has_content_len = false;
    int header_len = 0;
    int content_len = 0;

    while ((bytes_read = Recv(buffer, RECV_BUFFER_SIZE)) > 0)
    {
        bytes_total += bytes_read;
        memcpy(response + (bytes_total - bytes_read), buffer, static_cast<unsigned int>(bytes_read));

        if (!header_got)
        {
            if ((head_end = strstr(response, "\r\n\r\n")) != nullptr ||
                (head_end = strstr(response, "\n\n")) != nullptr)
            {
                header_got = true;

                header_len = static_cast<int>(head_end - response);
                memcpy(header, response, header_len);
                header[header_len] = '\0';

                if (*(response + header_len) == '\n') // LFLF
                {
                    bytes_total -= (header_len + 2);
                    memmove(response, (response + (header_len + 2)), bytes_total);
                }
                else // assume CRLFCRLF
                {
                    bytes_total -= (header_len + 4);
                    memmove(response, (response + (header_len + 4)), bytes_total);
                }

                if ((pcontent_buf = RuntilH::Util_stristr(header, "CONTENT-LENGTH:")) != nullptr)
                {
                    has_content_len = true;

                    pcontent_buf += 16;
                    while (*pcontent_buf != '\n' && *pcontent_buf && *pcontent_buf != '\r')
                    {
                        content_len_str[content_len++] = *pcontent_buf++;
                    }
                    content_len_str[content_len] = '\0';

                    content_len = atoi(content_len_str);
                    if (content_len > MAX_ENTITY_LENGTH)
                    {
                        CloseConnection();
                        m_iError = HTTP_ERROR_CONTENT_TOO_BIG;
                        return;
                    }
                }
            }
        }

        if (header_got && has_content_len && bytes_total >= content_len)
            break;
    }

    CloseConnection();

    response[bytes_total] = '\0';

    char response_code_str[4];
    memcpy(response_code_str, header + 9, 3);
    response_code_str[3] = '\0';
    m_Response.response_code = atoi(response_code_str);

    memcpy(m_Response.header, header, header_len + 1);
    m_Response.header_len = header_len;
    memcpy(m_Response.response, response, bytes_total + 1);
    m_Response.response_len = bytes_total;

    m_Response.content_type = CONTENT_TYPE_HTML;

    char szContentType[256];
    if (GetHeaderValue("CONTENT-TYPE:", szContentType, sizeof(szContentType)))
    {
        if (strstr(szContentType, "text/html") != nullptr)
        {
            m_Response.content_type = CONTENT_TYPE_HTML;
        }
        else if (strstr(szContentType, "text/plain") != nullptr)
        {
            m_Response.content_type = CONTENT_TYPE_TEXT;
        }
        else if (strstr(szContentType, "application/json") != nullptr)
        {
            m_Response.content_type = CONTENT_TYPE_JSON;
        }
        else if (strstr(szContentType, "application/xml") != nullptr)
        {
            m_Response.content_type = CONTENT_TYPE_XML;
        }
        else if (strstr(szContentType, "application/javascript") != nullptr)
        {
            m_Response.content_type = CONTENT_TYPE_JAVASCRIPT;
        }
        else if (strstr(szContentType, "image/jpeg") != nullptr)
        {
            m_Response.content_type = CONTENT_TYPE_IMAGE_JPEG;
        }
        else if (strstr(szContentType, "image/png") != nullptr)
        {
            m_Response.content_type = CONTENT_TYPE_IMAGE_PNG;
        }
        else if (strstr(szContentType, "text/css") != nullptr)
        {
            m_Response.content_type = CONTENT_TYPE_CSS;
        }
        else
        {
            m_Response.content_type = CONTENT_TYPE_UNKNOWN;
        }
    }
}

//----------------------------------------------------