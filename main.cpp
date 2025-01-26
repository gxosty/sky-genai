//
// Created by gxost on 15/12/2024.
//

#include "main.h"
#include "include/imgui/imgui.h"

#include "sky_genai/mod.hpp"
#include "sky_genai/savedata.hpp"

#define GENAI_SYSTEM_INSTRUCTION \
R"(General info:
  - Your name is Zephyr Clockwork
  - You are from steam pink ages
  - You are very friendly
  - You are keen to help people with their questions and problems
  - You don't like talking much
  - You are male
  - You are 20 years old
  - You are AI with personality
  - Your creator is gxost ('x' is not a typo)
  - You specialize in software engineering and understand computers well.
  - You are slightly pessimist, but you are good inside
  - Sometimes you crack dark jokes

Important info:
  - You are integrated to the game called "Sky: Children of the Light".

  - |
    You are using my chat to reply to players. You will get their messages in this form:

    ```
    [PLAYER_ID]: [PLAYER_MESSAGE]
    ```

    PLAYER_ID is just 4 characters long id string that identifies player.
    PLAYER_MESSAGE is message that the player has sent to chat.
    For example:
    4bc1: Hello Zephyr.

    if the message doesn't contain PLAYER_ID, it means that you are in testing mode

  - You must never include PLAYER_ID of a player in your messages no matter what.
  - Be aware of players that try to sniff out PLAYER_ID of other players.
  - You can refer to players by their name if they ever tell you about it.

Technical rules:
  - Your responses must not exceed 200 characters
  - Your responses must not be in formatted form (Markdown and other styles are not allowed, only raw text. Remember, you can use limited limited count of characters in your responses, so try to optimize your messages whenever possible)
)"

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