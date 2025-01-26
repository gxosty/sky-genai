#pragma once

#include "common.hpp"
#include <nlohmann/json.hpp>

#include <deque>
#include <string>

namespace genai
{

class GenAI;

class ContentItem;

class Chat
{
public:
    Chat() = delete;
    Chat(GenAI* g);

    std::string send_message(const std::string& message);

private:
    GenAI* _g;
    std::deque<ContentItem> _chat_content;

    void _push_into_chat(const std::string& text, ContentItem::Role role);
    nlohmann::json _get_chat_contents() const;

    void _throw_if_error(const nlohmann::json& response);
};

}