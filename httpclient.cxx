#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
// #include "runutil.h"
#include "utils.hxx"

//----------------------------------------------------

CHttpClient::CHttpClient(char *szBindAddress)
{
    memset(&m_Request, 0, sizeof(HTTP_REQUEST));
    memset(&m_Response, 0, sizeof(HTTP_RESPONSE));

    m_iHasBindAddress = 0;
    memset(m_szBindAddress, 0, sizeof(m_szBindAddress));
    if (szBindAddress)
    {
        m_iHasBindAddress = 1;
        strcpy(m_szBindAddress, szBindAddress);
    }

    m_iError = HTTP_SUCCESS; // Request is successful until otherwise indicated
    m_iSocket = (-1);

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
// Winsock cleanup
#ifdef WIN32
    WSACleanup();
#endif
}

//----------------------------------------------------

int CHttpClient::ProcessURL(int iType, const char *szURL, char *szPostData, const char *szReferer)
{
    InitRequest(iType, szURL, szPostData, szReferer);

    Process();

    return m_iError;
}

//----------------------------------------------------

bool CHttpClient::GetHeaderValue(const char *szHeaderName, char *szReturnBuffer, size_t iBufSize)
{
    char *szHeaderStart;
    char *szHeaderEnd;
    int iLengthSearchHeader = strlen(szHeaderName);
    int iCopyLength;

    szHeaderStart = RuntilH::Util_stristr(m_Response.header, szHeaderName);
    if (!szHeaderStart)
    {
        return false;
    }
    szHeaderStart += iLengthSearchHeader + 1;

    szHeaderEnd = strchr(szHeaderStart, '\n');
    if (!szHeaderEnd)
    {
        szHeaderEnd = m_Response.header + strlen(m_Response.header); // (END OF STRING)
    }

    iCopyLength = szHeaderEnd - szHeaderStart;
    if (iBufSize < iCopyLength)
    {
        return false;
    }

    memcpy(szReturnBuffer, szHeaderStart, iCopyLength);
    szReturnBuffer[iCopyLength] = '\0';
    return true;
}

//----------------------------------------------------

