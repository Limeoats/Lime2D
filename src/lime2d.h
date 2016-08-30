/*!
 * \file lime2d.h
 * \author Mark Guerra
 * \date 6/9/2016
 * \copyright 2016 Limeoats
 */

#ifndef LIME2D_LIME2D_H
#define LIME2D_LIME2D_H

#include <SFML/Graphics.hpp>
#include <array>

#include "lime2d_internal.h"

namespace l2d {
    /*!
     * The Editor class is the core of Lime2D.
     * It contains all functionality that will be exposed to the user.
     */
    class Editor {
    public:
        /*!
         * Lime2d's constructor
         * Sets member variables and initializes the program
         * \param enabled Whether or not Lime2D should start out enabled
         * \param window The game window being drawn to
         */
        explicit Editor(bool enabled, sf::RenderWindow* window);
        /*!
         * Toggles the Lime2D editor
         */
        void toggle();
        /*!
         * Processes events sent from SFML
         * \param event The event that has just occurred
         */
        void processEvent(sf::Event &event);
        /*!
         * Renders Lime2D to the game window
         */
        void render();
        /*!
         * Lime2d's update loop
         * \param elapsedTime The amount of time the previous frame took to execute
         */
        void update(sf::Time elapsedTime);
        /*!
         * Releases resources associated with Lime2D and general cleanup
         */
        void exit();
    private:
        bool _enabled; /*!< Whether or not Lime2D is currently enabled*/
        bool _windowHasFocus; /*!< Whether or not the game window currently has focus*/
        bool _showGridLines; /*!< Whether or not to show grid lines when using the map editor*/
        bool _showEntityList; /*!< Whether or not to show the entity list in the map editor*/
        bool _eraserActive; /*!< Whether or not the eraser is currently active*/
        bool _tilesetEnabled; /*!< Whether or not the tileset window is currently enabled*/

        l2d_internal::Features _currentFeature; /*!< The currently enabled Lime2D feature*/
        sf::RenderWindow* _window; /*!< The window being drawn to*/
        std::shared_ptr<l2d_internal::Graphics>  _graphics; /*!< A pointer to the internal graphics class*/
        l2d_internal::Level _level; /*!< The instance of the current internal level*/
        sf::Shader _ambientLight; /*!< The ambient light for the level*/
        std::vector<std::array<sf::Vertex, 2>> _gridLines; /*!< The grid lines to be drawn over a map*/
        l2d_internal::DrawShapes _currentDrawShape; /*!< The current draw shape*/
    };
}


#endif //LIME2D_LIME2D_H