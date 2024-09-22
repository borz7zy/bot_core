#include "main.hxx"
#include <logprint.hxx>
#include <lua.h>

logprint LogsCore("CORE", "./core.logs");
logprint *logger = &LogsCore;

int main()
{
    logger->LOGE("Hello!");
    return 0;
}