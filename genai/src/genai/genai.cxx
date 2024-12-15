#include <genai/genai.hpp>
#include <nlohmann/json.hpp>

#include <stdexcept>

namespace genai
{

static size_t write_function(char* ptr, size_t sz, size_t nmemb, std::string* out)
{
    out->append(ptr, sz * nmemb);
    return sz * nmemb;
}

GenAI::GenAI(
    const std::string& model_name,
    const std::string& api_key
) : _model_name(model_name), _api_key(api_key)
{
    _curl = curl_easy_init();

    if (!_curl)
    {
        throw std::runtime_error("_curl == 0");
    }

    _headers = nullptr;
    _headers = curl_slist_append(_headers, "Content-Type: application/json");

    curl_easy_setopt(_curl, CURLOPT_HTTPHEADER, _headers);
    curl_easy_setopt(_curl, CURLOPT_WRITEFUNCTION, &write_function);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(_curl, CURLOPT_SSL_VERIFYPEER, 0L);

    _self = std::shared_ptr<GenAI>(this);
}

GenAI::~GenAI()
{
    curl_slist_free_all(_headers);
    curl_easy_cleanup(_curl);
}

std::string GenAI::generate_content(const std::string& text)
{
    std::string url = _get_url();
    std::string output;

    nlohmann::json content = {
        {"contents", {
            {
                {"role", "user"},
                {"parts", {
                    {
                        {"text", text}
                    }
                }}
            }
        }},
        {"systemInstruction", {
            {"role", "user"},
            {"parts", {
                {
                    {"text", _system_instruction}
                }
            }}
        }},
        {"generationConfig", _generation_config.to_json()}
    };

    int status = _post(url, content.dump(), output);

    nlohmann::json json_data = nlohmann::json::parse(output);

    return json_data["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();
}

std::unique_ptr<Chat> GenAI::start_chat()
{
    return std::make_unique<Chat>(_self);
}

void GenAI::set_system_instruction(const std::string& instruction)
{
    _system_instruction = instruction;
}

const std::string& GenAI::get_system_instruction() const
{
    return _system_instruction;
}

void GenAI::set_generation_config(const GenerationConfig& config)
{
    _generation_config = config;
}

const GenerationConfig& GenAI::get_generation_config() const
{
    return _generation_config;
}

std::string GenAI::_get_url() const
{
    return "https://generativelanguage.googleapis.com/v1beta/models/" + _model_name + ":generateContent?key=" + _api_key;
}

int GenAI::_post(
    const std::string& url,
    const std::string& data,
    std::string& output)
{
    curl_easy_setopt(_curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(_curl, CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(_curl, CURLOPT_POST, 1L);
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(_curl, CURLOPT_POSTFIELDSIZE, data.size());

    CURLcode curle_code = CURLE_OK;

    curle_code = curl_easy_perform(_curl);

    if (curle_code != CURLE_OK)
    {
        throw std::runtime_error("CURLcode == " + std::to_string((int)curle_code));
    }

    long status_code = 0;
    curl_easy_getinfo(_curl, CURLINFO_RESPONSE_CODE, &status_code);

    return (int)status_code;
}

}