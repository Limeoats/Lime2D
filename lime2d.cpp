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
#include "lime2d.h"

#include "imgui/imgui.h"
#include "imgui/imgui-events-SFML.h"
#include "imgui/imgui-rendering-SFML.h"

l2d::Text::Text(sf::Font* font, unsigned int size, sf::Vector2f position, std::string text) {
    this->_text.setFont(*font);
    this->_text.setCharacterSize(size);
    this->_text.setPosition(position);
    this->_text.setString(text);
}

sf::Text l2d::Text::getText() const {
    return this->_text;
}

void l2d::Text::setText(std::string text) {
    this->_text.setString(text);
}

l2d::Text l2d::Text::operator+=(const std::string &str) {
    sf::String ss = this->getText().getString();
    ss += str;
    this->setText(ss);
    return *this;
}

/*******************
 *  Lime2D Editor  *
 *******************/

l2d::Editor::Editor(bool enabled, sf::RenderWindow* window) {
    this->_enabled = enabled;
    ImGui::SFML::SetRenderTarget(*window);
    ImGui::SFML::InitImGuiRendering();
    ImGui::SFML::SetWindow(*window);
    ImGui::SFML::InitImGuiEvents();
}

void l2d::Editor::render() {
    ImGui::Render();
}

void l2d::Editor::update(float elapsedTime, sf::Event &event) {
    ImGui::SFML::UpdateImGui();
    ImGui::SFML::UpdateImGuiRendering();

    ImGui::SFML::ProcessEvent(event);

    if (ImGui::BeginMainMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Exit")) {
                std::cout << "Exiting!" << std::endl;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
//        ImGui::Begin("Map Editor");
//        ImGui::Text("The Lime2D Map Editor is still under development!");
//        ImGui::Button("Okay!");
//        ImGui::End();
//        ImGui::Begin("Animation Editor");
//        ImGui::Text("The Lime2D Animation Editor is still under development!");
//        ImGui::Button("Okay!");
//        ImGui::End();
    ImGui::Begin("Background", nullptr, ImGui::GetIO().DisplaySize, 0.0f, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoBringToFrontOnFocus );
    ImGui::GetWindowDrawList()->AddText( ImVec2(10,30), ImColor(1.0f,1.0f,1.0f,1.0f), "Map Editor" );
    ImGui::End();
}

void l2d::Editor::exit() {
    ImGui::SFML::Shutdown();
}

