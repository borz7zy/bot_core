#ifndef _MAIN_HXX
#define _MAIN_HXX

#include "config_manager.hxx"
#include "core.hxx"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

ConfigManager cManager;
ConfigManager *config = &cManager;

void processScripts(const std::string &scripts, const std::string &basePath, std::vector<std::string> &output)
{
    std::istringstream iss(scripts);
    std::string script;
    while (iss >> script)
    {
        output.push_back(basePath + "/" + script);
    }
}

#endif