#ifndef IMGUI_SFML_EVENTS_BACKEND
#define IMGUI_SFML_EVENTS_BACKEND
#include <vector>
#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Window/Window.hpp>

namespace ImGui
{
    namespace ImImpl
    {
        static sf::Clock ImImpl_timeElapsed;
        static bool ImImpl_mousePressed[5] = { false, false, false, false, false };
        static sf::Window* ImImpl_window;
    }
    namespace SFML
    {
        static void SetWindow(sf::Window& window){ImImpl::ImImpl_window=&window;}
        static void ProcessEvent(sf::Event &event)
        {
            switch(event.type)
            {
                case sf::Event::MouseButtonPressed:
                {
                    ImImpl::ImImpl_mousePressed[event.mouseButton.button]=true;
                    break;
                }
                case sf::Event::MouseButtonReleased:
                {
                    ImImpl::ImImpl_mousePressed[event.mouseButton.button]=false;
                    break;
                }
                case sf::Event::MouseWheelMoved:
                {
                    ImGuiIO& io = ImGui::GetIO();
                    io.MouseWheel += (float)event.mouseWheel.delta;
                    break;
                }
                case sf::Event::KeyPressed:
                {
                    ImGuiIO& io = ImGui::GetIO();
                    io.KeysDown[event.key.code]=true;
                    io.KeyCtrl=event.key.control;
                    io.KeyShift=event.key.shift;
                    break;
                }
                case sf::Event::KeyReleased:
                {
                    ImGuiIO& io = ImGui::GetIO();
                    io.KeysDown[event.key.code]=false;
                    io.KeyCtrl=event.key.control;
                    io.KeyShift=event.key.shift;
                    break;
                }
                case sf::Event::TextEntered:
                {
                    if(event.text.unicode > 0 && event.text.unicode < 0x10000)
                        ImGui::GetIO().AddInputCharacter(event.text.unicode);
                    break;
                }
                default: break;
            }
        }

        static void InitImGuiEvents()
        {
            ImGuiIO& io = ImGui::GetIO();
            io.KeyMap[ImGuiKey_Tab] = sf::Keyboard::Tab;
            io.KeyMap[ImGuiKey_LeftArrow] = sf::Keyboard::Left;
            io.KeyMap[ImGuiKey_RightArrow] = sf::Keyboard::Right;
            io.KeyMap[ImGuiKey_UpArrow] = sf::Keyboard::Up;
            io.KeyMap[ImGuiKey_DownArrow] = sf::Keyboard::Down;
            io.KeyMap[ImGuiKey_Home] = sf::Keyboard::Home;
            io.KeyMap[ImGuiKey_End] = sf::Keyboard::End;
            io.KeyMap[ImGuiKey_Delete] = sf::Keyboard::Delete;
            io.KeyMap[ImGuiKey_Backspace] = sf::Keyboard::BackSpace;
            io.KeyMap[ImGuiKey_Enter] = sf::Keyboard::Return;
            io.KeyMap[ImGuiKey_Escape] = sf::Keyboard::Escape;
            io.KeyMap[ImGuiKey_A] = sf::Keyboard::A;
            io.KeyMap[ImGuiKey_C] = sf::Keyboard::C;
            io.KeyMap[ImGuiKey_V] = sf::Keyboard::V;
            io.KeyMap[ImGuiKey_X] = sf::Keyboard::X;
            io.KeyMap[ImGuiKey_Y] = sf::Keyboard::Y;
            io.KeyMap[ImGuiKey_Z] = sf::Keyboard::Z;
            ImImpl::ImImpl_timeElapsed.restart();
        }

        static void UpdateImGui()
        {
            ImGuiIO& io = ImGui::GetIO();
            static double time = 0.0f;
            const double current_time = ImImpl::ImImpl_timeElapsed.getElapsedTime().asSeconds();
            io.DeltaTime = (float)(current_time - time);
            time = current_time;
            sf::Vector2i mouse = sf::Mouse::getPosition(*ImImpl::ImImpl_window);
            io.MousePos = ImVec2((float)mouse.x, (float)mouse.y);
            io.MouseDown[0] = ImImpl::ImImpl_mousePressed[0] || sf::Mouse::isButtonPressed(sf::Mouse::Left);
            io.MouseDown[1] = ImImpl::ImImpl_mousePressed[1] || sf::Mouse::isButtonPressed(sf::Mouse::Right);
            ImGui::NewFrame();
        }
    }
}
#endif
