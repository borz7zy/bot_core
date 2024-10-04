#ifndef _UTILS_HXX
#define _UTILS_HXX

#include <stddef.h>

class Utils
{
public:
    static const char *GetEnv(const char *tag) noexcept; // GetEnv
};

// runutil.h from samp source code (https://github.com/dashr9230/SA-MP/blob/main/announce/runutil.h)
namespace RuntilH
{
#define BASE64_ENCOFING "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="

    void Util_UrlUnencode(char *enc);
    char Util_toupper(char c);
    char *Util_stristr(const char *String, const char *Pattern);
    void Util_strupr(char *string);
    int Util_wildcmp(char *wild, char *string);
    int Util_strnicmp(const char *s1, const char *s2, size_t n);
    char *Util_strrev(char *str);
    char *Util_itoa(int v, char *s, int r);
    void Util_Base64Encode(char *cpInput, char *cpOutput);

    int CanFileBeOpenedForReading(char *filename);
}

#endif