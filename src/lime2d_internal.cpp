//
// Created by cw1650 on 6/10/2016.
//

#include <sstream>
#include "lime2d_internal.h"

#include "../libext/tinyxml2.h"

#include "lime2d_config.h"

using namespace tinyxml2;

/*
 * Utils
 */

std::vector<std::string> l2d_internal::utils::split(std::string str, char c) {
    std::vector<std::string> list;
    std::stringstream ss(str);
    std::string item;
    while (std::getline(ss, item, c)) {
        list.push_back(item);
    }
    return list;
}

/*
 * Graphics
 */

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

/*
 * Sprite
 */

l2d_internal::Sprite::Sprite(std::shared_ptr<Graphics> graphics, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size,
                            sf::Vector2f destPos) {
    this->_texture = graphics->loadImage(filePath);
    this->_sprite = sf::Sprite(this->_texture, sf::IntRect(srcPos.x, srcPos.y, size.x, size.y));
    this->_sprite.setPosition(destPos);
    this->_sprite.setScale(l2d::Config::SpriteScale.x, l2d::Config::SpriteScale.y);
    this->_graphics = graphics;
}

void l2d_internal::Sprite::draw() {
    this->_graphics->draw(this->_sprite);
}

void l2d_internal::Sprite::update(float elapsedTime) {
    (void)elapsedTime;
}

/*
 * Tile
 */

l2d_internal::Tile::Tile(std::shared_ptr<Graphics> graphics, std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos,
                         int tilesetId) :
    Sprite(graphics, filePath, srcPos, size, destPos),
    _tilesetId(tilesetId)
{
    this->_sprite.setScale(l2d::Config::SpriteScale.x, l2d::Config::SpriteScale.y);
}

l2d_internal::Tile::~Tile() {}

void l2d_internal::Tile::update(float elapsedTime) {
    Sprite::update(elapsedTime);
}

void l2d_internal::Tile::draw() {
    Sprite::draw();
}


/*
 * Tileset
 */

l2d_internal::Tileset::Tileset(int id, std::string name, std::string path, sf::Vector2i size) :
        Id(id),
        Name(name),
        Path(path),
        Size(size)
{}

/*
 * Layer
 */

void l2d_internal::Layer::draw() {
    for (std::shared_ptr<l2d_internal::Tile> &t : this->Tiles) {
        t->draw();
    }
}

/*
 * Level
 */

l2d_internal::Level::Level(std::shared_ptr<Graphics> graphics, std::string name) {
    this->loadMap(name);
    this->_graphics = graphics;
}

l2d_internal::Level::~Level() {

}

std::string l2d_internal::Level::getName() const {
    return this->_name;
}

sf::Vector2i l2d_internal::Level::getSize() const {
    return this->_size;
}

sf::Vector2i l2d_internal::Level::getTileSize() const {
    return this->_tileSize;
}

void l2d_internal::Level::loadMap(std::string &name) {
    if (name == "l2dSTART") {
        return;
    }
    this->_name = name;

    XMLDocument document;
    std::stringstream ss;
    ss << l2d::Config::MapPath << name << ".xml";
    document.LoadFile(ss.str().c_str());

    XMLElement* pMap = document.FirstChildElement("map");

    //Get the width and height of the map
    int width, height;
    pMap->QueryIntAttribute("width", &width);
    pMap->QueryIntAttribute("height", &height);
    this->_size = sf::Vector2i(width, height);

    //Get the width and height of the tiles
    int tWidth, tHeight;
    pMap->QueryIntAttribute("tileWidth", &tWidth);
    pMap->QueryIntAttribute("tileHeight", &tHeight);
    this->_tileSize = sf::Vector2i(tWidth, tHeight);

    //Loading the tilesets
    XMLElement* pTileset = pMap->FirstChildElement("tileset");
    if (pTileset != nullptr) {
        while (pTileset) {
            int tsId, tsWidth, tsHeight;
            std::string tsName, tsPath;
            pTileset->QueryIntAttribute("id", &tsId);
            pTileset->QueryIntAttribute("width", &tsWidth);
            pTileset->QueryIntAttribute("height", &tsHeight);
            tsName = pTileset->Attribute("name");
            tsPath = pTileset->Attribute("path");
            this->_tilesetList.push_back(Tileset(tsId, tsName, tsPath, sf::Vector2i(tsWidth, tsHeight)));
            pTileset = pTileset->NextSiblingElement("tileset");
        }
    }

    //Load the layers
    XMLElement* pLayer = pMap->FirstChildElement("layer");
    if (pLayer != nullptr) {
        while (pLayer) {
            Layer newLayer;
            newLayer.Id = pLayer->IntAttribute("id");
            newLayer.Name = pLayer->Attribute("name");
            XMLElement* pTiles = pLayer->FirstChildElement("tiles");
            if (pTiles != nullptr) {
                while (pTiles) {
                    //Each pTiles will be for a different tileset
                    //Find the correct tileset and get the path
                    int tilesetId = pTiles->IntAttribute("tileset");
                    std::string tlsPath = "";
                    sf::Vector2i tlsSize;
                    for (auto &tls : this->_tilesetList) {
                        if (tls.Id == tilesetId) {
                            tlsPath = tls.Path;
                            tlsSize = tls.Size;
                            break;
                        }
                    }
                    std::string tileData = pTiles->GetText();
                    //Parse the comma separated list
                    std::vector<std::string> tiles = utils::split(tileData, ',');
                    int c = 0;
                    for (int y = 0; y < this->_size.y; ++y) {
                        for (int x = 0; x < this->_size.x; ++x) {
                            if (atoi(tiles[c].c_str()) == 0) {
                                ++c;
                                continue;
                            }
                            sf::Vector2f destPos(x * this->_tileSize.x * l2d::Config::TileScale.x, y * this->_tileSize.y * l2d::Config::TileScale.y);
                            newLayer.Tiles.push_back(std::make_shared<Tile>(this->_graphics, tlsPath,
                                                                            sf::Vector2i((atoi(tiles[c].c_str()) % tlsSize.x - 1) * this->_tileSize.x,
                                                                                          atoi(tiles[c].c_str()) <= tlsSize.x ? 0 : (atoi(tiles[c].c_str()) % tlsSize.x) * this->_tileSize.y),
                                                                             this->_tileSize, destPos, tilesetId));
                            ++c;
                        }
                    }
                    pTiles = pTiles->NextSiblingElement("tiles");
                }
            }
            this->_layerList.push_back(newLayer);
            pLayer = pLayer->NextSiblingElement("layer");
        }
    }
}

void l2d_internal::Level::draw() {
    for (Layer &layer : this->_layerList) {
        layer.draw();
    }
}

void l2d_internal::Level::update(float elapsedTime) {
    (void)elapsedTime;
}