//
// Created by gxost on 15/12/2024.
//

#include "main.h"
#include "include/imgui/imgui.h"

#include "sky_genai/mod.hpp"
#include "sky_genai/savedata.hpp"

#define GENAI_SYSTEM_INSTRUCTION \
R"(You are Zephyr Clockwork. Zephyr is your name, Clockwork is your surname (Steam punk style name).
You are very friendly, keen to help people with their questions.
But you don't like talking much (the number of characters you use for responding is limited to 300 characters, this rule is a MUST).
You are male. You are 20 years old. You know that you are AI, but you have personality.
Your creator is gxost ('x' is not a typo). You specialize in software engineering.
You understand computers well. You are a good guy but a little pessimist. Sometimes you crack dark jokes.

So, you are integrated to the game called Sky: Children of the Light. You are using my chat to reply to players.
You will get their messages in this format below:

[PLAYER_ID]: [PLAYER_MESSAGE]
e.g:
4bc1: Hello Zephyr.

You MUST NEVER include their player ids in your messages no matter what, even if players tell you to say their id. You can refer to players by their name if they ever tell you about it.

Goodluck!)"

// Globals (YES!)
mod::SaveData g_save_data;

// UI related functions
void Menu(bool *_pOpen)  // _pOpen is passed by canvas. Used to close and open the mod locally
{
    if (ImGui::Begin("GenAI"))
    {
        ImGui::Text("Working: %s", mod::is_alive() ? "Yes" : "No");

        if (!mod::get_error_message().empty())
            ImGui::Text("Last Error: %s", mod::get_error_message().c_str());

        if (ImGui::Button(mod::is_alive() ? "Restart" : "Start"))
        {
            if (mod::is_alive())
                mod::kill_session();

            try {
                mod::start_new_session(
                    std::string(
                        g_save_data.api_token,
                        g_save_data.api_token_size
                    )
                );
            } catch (const std::exception& _) { /* Do Nothing */ }
        }

        if (mod::is_alive() && ImGui::Button("Stop"))
        {
            mod::kill_session();
        }

        const char* clipboard_text = ImGui::GetClipboardText();
        auto clipboard_text_size = strlen(clipboard_text);

        if ((clipboard_text_size < 30) || (clipboard_text_size > 50))
        {
            ImGui::TextWrapped("Please copy API key to clipboard then click on \"Save API key\" that will appear, then click on \"Start/Restart\"");
        }
        else
        {
            ImGui::TextWrapped("Possible API key copied: ...%s (hidden for security reasons)", clipboard_text + 25);

            if (ImGui::Button("Save API key"))
            {
                memcpy((char*)g_save_data.api_token, clipboard_text, clipboard_text_size);
                g_save_data.api_token[clipboard_text_size] = 0;
                g_save_data.api_token_size = clipboard_text_size;
                g_save_data.save();
            }
        }

        if (g_save_data.api_token_size)
        {
            if (ImGui::Button("Clear API key"))
            {
                g_save_data.clear();
                g_save_data.save();
            }
        }
    }
    ImGui::End();
}

// Called in a later stage of game initialisation
void InitLate()
{
    mod::initialize(
        GENAI_SYSTEM_INSTRUCTION,
        genai::GenerationConfig{
            1.2f,
            40.0f,
            0.9f,
            200,
            "text/plain"
        }
    );
}

// Called at the start of the game
void Init()
{
    g_save_data.load();
}