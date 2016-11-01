/*
 * Lime2d.h
 * By: Mark Guerra
 * Created on: 6/9/2016
 * Copyright (c) 2016 Limeoats
 */

#ifndef LIME2D_LIME2D_H
#define LIME2D_LIME2D_H

#include <SFML/Graphics.hpp>
#include <array>

#include "lime2d_internal.h"

namespace l2d {
    class Editor {
    public:
        explicit Editor(bool enabled, sf::RenderWindow* window);
        void toggle(std::string mapName = "l2dSTART");
        bool isEnabled();
        bool isMapLoaded();
        std::string getLevelName();
        void processEvent(sf::Event &event);
        void render();
        void update(sf::Time elapsedTime);
        void exit();
    private:
        bool _enabled;
        bool _windowHasFocus;
        bool _showGridLines;
        bool _showEntityList;
        bool _eraserActive;
        bool _tilesetEnabled;
        bool _mainHasFocus;
        bool _removingShape;
        bool _hideShapes;

        int _menuClicks;

        sf::Vector2f _lastFrameMousePos;

        l2d_internal::Features _currentFeature;
        sf::RenderWindow* _window;
        std::shared_ptr<l2d_internal::Graphics>  _graphics;
        l2d_internal::Level _level;
        sf::Shader _ambientLight;
        std::vector<std::array<sf::Vertex, 2>> _gridLines;
        l2d_internal::DrawShapes _currentDrawShape;
        l2d_internal::MapEditorMode _currentMapEditorMode;
        sf::Event _currentEvent;
        std::shared_ptr<l2d_internal::Shape> _selectedShape;
        l2d_internal::WindowTypes _currentWindowType;

        void createGridLines(bool always = false);
    };
}


#endif //LIME2D_LIME2D_H