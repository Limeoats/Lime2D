//
// Created by cw1650 on 6/9/2016.
//

#ifndef LIME2D_LIME2D_H
#define LIME2D_LIME2D_H

#include <SFML/Graphics.hpp>

namespace l2d {
    class Text {
    public:
        Text(sf::Font* font, unsigned int size, sf::Vector2f position, std::string text);
        sf::Text getText() const;
        void setText(std::string text);
        Text operator+=(const std::string &str);
    private:
        sf::Text _text;
    };

    class Editor {
    public:
        explicit Editor(bool enabled, sf::RenderWindow* window);
        void render();
        void update(float elapsedTime, sf::Event &event);
        void exit();
    private:
        bool _enabled;
    };
}


#endif //LIME2D_LIME2D_H
