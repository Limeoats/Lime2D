//
// Created by cw1650 on 6/9/2016.
//

#include <sstream>
#include "lime2d.h"

l2d::Text::Text(sf::Font* font, unsigned int size, sf::Vector2f position, std::string text) {
    this->_text.setFont(*font);
    this->_text.setCharacterSize(size);
    this->_text.setPosition(position);
    this->_text.setString(text);
}

sf::Text l2d::Text::getText() const {
    return this->_text;
}

void l2d::Text::setText(std::string text) {
    this->_text.setString(text);
}

l2d::Text l2d::Text::operator+=(const std::string &str) {
    sf::String ss = this->getText().getString();
    ss += str;
    this->setText(ss);
    return *this;
}