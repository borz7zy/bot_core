#include "logger.hxx"

void logger::printlf(const char *format, ...)
{
    va_list ap;
    va_start(ap, format);

    char buffer[2048];
    vsnprintf(buffer, sizeof buffer, format, ap);

    va_end(ap);

#ifdef _WIN32
    char output[2048];
    CharToOem(buffer, output);
    puts(output);
    fflush(stdout);
#else
    puts(buffer);
#endif
    if (pLogFile)
    {
        const struct tm *tm;
        time_t now;
        now = time(NULL);
        tm = localtime(&now);
        char s[256];
        strftime(s, 256, "[%Y-%m-%d %H:%M:%S]", tm);
        fprintf(pLogFile, "%s %s\n", s, buffer);

        fflush(pLogFile);
    }
}