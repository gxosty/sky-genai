#pragma once

#include <string>

#define GENAI_ERROR_CODE_MODEL_UNAVAILABLE 503

namespace genai
{

class GenAIError
{
public:
    GenAIError(const std::string& message) : message(message) {}

    virtual std::string what() const noexcept
    {
        return message;
    }

protected:
    std::string message;
};

class ModelResponseError : public GenAIError
{
public:
    ModelResponseError(int code, const std::string message)
        : _code{code}, GenAIError(message) {};

    std::string what() const noexcept override
    {
        return message + " (code: " + std::to_string(_code) + ")";
    }

    int code() const noexcept
    {
        return _code;
    }

private:
    int _code;
};

class ModelUnavailableError : public ModelResponseError
{
public:
    ModelUnavailableError()
        : ModelResponseError(503, "The model is overloaded. Please try again later.") {}
};

}