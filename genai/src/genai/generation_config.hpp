#pragma once

#include <string>

#include <nlohmann/json.hpp>

namespace genai
{

/**
{
  "generationConfig": {
    "temperature": 1,
    "topK": 40,
    "topP": 0.95,
    "maxOutputTokens": 200,
    "responseMimeType": "text/plain"
  }
}
*/

class GenerationConfig
{
public:
    float temperature = 1.0f;
    float top_k = 40.0f;
    float top_p = 0.95f;
    int max_output_tokens = 8192;
    std::string response_mime_type = "text/plain";
    std::string response_schema; // valid if response_mime_type == "application/json"

    inline nlohmann::json to_json() const
    {
        return nlohmann::json{
            {"temperature", temperature},
            {"topK", top_k},
            {"topP", top_p},
            {"maxOutputTokens", max_output_tokens},
            {"responseMimeType", response_mime_type}

            // not supported yet
            // {"responseSchema", response_schema}
        };
    }
};

}