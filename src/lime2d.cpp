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

        -Change internal level's loadMap so that the function returns a string. Then, whenever I call loadMap
         from l2d, check return value with if (!level->loadMap("asdf")) and show an ImGui popup with the
         error message. Make sure in level loadMap to check every possible place it can break.
         (make sure tileset exists, make sure map exists, make sure any sprites exist, etc)

        -Draw grid lines on the map editor when a level is loaded. For this, we need to find out the level size,
         which we should already have in level internal, as well as the tile size. From there, start at 0,0 and
         draw the lines accordingly. NOTE: this could be difficult since ImGui doesn't have any idea what the camera is.
         Meaning we need to implement an internal camera as well...
 */


#include <sstream>

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
        this->_level.draw();
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
        static bool mapSelectBoxVisible = false;

        static int mapSelectIndex = 0;

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

        if (mapSelectBoxVisible) {
            std::stringstream ss;
            ss << l2d::Config::MapPath << "*";
            std::vector<const char*> mapFiles = l2d_internal::utils::getFilesInDirectory(ss.str());
            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(500, 400));
            ImGui::Begin("Select a map", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Select a map from the list below.");
            ImGui::Separator();
            ImGui::PushItemWidth(-1);
            ImGui::ListBox("", &mapSelectIndex, &mapFiles[0], mapFiles.size(), 10);
            ImGui::Separator();
            if (ImGui::Button("Open")) {
                //Get the name of the file
                std::vector<std::string> fullNameSplit = l2d_internal::utils::split(mapFiles[mapSelectIndex], '/');
                std::vector<std::string> fileNameSplit = l2d_internal::utils::split(fullNameSplit.back(), '.');
                this->_level.loadMap(fileNameSplit.front());
                mapSelectBoxVisible = false;
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                mapSelectBoxVisible = false;
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
                    mapSelectBoxVisible = true;
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

        this->_level.update(elapsedTime);
    }
}

void l2d::Editor::exit() {
    ImGui::SFML::Shutdown();
}
