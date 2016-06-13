/*
 * main.cpp
 * By: Mark Guerra
 * Created on 6/8/2016
 */

#include <iostream>
#include <SFML/Graphics.hpp>

#include "lime2d.h"
#include "lime2d_config.h"


using namespace std;

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Lime2D", sf::Style::Titlebar | sf::Style::Close);

    l2d::Editor editor(false, &window);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
            else if (event.type == sf::Event::KeyReleased && event.key.code == l2d::Config::ShowHide) {
                editor.toggle();
            }
        }
        editor.update(1.0f, event);
        window.clear();
        editor.render();
        window.display();
    }
    editor.exit();
    return 0;
}