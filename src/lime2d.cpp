/*
 * Lime2d.cpp
 * By: Mark Guerra
 * Created on: 6/9/2016
 * Copyright (c) 2016 Limeoats
 */


#include <sstream>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <cmath>

#include "lime2d.h"

#include "../libext/imgui.h"
#include "../libext/imgui-SFML.h"
#include "../libext/imgui_internal.h"

/*******************
 *  Lime2D Editor  *
 *******************/

l2d::Editor::Editor(bool enabled, sf::RenderWindow* window) :
        _windowHasFocus(true),
        _showGridLines(true),
        _showEntityList(false),
        _eraserActive(false),
        _tilesetEnabled(false),
        _mainHasFocus(true),
        _currentFeature(l2d_internal::Features::None),
        _graphics(new l2d_internal::Graphics(window)),
        _level(this->_graphics, "l2dSTART"),
        _currentDrawShape(l2d_internal::DrawShapes::None),
        _currentMapEditorMode(l2d_internal::MapEditorMode::None),
        _currentEvent()
{
    this->_enabled = enabled;
    ImGui::SFML::Init(*window);
    this->_window = window;
    if (!this->_ambientLight.loadFromFile("content/shaders/ambient.frag", sf::Shader::Fragment)) {
        return;
    }
}

void l2d::Editor::toggle() {
    this->_enabled = !this->_enabled;
}

void l2d::Editor::processEvent(sf::Event &event) {
    ImGui::SFML::ProcessEvent(event);
    this->_currentEvent = event;
    switch (event.type) {
        case sf::Event::GainedFocus:
            this->_windowHasFocus = true;
            break;
        case sf::Event::LostFocus:
            this->_windowHasFocus = false;
            break;
        case sf::Event::KeyReleased:
            switch (event.key.code) {
                case sf::Keyboard::T:
                    if (this->_level.getName() != "l2dSTART" && this->_currentFeature == l2d_internal::Features::Map &&
                            this->_currentMapEditorMode == l2d_internal::MapEditorMode::Tile) {
                        this->_tilesetEnabled = !this->_tilesetEnabled;
                    }
                    break;
                case sf::Keyboard::G:
                    if (this->_level.getName() != "l2dSTART" && this->_currentFeature == l2d_internal::Features::Map) {
                        this->_showGridLines = !this->_showGridLines;
                    }
                    break;
                case sf::Keyboard::U:
                    this->_level.undo();
                    break;
                case sf::Keyboard::R:
                    this->_level.redo();
                    break;
                case sf::Keyboard::E:
                    if (this->_level.getName() != "l2dSTART" && this->_currentFeature == l2d_internal::Features::Map &&
                            this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
                        this->_showEntityList = !this->_showEntityList;
                    }
                    break;
                case sf::Keyboard::M:
                    if (this->_currentFeature == l2d_internal::Features::Map) {
                        this->_currentMapEditorMode =
                                this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object ?
                                l2d_internal::MapEditorMode::Tile : l2d_internal::MapEditorMode::Object;
                    }
                default:
                    break;
            }
            break;
        case sf::Event::MouseWheelScrolled:
            if (this->_currentFeature == l2d_internal::Features::Map && this->_level.getName() != "l2dSTART") {
                if (event.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel) {
                    this->_graphics.get()->zoom(event.mouseWheelScroll.delta,
                                                {event.mouseWheelScroll.x, event.mouseWheelScroll.y});
                }
            }
            break;
        default:
            break;
    }
}

