#include "config_manager.hxx"
#include <fstream>
#include <sys/stat.h>
#include <filesystem>
#include "globals.hxx"
namespace fs = std::filesystem;

ConfigManager::ConfigManager()
{
}
ConfigManager::~ConfigManager()
{
}

bool ConfigManager::load(const char *filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        logp->printlf("Error opening file: %s", filename);
        return false;
    }

    std::string line;
    while (std::getline(file, line))
    {
        if (line.empty())
        {
            continue;
        }

        if (line.empty() || line[0] == '#')
        {
            continue;
        }

        auto commentPos = line.find('#');
        if (commentPos != std::string::npos)
        {
            line.erase(commentPos);
        }

        auto pos = line.find('=');
        if (pos != std::string::npos)
        {
            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);
            trim(key);
            trim(value);
            configMap[key] = value;
        }
    }

    file.close();
    return true;
}

std::string ConfigManager::get(const char *key) const
{
    auto it = configMap.find(key);
    if (it != configMap.end())
    {
        return it->second;
    }

    return "";
}

void ConfigManager::trim(std::string &str) const
{
    size_t first = str.find_first_not_of(' ');
    size_t last = str.find_last_not_of(' ');
    if (first != std::string::npos && last != std::string::npos)
    {
        str = str.substr(first, (last - first + 1));
    }
    else
    {
        str.clear();
    }
}

bool ConfigManager::directoryExists(const std::string &path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

void ConfigManager::createDirectoryIfNotExists(const std::string &path)
{
    if (!directoryExists(path))
    {
        fs::create_directory(path);
    }
}

bool ConfigManager::generateConfig()
{
    const char *filename = "bot_core.conf";

    std::ofstream configFile(filename);

    if (!configFile)
    {
        logp->printlf("Error: failed to create a file %s", filename);
        return false;
    }

    configFile << "# Example config for bot core\n";
    configFile << "# Attention! the core only works with pre-compiled lua scripts ( luac )\n";
    configFile << "#\n";
    configFile << "main_script_path=./botscripts # the path to the main script\n";
    configFile << "minor_scripts_path=./minorscripts # the path to secondary scripts\n";
    configFile << "plugins_path=./plugins # the path to the plugins\n";
    configFile << "minor_scripts=test_minor.luac # all secondary scripts are indicated here, separated by a space, indicating the extension\n";
    configFile << "main_script=script.luac # a single script is specified here, also with the extension\n";
    configFile << "ticks=20 # the number of ticks per second. to poll the plugin and scripts\n";
    configFile << "#\n";
    configFile << "# you can also specify your fields here, if your script needs it.\n";

    configFile.close();

    createDirectoryIfNotExists("./botscripts");
    createDirectoryIfNotExists("./minorscripts");
    createDirectoryIfNotExists("./plugins");
    createDirectoryIfNotExists("./modules");

    return true;
}