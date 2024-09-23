#include "core.hxx"

void Core::loadMinor()
{
    MinorScripts minScripts(LogsCore);
    MinorScripts *minor = &minScripts;
    for (const auto &minScript : minorScripts)
    {
        minor->load(L, minScript.c_str());
    }
}

void Core::loadMain()
{
    MainScript mScript(LogsCore);
    MainScript *main = &mScript;

    main->load(L, mainScript.c_str());

    lua_close(L);
}