void l2d::Editor::render() {
    if (this->_enabled) {
        //If map editor
        if (this->_level.getName() != "l2dSTART" && this->_currentFeature == l2d_internal::Features::Map) {
            this->_ambientLight.setUniform("texture", sf::Shader::CurrentTexture);
            this->_ambientLight.setUniform("color", sf::Glsl::Vec3(this->_level.getAmbientColor().r / 255.0f, this->_level.getAmbientColor().g / 255.0f, this->_level.getAmbientColor().b / 255.0f));
            this->_ambientLight.setUniform("intensity", this->_level.getAmbientIntensity());
            this->_level.draw(&this->_ambientLight);
            if (this->_showGridLines) {
                for (auto &t : this->_gridLines) {
                    this->_graphics->draw(t.data(), 2, sf::Lines);
                }
                if (this->_currentDrawShape == l2d_internal::DrawShapes::None && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Tile) {
                    //Get the mouse position and draw a square around the correct grid tile
                    sf::Vector2f mousePos = this->_window->mapPixelToCoords(sf::Vector2i(
                            sf::Mouse::getPosition(*this->_window).x + static_cast<int>(this->_graphics->getView().getViewport().left),
                            sf::Mouse::getPosition(*this->_window).y + static_cast<int>(this->_graphics->getView().getViewport().top)));


                    if (mousePos.x >= 0 && mousePos.x <= (this->_level.getSize().x * this->_level.getTileSize().x *
                                                          std::stof(
                                                                  l2d_internal::utils::getConfigValue("tile_scale_x")) -
                                                          1) &&
                        mousePos.y >= 0 && mousePos.y <= (this->_level.getSize().y * this->_level.getTileSize().y *
                                                          std::stof(
                                                                  l2d_internal::utils::getConfigValue("tile_scale_y")) -
                                                          1)) {
                        sf::RectangleShape rectangle;
                        rectangle.setSize(sf::Vector2f(this->_level.getTileSize().x *
                                                       std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")) -
                                                       1,
                                                       this->_level.getTileSize().y *
                                                       std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")) -
                                                       1));
                        rectangle.setOutlineColor(this->_eraserActive ? sf::Color::Blue : sf::Color::Magenta);
                        rectangle.setOutlineThickness(2);
                        rectangle.setPosition(std::floor(mousePos.x - ((int) mousePos.x % (int) (this->_level.getTileSize().x *
                                                                                       std::stof(
                                                                                               l2d_internal::utils::getConfigValue(
                                                                                                       "tile_scale_x"))))),
                                              std::floor(mousePos.y - ((int) mousePos.y % (int) (this->_level.getTileSize().y *
                                                                                      std::stof(
                                                                                              l2d_internal::utils::getConfigValue(
                                                                                                      "tile_scale_y"))))));
                        rectangle.setFillColor(sf::Color::Transparent);
                        this->_window->draw(rectangle);
                    }
                }
            }
            for (std::shared_ptr<l2d_internal::Shape> shape : this->_level.getShapeList()) {
                shape.get()->draw(this->_window);
            }
        }
        sf::RectangleShape rectangle;
        rectangle.setSize(sf::Vector2f(this->_window->getSize().x, 30));
        rectangle.setFillColor(sf::Color::Black);
        rectangle.setPosition(0 + this->_graphics->getView().getViewport().left, this->_window->getSize().y - 30 + this->_graphics->getView().getViewport().top);
        this->_window->draw(rectangle);

        //Shape creation
        //Rectangles
        if (this->_currentDrawShape == l2d_internal::DrawShapes::Rectangle && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object && this->_mainHasFocus) {
            static sf::Vector2f mousePos = this->_window->mapPixelToCoords(sf::Vector2i(
                    sf::Mouse::getPosition(*this->_window).x + static_cast<int>(this->_graphics->getView().getViewport().left),
                    sf::Mouse::getPosition(*this->_window).y + static_cast<int>(this->_graphics->getView().getViewport().top)));
            static sf::Vector2f startPos = mousePos;
            static sf::Vector2f currentPosition;
            static sf::RectangleShape rect;
            static bool started = false;
            if (this->_currentEvent.type == sf::Event::MouseButtonPressed && this->_currentEvent.mouseButton.button == sf::Mouse::Left) {
                mousePos = this->_window->mapPixelToCoords(sf::Vector2i(
                        sf::Mouse::getPosition(*this->_window).x + static_cast<int>(this->_graphics->getView().getViewport().left),
                        sf::Mouse::getPosition(*this->_window).y + static_cast<int>(this->_graphics->getView().getViewport().top)));
                startPos.x = mousePos.x;
                startPos.y = mousePos.y;
            }
            if (this->_currentEvent.type == sf::Event::MouseMoved && sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                mousePos = this->_window->mapPixelToCoords(sf::Vector2i(
                        sf::Mouse::getPosition(*this->_window).x + static_cast<int>(this->_graphics->getView().getViewport().left),
                        sf::Mouse::getPosition(*this->_window).y + static_cast<int>(this->_graphics->getView().getViewport().top)));
                currentPosition.x = mousePos.x - startPos.x;
                currentPosition.y = mousePos.y - startPos.y;
                rect.setSize(currentPosition);
                rect.setPosition(startPos.x, startPos.y);
                rect.setFillColor(sf::Color(0, 0, 0, 100));
                rect.setOutlineThickness(2.0f);
                rect.setOutlineColor(sf::Color(0, 0, 0, 160));
                started = true;
                this->_window->draw(rect); //Temporarily draw the box while it's being created
            }
            if (this->_currentEvent.type == sf::Event::MouseButtonReleased && this->_currentEvent.mouseButton.button == sf::Mouse::Left && started) {
                std::cout << "added" << std::endl;
                this->_level.addShape(std::make_shared<l2d_internal::Rectangle>("Rectangle", sf::Color::White, l2d_internal::ObjectTypes::Collision, rect));
                this->_currentEvent = sf::Event();
                started = false;
                this->_currentDrawShape = l2d_internal::DrawShapes::None; //Stop drawing rectangles and return to select mode
            }
        }

        //Shape selection
        if (this->_currentEvent.type == sf::Event::MouseButtonPressed && this->_currentEvent.mouseButton.button == sf::Mouse::Left &&
                this->_currentDrawShape == l2d_internal::DrawShapes::None && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
            //Check the mouse pos and determine if it is inside a shape.
            sf::Vector2f mousePos = this->_window->mapPixelToCoords(sf::Vector2i(
                    sf::Mouse::getPosition(*this->_window).x + static_cast<int>(this->_graphics->getView().getViewport().left),
                    sf::Mouse::getPosition(*this->_window).y + static_cast<int>(this->_graphics->getView().getViewport().top)));
            for (auto &s: this->_level.getShapeList()) {
                if (s->isPointInside(mousePos)) {
                    //Highlight the shape and enable shape selected mode
                    break;
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
        static bool tilesetWindowVisible = false;
        static bool lightEditorWindowVisible = false;
        static bool newAnimatedSpriteWindowVisible = false;
        static bool newAnimationWindowVisible = false;
        static bool removeAnimationWindowVisible = false;
        static bool cbShowEntityList = false;
        static bool showEntityProperties = false;
        static bool shapeColorWindowVisible = false;

        static sf::Vector2f mousePos(0.0f, 0.0f);

        static std::string currentFeature = "Lime2D";
        static std::string currentStatus = "";
        static bool showCurrentStatus = false;
        static int currentStatusTimer = 0;

        static int mapSelectIndex = 0;
        static int animationSelectIndex = -1;
        static int animationSpriteSelectIndex = -1;
        static int spritesheetSelectIndex = -1;
        static std::string selectedAnimationFileName = "";
        static std::string selectedAnimationName = "";

        //Entity list variables
        static ImVec4 selectedEntityColor = sf::Color::White;
        static int selectedEntitySelectedObjectTypeIndex = -1;

        //Rectangle
        static std::shared_ptr<l2d_internal::Rectangle> selectedEntityRectangle;
        static std::shared_ptr<l2d_internal::Rectangle> originalSelectedEntityRectangle;

        //Drawing tiles variables
        static bool tileHasBeenSelected = false;
        static std::string selectedTilesetPath = "content/tilesets/outside.png";
        static sf::Vector2i selectedTileSrcPos(0,0);
        static int selectedTileLayer = 1;
        static sf::Vector2i selectedTilesetSize(0,0);

        //Light variables

        static l2d_internal::LightType selectedLightType = l2d_internal::LightType::None;

        //startStatusTimer function is written like this so that it can exist within the update function
        //This way, it can access the static timer variables without making them member variables
        static auto startStatusTimer = [&](std::string newStatus, int time) {
            currentStatus = newStatus;
            currentStatusTimer = time;
            showCurrentStatus = true;
        };

        //FilterLuaKeyInput function is a lambda function because it's only relevant in this function
        //Use it on ImGui::TextInputs to avoid bad characters
        static auto FilterLuaKeyInput = [](ImGuiTextEditCallbackData* data)->int {
            if (data->EventChar == ' ' || data->EventChar == '.' || data->EventChar == '{' || data->EventChar == '}' ||
                data->EventChar == '<' || data->EventChar == '>' || data->EventChar == '-' || data->EventChar == ';' ||
                data->EventChar == '~' || data->EventChar == '`' || data->EventChar == '!' || data->EventChar == '@' ||
                data->EventChar == '#' || data->EventChar == '$' || data->EventChar == '%' || data->EventChar == '^' ||
                data->EventChar == '&' || data->EventChar == '*' || data->EventChar == '(' || data->EventChar == ')' ||
                data->EventChar == '+' || data->EventChar == '=' || data->EventChar == '\'' || data->EventChar == '\"' ||
                data->EventChar == '?' || data->EventChar == '[' || data->EventChar == ']' || data->EventChar == '\\' ||
                data->EventChar == '|' || data->EventChar == ',' || data->EventChar == '/')
                return 1;
            return 0;
        };

        //createGridLines function is only called when a map is loaded to avoid recreating the list of lines every time
        //the render function is called.
        static auto createGridLines = [&]() {
            if (this->_level.getName() != "l2dSTART" && this->_currentFeature == l2d_internal::Features::Map) {
                std::array<sf::Vertex, 2> line;
                //Horizontal lines
                for (int i = 0; i < this->_level.getSize().y + 1; ++i) {
                    line = {
                        sf::Vertex(sf::Vector2f(0, i * (this->_level.getTileSize().y * std::stof(
                                    l2d_internal::utils::getConfigValue("tile_scale_y"))))),
                        sf::Vertex(sf::Vector2f(this->_level.getSize().x * this->_level.getTileSize().x *
                                                        std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")),
                                                i * (this->_level.getTileSize().y *
                                                        std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")))))
                    };
                    this->_gridLines.push_back(line);
                }
                //Vertical lines
                for (int i = 0; i < this->_level.getSize().x + 1; ++i) {
                    line = {
                        sf::Vertex(sf::Vector2f(i * (this->_level.getTileSize().x * std::stof(
                                    l2d_internal::utils::getConfigValue("tile_scale_x"))), 0)),
                        sf::Vertex(sf::Vector2f(i * this->_level.getTileSize().x *
                                                    std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")),
                                                    this->_level.getSize().y * (this->_level.getTileSize().y *
                                                            std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")))))
                    };
                    this->_gridLines.push_back(line);
                }
            }
        };

        //Set mainHasFocus (very important)
        //This tells Lime2D that it can draw tiles to the screen. We don't want it drawing if other windows have focus.
        this->_mainHasFocus = !(tilesetWindowVisible || newMapBoxVisible || tilePropertiesWindowVisible || configWindowVisible || mapSelectBoxVisible || aboutBoxVisible || lightEditorWindowVisible || newAnimatedSpriteWindowVisible ||
                         removeAnimationWindowVisible || cbShowEntityList || shapeColorWindowVisible);

        cbShowGridLines = this->_showGridLines;
        cbShowEntityList = this->_showEntityList;


        //Config window
        if (configWindowVisible) {
            static bool loaded = false;

            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(500, 480));
            static std::string configureMapErrorText = "";
            ImGui::Begin("Configure", nullptr, ImVec2(500,480), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_ShowBorders);

            ImGui::PushID("ConfigureMapPath");
            ImGui::Text("Map path");
            static char mapPath[500] = "";
            if (l2d_internal::utils::getConfigValue("map_path") != "" && !loaded) {
                strcpy(mapPath, l2d_internal::utils::getConfigValue("map_path").c_str());
            }
            ImGui::PushItemWidth(300);
            ImGui::InputText("", mapPath, 500);
            ImGui::PopItemWidth();
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PushID("ConfigureTilesetPath");
            ImGui::Text("Tileset path");
            static char tilesetPath[500] = "";
            if (l2d_internal::utils::getConfigValue("tileset_path") != "" && !loaded) {
                strcpy(tilesetPath, l2d_internal::utils::getConfigValue("tileset_path").c_str());
            }
            ImGui::PushItemWidth(300);
            ImGui::InputText("", tilesetPath, 500);
            ImGui::PopItemWidth();
            ImGui::Separator();
            ImGui::PopID();

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

            ImGui::PushID("ConfigureSpritesPath");
            ImGui::Text("Sprite path");
            static char spritePath[500] = "";
            if (l2d_internal::utils::getConfigValue("sprite_path") != "" && !loaded) {
                strcpy(spritePath, l2d_internal::utils::getConfigValue("sprite_path").c_str());
            }
            ImGui::PushItemWidth(300);
            ImGui::InputText("", spritePath, 500);
            ImGui::PopItemWidth();
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PushID("ConfigureAnimationPath");
            ImGui::Text("Animation path");
            static char animationPath[500] = "";
            if (l2d_internal::utils::getConfigValue("animation_path") != "" && !loaded) {
                strcpy(animationPath, l2d_internal::utils::getConfigValue("animation_path").c_str());
            }
            ImGui::PushItemWidth(300);
            ImGui::InputText("", animationPath, 500);
            ImGui::PopItemWidth();
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PushID("ConfigureCameraPanAmount");
            ImGui::Text("Camera pan amount");
            static float cameraPanAmount = l2d_internal::utils::getConfigValue("camera_pan_amount") == "" ? 4.0f : std::stof(l2d_internal::utils::getConfigValue("camera_pan_amount"));
            ImGui::InputFloat("", &cameraPanAmount, 0.25f, 0.0f, 2);
            ImGui::Separator();
            ImGui::PopID();

            ImGui::PopItemWidth();

            if (ImGui::Button("Save")) {
                //Lots of error checking on all of the values
                //Saving to lime2d.config
                if (strlen(mapPath) <= 0) {
                    configureMapErrorText = "You must enter the location of your maps!";
                }
                else if (strlen(tilesetPath) <= 0) {
                    configureMapErrorText = "You must enter the location of your tilesets!";
                }
                else if (strlen(spritePath) <= 0) {
                    configureMapErrorText = "You must enter the location of your sprites!";
                }
                else if (strlen(animationPath) <= 0) {
                    configureMapErrorText = "You must enter the location of your animations!";
                }
                else {
                    configureMapErrorText = "";
                    //Everything checks out, so save.
                    std::ofstream os("lime2d.config");
                    if (os.is_open()) {
                        os << "map_path=" << mapPath << "\n";
                        os << "tileset_path=" << tilesetPath << "\n";
                        os << "sprite_scale_x=" << spriteScaleX << "\n";
                        os << "sprite_scale_y=" << spriteScaleY << "\n";
                        os << "tile_scale_x=" << tileScaleX << "\n";
                        os << "tile_scale_y=" << tileScaleY << "\n";
                        os << "screen_size_x=" << screenSizeX << "\n";
                        os << "screen_size_y=" << screenSizeY << "\n";
                        os << "sprite_path=" << spritePath << "\n";
                        os << "animation_path=" << animationPath << "\n";
                        os << "camera_pan_amount=" << cameraPanAmount << "\n";
                        os.close();
                        if (this->_level.getName() != "l2dSTART") {
                            std::string name = this->_level.getName();
                            configureMapErrorText = this->_level.loadMap(name);
                            if (configureMapErrorText.length() <= 0) {
                                createGridLines();
                                configWindowVisible = false;
                                startStatusTimer("Configurations saved successfully!", 200);
                            }
                        }
                        else {
                            configWindowVisible = false;
                            startStatusTimer("Configurations saved successfully!", 200);
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
            ImGui::Text("%s", configureMapErrorText.c_str());

            spriteScaleX = std::max(0.0f, spriteScaleX);
            spriteScaleY = std::max(0.0f, spriteScaleY);
            tileScaleX = std::max(0.0f, tileScaleX);
            tileScaleY = std::max(0.0f, tileScaleY);
            screenSizeX = std::max(0, screenSizeX);
            screenSizeY = std::max(0, screenSizeY);
            cameraPanAmount = std::max(0.0f, cameraPanAmount);

            ImGui::End();
            loaded = true;
        }

        //About box
        if (aboutBoxVisible) {
            ImGui::SetNextWindowSize(ImVec2(300, 130));
            ImGui::Begin("About Lime2D", nullptr, ImVec2(300, 130), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_ShowBorders);
            ImGui::Text("Lime2D Editor\nVersion: 1.0\n\nBy: Limeoats\nCopyright \u00a9 2016");
            ImGui::Separator();
            if (ImGui::Button("Close")) {
                aboutBoxVisible = false;
            }
            ImGui::End();
        }

        //Map select box
        if (mapSelectBoxVisible) {
            std::string mapSelectErrorMessage = "";
            std::stringstream ss;
            ss << l2d_internal::utils::getConfigValue("map_path") << "*";
            std::vector<const char*> mapFiles = l2d_internal::utils::getFilesInDirectory(ss.str());
            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(500, 270));
            ImGui::Begin("Select a map", nullptr, ImVec2(500, 270), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_ShowBorders);
            ImGui::Text("Select a map from the list below.");
            ImGui::Separator();
            ImGui::PushItemWidth(-1);
            ImGui::ListBox("", &mapSelectIndex, &mapFiles[0], static_cast<int>(mapFiles.size()), 10);
            ImGui::Separator();
            if (ImGui::Button("Open")) {
                //Get the name of the file
                std::vector<std::string> fullNameSplit = l2d_internal::utils::split(mapFiles[mapSelectIndex], '/');
                std::vector<std::string> fileNameSplit = l2d_internal::utils::split(fullNameSplit.back(), '.');
                mapSelectErrorMessage = this->_level.loadMap(fileNameSplit.front());
                if (mapSelectErrorMessage.length() <= 0) {
                    createGridLines();
                    mapSelectBoxVisible = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                mapSelectBoxVisible = false;
            }
            ImGui::Text("%s", mapSelectErrorMessage.c_str());
            ImGui::End();
        }

        //New map box
        if (newMapBoxVisible) {
            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(500, 240));
            static std::string newMapErrorText = "";
            ImGui::Begin("New map properties", nullptr, ImVec2(500, 240), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_ShowBorders);
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
            ImGui::Text("Tile size");
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
                        createGridLines();
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
            ImGui::Text("%s", newMapErrorText.c_str());
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
                this->_mainHasFocus = false;
                if (ImGui::MenuItem("Configure")) {
                    configWindowVisible = true;
                }
                if (ImGui::MenuItem("Exit")) {
                    this->_enabled = false; //TODO: do you want to save?
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Edit")) {
                this->_mainHasFocus = false;
                if (ImGui::MenuItem("Undo", "U", false, !this->_level.isUndoListEmpty())) {
                    this->_mainHasFocus = false;
                    this->_level.undo();
                }
                if (ImGui::MenuItem("Redo", "R", false, !this->_level.isRedoListEmpty())) {
                    this->_mainHasFocus = false;
                    this->_level.redo();
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("View")) {
                this->_mainHasFocus = false;
                if (ImGui::Checkbox("Map Editor", &cbMapEditor)) {
                    cbAnimationEditor = false;
                    if (cbMapEditor) {
                        this->_currentFeature = l2d_internal::Features::Map;
                        this->_currentMapEditorMode = l2d_internal::MapEditorMode::Object;
                    }
                    else {
                        this->_currentFeature = l2d_internal::Features::None;
                        currentFeature = "Lime2D";
                    }
                }
                if (ImGui::Checkbox("Animation Editor", &cbAnimationEditor)) {
                    cbMapEditor = false;
                    if (cbAnimationEditor) {
                        this->_currentFeature = l2d_internal::Features::Animation;
                        currentFeature = "Animation Editor";
                    }
                    else {
                        this->_currentFeature = l2d_internal::Features::None;
                        currentFeature = "Lime2D";
                    }
                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Map", cbMapEditor)) {
                this->_mainHasFocus = false;
                if (ImGui::MenuItem("New map")) {
                    newMapBoxVisible = true;
                    this->_mainHasFocus = false;
                }
                if (ImGui::MenuItem("Load map")) {
                    mapSelectBoxVisible = true;
                    this->_mainHasFocus = false;
                }
                if (this->_level.getName() != "l2dSTART") {
                    if (ImGui::MenuItem("Save map")) {
                        this->_level.saveMap(this->_level.getName());
                        startStatusTimer("Map saved successfully!", 200);
                        this->_mainHasFocus = false;
                    }
                }
                if (this->_level.getName() != "l2dSTART") {
                    ImGui::Separator();
                    if (ImGui::BeginMenu("Add")) {
                        if (ImGui::BeginMenu("Light")) {
                            if (ImGui::MenuItem("Ambient light") && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
                                lightEditorWindowVisible = true;
                                selectedLightType = l2d_internal::LightType::Ambient;
                            }
                            if (ImGui::MenuItem("Point light") && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
                                lightEditorWindowVisible = true;
                                selectedLightType = l2d_internal::LightType::Point;
                            }
                            ImGui::EndMenu();
                        }
                        if (ImGui::BeginMenu("Object")) {
                            if (ImGui::BeginMenu("Draw shape")) {
                                if (ImGui::MenuItem("Rectangle") && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
                                    this->_currentDrawShape = l2d_internal::DrawShapes::Rectangle;
                                }
                                ImGui::EndMenu();
                            }
                            ImGui::EndMenu();
                        }
                        ImGui::EndMenu();
                    }
                    if (ImGui::Checkbox("Show grid lines    G", &cbShowGridLines)) {
                        this->_showGridLines = cbShowGridLines;
                    }
                    if (ImGui::Checkbox("Show entity list   E", &cbShowEntityList) && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
                        this->_showEntityList = cbShowEntityList;
                    }
                    this->_mainHasFocus = false;
                }
                ImGui::Separator();
                if (ImGui::MenuItem("Toggle mode", "M")) {
                    this->_currentMapEditorMode = this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object ?
                                                  l2d_internal::MapEditorMode::Tile : l2d_internal::MapEditorMode::Object;

                }
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Animation", cbAnimationEditor)) {
                if (ImGui::BeginMenu("New")) {
                    if (ImGui::MenuItem("Animated sprite")) {
                        newAnimatedSpriteWindowVisible = true;
                        newAnimationWindowVisible = false;
                        removeAnimationWindowVisible = false;
                        this->_mainHasFocus = false;
                    }
                    if (ImGui::MenuItem("Animation", nullptr, false, animationSpriteSelectIndex > -1)) {
                        //Open new animation window
                        newAnimationWindowVisible = true;
                        newAnimatedSpriteWindowVisible = false;
                        removeAnimationWindowVisible = false;
                        this->_mainHasFocus = false;
                    }
                    this->_mainHasFocus = false;
                    ImGui::EndMenu();
                }
                if (ImGui::MenuItem("Delete selected animation", nullptr, false, animationSelectIndex > -1)) {
                    //Open remove animation window
                    removeAnimationWindowVisible = true;
                    newAnimationWindowVisible = false;
                    newAnimatedSpriteWindowVisible = false;
                    this->_mainHasFocus = false;
                }
                this->_mainHasFocus = false;
                ImGui::EndMenu();
            }
            if (ImGui::BeginMenu("Help")) {
                this->_mainHasFocus = false;
                if (ImGui::MenuItem("About Lime2D")) {
                    aboutBoxVisible = true;
                }
                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if (this->_level.getName() != "l2dSTART" && this->_currentFeature == l2d_internal::Features::Map) {
            //Clicking on a tile normally
            if (tileHasBeenSelected && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Tile) {
                sf::Vector2f drawingMousePos = this->_window->mapPixelToCoords(sf::Vector2i(
                        sf::Mouse::getPosition(*this->_window).x + static_cast<int>(this->_graphics->getView().getViewport().left),
                        sf::Mouse::getPosition(*this->_window).y + static_cast<int>(this->_graphics->getView().getViewport().top)));
                drawingMousePos = sf::Vector2f(std::floor(drawingMousePos.x), std::floor(drawingMousePos.y));
                if (ImGui::IsMouseDown(0) && this->_mainHasFocus) {
                    sf::Vector2f tilePos(
                            (drawingMousePos.x - ((int) drawingMousePos.x % (int) (this->_level.getTileSize().x * std::stof(
                                    l2d_internal::utils::getConfigValue("tile_scale_x"))))) / this->_level.getTileSize().x /
                            (int) std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")) + 1,
                            (drawingMousePos.y - ((int) drawingMousePos.y % (int) (this->_level.getTileSize().y * std::stof(
                                    l2d_internal::utils::getConfigValue("tile_scale_y"))))) / this->_level.getTileSize().y /
                            (int) std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")) + 1);
                    if (tilePos.x >= 1 && tilePos.y >= 1 && tilePos.x <= this->_level.getSize().x && tilePos.y <= this->_level.getSize().y) {
                        if (this->_eraserActive) {
                            this->_level.removeTile(selectedTileLayer, sf::Vector2f((tilePos.x - 1) * this->_level.getTileSize().x * std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")),
                                                                                    (tilePos.y - 1) * this->_level.getTileSize().y * std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))));
                        }
                        else {
                            this->_level.updateTile(selectedTilesetPath, selectedTilesetSize, selectedTileSrcPos, tilePos, 1, selectedTileLayer);
                        }
                    }
                }
            }

            //Tileset open/close event
            tilesetWindowVisible = this->_tilesetEnabled;

            //Tileset window
            if (tilesetWindowVisible && this->_currentFeature == l2d_internal::Features::Map &&
                    this->_currentMapEditorMode == l2d_internal::MapEditorMode::Tile) {
                static int tilesetComboIndex = -1;
                static bool showTilesetImage = false;
                static sf::Texture tilesetTexture;
                static sf::Vector2f tilesetViewSize(384, 128);
                static sf::Vector2f selectedTilePos(0,0);

                float tw = (tilesetViewSize.x * this->_level.getTileSize().x) / tilesetTexture.getSize().x;
                float th = (tilesetViewSize.y * this->_level.getTileSize().y) / tilesetTexture.getSize().y;
                static float dx = 0, dy = 0;

                ImGui::SetNextWindowPosCenter();
                ImGui::SetNextWindowSize(ImVec2(540, 300));
                ImGui::Begin("Tilesets", nullptr, ImVec2(500, 300), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);
                std::stringstream ss;
                ss << l2d_internal::utils::getConfigValue("tileset_path") << "*";
                std::vector<const char*> tilesetFiles = l2d_internal::utils::getFilesInDirectory(ss.str());
                ImGui::PushItemWidth(400);
                if (ImGui::Combo("Select tileset", &tilesetComboIndex, &tilesetFiles[0], static_cast<int>(tilesetFiles.size()))) {
                    showTilesetImage = true;
                    selectedTilesetPath = tilesetFiles[tilesetComboIndex];
                    selectedTileLayer = 1;
                    selectedTileSrcPos = sf::Vector2i(0,0);
                }
                ImGui::PopItemWidth();
                if (tilesetComboIndex > -1) {

                    ImGui::PushItemWidth(80);
                    if (ImGui::Button("+", ImVec2(20, 20))) {
                        tilesetViewSize *= 1.2f; //TODO: MAKE THIS 1.2 VALUE CONFIGURABLE
                        tw = (tilesetViewSize.x * this->_level.getTileSize().x) / tilesetTexture.getSize().x;
                        th = (tilesetViewSize.y * this->_level.getTileSize().y) / tilesetTexture.getSize().y;
                        dx = 0;
                        dy = 0;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("-", ImVec2(20, 20))) {
                        tilesetViewSize /= 1.2f;
                        tw = (tilesetViewSize.x * this->_level.getTileSize().x) / tilesetTexture.getSize().x;
                        th = (tilesetViewSize.y * this->_level.getTileSize().y) / tilesetTexture.getSize().y;
                        dx = 0;
                        dy = 0;
                    }
                    ImGui::PopItemWidth();
                    ImGui::SameLine();
                    ImGui::Text("      ");
                    ImGui::SameLine();


                    ImGui::PushItemWidth(84);
                    ImGui::PushID("nLayer");
                    ImGui::InputInt("Layer", &selectedTileLayer, 1);
                    ImGui::PopID();
                    ImGui::PopItemWidth();

                    ImGui::SameLine();
                    ImGui::Text("      ");
                    ImGui::SameLine();

                    ImGui::PushItemWidth(84);
                    ImGui::PushID("nEraser");
                    ImGui::Checkbox("Eraser", &this->_eraserActive);
                    ImGui::PopID();
                    ImGui::PopItemWidth();
                }
                if (showTilesetImage) {
                    ImGui::BeginChild("tilesetChildArea", ImVec2(500, 200), true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);

                    auto pos = ImGui::GetCursorScreenPos();

                    tilesetTexture = this->_graphics->loadImage(tilesetFiles[tilesetComboIndex]);
                    selectedTilesetSize = sf::Vector2i(tilesetTexture.getSize());
                    ImGui::Image(tilesetTexture, tilesetViewSize);
                    //Tileset grid
                    ImGui::SetItemAllowOverlap();

                    for (unsigned int i = 0; i < (tilesetTexture.getSize().x / this->_level.getTileSize().x) + 1; ++i) {
                        ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x + (i * tw), pos.y),
                                                            ImVec2(pos.x + (i * tw), pos.y + tilesetViewSize.y), ImColor(255,255,255,255));
                    }
                    for (unsigned int i = 0; i < (tilesetTexture.getSize().y / this->_level.getTileSize().y) + 1; ++i) {
                        ImGui::GetWindowDrawList()->AddLine(ImVec2(pos.x, pos.y + (i * th)),
                                                            ImVec2(pos.x + tilesetViewSize.x, pos.y + (i * th)), ImColor(255,255,255,255));
                    }

                    //Tileset selected item
                    //We're going to use lines for this :/
                    if (!this->_eraserActive) {
                        ImGui::GetWindowDrawList()->AddLine(
                                ImVec2(pos.x + selectedTilePos.x, pos.y + selectedTilePos.y),
                                ImVec2((pos.x + selectedTilePos.x + tw), pos.y + selectedTilePos.y),
                                ImColor(255, 0, 0, 255), 2.0f); //Top
                        ImGui::GetWindowDrawList()->AddLine(
                                ImVec2(pos.x + selectedTilePos.x, pos.y + selectedTilePos.y),
                                ImVec2(pos.x + selectedTilePos.x, (pos.y + selectedTilePos.y + th)),
                                ImColor(255, 0, 0, 255), 2.0f); //Left
                        ImGui::GetWindowDrawList()->AddLine(
                                ImVec2(pos.x + selectedTilePos.x, (pos.y + selectedTilePos.y + th)),
                                ImVec2((pos.x + selectedTilePos.x + tw), (pos.y + selectedTilePos.y + th)),
                                ImColor(255, 0, 0, 255), 2.0f); //Bottom
                        ImGui::GetWindowDrawList()->AddLine(
                                ImVec2((pos.x + selectedTilePos.x + tw), pos.y + selectedTilePos.y),
                                ImVec2((pos.x + selectedTilePos.x + tw), (pos.y + selectedTilePos.y + th)),
                                ImColor(255, 0, 0, 255), 2.0f); //Right
                    }

                    //Click event on the tileset
                    if (ImGui::IsMouseClicked(0) && ImGui::IsWindowFocused()) {
                        ImVec2 mPos = ImGui::GetMousePos();
                        dx = mPos.x - pos.x;
                        dy = mPos.y - pos.y;
                    }

                    selectedTilePos = ImVec2(tw * (static_cast<int>(dx) / static_cast<int>(tw)), th * (static_cast<int>(dy) / static_cast<int>(th)));
                    tileHasBeenSelected = true;

                    selectedTileSrcPos = sf::Vector2i((static_cast<int>(dx) / static_cast<int>(tw)) * this->_level.getTileSize().x, (static_cast<int>(dy) / static_cast<int>(th)) * this->_level.getTileSize().y);

                    ImGui::EndChild();
                }
                ImGui::End();
            }

        }

        if (lightEditorWindowVisible && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(300, 400));
            ImGui::Begin("Light editor", nullptr, ImVec2(300, 400), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);
            if (selectedLightType == l2d_internal::LightType::Ambient) {
                ImGui::Text("Ambient light editor");
                ImGui::Separator();
                static ImVec4 col = ImVec4(this->_level.getAmbientColor());
                ImGui::ColorPicker3("", &col.x);
                ImVec4 col2 = {col.x, col.y, col.z, col.w};
                this->_level.setAmbientColor(col2);
                ImGui::Separator();
                static float intensity = this->_level.getAmbientIntensity();
                ImGui::SliderFloat("Intensity", &intensity, 0, 10, "%.2f");
                this->_level.setAmbientIntensity(intensity);
                ImGui::Separator();
                ImGui::Spacing();
                if (ImGui::Button("All done")) {
                    lightEditorWindowVisible = false;
                }
            }
            else if (selectedLightType == l2d_internal::LightType::Point) {
                ImGui::Text("Point lights coming soon in version 2.0!");
                ImGui::Separator();
                if (ImGui::Button("Okay")) {
                    lightEditorWindowVisible = false;
                }
            }
            ImGui::End();
        }

        if (shapeColorWindowVisible && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(300, 400));
            ImGui::Begin("Shape color editor", nullptr, ImVec2(300, 400), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);
            ImGui::Text("Shape color editor");
            ImGui::Separator();
            ImGui::ColorPicker3("", (float*)&selectedEntityColor);
            ImGui::Separator();
            if (ImGui::Button("Close")) {
                shapeColorWindowVisible = false;
            }
            ImGui::End();
        }

        /*
         * Entity list
         */
        if (cbShowEntityList && this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object) {
            static bool loaded = false;
            auto fillObjectSection = [&](l2d_internal::ObjectTypes objectType, std::string strObjectType) {
                std::vector<std::shared_ptr<l2d_internal::Shape>> otherShapes;
                for (std::shared_ptr<l2d_internal::Shape> shape : this->_level.getShapeList()) {
                    if (shape.get()->getObjectType() == objectType) {
                        otherShapes.push_back(shape);
                    }
                }
                for (unsigned int i = 0; i < otherShapes.size(); ++i) {
                    std::string strId = strObjectType + std::to_string(i);
                    ImGui::PushID(strId.c_str());
                    if (ImGui::Selectable(otherShapes[i].get()->getName().c_str())) {
                        loaded = false;
                        //Figure out what type of shape and fill the appropriate variable
                        //ALSO, SET THE REST TO NULL.
                        auto s = std::dynamic_pointer_cast<l2d_internal::Rectangle>(otherShapes[i]);
                        if (s != nullptr) {
                            selectedEntityRectangle = s;
                            originalSelectedEntityRectangle = std::make_shared<l2d_internal::Rectangle>(s->getName(), s->getColor(), s->getObjectType(), s->getRectangle());
                        }
                        selectedEntityColor = otherShapes[i]->getColor();
                        selectedEntitySelectedObjectTypeIndex = static_cast<int>(otherShapes[i]->getObjectType());

                        showEntityProperties = true;
                    }
                    ImGui::PopID();
                }
            };
            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(500, 450));
            ImGui::Begin("Entity list", nullptr, ImVec2(500, 300), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);
            ImGui::BeginChild("list", ImVec2(482, 170), false, ImGuiWindowFlags_ShowBorders);
            if (ImGui::TreeNode("Entities")) {
                if (ImGui::TreeNode("Objects")) {
                    if (ImGui::TreeNode("Collision")) {
                        fillObjectSection(l2d_internal::ObjectTypes::Collision, "Collision");
                        ImGui::TreePop();
                    }
                    if (ImGui::TreeNode("Other")) {
                        fillObjectSection(l2d_internal::ObjectTypes::Other, "Other");
                        ImGui::TreePop();
                    }
                    ImGui::TreePop();
                }
                if (ImGui::TreeNode("Lights")) {
                    if (this->_level.getAmbientColor() != sf::Color::White && this->_level.getAmbientIntensity() != 1.0f) {
                        if (ImGui::Selectable("Ambient light!")) {
                            selectedLightType = l2d_internal::LightType::Ambient;
                            lightEditorWindowVisible = true;
                        }
                    }
                    ImGui::TreePop();
                }
                ImGui::TreePop();
            }
            ImGui::EndChild();
            ImGui::Separator();
            ImGui::BeginChild("properties", ImVec2(482, 234), false, ImGuiWindowFlags_ShowBorders);
            if (showEntityProperties) {
                ImGui::PushID("SelectedEntityName");
                static char name[500] = "";
                if (!loaded) {
                    strcpy(name, selectedEntityRectangle != nullptr ? selectedEntityRectangle->getName().c_str() : "");
                }
                ImGui::PushItemWidth(200);
                ImGui::InputText("Name", name, sizeof(name));
                ImGui::PopItemWidth();
                ImGui::PopID();

                ImGui::Separator();

                ImGui::PushID("SelectedEntityColor");
                ImGui::PushItemWidth(200);
                if (ImGui::ColorButton(selectedEntityColor, false, true)) {
                    shapeColorWindowVisible = true;
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::Text("Color");
                ImGui::PopID();

                ImGui::Separator();

                ImGui::PushID("SelectedEntityObjectType");
                std::vector<const char*> objectTypeList = l2d_internal::utils::getObjectTypesForList();
                if (!loaded) {
                    selectedEntitySelectedObjectTypeIndex = static_cast<int>(selectedEntityRectangle->getObjectType());
                }
                ImGui::Combo("Object type", &selectedEntitySelectedObjectTypeIndex, &objectTypeList[0], static_cast<int>(objectTypeList.size()));
                ImGui::PopID();

                if (selectedEntityRectangle != nullptr) {
                    ImGui::Separator();
                    static float rotation = 0.0f;
                    ImGui::SliderFloat("Rotation", &rotation, 0.0f, 360.0f);
                    selectedEntityRectangle->setRotation(rotation);
                }

                if (ImGui::Button("Update")) {
                    if (selectedEntityRectangle != nullptr) {
                        selectedEntityRectangle->setName(name);
                        selectedEntityRectangle->setColor(selectedEntityColor);
                        selectedEntityRectangle->setObjectType(static_cast<l2d_internal::ObjectTypes>(selectedEntitySelectedObjectTypeIndex));
                        this->_level.updateShape(originalSelectedEntityRectangle, selectedEntityRectangle);
                        this->_level.saveMap(this->_level.getName());
                        startStatusTimer("Rectangle saved successfully!", 200);
                    }
                }
                loaded = true;

            }
            ImGui::EndChild();
            ImGui::End();
        }



        //Set the current feature to the correct map mode if the selected feature is map
        if (this->_currentFeature == l2d_internal::Features::Map) {
            std::stringstream ss;

            ss << "Map Editor - " << (this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object ? "Object mode" : "Tile mode")
               << (this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object && this->_currentDrawShape == l2d_internal::DrawShapes::Rectangle ? " - Rectangle" :
                   this->_currentMapEditorMode == l2d_internal::MapEditorMode::Object && this->_currentDrawShape == l2d_internal::DrawShapes::None ? " - Select" : "");
            currentFeature = ss.str();
        }


        /*
         * Animation editor
         */

        //Add a new animated sprite
        if (cbAnimationEditor && newAnimatedSpriteWindowVisible) {
            static std::string newSpriteErrorMessage = "";

            std::stringstream ss;
            ss << l2d_internal::utils::getConfigValue("sprite_path") << "*";
            std::vector<const char*> spriteList = l2d_internal::utils::getFilesInDirectory(ss.str());

            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(380, 160));
            ImGui::Begin("Create new animated sprite", nullptr, ImVec2(380, 200), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);

            ImGui::PushItemWidth(364);
            ImGui::PushID("NewAnimatedSpriteName");
            ImGui::Text("Sprite name");
            static char newSpriteName[1000] = "";
            ImGui::InputText("", newSpriteName, sizeof(newSpriteName), ImGuiInputTextFlags_CallbackCharFilter, FilterLuaKeyInput);
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Separator();

            ImGui::PushItemWidth(364);
            ImGui::PushID("NewAnimationSpriteSheet");
            ImGui::Text("Sprite sheet");
            ImGui::Combo("", &spritesheetSelectIndex, &spriteList[0], static_cast<int>(spriteList.size()));
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Separator();

            if (ImGui::Button("Create")) {
                if (strlen(newSpriteName) <= 0) {
                    newSpriteErrorMessage = "You must enter a name for the sprite!";
                }
                else if (spritesheetSelectIndex < 0) {
                    newSpriteErrorMessage = "You must select a sprite sheet for the new sprite!";
                }
                else {
                    newSpriteErrorMessage = "";
                    l2d_internal::utils::createNewAnimationFile(newSpriteName, spriteList[spritesheetSelectIndex]);
                    newAnimatedSpriteWindowVisible = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                strcpy(newSpriteName, "");
                newSpriteErrorMessage = "";
                newAnimatedSpriteWindowVisible = false;
            }
            ImGui::Text("%s", newSpriteErrorMessage.c_str());
            ImGui::End();
        }

        //Add a new animation to the currently loaded animated sprite
        if (cbAnimationEditor && newAnimationWindowVisible) {
            static std::string newAnimationErrorMessage = "";

            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(380, 160));
            ImGui::Begin("Create new animation", nullptr, ImVec2(380, 200), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);

            ImGui::PushItemWidth(364);
            ImGui::PushID("NewAnimationName");
            ImGui::Text("Animation name");
            static char newAnimationNameArray[1000] = "";
            ImGui::InputText("", newAnimationNameArray, sizeof(newAnimationNameArray), ImGuiInputTextFlags_CallbackCharFilter, FilterLuaKeyInput);
            ImGui::PopID();
            ImGui::PopItemWidth();
            ImGui::Separator();

            static auto animationAlreadyExists = [](char* name)->bool {
                static std::unique_ptr<l2d_internal::LuaScript> script = std::make_unique<l2d_internal::LuaScript>(selectedAnimationFileName);
                auto keys = script->getTableKeys("animations.list");
                for (auto &key : keys) {
                    if (key == name) {
                        return true;
                    }
                }
                return false;
            };

            if (ImGui::Button("Create")) {
                if (strlen(newAnimationNameArray) <= 0) {
                    newAnimationErrorMessage = "You must enter a name for the animation!";
                }
                else if (animationAlreadyExists(newAnimationNameArray)) {
                    newAnimationErrorMessage = "An animation with that name already exists!";
                }
                else {
                    newAnimationErrorMessage = "";
                    l2d_internal::utils::addNewAnimationToAnimationFile(selectedAnimationFileName, newAnimationNameArray);
                    animationSpriteSelectIndex = -1;
                    animationSelectIndex = -1;
                    strcpy(newAnimationNameArray, "");
                    newAnimationWindowVisible = false;
                }
            }
            ImGui::SameLine();
            if (ImGui::Button("Cancel")) {
                strcpy(newAnimationNameArray, "");
                newAnimationErrorMessage = "";
                newAnimationWindowVisible = false;
            }
            ImGui::Text("%s", newAnimationErrorMessage.c_str());



            ImGui::End();
        }

        //Remove the selected animation
        if (cbAnimationEditor && removeAnimationWindowVisible) {
            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(280, 100));
            ImGui::Begin("Remove selected animation", nullptr, ImVec2(280, 100), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);
            std::string txt = "Are you sure you want to delete \nthe selected animation?\n\"" + selectedAnimationName + "\"";
            ImGui::Text("%s", txt.c_str());
            ImGui::Separator();
            if (ImGui::Button("Yes")) {
                l2d_internal::utils::removeAnimationFromAnimationFile(selectedAnimationFileName, selectedAnimationName);
                selectedAnimationName = "";
                removeAnimationWindowVisible = false;
                animationSelectIndex = -1;
                animationSpriteSelectIndex = -1; //Reset the whole sprite so the animation names will be updated from the Lua file
                startStatusTimer("Animation deleted successfully", 200);
            }
            ImGui::SameLine();
            if (ImGui::Button("No ")) {
                removeAnimationWindowVisible = false;
            }
            ImGui::End();
        }

        if (cbAnimationEditor) {
            static ImVec2 spriteDisplaySize;
            static std::unique_ptr<l2d_internal::LuaScript> script = nullptr;
            static std::shared_ptr<l2d_internal::AnimatedSprite> sprite = nullptr;
            static int frames;
            static std::string animationName;
            static std::string originalAnimationName;
            static std::string animationDescription;
            static std::string animationPath;
            static sf::Vector2i srcPos;
            static sf::Vector2i size;
            static sf::Vector2i offset;
            static float timeToUpdate;

            ImGui::SetNextWindowPosCenter();
            ImGui::SetNextWindowSize(ImVec2(this->_window->getSize().x - 20, this->_window->getSize().y - 80));
            ImGui::Begin("Animation editor", nullptr, ImVec2(this->_window->getSize().x - 20, this->_window->getSize().y - 80), 100.0f, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_ShowBorders);

            std::stringstream ss;
            ss << l2d_internal::utils::getConfigValue("animation_path") << "*";
            std::vector<const char*> existingAnimationSprites = l2d_internal::utils::getFilesInDirectory(ss.str());

            ImGui::PushItemWidth(500);
            if (ImGui::Combo("Choose an animated sprite", &animationSpriteSelectIndex, &existingAnimationSprites[0], static_cast<int>(existingAnimationSprites.size()))) {
                animationSelectIndex = -1;
                selectedAnimationFileName = existingAnimationSprites[animationSpriteSelectIndex];
            }
            ImGui::PopItemWidth();
            ImGui::Separator();

            //Based on animationSpriteSelectIndex, parse the lua file and get the list of animations
            if (animationSpriteSelectIndex > -1) {
                script = std::make_unique<l2d_internal::LuaScript>(existingAnimationSprites[animationSpriteSelectIndex]);
                std::vector<std::string> existingAnimationsStrings = script.get()->getTableKeys("animations.list");
                std::vector<const char*> existingAnimations;
                for (auto &str : existingAnimationsStrings) {
                    existingAnimations.push_back(str.c_str());
                }

                static auto setAnimationFromScript = [&]() {
                    frames = script.get()->get<int>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".frames");
                    animationName = script.get()->get<std::string>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".name");
                    animationDescription = script.get()->get<std::string>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".description");
                    animationPath = script.get()->get<std::string>("animations.sprite_path");
                    srcPos = sf::Vector2i(script.get()->get<int>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".src_pos.x"), script.get()->get<int>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".src_pos.y"));
                    size = sf::Vector2i(script.get()->get<int>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".size.w"), script.get()->get<int>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".size.h"));
                    offset = sf::Vector2i(script.get()->get<int>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".offset.x"), script.get()->get<int>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".offset.y"));
                    timeToUpdate = script.get()->get<float>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".time_to_update");
                };
                static bool loaded = false;
                ImGui::PushItemWidth(500);
                if (ImGui::Combo("Choose an animation", &animationSelectIndex, &existingAnimations[0],
                                 static_cast<int>(existingAnimations.size()))) {
                    loaded = false;
                    setAnimationFromScript();
                    selectedAnimationName = animationName;
                    originalAnimationName = script.get()->get<std::string>("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".name");
                    sprite = std::make_shared<l2d_internal::AnimatedSprite>(
                            this->_graphics, animationPath, srcPos, size, sf::Vector2f(0,0), timeToUpdate);
                    sprite->addAnimation(frames, srcPos, animationName, size, offset);
                    sprite->playAnimation(animationName);

                    spriteDisplaySize = ImVec2(size.x, size.y);
                }
                ImGui::PopItemWidth();
                ImGui::Separator();

                if (animationSelectIndex > -1) {
                    sprite->update(t.asSeconds());
                    sprite->updateAnimation(frames, srcPos, animationName, size, offset, timeToUpdate <= 0 ? 0 : timeToUpdate);

                    ImGui::Image(sprite->getSprite(), spriteDisplaySize);
                    ImGui::SameLine();
                    //Zoom buttons
                    if (ImGui::Button("-", ImVec2(20, 20))) {
                        spriteDisplaySize.x /= 1.2f;
                        spriteDisplaySize.y /= 1.2f;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("+", ImVec2(20, 20))) {
                        spriteDisplaySize.x *= 1.2f;
                        spriteDisplaySize.y *= 1.2f;
                    }

                    ImGui::Separator();

                    ss.str("");
                    ss << l2d_internal::utils::getConfigValue("sprite_path") << "*";
                    std::vector<const char*> spriteList = l2d_internal::utils::getFilesInDirectory(ss.str());
                    std::string p = script.get()->get<std::string>(
                            "animations.sprite_path");
                    for (unsigned int i = 0; i < spriteList.size(); ++i) {
                        if (strcmp(spriteList[i], p.c_str()) == 0) {
                            spritesheetSelectIndex = i;
                        }
                    }



                    ImGui::PushItemWidth(500);
                    ImGui::PushID("AnimationName");
                    static char animationNameArray[500] = "";
                    if (!loaded) {
                        strncpy(animationNameArray, animationName.c_str(), sizeof(animationNameArray));
                    }
                    ImGui::InputText("Name", animationNameArray, sizeof(animationNameArray), ImGuiInputTextFlags_CallbackCharFilter, FilterLuaKeyInput);
                    ImGui::PopID();
                    ImGui::PopItemWidth();
                    ImGui::Separator();

                    ImGui::PushItemWidth(500);
                    ImGui::PushID("AnimationDescription");
                    static char animationDescriptionArray[1000] = "";
                    if (!loaded) {
                        strncpy(animationDescriptionArray, animationDescription.c_str(), sizeof(animationDescriptionArray));
                    }
                    ImGui::InputText("Description", animationDescriptionArray, sizeof(animationDescriptionArray));
                    ImGui::PopID();
                    ImGui::PopItemWidth();
                    ImGui::Separator();

                    ImGui::PushItemWidth(500);
                    ImGui::PushID("AnimationSpriteSheet");
                    ImGui::Combo("Sprite sheet", &spritesheetSelectIndex, &spriteList[0], static_cast<int>(spriteList.size()));
                    ImGui::PopID();
                    ImGui::PopItemWidth();
                    ImGui::Separator();

                    ImGui::PushItemWidth(100);
                    ImGui::PushID("AnimationSrcPos");
                    ImGui::Text("Source position");
                    ImGui::InputInt("x", &srcPos.x, 1);
                    ImGui::InputInt("y", &srcPos.y, 1);
                    ImGui::Separator();
                    ImGui::PopID();

                    ImGui::PushItemWidth(100);
                    ImGui::PushID("AnimationSize");
                    ImGui::Text("Size");
                    if (ImGui::InputInt("w", &size.x, 1)) {
                        spriteDisplaySize = ImVec2(size.x, size.y);
                    }
                    if (ImGui::InputInt("h", &size.y, 1)) {
                        spriteDisplaySize = ImVec2(size.x, size.y);
                    }
                    ImGui::Separator();
                    ImGui::PopID();

                    ImGui::PushItemWidth(100);
                    ImGui::PushID("AnimationOffset");
                    ImGui::Text("Offset");
                    ImGui::InputInt("x", &offset.x, 1);
                    ImGui::InputInt("y", &offset.y, 1);
                    ImGui::Separator();
                    ImGui::PopID();

                    ImGui::PushItemWidth(100);
                    ImGui::PushID("AnimationEditorTimeToUpdate");
                    ImGui::Text("Time to update");
                    ImGui::InputFloat("", &timeToUpdate, 0.01f, 1.0f, 2);
                    ImGui::Separator();
                    ImGui::PopID();

                    ImGui::PushItemWidth(100);
                    ImGui::PushID("AnimationEditorFrames");
                    ImGui::Text("Frames");
                    ImGui::InputInt("", &frames, 1);
                    ImGui::Separator();
                    ImGui::PopID();

                    if (ImGui::Button("Save")) {
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".name", animationNameArray);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".description", animationDescriptionArray);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".frames", frames);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".src_pos.x", srcPos.x);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".src_pos.y", srcPos.y);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".size.w", size.x);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".size.h", size.y);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".offset.x", offset.x);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".offset.y", offset.y);
                        script.get()->lua_set("animations.list." + existingAnimationsStrings[animationSelectIndex] + ".time_to_update", timeToUpdate);
                        script.get()->lua_set("animations.sprite_path", animationPath);
                        script.get()->lua_save("animations");
                        if (originalAnimationName != animationNameArray) {
                            script.get()->updateKeyName(originalAnimationName, std::string(animationNameArray));
                            originalAnimationName = std::string(animationNameArray); //Update originalAnimationName in case the name changes again before reloading animation
                        }
                        startStatusTimer("Animation saved successfully!", 200);
                    }
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel")) {
                        setAnimationFromScript();
                    }

                    loaded = true;

                    //Prevent negatives
                    srcPos.x = std::max(srcPos.x, 0);
                    srcPos.y = std::max(srcPos.y, 0);
                    size.x = std::max(size.x, 0);
                    size.y = std::max(size.y, 0);
                    timeToUpdate = std::max(timeToUpdate, 0.0f);
                    frames = std::max(frames, 1);
                }
            }
            ImGui::End();
        }

        //Status bar
        ImGui::Begin("Background", nullptr, ImGui::GetIO().DisplaySize, 0.0f,
                     ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                     ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse |
                     ImGuiWindowFlags_NoCollapse |
                     ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
                     ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus);
        ImGui::GetWindowDrawList()->AddText(ImVec2(6, this->_window->getSize().y  - 20), ImColor(1.0f, 1.0f, 1.0f, 1.0f), currentFeature.c_str());
        //Map zoom percentage
        if (this->_currentFeature == l2d_internal::Features::Map && this->_level.getName() != "l2dSTART") {
            std::stringstream ss;
            ss << std::fixed << std::setprecision(2) << this->_graphics.get()->getZoomPercentage() << "%";
            ImGui::GetWindowDrawList()->AddText(ImVec2(this->_window->getSize().x - 80, this->_window->getSize().y - 20),
                                                ImColor(1.0f, 1.0f, 1.0f, 1.0f), ss.str().c_str());
        }
        if (showCurrentStatus) {
            ImGui::GetWindowDrawList()->AddText(ImVec2(180, this->_window->getSize().y - 20),
                                                ImColor(1.0f, 1.0f, 1.0f, 1.0f), currentStatus.c_str());

        }
        ImGui::End();

        //Status timer
        if (currentStatusTimer > 0) {
            currentStatusTimer -= 1;
        }
        else {
            showCurrentStatus = false;
        }

        //Updating internal classes
        this->_level.update(t.asSeconds());
        this->_graphics->update(t.asSeconds(), sf::Vector2f(this->_level.getTileSize()), this->_windowHasFocus);
    }
}

void l2d::Editor::exit() {
    ImGui::SFML::Shutdown();
}
