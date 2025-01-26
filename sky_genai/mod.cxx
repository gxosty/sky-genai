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
#include <Cipher/CipherUtils.h>

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
genai::Chat* _chat = nullptr;

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
    static const uintptr_t chat_send_ptr = \
    CipherUtils::CipherScanPattern(
        // 0.27.5 | E9 03 17 D1 3F E9 7B 92 08 96 9A 52 F6 03 00 AA 18 00 08 8B
        // 0.27.6 | E9 03 17 D1 3F E9 7B 92 08 96 9A 52 F6 03 00 AA 18 00 08 8B
        "E9 03 17 D1 3F E9 7B 92 08 96 9A 52 F6 03 00 AA 18 00 08 8B",
        Flags::ReadAndExecute
    );

    // 0.27.5
    // static const uintptr_t chat_send_ptr = Cipher::get_libBase() + 0xF0537C;

    if (chat_send_ptr)
    {
        auto chat_send_message = ((void(*)(void* _this, const char* text, char a3, char a4))(chat_send_ptr - 0x1C));
        chat_send_message(_chat_instance, ("Z: " + message).c_str(), 1, 1);
    }
}

void _process_message(MessageInfo* info)
{
    std::string message = (info->uuid.substr(32) + ": " + info->message);
    delete info;
    LOGI("Message from %s", message.c_str());

    message = _chat->send_message(message);

    if (message.empty())
    {
        throw std::runtime_error("response is empty");
    }

    while (message.back() == '\n')
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
                catch (const genai::GenAIError& err)
                {
                    _error_message = "GenAI error: " + err.what();
                    LOGE("GenAI error: %s", err.what().c_str());
                }
                catch (const std::exception& err)
                {
                    _error_message = "_process_message error: " + std::string(err.what());
                    LOGE("_process_message error: %s", err.what());
                }
            }
        }
    }
}

void _apply_hooks()
{

    const uintptr_t chat_add_message_ptr = \
    CipherUtils::CipherScanPattern(
        // 0.27.5 | E9 03 27 D1 3F E9 7B 92 49 28 40 A9 08 A7 00 90 F7 03 04 AA
        // 0.27.6 | E9 03 27 D1 3F E9 7B 92 49 28 40 A9 08 A7 00 90 F7 03 04 AA
        // 0.28.0 | E9 03 27 D1 3F E9 7B 92 49 28 40 A9 E8 A9 00 D0 F7 03 04 AA
        "E9 03 27 D1 3F E9 7B 92 49 28 40 A9 ?? ?? 00 ?? F7 03 04 AA",
        Flags::ReadAndExecute
    );

    if (chat_add_message_ptr)
        SETUP_HOOK(_chat_add_chat_message, chat_add_message_ptr - 0x1C);
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

void start_new_session(const std::string& api_key)
{
    if (_g)
    {
        _error_message = "Session has already been started";
        throw std::runtime_error("Session has already been started");
    }

    if (api_key.empty())
    {
        _error_message = "API Key is empty";
        throw std::runtime_error("API Key is empty");
    }

    _g = new genai::GenAI("gemini-1.5-flash", api_key);
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

    delete _chat;
    _chat = nullptr;

    delete _g;
    _g = nullptr;
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