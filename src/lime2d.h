/*
 * lime2d.h
 * By: Mark Guerra
 * Created on 6/9/2016
 */

#ifndef LIME2D_LIME2D_H
#define LIME2D_LIME2D_H

#include <SFML/Graphics.hpp>

#include "lime2d_internal.h"

namespace l2d {
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
        std::shared_ptr<l2d_internal::Graphics>  _graphics;
        l2d_internal::Level _level;

    };
}


#endif //LIME2D_LIME2D_H
