#include <iostream>
#include <string>
#include <fstream>
#include "./../src/global_sdk.hxx"
#include "./../src/globals.hxx"

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

    if (lua_dump(L, writer, &outputFile, 0) != 0)
    {
        std::cerr << "Error while dumping bytecode!" << std::endl;
    }

    outputFile.close();
    std::cout << "Script " << scriptName << " compiled in " << outputFilePath << std::endl;
}

int main(int argc, char *argv[])
{

    std::string inputFile;
    std::string outputFile;
    bool debug = false;

    for (int i = 1; i < argc; ++i)
    {
        std::string arg = argv[i];

        if (arg.substr(0, 2) == "-i")
        {
            inputFile = arg.substr(2);
        }
        else if (arg.substr(0, 2) == "-o")
        {
            outputFile = arg.substr(2);
        }
        else if (arg == "-d")
        {
            debug = true;
        }
    }

    if (!inputFile.empty() && !outputFile.empty())
    {
        lua_State *L = luaL_newstate();
        if (L == nullptr)
        {
            std::cerr << "Error creating Lua state!" << std::endl;
            return 1;
        }

        luaL_requiref(L, "string", luaopen_string, 1);
        lua_pop(L, 1);

        luaL_requiref(L, "math", luaopen_math, 1);
        lua_pop(L, 1);

        luaL_requiref(L, "table", luaopen_table, 1);
        lua_pop(L, 1);

        if (debug)
        {
            luaL_requiref(L, "debug", luaopen_debug, 1);
            lua_pop(L, 1);
        }

        compileLuaScript(L, inputFile, outputFile);

        lua_close(L);
    }
    else
    {
        std::cerr << "The compiler accepts the following arguments: -i (input file) -o (output file). -d argument (include standard lua debug headers (requires debug version of bot core))!" << std::endl;
        return 1;
    }
    return 0;
}
