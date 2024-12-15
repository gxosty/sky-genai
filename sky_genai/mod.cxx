#include "mod.hpp"
#include "utils.hpp"
#include "misc/Logger.h"
#include <genai/genai.hpp>

#include <unistd.h>
#include <poll.h>
#include <errno.h>

#include <memory>
#include <thread>

#include <Cipher/Cipher.h>
#define SETUP_HOOK(_func, func_addr) \
    (new CipherHook()) \
    ->set_Hook((uintptr_t)my##_func) \
    ->set_Callback((uintptr_t)&orig##_func) \
    ->set_Address(func_addr, false)->Fire();

#define HOOK_DEF(ret, func, ...) unsigned long addr##func; ret (*orig##func)(__VA_ARGS__); ret my##func(__VA_ARGS__)

namespace mod
{

// AI related
std::string _system_instruction;
genai::GenerationConfig _generation_config;

genai::GenAI* _g = nullptr;
std::unique_ptr<genai::Chat> _chat;

// Mod related
std::string _error_message;
int _ai_pipe;
void* _chat_instance;

struct MessageInfo
{
    std::string message;
    std::string uuid;

    MessageInfo(char* _message, uint8_t* _uuid)
    {
        message = std::string(_message);
        char cuuid[37];
        uuid_to_string(_uuid, const_cast<char*>(cuuid));
        uuid = std::string(cuuid, 36);
    }
};

HOOK_DEF(uint64_t, _chat_add_chat_message, uintptr_t _this, char* text, uint8_t* uuid, int8_t a4, int8_t* a5)
{
    if (
        (text[0] == '-') &&
        is_alive()
    ) {
        MessageInfo* info = new MessageInfo(text[1] == ' ' ? text + 2 : text + 1, uuid);
        if (write(_ai_pipe, (char*)&info, sizeof(MessageInfo*)) == -1)
        {
            LOGE("_ai_pipe write error: %d", errno);
            delete info;
        }
    }

    if (_this)
        _chat_instance = (void*)_this;

    return orig_chat_add_chat_message(_this, text, uuid, a4, a5);
}

void _ai_chat_send(const std::string& message)
{
    static const uintptr_t chat_send_ptr = Cipher::get_libBase() + 0xF0537C;
    auto chat_send_message = ((void(*)(void* _this, const char* text, char a3, char a4))chat_send_ptr);

    chat_send_message(_chat_instance, ("Z: " + message).c_str(), 1, 1);
}

void _process_message(MessageInfo* info)
{
    std::string message = (info->uuid.substr(32) + ": " + info->message);
    delete info;
    LOGI("Message from %s", message.c_str());

    message = _chat->send_message(message);

    if (message.back() == '\n')
    {
        message.pop_back();
    }

    _ai_chat_send(message);
}

void _ai_loop()
{
    int ai_pipe[2];

    if (pipe(ai_pipe) == -1)
    {
        LOGE("ai_pipe create error");
        return;
    }

    _ai_pipe = ai_pipe[1];

    pollfd fds[1];
    fds[0].fd = ai_pipe[0];
    fds[0].events = POLLIN;
    fds[0].revents = 0;

    while (true)
    {
        int res = poll(fds, 1, 10000);

        if (res == -1)
        {
            LOGE("poll error: %d", errno);
            break;
        }
        // res == 0 is just poll timeout
        else if (res)
        {
            if (fds[0].revents & POLLIN)
            {
                MessageInfo* info;
                if (read(ai_pipe[0], (char*)&info, sizeof(MessageInfo*)) == -1)
                {
                    LOGE("ai_pipe[0] read error: %d", errno);
                    continue;
                }

                try {
                    _process_message(info);
                }
                catch (const std::exception& err)
                {
                    LOGE("_process_message error: %s", err.what());
                    _error_message = "_process_message error: " + std::string(err.what());
                }
            }
        }
    }
}

void _apply_hooks()
{
    SETUP_HOOK(_chat_add_chat_message, Cipher::get_libBase() + 0xF09A7C);
}

void initialize(
    const std::string& system_instructions,
    const genai::GenerationConfig& generation_config
) {
    _system_instruction = system_instructions;
    _generation_config = generation_config;

    std::thread ai_loop_th(_ai_loop);
    ai_loop_th.detach();

    _apply_hooks();
}

void start_new_session()
{
    if (_g)
    {
        throw std::runtime_error("Session has already been started");
    }

    _g = new genai::GenAI("gemini-1.5-flash-8b", __GENAI_API_KEY__);
    _g->set_system_instruction(_system_instruction);
    _g->set_generation_config(_generation_config);

    _chat = _g->start_chat();
}

void kill_session()
{
    if (!_g)
    {
        throw std::runtime_error("No session is running");
    }

    delete _g;
    _g = nullptr;

    genai::Chat* chat = _chat.get();
    _chat.release();
    delete chat;
}

bool is_alive()
{
    return _g != nullptr;
}

const std::string& get_error_message()
{
    return _error_message;
}

}