//
// Created by gxost on 15/12/2024.
//

#include "main.h"
#include "include/imgui/imgui.h"

#include "sky_genai/mod.hpp"

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

// UI related functions
void Menu(bool *_pOpen)  // _pOpen is passed by canvas. Used to close and open the mod locally
{
    if (ImGui::Begin("GenAI"))
    {
        ImGui::Text("Working: %s", mod::is_alive() ? "Yes" : "No");

        if (!mod::get_error_message().empty())
            ImGui::Text("Last Error: %s", mod::get_error_message().c_str());

        if (ImGui::Button("Restart"))
        {
            mod::kill_session();
            mod::start_new_session();
        }
    }
    ImGui::End();
}

// Called in a later stage of game initialisation
void InitLate()
{

}

// Called at the start of the game
void Init()
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
    mod::start_new_session();
}