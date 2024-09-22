#ifndef _CONFIG_MANAGER_HXX
#define _CONFIG_MANAGER_HXX

#include <string>
#include <unordered_map>
#include <logprint.hxx>

class ConfigManager
{
public:
    ConfigManager() : logp("CONFIG MANAGER", "./config_manager.log")
    {
    }
    ~ConfigManager()
    {
    }
    bool load(const char *filename);
    std::string get(const char *key) const;

private:
    std::unordered_map<std::string, std::string> config;
    logprint logp;
    logprint *logger = &logp;
    void trim(std::string &str) const
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
};

#endif