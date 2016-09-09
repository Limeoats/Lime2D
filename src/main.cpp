/*!
 * Main.cpp
 * By: Mark Guerra
 * Created on: 6/8/2016
 * Copyright (c) 2016 Limeoats
 */

#include <iostream>
#include <SFML/Graphics.hpp>

#include "lime2d.h"

using namespace std;

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Lime2D", sf::Style::Titlebar | sf::Style::Close);
    sf::Image img;
    img.loadFromFile("content/sprites/mstile-310x310.png");
    window.setIcon(img.getSize().x, img.getSize().y, img.getPixelsPtr());
    window.setVerticalSyncEnabled(true);
    l2d::Editor editor(false, &window);
    sf::Clock timer;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            editor.processEvent(event);
            if (event.type == sf::Event::Closed || sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
            }
            else if (event.type == sf::Event::TextEntered && event.text.unicode == '=') {
                editor.toggle();
            }
        }
        editor.update(timer.restart());
        window.clear();
        editor.render();
        window.display();
    }
    editor.exit();
    return 0;
}
