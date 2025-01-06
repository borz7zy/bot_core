#ifdef WIN32
#pragma once
#endif

#ifndef _CONFIG_MANAGER_HXX
#define _CONFIG_MANAGER_HXX

#include <string>
#include <unordered_map>
#include <string>

class ConfigManager
{
public:
    ConfigManager();
    ~ConfigManager();
    bool load(const char *filename);
    std::string get(const char *key) const;
    bool generateConfig();

private:
    std::unordered_map<std::string, std::string> configMap;
    void trim(std::string &str) const;
    bool directoryExists(const std::string &path);
    void createDirectoryIfNotExists(const std::string &path);
};

#endif