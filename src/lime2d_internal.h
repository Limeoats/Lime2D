//
// Created by cw1650 on 6/10/2016.
//

#ifndef LIME2D_LIME2D_INTERNAL_H
#define LIME2D_LIME2D_INTERNAL_H


#include <SFML/Graphics.hpp>

namespace l2d_internal {
    //Internal graphics class
    class Graphics {
    public:
        Graphics(sf::RenderWindow* window);
        void draw(sf::Drawable &drawable);
        sf::Texture loadImage(const std::string &filePath);
    private:
        std::map<std::string, sf::Texture> _spriteSheets;
        sf::RenderWindow* _window;
    };

    //Internal sprite class
    class Sprite {
    public:
        Sprite(Graphics* graphics, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos);
        Sprite(const Sprite&) = delete;
        Sprite& operator=(const Sprite&) = delete;
        virtual void update(float elapsedTime);
        virtual void draw();
    protected:
        sf::Texture _texture;
        sf::Sprite _sprite;
        Graphics* _graphics;
    };
}


#endif //LIME2D_LIME2D_INTERNAL_H
