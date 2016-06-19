/*
 * lime2d_internal.cpp
 * By: Mark Guerra
 * Created on 6/10/2016
 */

#include <glob.h>

#include <sstream>
#include <fstream>
#include "lime2d_internal.h"

#include "../libext/tinyxml2.h"

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

std::vector<const char*> l2d_internal::utils::getFilesInDirectory(std::string directory) {
    glob_t glob_result;
    std::vector<const char*> mapFiles;
    glob(directory.c_str(), GLOB_MARK, NULL, &glob_result);
    for (unsigned int i = 0; i < glob_result.gl_pathc; ++i) {
        mapFiles.push_back(glob_result.gl_pathv[i]);
    }
    return mapFiles;
}

std::string l2d_internal::utils::getConfigValue(std::string key) {
    std::ifstream in("lime2d.config");
    std::map<std::string, std::string> configMap;
    if (!in.fail()) {
        for (std::string line; std::getline(in, line); ) {
            configMap.insert(std::pair<std::string, std::string>(l2d_internal::utils::split(line, '=')[0], l2d_internal::utils::split(line, '=')[1]));
        }
    }
    return configMap.size() <= 0 ? "" : configMap[key];
}

/*
 * Graphics
 */

l2d_internal::Graphics::Graphics(sf::RenderWindow* window) {
    this->_window = window;
    this->_camera = std::make_shared<Camera>();
}

void l2d_internal::Graphics::draw(sf::Drawable &drawable) {
    sf::View view(this->_camera->getRect());
    this->_window->setView(view);
    this->_window->draw(drawable);
}

void l2d_internal::Graphics::draw(const sf::Vertex *vertices, unsigned int vertexCount, sf::PrimitiveType type,
                                  const sf::RenderStates &states) {
    sf::View view(this->_camera->getRect());
    this->_window->setView(view);
    this->_window->draw(vertices, vertexCount, type, states);
}

sf::Texture l2d_internal::Graphics::loadImage(const std::string &filePath) {
    if (this->_spriteSheets.count(filePath) == 0) {
        sf::Texture texture;
        texture.loadFromFile(filePath);
        this->_spriteSheets[filePath] = texture;
    }
    return this->_spriteSheets[filePath];
}

std::shared_ptr<l2d_internal::Camera> l2d_internal::Graphics::getCamera() {
    return this->_camera;
}

void l2d_internal::Graphics::update(float elapsedTime, sf::Vector2f tileSize, bool windowHasFocus) {
    this->_camera->update(elapsedTime, tileSize, windowHasFocus);
}

/*
 * Sprite
 */

l2d_internal::Sprite::Sprite(std::shared_ptr<Graphics> graphics, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size,
                            sf::Vector2f destPos) {
    this->_texture = graphics->loadImage(filePath);
    this->_sprite = sf::Sprite(this->_texture, sf::IntRect(srcPos.x, srcPos.y, size.x, size.y));
    this->_sprite.setPosition(destPos);
    this->_sprite.setScale(std::stof(l2d_internal::utils::getConfigValue("sprite_scale_x")), std::stof(l2d_internal::utils::getConfigValue("sprite_scale_y")));
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
    this->_sprite.setScale(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")), std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")));
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
    for (auto &t : this->Tiles) {
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
    this->_name = name;
    if (name == "l2dSTART") {
        return;
    }

    this->_layerList.clear();
    this->_tilesetList.clear();

    XMLDocument document;
    std::stringstream ss;
    ss << l2d_internal::utils::getConfigValue("map_path") << name << ".xml";
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

    XMLElement* pTiles = pMap->FirstChildElement("tiles");
    if (pTiles != nullptr) {
        while (pTiles) {
            XMLElement* pPos = pTiles->FirstChildElement("pos");
            if (pPos != nullptr) {
                while (pPos) {
                    int posX = pPos->IntAttribute("x");
                    int posY = pPos->IntAttribute("y");
                    XMLElement* pTile = pPos->FirstChildElement("tile");
                    if (pTile != nullptr) {
                        while (pTile) {
                            int layer = pTile->IntAttribute("layer");
                            int tileset = pTile->IntAttribute("tileset");
                            int tile = std::stoi(pTile->GetText());
                            if (tile == 0) {
                                pTile = pTile->NextSiblingElement("tile");
                                continue;
                            }
                            //Get the layer or start a new one
                            std::shared_ptr<Layer> l;
                            for (int i = 0; i < this->_layerList.size(); ++i) {
                                if (this->_layerList[i]->Id == layer) {
                                    l = this->_layerList[i];
                                    break;
                                }
                            }
                            if (l == nullptr) {
                                l = std::make_shared<Layer>();
                                this->_layerList.push_back(l);
                            }

                            //Get the tileset
                            std::string tlsPath = "";
                            sf::Vector2i tlsSize;
                            for (auto &tls : this->_tilesetList) {
                                if (tls.Id == tileset) {
                                    tlsPath = tls.Path;
                                    tlsSize = tls.Size;
                                    break;
                                }
                            }
                            sf::Vector2i srcPos(((tile % (tlsSize.x + 1)) * this->_tileSize.x) - (tile <= tlsSize.x ? this->_tileSize.x : 0), tile <= tlsSize.x ? 0 : (tile % tlsSize.x) * this->_tileSize.y);
                            sf::Vector2f destPos((posX - 1) * this->_tileSize.x * std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")),
                                                 (posY - 1) * this->_tileSize.y * std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")));
                            l->Tiles.push_back(std::make_shared<Tile>(this->_graphics, tlsPath, srcPos, this->_tileSize, destPos, tileset));
                            pTile = pTile->NextSiblingElement("tile");
                        }
                    }
                    pPos = pPos->NextSiblingElement("pos");
                }
            }
            pTiles = pTiles->NextSiblingElement("tiles");
        }
    }
}

void l2d_internal::Level::draw() {
    for (auto &layer : this->_layerList) {
        layer->draw();
    }
}

void l2d_internal::Level::update(float elapsedTime) {
    (void)elapsedTime;
}

l2d_internal::Camera::Camera() {
    this->_rect = {-1.0f, -20.0f, std::stof(l2d_internal::utils::getConfigValue("screen_size_x")), std::stof(l2d_internal::utils::getConfigValue("screen_size_y")) };
}

sf::FloatRect l2d_internal::Camera::getRect() {
    return this->_rect;
}

void l2d_internal::Camera::update(float elapsedTime, sf::Vector2f tileSize, bool windowHasFocus) {
    (void)elapsedTime;
    float amountToMoveX = (tileSize.x * std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) / 4.0f;
    float amountToMoveY = (tileSize.y * std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))) / 4.0f;
    if (windowHasFocus) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            this->_rect.top += amountToMoveY;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            this->_rect.top -= amountToMoveY;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            this->_rect.left -= amountToMoveX;
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            this->_rect.left += amountToMoveX;
        }
    }

//    this->_rect.left = std::max(this->_rect.left, 0.0f);
//    this->_rect.top = std::max(this->_rect.top, -40.0f);
}