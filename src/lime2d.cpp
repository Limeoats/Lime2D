/*
 * lime2d.cpp
 * By: Mark Guerra
 * Created on 6/9/2016
 */

/*
    TODO:
        -Determine if l2d::Editor::render() is necessary or if we can put ImGui::Render() in l2d::Editor::update()
         and just call l2d::Editor::update() from between window.clear() and window.display(). This would get rid of
         an entire function and extra call to l2d from the game loop.
 */

#include <sstream>
#include <glob.h>
#include "lime2d.h"

#include "../libext/imgui.h"
#include "../libext/imgui-events-SFML.h"
#include "../libext/imgui-rendering-SFML.h"
#include "../libext/imgui_internal.h"
#include "lime2d_config.h"

/*******************
 *  Lime2D Editor  *
 *******************/

l2d::Editor::Editor(bool enabled, sf::RenderWindow* window) :
    _graphics(new l2d_internal::Graphics(window)),
    _level(this->_graphics, "l2dSTART")
{
    this->_enabled = enabled;
    ImGui::SFML::SetRenderTarget(*window);
    ImGui::SFML::InitImGuiRendering();
    ImGui::SFML::SetWindow(*window);
    ImGui::SFML::InitImGuiEvents();

    this->_window = window;
}

void l2d::Editor::toggle() {
    this->_enabled = !this->_enabled;
}

void l2d::Editor::render() {
    if (this->_enabled) {
        ImGui::Render();
    }
}

void l2d::Editor::update(float elapsedTime, sf::Event &event) {
    if (this->_enabled) {
        ImGui::SFML::UpdateImGui();
        ImGui::SFML::UpdateImGuiRendering();

        ImGui::SFML::ProcessEvent(event);

        /*
         *  Menu
         *  File, View, Map, Animation, Help
         */
        static bool cbMapEditor = false;
        static bool cbAnimationEditor = false;
        static bool aboutBoxVisible = false;

        //About box
        if (aboutBoxVisible) {
            ImGui::SetNextWindowSize(ImVec2(300, 200));
            ImGui::Begin("About Lime2D", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Lime2D Editor\n\nBy: Limeoats\nCopyright \u00a9 2016");
            ImGui::Separator();
            if (ImGui::Button("Close")) {
                aboutBoxVisible = false;
            }
            ImGui::End();
        }

        if (ImGui::BeginMainMenuBar()) {
            if (ImGui::BeginMenu("File")) {
                if (ImGui::MenuItem("Exit")) {
                    this->_enabled = false; //TODO: do you want to save?
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                if (ImGui::Checkbox("Map Editor", &cbMapEditor)) {
                    cbAnimationEditor = false;
                }
                if (ImGui::Checkbox("Animation Editor", &cbAnimationEditor)) {
                    cbMapEditor = false;
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Map", cbMapEditor)) {
                if (ImGui::MenuItem("Load map")) {
                    glob_t glob_result;
                    std::stringstream ss;
                    ss << l2d::Config::MapPath << "*";
                    glob(ss.str().c_str(), GLOB_TILDE, NULL, &glob_result);
                    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
                        std::cout << "File: " << glob_result.gl_pathv[i] << std::endl;
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Animation", cbAnimationEditor)) {
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                if (ImGui::MenuItem("About Lime2D")) {
                    aboutBoxVisible = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (cbMapEditor) {
            /*
             * Map Editor
             */
            ImGui::Begin("Background", nullptr, ImGui::GetIO().DisplaySize, 0.0f,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
            ImGui::GetWindowDrawList()->AddText(ImVec2(10, 30), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "Map Editor");

            //Draw the grid
            //ImGui::GetWindowDrawList()->AddLine(ImVec2(0, 100), ImVec2(800, 100), ImColor(200, 200, 200, 255), 0.4f);

            ImGui::End();
        }
        else if (cbAnimationEditor) {
            /*
             * Animation Editor
             */
            ImGui::Begin("Background", nullptr, ImGui::GetIO().DisplaySize, 0.0f,
                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                         ImGuiWindowFlags_NoCollapse |
                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
            ImGui::GetWindowDrawList()->AddText(ImVec2(10, 30), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "Animation Editor");
            ImGui::End();
        }
    }
}

void l2d::Editor::exit() {
    ImGui::SFML::Shutdown();
}
