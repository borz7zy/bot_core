#include "main.hxx"

void tick_update()
{
}

int main()
{
    if (config->load("./bot_core.conf"))
    {
        std::string masPath = config->get("main_script_path");
        std::string misPath = config->get("minor_scripts_path");
        std::string plPath = config->get("plugins_path");

        std::string minorScripts = config->get("minor_scripts");
        std::string mainScript = config->get("main_script");
        std::string plugins = config->get("plugins");
        int ticks = std::stoi(config->get("ticks"));

        std::vector<std::string> fullPlugins;
        processScripts(plugins, plPath, fullPlugins);

        std::vector<std::string> fullMinorScripts;
        processScripts(minorScripts, misPath, fullMinorScripts);

        std::string mainScriptFullPath = masPath + "/" + mainScript;

        Core c(fullMinorScripts, mainScriptFullPath);
        Core *core = &c;

        TickSys tickSys(20, tick_update); // 20 тиков в секунду
        tickSys.start();                  // Бесконечный запуск системы тиков
    }

    return 0;
}