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
 */


#include <sstream>
#include <fstream>
#include <iostream>

#include "lime2d.h"

#include "../libext/imgui.h"
#include "../libext/imgui-SFML.h"
#include "../libext/imgui_internal.h"

/*******************
 *  Lime2D Editor  *
 *******************/

l2d::Editor::Editor(bool enabled, sf::RenderWindow* window) :
    _graphics(new l2d_internal::Graphics(window)),
    _level(this->_graphics, "l2dSTART")
{
    this->_enabled = enabled;
    ImGui::SFML::Init(*window);

    this->_window = window;
}

void l2d::Editor::toggle() {
    this->_enabled = !this->_enabled;
}

void l2d::Editor::processEvent(sf::Event &event) {
    ImGui::SFML::ProcessEvent(event);
}

void l2d::Editor::render() {
    if (this->_enabled) {
        this->_level.draw();
        //Draw the grid lines if appropriate
        if (this->_level.getName() != "l2dSTART") {
            for (int i = 0; i < this->_level.getSize().y; ++i) {
                sf::Vertex line[] = {
                        sf::Vertex(sf::Vector2f(0, i * (this->_level.getTileSize().y * std::stof(
                                l2d_internal::utils::getConfigValue("tile_scale_y"))))),
                        sf::Vertex(sf::Vector2f(this->_level.getSize().x * this->_level.getTileSize().x *
                                                std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")),
                                                i * (this->_level.getTileSize().y *
                                                     std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")))))
                };
                this->_graphics->draw(line, 2, sf::Lines);
            }
            for (int i = 0; i < this->_level.getSize().x; ++i) {
                sf::Vertex line[] = {
                    sf::Vertex(sf::Vector2f(i * (this->_level.getTileSize().x * std::stof(
                            l2d_internal::utils::getConfigValue("tile_scale_x"))),0)),
                        sf::Vertex(sf::Vector2f(i * this->_level.getTileSize().y *
                                                std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")),
                                                this->_level.getSize().x * (this->_level.getTileSize().x *
                                                        std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")))))
                };
                this->_graphics->draw(line, 2, sf::Lines);
            }
            //Get the mouse position
            std::cout << sf::Mouse::getPosition(*this->_window).x << std::endl;
        }
        ImGui::Render();
    }
}

void l2d::Editor::update(sf::Time t) {
    if (this->_enabled) {
        ImGui::SFML::Update(t);


        /*
         *  Menu
         *  File, View, Map, Animation, Help
         */
        static bool cbMapEditor = false;
        static bool cbAnimationEditor = false;
        static bool aboutBoxVisible = false;
        static bool mapSelectBoxVisible = false;
        static bool configWindowVisible = false;

        static int mapSelectIndex = 0;

        //Config window
        if (configWindowVisible) {
            static bool loaded = false;

            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(500, 400));
            static std::string configureMapErrorText = "";
            ImGui::Begin("Configure map editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("Map path");
            static char mapPath[100] = "";
            if (l2d_internal::utils::getConfigValue("map_path") != "" && !loaded) {
                strcpy(mapPath, l2d_internal::utils::getConfigValue("map_path").c_str());
            }
            ImGui::PushItemWidth(300);
            ImGui::InputText("", mapPath, 100);
            ImGui::PopItemWidth();
            ImGui::Separator();

            ImGui::PushItemWidth(100);

            ImGui::PushID("ConfigureSpriteScale");
            ImGui::Text("Sprite scale");
            static float spriteScaleX = l2d_internal::utils::getConfigValue("sprite_scale_x") == "" ? 1.0f : std::stof(l2d_internal::utils::getConfigValue("sprite_scale_x"));
            static float spriteScaleY = l2d_internal::utils::getConfigValue("sprite_scale_y") == "" ? 1.0f : std::stof(l2d_internal::utils::getConfigValue("sprite_scale_y"));
            ImGui::InputFloat("x", &spriteScaleX, 0.1f, 0.0f, 2);
            ImGui::InputFloat("y", &spriteScaleY, 0.1f, 0.0f, 2);
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PushID("ConfigureTileScale");
            ImGui::Text("Tile scale");
            static float tileScaleX = l2d_internal::utils::getConfigValue("tile_scale_x") == "" ? 1.0f : std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"));
            static float tileScaleY = l2d_internal::utils::getConfigValue("tile_scale_y") == "" ? 1.0f : std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"));
            ImGui::InputFloat("x", &tileScaleX, 0.1f, 0.0f, 2);
            ImGui::InputFloat("y", &tileScaleY, 0.1f, 0.0f, 2);
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PushID("ConfigureScreenSize");
            ImGui::Text("Screen size");
            static int screenSizeX = l2d_internal::utils::getConfigValue("screen_size_x") == "" ? 1 : std::stoi(l2d_internal::utils::getConfigValue("screen_size_x"));
            static int screenSizeY = l2d_internal::utils::getConfigValue("screen_size_y") == "" ? 1 : std::stoi(l2d_internal::utils::getConfigValue("screen_size_y"));
            ImGui::InputInt("x", &screenSizeX, 5);
            ImGui::InputInt("y", &screenSizeY, 5);
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PopItemWidth();

            if (ImGui::Button("Save")) {
                //Lots of error checking on all of the values
                //Saving to lime2d.config
                if (strlen(mapPath) <= 0) {
                    configureMapErrorText = "You must enter the location of your maps!";
                }
                else if (spriteScaleX < 0 || spriteScaleY < 0) {
                    configureMapErrorText = "Sprite scale cannot be negative!";
                }
                else if (tileScaleX < 0 || tileScaleY < 0) {
                    configureMapErrorText = "Tile scale cannot be negative!";
                }
                else if (screenSizeX < 0 || screenSizeY < 0) {
                    configureMapErrorText = "Screen size cannot be negative!";
                }
                else {
                    configureMapErrorText = "";
                    //Everything checks out, so save.
                    std::ofstream os("lime2d.config");
                    if (os.is_open()) {
                        os << "map_path=" << mapPath << "\n";
                        os << "sprite_scale_x=" << spriteScaleX << "\n";
                        os << "sprite_scale_y=" << spriteScaleY << "\n";
                        os << "tile_scale_x=" << tileScaleX << "\n";
                        os << "tile_scale_y=" << tileScaleY << "\n";
                        os << "screen_size_x=" << screenSizeX << "\n";
                        os << "screen_size_y=" << screenSizeY << "\n";
                        os.close();
                        configureMapErrorText = "Save successful.";
                    }
                    else {
                        configureMapErrorText = "Unable to save file. Please refer to www.limeoats.com/lime2d for more information.";
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                configWindowVisible = false;
                configureMapErrorText = "";
            }
            ImGui::Text(configureMapErrorText.c_str());
            ImGui::End();
            loaded = true;
        }

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

        //Map select box
        if (mapSelectBoxVisible) {
            std::stringstream ss;
            ss << l2d_internal::utils::getConfigValue("map_path") << "*";
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
                if (ImGui::MenuItem("Configure")) {
                    configWindowVisible = true;
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
        this->_level.update(t.asSeconds());
        this->_graphics->update(t.asSeconds());
    }
}

void l2d::Editor::exit() {
    ImGui::SFML::Shutdown();
}
