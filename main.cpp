#include <iostream>
#include <sstream>
#include <SFML/Graphics.hpp>

#include "imgui/imgui.h"
#include "imgui/imgui-events-SFML.h"
#include "imgui/imgui-rendering-SFML.h"

#include "lime2d.h"


using namespace std;

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Lime2D", sf::Style::Titlebar | sf::Style::Close);
    ImGui::SFML::SetRenderTarget(window);
    ImGui::SFML::InitImGuiRendering();
    ImGui::SFML::SetWindow(window);
    ImGui::SFML::InitImGuiEvents();

    while (window.isOpen()) {
            ImGui::SFML::UpdateImGui();
            ImGui::SFML::UpdateImGuiRendering();
        sf::Event event;
        while (window.pollEvent(event)) {
            ImGui::SFML::ProcessEvent(event);
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
        }
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
        window.clear();
        ImGui::Render();
        window.display();
    }
    ImGui::SFML::Shutdown();
    return 0;
}