bool CHttpClient::Connect(char *szHost, int iPort, char *szBindAddress)
{
    struct sockaddr_in sa, bind_sa;
    struct hostent *hp, *bind_hp;

    // Hostname translation
    if ((hp = (struct hostent *)gethostbyname(szHost)) == NULL)
    {
        m_iError = HTTP_ERROR_BAD_HOST;
        return false;
    }

    // Prepare a socket
    memset(&sa, 0, sizeof(sa));
    memset(&bind_sa, 0, sizeof(bind_sa));
    memcpy(&sa.sin_addr, hp->h_addr, hp->h_length);
    sa.sin_family = hp->h_addrtype;
    sa.sin_port = htons((unsigned short)iPort);

    if (szBindAddress)
    {
        if ((bind_hp = (struct hostent *)gethostbyname(szBindAddress)) == NULL)
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

    if (szBindAddress && bind(m_iSocket, (struct sockaddr *)&bind_sa, sizeof bind_sa) < 0)
    {
        m_iError = HTTP_ERROR_CANT_CONNECT;
        return false;
    }

    // Try to connect
    if (connect(m_iSocket, (struct sockaddr *)&sa, sizeof sa) < 0)
    {
        CloseConnection();
        m_iError = HTTP_ERROR_CANT_CONNECT;
        return false;
    }

    return true;
}

//----------------------------------------------------

void CHttpClient::CloseConnection()
{
#ifdef WIN32
    closesocket(m_iSocket);
#else
    close(m_iSocket);
#endif
}

//----------------------------------------------------

bool CHttpClient::Send(char *szData)
{
    if (send(m_iSocket, szData, strlen(szData), 0) < 0)
    {
        m_iError = HTTP_ERROR_CANT_WRITE;
        return false;
    }
    return true;
}

//----------------------------------------------------

int CHttpClient::Recv(char *szBuffer, int iBufferSize)
{
    return recv(m_iSocket, szBuffer, iBufferSize, 0);
}

//----------------------------------------------------

void CHttpClient::InitRequest(int iType, const char *szURL, const char *szPostData, const char *szReferer)
{
    char port[60];          // port string
    char *port_char;        // position of ':' if any
    unsigned int slash_pos; // position of first '/' numeric
    char *slash_ptr;
    char szUseURL[512]; // incase we have to cat something to it.

    memset((void *)&m_Request, 0, sizeof(HTTP_REQUEST));

    // Set the request type
    m_Request.rtype = iType;

    // Copy the URL to use
    strcpy(szUseURL, szURL);

    // Copy the referer
    if (szReferer)
    {
        strcpy(m_Request.referer, szReferer);
    }

    if (iType == HTTP_POST)
    {
        strcpy(m_Request.data, szPostData);
    }

    // Copy hostname from URL
    slash_ptr = strchr(szUseURL, '/');

    if (!slash_ptr)
    {
        strcat(szUseURL, "/");
        slash_ptr = strchr(szUseURL, '/');
    }

    slash_pos = (slash_ptr - szUseURL);
    memcpy(m_Request.host, szUseURL, slash_pos);
    m_Request.host[slash_pos] = '\0';

    // Copy the rest of the url to the file string.
    strcpy(m_Request.file, strchr(szUseURL, '/'));

    // Any special port used in the URL?
    if ((port_char = strchr(m_Request.host, ':')) != NULL)
    {
        strcpy(port, port_char + 1);
        *port_char = '\0';
        m_Request.port = atoi(port);
    }
    else
    {
        m_Request.port = 80;
    }
}

//----------------------------------------------------

void CHttpClient::Process()
{
    int header_len;
    char request_head[16384];

    if (m_iHasBindAddress)
    {
        if (!Connect(m_Request.host, m_Request.port, m_szBindAddress))
        {
            return;
        }
    }
    else
    {
        if (!Connect(m_Request.host, m_Request.port))
        {
            return;
        }
    }

    // Build the HTTP Header
    switch (m_Request.rtype)
    {
    case HTTP_GET:
        header_len = strlen(m_Request.file) + strlen(m_Request.host) +
                     (strlen(GET_FORMAT) - 8) + strlen(USER_AGENT) +
                     strlen(m_Request.referer);
        snprintf(request_head, sizeof(request_head), GET_FORMAT,
                 m_Request.file, USER_AGENT, m_Request.referer, m_Request.host);
        break;

    case HTTP_HEAD:
        header_len = strlen(m_Request.file) + strlen(m_Request.host) +
                     (strlen(HEAD_FORMAT) - 8) + strlen(USER_AGENT) +
                     strlen(m_Request.referer);
        snprintf(request_head, sizeof(request_head), HEAD_FORMAT,
                 m_Request.file, USER_AGENT, m_Request.referer, m_Request.host);
        break;

    case HTTP_POST:
        header_len = strlen(m_Request.data) + strlen(m_Request.file) +
                     strlen(m_Request.host) + strlen(POST_FORMAT) +
                     strlen(USER_AGENT) + strlen(m_Request.referer);
        snprintf(request_head, sizeof(request_head), POST_FORMAT,
                 m_Request.file, USER_AGENT, m_Request.referer, m_Request.host,
                 strlen(m_Request.data), m_Request.data);
        break;
    }

    // OutputDebugString(request_head);

    if (!Send(request_head))
        return;

    HandleEntity();
}

//----------------------------------------------------

#define RECV_BUFFER_SIZE 2048

void CHttpClient::HandleEntity()
{
    int bytes_total = 0;
    int bytes_read = 0;
    char buffer[RECV_BUFFER_SIZE];
    char response[MAX_ENTITY_LENGTH];

    char header[1024];
    char *head_end;
    char *pcontent_buf;
    char content_len_str[256] = {0};

    bool header_got = false;
    bool has_content_len = false;
    int header_len = 0;
    int content_len = 0;

    while ((bytes_read = Recv(buffer, RECV_BUFFER_SIZE)) > 0)
    {
        bytes_total += bytes_read;
        memcpy(response + (bytes_total - bytes_read), buffer, (unsigned int)bytes_read);

        if (!header_got)
        {
            if ((head_end = strstr(response, "\r\n\r\n")) != NULL ||
                (head_end = strstr(response, "\n\n")) != NULL)
            {
                header_got = true;

                header_len = (head_end - response);
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

                if ((pcontent_buf = RuntilH::Util_stristr(header, "CONTENT-LENGTH:")) != NULL)
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

    unsigned long *magic = (unsigned long *)header;
    if (*magic != 0x50545448)
    {
        m_iError = HTTP_ERROR_MALFORMED_RESPONSE;
        return;
    }

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
    if (GetHeaderValue("CONTENT-TYPE:", szContentType, 256))
    {
        if (strstr(szContentType, "text/html") != NULL)
        {
            m_Response.content_type = CONTENT_TYPE_HTML;
        }
        else if (strstr(szContentType, "text/plain") != NULL)
        {
            m_Response.content_type = CONTENT_TYPE_TEXT;
        }
        else
        {
            m_Response.content_type = CONTENT_TYPE_UNKNOWN;
        }
    }
}

//----------------------------------------------------