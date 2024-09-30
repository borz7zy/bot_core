#ifndef _LOGGER_HXX
#define _LOGGER_HXX

#include <cstdio>
#include <cstdarg>
#include <ctime>
#ifdef _WIN32
#include <windows.h>
#endif

class logger
{
public:
    logger(const char *log_file)
    {
        if (pLogFile)
        {
            fclose(pLogFile);
        }
        pLogFile = fopen(log_file, "a");
        if (pLogFile)
        {
            printf("\n");
            printf("----------\n");
            printf("Loaded log file: \"bot_core.log\".\n");
            printf("----------\n");
        }
        else
        {
            printf("Failed to load log file: \"bot_core.log\".\n");
        }
    }
    ~logger()
    {
        fclose(pLogFile);
    }
    void printlf(const char *format, ...);

private:
    FILE *pLogFile;
};

#endif