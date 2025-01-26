#pragma once

#include "common.hpp"
#include "exceptions.hpp"
#include "chat.hpp"
#include "generation_config.hpp"

#include <curl/curl.h>

#include <string>

namespace genai
{

class GenAI
{
    friend class Chat;

public:
    GenAI(
        const std::string& model_name,
        const std::string& api_key
    );
    ~GenAI();

    std::string generate_content(const std::string& text);
    Chat* start_chat();

    void set_system_instruction(const std::string& instruction);
    const std::string& get_system_instruction() const;

    void set_generation_config(const GenerationConfig& config);
    const GenerationConfig& get_generation_config() const;

private:
    std::string _get_url() const;

    int _post(
        const std::string& url,
        const std::string& data,
        std::string& out_text);

private:
    std::string _model_name;
    std::string _api_key;

private:
    CURL* _curl;
    curl_slist* _headers;

    std::string _system_instruction;
    GenerationConfig _generation_config;
};

}