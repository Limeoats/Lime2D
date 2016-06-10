#include <iostream>
#include <SFML/Graphics.hpp>

#include "lime2d.h"


using namespace std;

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "Lime2D", sf::Style::Titlebar | sf::Style::Close);

    l2d::Editor editor(true, &window);

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)) {
                window.close();
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