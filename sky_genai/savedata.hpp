#pragma once

#include <Cipher/CipherUtils.h>

#include <string>
#include <fstream>
#include <filesystem>
#include <cstdint>
#include <cstdlib>

namespace mod
{

struct SaveData
{
    uint32_t api_token_size;
    char api_token[256];

    inline void clear()
    {
        api_token_size = 0;
        memset(api_token, 0, 256);
    }

    inline void load()
    {
        clear();

        namespace fs = std::filesystem;

        static const fs::path save_file_path = fs::path(CipherUtils::get_ConfigsPath()) / "skygenai.bin";

        std::ifstream save_file(save_file_path);

        if (!save_file.is_open())
            return;

        save_file.read((char*)this, sizeof(SaveData));
        save_file.close();
    }

    inline void save() const
    {
        namespace fs = std::filesystem;

        static const fs::path save_file_path = fs::path(CipherUtils::get_ConfigsPath()) / "skygenai.bin";

        std::ofstream save_file(save_file_path);

        if (!save_file.is_open())
            return;

        save_file.write((char*)this, sizeof(SaveData));
        save_file.close();
    }
};

}