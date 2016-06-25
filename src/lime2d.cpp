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
#include <algorithm>

#include "lime2d.h"

#include "../libext/imgui.h"
#include "../libext/imgui-SFML.h"
#include "../libext/imgui_internal.h"

/*******************
 *  Lime2D Editor  *
 *******************/

l2d::Editor::Editor(bool enabled, sf::RenderWindow* window) :
    _graphics(new l2d_internal::Graphics(window)),
    _level(this->_graphics, "l2dSTART"),
    _showGridLines(true)
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
    if (event.type == sf::Event::GainedFocus) {
        this->_windowHasFocus = true;
    }
    if (event.type == sf::Event::LostFocus) {
        this->_windowHasFocus = false;
    }
}

void l2d::Editor::render() {
    if (this->_enabled) {
        this->_level.draw();
        //Draw the grid lines if appropriate
        if (this->_level.getName() != "l2dSTART") {
            if (this->_showGridLines) {
                //Horizontal lines
                for (int i = 0; i < this->_level.getSize().y + 1; ++i) {
                    sf::Vertex line[] = {
                            sf::Vertex(sf::Vector2f(0, i * (this->_level.getTileSize().y * std::stof(
                                    l2d_internal::utils::getConfigValue("tile_scale_y"))))),
                            sf::Vertex(sf::Vector2f(this->_level.getSize().x * this->_level.getTileSize().x *
                                                    std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")),
                                                    i * (this->_level.getTileSize().y *
                                                         std::stof(
                                                                 l2d_internal::utils::getConfigValue("tile_scale_y")))))
                    };
                    this->_graphics->draw(line, 2, sf::Lines);
                }
                //Vertical lines
                for (int i = 0; i < this->_level.getSize().x + 1; ++i) {
                    sf::Vertex line[] = {
                            sf::Vertex(sf::Vector2f(i * (this->_level.getTileSize().x * std::stof(
                                    l2d_internal::utils::getConfigValue("tile_scale_x"))), 0)),
                            sf::Vertex(sf::Vector2f(i * this->_level.getTileSize().x *
                                                    std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")),
                                                    this->_level.getSize().y * (this->_level.getTileSize().y *
                                                                                std::stof(
                                                                                        l2d_internal::utils::getConfigValue(
                                                                                                "tile_scale_y")))))
                    };
                    this->_graphics->draw(line, 2, sf::Lines);
                }
                //Get the mouse position and draw a square or something around the correct grid tile
                sf::Vector2f mousePos(
                        sf::Mouse::getPosition(*this->_window).x + this->_graphics->getCamera()->getRect().left,
                        sf::Mouse::getPosition(*this->_window).y + this->_graphics->getCamera()->getRect().top);

                if (mousePos.x >= 0 && mousePos.x <= (this->_level.getSize().x * this->_level.getTileSize().x *
                                                      std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) &&
                    mousePos.y >= 0 && mousePos.y <= (this->_level.getSize().y * this->_level.getTileSize().y *
                                                      std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")))) {
                    sf::RectangleShape rectangle;
                    rectangle.setSize(sf::Vector2f(this->_level.getTileSize().x *
                                                   std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")) - 1,
                                                   this->_level.getTileSize().y *
                                                   std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")) - 1));
                    rectangle.setOutlineColor(sf::Color::Magenta);
                    rectangle.setOutlineThickness(2);
                    rectangle.setPosition((mousePos.x - ((int) mousePos.x % (int) (this->_level.getTileSize().x *
                                                                                   std::stof(
                                                                                           l2d_internal::utils::getConfigValue(
                                                                                                   "tile_scale_x"))))),
                                          mousePos.y - ((int) mousePos.y % (int) (this->_level.getTileSize().y *
                                                                                  std::stof(
                                                                                          l2d_internal::utils::getConfigValue(
                                                                                                  "tile_scale_y")))));
                    rectangle.setFillColor(sf::Color::Transparent);
                    this->_window->draw(rectangle);
                }
            }
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
        static bool cbShowGridLines = true;
        static bool aboutBoxVisible = false;
        static bool mapSelectBoxVisible = false;
        static bool configWindowVisible = false;
        static bool tilePropertiesWindowVisible = false;
        static bool newMapBoxVisible = false;
        static bool newMapExistsOverwriteVisible = false;

        static sf::Vector2f mousePos(0.0f, 0.0f);

        static std::vector<std::shared_ptr<sf::Texture>> tilesets;

        static int mapSelectIndex = 0;

        static bool showSpecificTileProperties = false;
        static std::shared_ptr<l2d_internal::Tile> showSpecificTilePropertiesTile = nullptr;
        static std::shared_ptr<l2d_internal::Layer> showSpecificTilePropertiesLayer = nullptr;

        //Config window
        if (configWindowVisible) {
            static bool loaded = false;

            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(500, 400));
            static std::string configureMapErrorText = "";
            ImGui::Begin("Configure map editor", nullptr, ImVec2(500,400), 100.0f, ImGuiWindowFlags_AlwaysAutoResize);

            ImGui::Text("Map path");
            static char mapPath[500] = "";
            if (l2d_internal::utils::getConfigValue("map_path") != "" && !loaded) {
                strcpy(mapPath, l2d_internal::utils::getConfigValue("map_path").c_str());
            }
            ImGui::PushItemWidth(300);
            ImGui::InputText("", mapPath, 500);
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
                        if (this->_level.getName() != "l2dSTART") {
                            std::string name = this->_level.getName();
                            this->_level.loadMap(name);
                        }
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
            ImGui::SetNextWindowSize(ImVec2(300, 140));
            ImGui::Begin("About Lime2D", nullptr, ImVec2(300, 200), 100.0f, ImGuiWindowFlags_AlwaysAutoResize);
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
            ImGui::Begin("Select a map", nullptr, ImVec2(500, 400), 100.0f, ImGuiWindowFlags_AlwaysAutoResize);
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

        //New map box
        if (newMapBoxVisible) {
            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(500, 400));
            static std::string newMapErrorText = "";
            ImGui::Begin("New map properties", nullptr, ImVec2(500, 400), 100.0f, ImGuiWindowFlags_AlwaysAutoResize);
            ImGui::Text("Name");
            static char name[500] = "";
            ImGui::PushItemWidth(300);
            ImGui::InputText("", name, 500);
            ImGui::PopItemWidth();
            ImGui::Separator();

            ImGui::PushItemWidth(100);

            ImGui::PushID("NewMapSize");
            ImGui::Text("Size");
            static int mapSizeX = 0;
            static int mapSizeY = 0;
            ImGui::InputInt("x", &mapSizeX, 1, 0);
            ImGui::InputInt("y", &mapSizeY, 1, 0);
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PushID("NewMapTileSize");
            ImGui::Text("Size");
            static int mapTileSizeX = 8;
            static int mapTileSizeY = 8;
            ImGui::InputInt("x", &mapTileSizeX, 1, 0);
            ImGui::InputInt("y", &mapTileSizeY, 1, 0);
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PopItemWidth();
            if (ImGui::Button("Create")) {
                if (strlen(name) <= 0) {
                    newMapErrorText = "You must enter a name for the new map!";
                }
                else if (mapSizeX < 0 || mapSizeY < 0) {
                    newMapErrorText = "You cannot have a negative map size!";
                }
                else if (mapTileSizeX < 0 || mapTileSizeY < 0) {
                    newMapErrorText = "You cannot have a negative tile size!";
                }
                else {
                    //Check if map with that name already exists. If so, give a box asking to overwrite
                    std::stringstream ss;
                    ss << l2d_internal::utils::getConfigValue("map_path") << "*";
                    std::vector<const char*> mapFiles = l2d_internal::utils::getFilesInDirectory(ss.str());
                    ss.str("");
                    ss << l2d_internal::utils::getConfigValue("map_path") << name << ".xml";
                    if (l2d_internal::utils::contains(mapFiles, ss.str())) {
                        newMapExistsOverwriteVisible = true;
                    }
                    else {
                        this->_level.createMap(std::string(name), sf::Vector2i(mapSizeX, mapSizeY),
                                               sf::Vector2i(mapTileSizeX, mapTileSizeY));
                        strcpy(name, "");
                        mapSizeX = 0;
                        mapSizeY = 0;
                        mapTileSizeX = 8;
                        mapTileSizeY = 8;
                        newMapErrorText = "";
                        newMapBoxVisible = false;
                        newMapExistsOverwriteVisible = false;
                    }
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                strcpy(name, "");
                mapSizeX = 0;
                mapSizeY = 0;
                mapTileSizeX = 8;
                mapTileSizeY = 8;
                newMapErrorText = "";
                newMapBoxVisible = false;
                newMapExistsOverwriteVisible = false;
            }
            ImGui::Text(newMapErrorText.c_str());
            if (newMapExistsOverwriteVisible) {
                newMapErrorText = "";
                ImGui::Separator();
                ImGui::Text("The name you have chosen already exists in your map directory.");
                ImGui::Text("Would you like to overwrite the existing map?");
                if (ImGui::Button("Sure!")) {
                    this->_level.createMap(std::string(name), sf::Vector2i(mapSizeX, mapSizeY),
                                           sf::Vector2i(mapTileSizeX, mapTileSizeY));
                    strcpy(name, "");
                    mapSizeX = 0;
                    mapSizeY = 0;
                    mapTileSizeX = 8;
                    mapTileSizeY = 8;
                    newMapErrorText = "";
                    newMapBoxVisible = false;
                    newMapExistsOverwriteVisible = false;
                }
                ImGui::SameLine();
                if (ImGui::Button("No!")) {
                    newMapExistsOverwriteVisible = false;
                }
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
                if (ImGui::MenuItem("New map")) {
                    newMapBoxVisible = true;
                }
                if (ImGui::MenuItem("Load map")) {
                    mapSelectBoxVisible = true;
                }
                if (this->_level.getName() != "l2dSTART") {
                    if (ImGui::MenuItem("Save map")) {
                        this->_level.saveMap(this->_level.getName());
                    }
                }
                if (ImGui::MenuItem("Configure")) {
                    configWindowVisible = true;
                }
                if (this->_level.getName() != "l2dSTART") {
                    ImGui::Separator();
                    if (ImGui::Checkbox("Show grid lines", &cbShowGridLines)) {
                        this->_showGridLines = cbShowGridLines;
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

        if (this->_level.getName() != "l2dSTART") {
            if (ImGui::IsMouseClicked(1)) {
                mousePos = sf::Vector2f(
                        sf::Mouse::getPosition(*this->_window).x + this->_graphics->getCamera()->getRect().left,
                        sf::Mouse::getPosition(*this->_window).y + this->_graphics->getCamera()->getRect().top);
                ImGui::OpenPopup("right click on tile");
            }
            if (ImGui::BeginPopup("right click on tile")) {
                if (ImGui::MenuItem("Properties")) {
                    tilePropertiesWindowVisible = true;
                }
                ImGui::Separator();
                ImGui::EndPopup();
            }

            //Tile info window
            if (tilePropertiesWindowVisible) {
                ImGui::SetNextWindowPosCenter();
                ImGui::SetNextWindowSize(ImVec2(300, 400));
                ImGui::Begin("Properties", nullptr, ImVec2(300, 400), 100.0f);

                sf::Vector2f tilePos(mousePos.x - ((int) mousePos.x % (int) (this->_level.getTileSize().x * std::stof(
                        l2d_internal::utils::getConfigValue("tile_scale_x")))),
                                     (mousePos.y - ((int) mousePos.y % (int) (this->_level.getTileSize().y *
                                                                              std::stof(
                                                                                      l2d_internal::utils::getConfigValue(
                                                                                              "tile_scale_y"))))));
                ImGui::Text("put a picture of the tile here");
                for(int i = 0; i < this->_level.getLayerList().size(); ++i) {
                    std::for_each(this->_level.getLayerList()[i].get()->Tiles.begin(), this->_level.getLayerList()[i].get()->Tiles.end(), [&](const std::shared_ptr<l2d_internal::Tile> &tile) {
                        if (tile.get()->getSprite().getPosition() == tilePos) {
                            if (i > 0) {
                                ImGui::SameLine();
                            }
                            if (ImGui::ImageButton(tile->getSprite(), sf::Vector2f(32.0f, 32.0f), 1, sf::Color::Transparent)) {
                                showSpecificTilePropertiesTile = std::make_shared<l2d_internal::Tile>(*tile.get());
                                showSpecificTilePropertiesLayer = std::make_shared<l2d_internal::Layer>(*this->_level.getLayerList()[i].get());
                                showSpecificTileProperties = true;
                            }
                        }
                    });
                }
                ImGui::Separator();
                ImGui::Text("Position: %d, %d", (int)tilePos.x, (int)tilePos.y);
                ImGui::Separator();
                if (showSpecificTileProperties) {
                    ImGui::Image(showSpecificTilePropertiesTile->getSprite(), sf::Vector2f(32.0f, 32.0f));
                    ImGui::Text("Layer: %d", showSpecificTilePropertiesLayer->Id);
                }
                if (ImGui::Button("Close")) {
                    showSpecificTileProperties = false;
                    tilePropertiesWindowVisible = false;
                }
                ImGui::End();
            }
        }


        if (cbMapEditor) {
            /*
             * Map Editor
             */
//            ImGui::Begin("Background", nullptr, ImGui::GetIO().DisplaySize, 0.0f,
//                         ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
//                         ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
//                         ImGuiWindowFlags_NoCollapse |
//                         ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
//                         ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
//            ImGui::GetWindowDrawList()->AddText(ImVec2(10, 30), ImColor(1.0f, 1.0f, 1.0f, 1.0f), "Map Editor");
//            ImGui::End();
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
        this->_graphics->update(t.asSeconds(), sf::Vector2f(this->_level.getTileSize()), this->_windowHasFocus);
    }
}

void l2d::Editor::exit() {
    ImGui::SFML::Shutdown();
}
