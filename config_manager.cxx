#include "config_manager.hxx"
#include <fstream>

bool ConfigManager::load(const char *filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        logger->LOGE("Error opening file: %s", filename);
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
            config[key] = value;
        }
    }

    file.close();
    return true;
}

std::string ConfigManager::get(const char *key) const
{
    auto it = config.find(key);
    if (it != config.end())
    {
        return it->second;
    }

    return "";
}