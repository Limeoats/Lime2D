#ifndef LIME2D_LIME2D_CONFIG_H
#define LIME2D_LIME2D_CONFIG_H

#include <SFML/Graphics.hpp>

namespace l2d {
    class Config {
    public:
        static const std::string MapPath;
        static const sf::Vector2f TileScale;
        static const sf::Vector2f SpriteScale;
    };
}


#endif //LIME2D_LIME2D_CONFIG_H
