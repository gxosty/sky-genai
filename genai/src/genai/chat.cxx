#include "chat.hpp"
#include "genai.hpp"
#include "exceptions.hpp"

#include <cstdio>

#define MAX_CHAT_CONTENT_COUNT 512

namespace genai
{

Chat::Chat(std::weak_ptr<GenAI> g) : _g(g) {}

std::string Chat::send_message(const std::string& message)
{
    _push_into_chat(message, ContentItem::Role::User);

    if (auto g = _g.lock())
    {
        std::string response_message;

        nlohmann::json data = {
            {"contents", _get_chat_contents()},
            {"systemInstruction", {
                {"role", "user"},
                {"parts", {
                    {
                        {"text", g->_system_instruction}
                    }
                }}
            }},
            {"generationConfig", g->_generation_config.to_json()}
        };

        // printf("-> %s\n", data.dump().c_str());

        g->_post(g->_get_url(), data.dump(), response_message);

        // printf("<- %s\n", response_message.c_str());

        auto json_data = nlohmann::json::parse(response_message);
        _throw_if_error(json_data);
        std::string response_text = json_data["candidates"][0]["content"]["parts"][0]["text"].get<std::string>();

        _push_into_chat(response_text, ContentItem::Role::Model);

        return response_text;
    }

    return "";
}

void Chat::_push_into_chat(const std::string& text, ContentItem::Role role)
{
    if (_chat_content.size() == MAX_CHAT_CONTENT_COUNT)
    {
        _chat_content.pop_front();
    }

    _chat_content.push_back(ContentItem{role, {text}});
}

nlohmann::json Chat::_get_chat_contents() const
{
    nlohmann::json contents_json;

    for (auto& content_item : _chat_content)
    {
        contents_json.push_back({
            {"role", content_item.role == ContentItem::Role::User ? "user" : "model"},
            {"parts", {
                {
                    {"text", content_item.parts[0]}
                }
            }}
        });
    }

    return contents_json;
}

void Chat::_throw_if_error(const nlohmann::json& response)
{
    if (response.contains("error"))
    {
        if (response["error"]["code"].get<int>() == GENAI_ERROR_CODE_MODEL_UNAVAILABLE)
        {
            throw ModelUnavailableError();
        }

        throw ModelResponseError(
            response["error"]["code"].get<int>(),
            response["error"]["message"].get<std::string>()
        );
    }
}

}