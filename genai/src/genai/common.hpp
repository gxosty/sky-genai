#pragma once

#include <vector>
#include <string>

namespace genai
{

class ContentItem
{
public:
    enum class Role
    {
        User,
        Model
    };

public:
    Role role = Role::User;
    std::vector<std::string> parts;
};

}