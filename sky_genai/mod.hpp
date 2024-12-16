#pragma once

#include <string>
#include <genai/generation_config.hpp>

namespace mod
{

void initialize(
    const std::string& system_instruction,
    const genai::GenerationConfig& generation_config = genai::GenerationConfig{}
);

void start_new_session(const std::string& api_key);

void kill_session();

bool is_alive();

const std::string& get_error_message();

}