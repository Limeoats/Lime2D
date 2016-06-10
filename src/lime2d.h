//
// Created by cw1650 on 6/9/2016.
//

#ifndef LIME2D_LIME2D_H
#define LIME2D_LIME2D_H

#include <SFML/Graphics.hpp>

#include "lime2d_internal.h"

namespace l2d {
    class Graphics;

    class Config {
    public:
        static const sf::Keyboard::Key ShowHide = sf::Keyboard::Equal; //Show/Hide Lime2D
    };

    class Editor {
    public:
        explicit Editor(bool enabled, sf::RenderWindow* window);
        void toggle();
        void render();
        void update(float elapsedTime, sf::Event &event);
        void exit();
    private:
        bool _enabled;
        sf::RenderWindow* _window;
        l2d_internal::Graphics _graphics;


    };
}


#endif //LIME2D_LIME2D_H
