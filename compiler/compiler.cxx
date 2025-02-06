#include <iostream>
#include <string>
#include <fstream>
#include "../src/global_sdk.hxx"
#include "../src/globals.hxx"

int writer(lua_State *L, const void *p, size_t sz, void *ud)
{
    std::ofstream *file = static_cast<std::ofstream *>(ud);
    file->write(static_cast<const char *>(p), sz);
    return 0;
}

void compileLuaScript(lua_State *L, const std::string &scriptName, const std::string &outputFilePath)
{
    if (luaL_loadfile(L, scriptName.c_str()) != LUA_OK)
    {
        std::cerr << "File upload error: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return;
    }

    std::ofstream outputFile(outputFilePath, std::ios::binary);
    if (!outputFile)
    {
        std::cerr << "Error opening file for writing!" << std::endl;
        return;
    }

    if (lua_dump(L, writer, &outputFile) != 0)
    {
        std::cerr << "Error while dumping bytecode!" << std::endl;
    }

    outputFile.close();
    std::cout << "Script " << scriptName << " compiled to " << outputFilePath << std::endl;
}

int main(int argc, char *argv[])
{
    std::string inputFile;
    std::string outputFile;
    bool debug = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg == "-i" && i + 1 < argc)
        {
            inputFile = argv[++i];
        }
        else if (arg == "-o" && i + 1 < argc)
        {
            outputFile = argv[++i];
        }
        else if (arg == "-d")
        {
            debug = true;
        }
        else
        {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    if (inputFile.empty() || outputFile.empty())
    {
        std::cerr << "Usage: " << argv[0] << " -i <input_file> -o <output_file> [-d]" << std::endl;
        return 1;
    }

    lua_State *L = luaL_newstate();
    if (L == nullptr)
    {
        std::cerr << "Error creating Lua state!" << std::endl;
        return 1;
    }

    luaL_openlibs(L);

    compileLuaScript(L, inputFile, outputFile);

    lua_close(L);

    return 0;
}
