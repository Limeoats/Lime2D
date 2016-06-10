//
// Created by cw1650 on 6/10/2016.
//

#include "lime2d_internal.h"

l2d_internal::Graphics::Graphics(sf::RenderWindow* window) {
    this->_window = window;
}

void l2d_internal::Graphics::draw(sf::Drawable &drawable) {
    this->_window->draw(drawable);
}

sf::Texture l2d_internal::Graphics::loadImage(const std::string &filePath) {
    if (this->_spriteSheets.count(filePath) == 0) {
        sf::Texture texture;
        texture.loadFromFile(filePath);
        this->_spriteSheets[filePath] = texture;
    }
    return this->_spriteSheets[filePath];
}

l2d_internal::Sprite::Sprite(l2d_internal::Graphics* graphics, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size,
                            sf::Vector2f destPos) {
    this->_texture = graphics->loadImage(filePath);
    this->_sprite = sf::Sprite(this->_texture, sf::IntRect(srcPos.x, srcPos.y, size.x, size.y));
    this->_sprite.setPosition(destPos);
    //TODO: set scale when i figure out how to pass it in
    this->_graphics = graphics;
}

void l2d_internal::Sprite::draw() {
    this->_graphics->draw(this->_sprite);
}

void l2d_internal::Sprite::update(float elapsedTime) {
    (void)elapsedTime;
}