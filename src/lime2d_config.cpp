/*
 * lime2d_config.cpp
 * By: Mark Guerra 
 * 6/11/16
*/

#include "lime2d_config.h"


const sf::Keyboard::Key l2d::Config::ShowHide = sf::Keyboard::Equal;        //Show/Hide Lime2D
const std::string l2d::Config::MapPath = "content/maps/";                   //Path to the game's maps
const sf::Vector2f l2d::Config::SpriteScale = sf::Vector2f(4.0f, 4.0f);     //The global Sprite scale
const sf::Vector2f l2d::Config::TileScale = sf::Vector2f(4.0f, 4.0f);       //The global Tile scale