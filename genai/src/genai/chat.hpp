#pragma once

#include "common.hpp"
#include <nlohmann/json.hpp>

#include <deque>
#include <memory>
#include <string>

namespace genai
{

class GenAI;

class ContentItem;

class Chat
{
public:
    Chat() = delete;
    Chat(std::weak_ptr<GenAI> g);

    std::string send_message(const std::string& message);

private:
    std::weak_ptr<GenAI> _g;
    std::deque<ContentItem> _chat_content;

    void _push_into_chat(const std::string& text, ContentItem::Role role);
    nlohmann::json _get_chat_contents() const;
};

}