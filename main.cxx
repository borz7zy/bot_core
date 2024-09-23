#include "main.hxx"

int main()
{
    ConfigManager cManager;
    ConfigManager *config = &cManager;

    if (config->load("./bot_core.conf"))
    {
        std::string masPath = config->get("main_script_path");
        std::string misPath = config->get("minor_scripts_path");
        std::string plPath = config->get("plugins_path");

        std::string minorScripts = config->get("minor_scripts");
        std::string mainScript = config->get("main_script");
        std::string plugins = config->get("plugins");

        std::vector<std::string> fullPlugins;
        processScripts(plugins, plPath, fullPlugins);

        std::vector<std::string> fullMinorScripts;
        processScripts(minorScripts, misPath, fullMinorScripts);

        std::string mainScriptFullPath = masPath + "/" + mainScript;

        Core c(fullMinorScripts, mainScriptFullPath);
        // Core *core = &c;
    }

    return 0;
}