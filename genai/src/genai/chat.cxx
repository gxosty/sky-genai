#include "chat.hpp"
#include "genai.hpp"

// #include <cstdio>

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

        g->_post(g->_get_url(), data.dump(), response_message);

        auto json_data = nlohmann::json::parse(response_message);
        // printf("%s\n", json_data.dump().c_str());
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

}