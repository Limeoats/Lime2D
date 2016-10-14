/*
 * Lime2d_internal.cpp
 * By: Mark Guerra
 * Created on: 6/10/2016
 * Copyright (c) 2016 Limeoats
 */

#include <glob.h>
#include <sstream>
#include <fstream>
#include <tuple>
#include <iostream>
#include <cmath>
#include <memory>

#include "../libext/tinyxml2.h"
#include "lime2d_internal.h"

#define tx2 tinyxml2


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

std::vector<std::string> l2d_internal::utils::split(const std::string &str, const std::string &delim, int count) {
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) {
            pos = str.length();
        }
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) {
            tokens.push_back(token);
        }
        prev = pos + delim.length();

        //TODO: Temporary hack to make this work for removeAnimation function. Fix this in the future.
        if (count == 1) {
            tokens.push_back(str.substr(prev, str.length()));
            return tokens;
        }
        //end horrible hack
    } while (pos < str.length() && prev < str.length());
    return tokens;
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

void l2d_internal::utils::createNewAnimationFile(std::string name, std::string spriteSheetPath) {
    std::ofstream os(l2d_internal::utils::getConfigValue("animation_path") + name + ".lua");
    os << "animations = {" << std::endl;
    os << "\tlist = {" << std::endl;
    os << "\t\tanimation_1 = {" << std::endl;
    os << "\t\t\tdescription = \"\"," << std::endl;
    os << "\t\t\tframes = \"1\"," << std::endl;
    os << "\t\t\tname = \"" << "animation_1" << "\"," << std::endl;
    os << "\t\t\toffset = {" << std::endl;
    os << "\t\t\t\tx = \"0\"," << std::endl;
    os << "\t\t\t\ty = \"0\"," << std::endl;
    os << "\t\t\t}," << std::endl;
    os << "\t\t\tsize = {" << std::endl;
    os << "\t\t\t\tw = \"1\"," << std::endl;
    os << "\t\t\t\th = \"1\"," << std::endl;
    os << "\t\t\t}," << std::endl;
    os << "\t\t\tsrc_pos = {" << std::endl;
    os << "\t\t\t\tx = \"0\"," << std::endl;
    os << "\t\t\t\ty = \"0\"," << std::endl;
    os << "\t\t\t}," << std::endl;
    os << "\t\t\ttime_to_update = \"0\"," << std::endl;
    os << "\t\t__}," << std::endl;
    os << "\t}," << std::endl;
    os << "\tsprite_path = \"" << spriteSheetPath << "\"" << std::endl;
    os << "}";
    os.close();
}

void l2d_internal::utils::addNewAnimationToAnimationFile(std::string fileName, std::string animationName) {
    std::ifstream in(fileName);
    std::stringstream ss;
    ss << in.rdbuf();
    in.close();
    std::string str = ss.str();

    std::vector<size_t> positions;
    size_t pos = str.find(",");
    while (pos != std::string::npos) {
        positions.push_back(pos);
        pos = str.find(",", pos + 1);
    }
    size_t startPos = positions.size() <= 1 ? (str.find("list = {") + 9) : positions[positions.size() - 2];
    if (startPos != std::string::npos) {
        std::stringstream ssins;
        ssins << "" << (positions.size() > 1 ? "\n\t" : "") << "\t" << animationName << " = {" << std::endl;
        ssins << "\t\t\tdescription = \"\"," << std::endl;
        ssins << "\t\t\tframes = \"1\"," << std::endl;
        ssins << "\t\t\tname = \"" << animationName << "\"," << std::endl;
        ssins << "\t\t\toffset = {" << std::endl;
        ssins << "\t\t\t\tx = \"0\"," << std::endl;
        ssins << "\t\t\t\ty = \"0\"," << std::endl;
        ssins << "\t\t\t}," << std::endl;
        ssins << "\t\t\tsize = {" << std::endl;
        ssins << "\t\t\t\tw = \"1\"," << std::endl;
        ssins << "\t\t\t\th = \"1\"," << std::endl;
        ssins << "\t\t\t}," << std::endl;
        ssins << "\t\t\tsrc_pos = {" << std::endl;
        ssins << "\t\t\t\tx = \"0\"," << std::endl;
        ssins << "\t\t\t\ty = \"0\"," << std::endl;
        ssins << "\t\t\t}," << std::endl;
        ssins << "\t\t\ttime_to_update = \"0\"," << std::endl;
        ssins << "\t\t__},";
        if (positions.size() <= 1) ssins << "\n\t";
        str.insert(startPos + 1, ssins.str());
    }
    std::ofstream out(fileName, std::ios_base::trunc);
    out << str;
    out.close();
}

void l2d_internal::utils::removeAnimationFromAnimationFile(std::string fileName, std::string animationName) {
    std::ifstream in(fileName);
    std::stringstream ss;
    ss << in.rdbuf();
    in.close();
    std::string str = ss.str();
    animationName += " = {";
    std::vector<std::string> x = l2d_internal::utils::split(str, animationName);
    std::string before = x.front();
    auto t = x.back().find("__},");
    std::string after = x.back().substr(t+5); //+5 grabs __}, and the new line and gets rid of it too
    std::ofstream out(fileName, std::ios_base::trunc);
    out << before.substr(0, before.length() - 2) << after;
    out.close();
}

std::vector<const char*> l2d_internal::utils::getObjectTypesForList() {
    return std::vector<const char*> {"Collision", "Other"};
}

/*
 * Graphics
 */

l2d_internal::Graphics::Graphics(sf::RenderWindow* window) {
    this->_window = window;
    this->_view.reset(sf::FloatRect(-1.0f, -20.0f, this->_window->getSize().x, this->_window->getSize().y));
    this->_zoomPercentage = 100;
}

sf::View l2d_internal::Graphics::getView() const {
    return this->_view;
}

void l2d_internal::Graphics::draw(sf::Drawable &drawable, sf::Shader* ambientLight) {
    this->_window->setView(this->_view);
    if (ambientLight == nullptr) {
        this->_window->draw(drawable);
    }
    else {
        this->_window->draw(drawable, ambientLight);
    }
}

void l2d_internal::Graphics::draw(const sf::Vertex *vertices, unsigned int vertexCount, sf::PrimitiveType type,
                                  const sf::RenderStates &states) {
    this->_window->setView(this->_view);
    this->_window->draw(vertices, vertexCount, type, states);
}

void l2d_internal::Graphics::zoom(float n, sf::Vector2i pixel) {
    const sf::Vector2f before {this->_window->mapPixelToCoords(pixel) };
    if (n > 0) {
        this->_view.zoom(1.f / 1.06f);
        this->_zoomPercentage *= 1.06f;
    }
    else if (n < 0) {
        this->_view.zoom(1.06f);
        this->_zoomPercentage /= 1.06f;
    }
    this->_window->setView(this->_view);
    const sf::Vector2f after { this->_window->mapPixelToCoords(pixel) };
    const sf::Vector2f offset { before - after };
    this->_view.move(offset);
    this->_window->setView(this->_view);
}

float l2d_internal::Graphics::getZoomPercentage() const {
    return this->_zoomPercentage;
}

sf::Texture l2d_internal::Graphics::loadImage(const std::string &filePath) {
    if (this->_spriteSheets.count(filePath) == 0) {
        sf::Texture texture;
        texture.loadFromFile(filePath);
        this->_spriteSheets[filePath] = texture;
    }
    return this->_spriteSheets[filePath];
}

void l2d_internal::Graphics::update(float elapsedTime, sf::Vector2f tileSize, bool windowHasFocus) {
    float amountToMoveX = (tileSize.x * std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) / 4.0f;
    float amountToMoveY = (tileSize.y * std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))) / 4.0f;
    if (windowHasFocus) {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            this->_view.move(0, amountToMoveY);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            this->_view.move(0, -amountToMoveY);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            this->_view.move(-amountToMoveX, 0);
        }
        else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            this->_view.move(amountToMoveX, 0);
        }
        this->_window->setView(this->_view);
    }
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

void l2d_internal::Sprite::draw(sf::Shader* ambientLight) {
    this->_graphics->draw(this->_sprite, ambientLight);
}

void l2d_internal::Sprite::update(float elapsedTime) {
    (void)elapsedTime;
}

/*
 * Animated sprite
 */

l2d_internal::AnimatedSprite::AnimatedSprite(std::shared_ptr<Graphics> graphics, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size,
                                             sf::Vector2f destPos, float timeToUpdate) :
        Sprite(graphics, filePath, srcPos, size, destPos),
        _timeToUpdate(timeToUpdate),
        _currentAnimationOnce(false),
        _currentAnimation(""),
        _frameIndex(0),
        _visible(true)
{}

void l2d_internal::AnimatedSprite::addAnimation(int frames, sf::Vector2i srcPos, std::string name, sf::Vector2i size,
                                                sf::Vector2i offset) {
    std::vector<sf::IntRect> rectangles;
    for (int i = 0; i < frames; ++i) {
        rectangles.push_back({(i + srcPos.x) * size.x, srcPos.y, size.x, size.y});
    }
    this->_animations.insert(std::pair<std::string, std::vector<sf::IntRect>>(name, rectangles));
    this->_offsets.insert(std::pair<std::string, sf::Vector2i>(name, offset));
}

void l2d_internal::AnimatedSprite::updateAnimation(int frames, sf::Vector2i srcPos, std::string name, sf::Vector2i size,
                                                   sf::Vector2i offset, float timeToUpdate) {
    //Find the animation based on the name
    if (this->_animations.find(name) == this->_animations.end()) {
        return;
    }
    //Basically "redo" the stuff done in addAnimation to update it
    this->removeAnimation(name);

    this->addAnimation(frames, srcPos, name, size, offset);
    this->_timeToUpdate = timeToUpdate;
}

void l2d_internal::AnimatedSprite::removeAnimation(std::string name) {
    auto it = this->_animations.find(name);
    if (it == this->_animations.end()) {
        return;
    }
    this->_animations.erase(it);
}

void l2d_internal::AnimatedSprite::resetAnimation() {
    this->_animations.clear(), this->_offsets.clear();
}

void l2d_internal::AnimatedSprite::playAnimation(std::string animation, bool once) {
    this->_currentAnimationOnce = once;
    if (this->_currentAnimation != animation) {
        this->_currentAnimation = animation;
        this->_frameIndex = 0;
        this->_sprite.setTextureRect(sf::IntRect(this->_sprite.getTextureRect().left, this->_sprite.getTextureRect().top, this->_animations[this->_currentAnimation][0].width, this->_animations[this->_currentAnimation][0].height));
    }
}

void l2d_internal::AnimatedSprite::setVisible(bool visible) {
    this->_visible = visible;
}

void l2d_internal::AnimatedSprite::stopAnimation() {
    this->_frameIndex = 0;
}

void l2d_internal::AnimatedSprite::update(float elapsedTime) {
    Sprite::update(elapsedTime);
    this->_timeElapsed += elapsedTime;
    if (this->_timeElapsed > this->_timeToUpdate) {
        this->_timeElapsed = 0;
        if (this->_frameIndex < this->_animations[this->_currentAnimation].size() - 1) {
            ++this->_frameIndex;
            this->_sprite.setTextureRect(this->_animations[this->_currentAnimation][this->_frameIndex]);
        }
        else {
            if (this->_currentAnimationOnce) {
                this->setVisible(!this->_currentAnimationOnce);
            }
            this->stopAnimation();
        }
    }
}

void l2d_internal::AnimatedSprite::draw(sf::Shader *ambientLight) {
    if (this->_visible) {
        this->_sprite.setPosition((this->_sprite.getPosition().x + this->_offsets[this->_currentAnimation].x),
                                  (this->_sprite.getPosition().y + this->_offsets[this->_currentAnimation].y));
        this->_sprite.setTextureRect(this->_animations[this->_currentAnimation][this->_frameIndex]);
        Sprite::draw(ambientLight);
    }
}

sf::Sprite l2d_internal::AnimatedSprite::getSprite() const {
    return this->_sprite;
}

/*
 * Tile
 */

l2d_internal::Tile::Tile(std::shared_ptr<Graphics> graphics, std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos,
                         int tilesetId, int layer) :
        Sprite(graphics, filePath, srcPos, size, destPos),
        _tilesetId(tilesetId),
        _layer(layer)
{
    this->_sprite.setScale(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")), std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")));
}

l2d_internal::Tile::Tile(const Tile &tile) {
    this->_sprite = tile.getSprite();
    this->_tilesetId = tile._tilesetId;
}

l2d_internal::Tile::~Tile() {}

sf::Sprite l2d_internal::Tile::getSprite() const {
    return this->_sprite;
}

sf::Texture l2d_internal::Tile::getTexture() const {
    return this->_texture;
}

int l2d_internal::Tile::getTilesetId() const {
    return this->_tilesetId;
}

int l2d_internal::Tile::getLayer() const {
    return this->_layer;
}

void l2d_internal::Tile::update(float elapsedTime) {
    Sprite::update(elapsedTime);
}

void l2d_internal::Tile::draw(sf::Shader* ambientLight) {
    Sprite::draw(ambientLight);
}


/*
 * Tileset
 */

l2d_internal::Tileset::Tileset(int id, std::string path, sf::Vector2i size) :
        Id(id),
        Path(path),
        Size(size)
{}

/*
 * Layer
 */

l2d_internal::Layer::Layer() {

}

void l2d_internal::Layer::draw(sf::Shader* ambientLight) {
    for (auto &t : this->Tiles) {
        t->draw(ambientLight);
    }
}

/*
 * Level
 */

l2d_internal::Level::Level(std::shared_ptr<Graphics> graphics, std::string name) {
    this->loadMap(name);
    this->_graphics = graphics;
    this->_oldLayerList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_redoList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_ambientColor = sf::Color::White;
    this->_ambientIntensity = 1.0f;
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

std::vector<l2d_internal::Tileset> l2d_internal::Level::getTilesetList() {
    return this->_tilesetList;
}

std::vector<std::shared_ptr<l2d_internal::Layer>> l2d_internal::Level::getLayerList() {
    return this->_layerList;
}

float l2d_internal::Level::getAmbientIntensity() const {
    return this->_ambientIntensity;
}

sf::Color l2d_internal::Level::getAmbientColor() const {
    return this->_ambientColor;
}

void l2d_internal::Level::setAmbientIntensity(float intensity) {
    this->_ambientIntensity = intensity;
}

void l2d_internal::Level::setAmbientColor(sf::Color color) {
    this->_ambientColor = color;
}

void l2d_internal::Level::addShape(std::shared_ptr<l2d_internal::Shape> shape) {
    this->_shapeList.push_back(shape);
}

std::vector<std::shared_ptr<l2d_internal::Shape>> l2d_internal::Level::getShapeList() {
    return this->_shapeList;
}

void l2d_internal::Level::createMap(std::string name, sf::Vector2i size, sf::Vector2i tileSize) {
    this->_layerList.clear();
    this->_shapeList.clear();
    this->_oldLayerList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_redoList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_tilesetList.clear();
    this->_ambientColor = sf::Color::White;
    this->_ambientIntensity = 1.0f;
    this->_name = name;
    this->_size = size;
    this->_tileSize = tileSize;
    this->saveMap(name);
    this->loadMap(name);
}

std::string l2d_internal::Level::loadMap(std::string &name) {
    this->_name = name;
    if (name == "l2dSTART") {
        return "";
    }
    if (name.length() <= 0) {
        this->_name = "l2dSTART";
        return "You cannot open a map that has no name!";
    }

    this->_layerList.clear();
    this->_tilesetList.clear();
    this->_shapeList.clear();
    this->_oldLayerList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_redoList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_ambientColor = sf::Color::White;
    this->_ambientIntensity = 1.0f;

    tx2::XMLDocument document;
    std::stringstream ss;
    ss << l2d_internal::utils::getConfigValue("map_path") << name << ".xml";
    document.LoadFile(ss.str().c_str());

    tx2::XMLElement* pMap = document.FirstChildElement("map");

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
    tx2::XMLElement* pTileset = pMap->FirstChildElement("tileset");
    if (pTileset != nullptr) {
        while (pTileset) {
            int tsId, tsWidth, tsHeight;
            std::string tsPath;
            pTileset->QueryIntAttribute("id", &tsId);
            pTileset->QueryIntAttribute("width", &tsWidth);
            pTileset->QueryIntAttribute("height", &tsHeight);
            tsPath = pTileset->Attribute("path");
            this->_tilesetList.push_back(Tileset(tsId, tsPath, sf::Vector2i(tsWidth, tsHeight)));
            pTileset = pTileset->NextSiblingElement("tileset");
        }
    }

    tx2::XMLElement* pTiles = pMap->FirstChildElement("tiles");
    if (pTiles != nullptr) {
        while (pTiles) {
            tx2::XMLElement* pPos = pTiles->FirstChildElement("pos");
            if (pPos != nullptr) {
                while (pPos) {
                    int posX = pPos->IntAttribute("x");
                    int posY = pPos->IntAttribute("y");
                    tx2::XMLElement* pTile = pPos->FirstChildElement("tile");
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
                            for (unsigned int i = 0; i < this->_layerList.size(); ++i) {
                                if (this->_layerList[i]->Id == layer) {
                                    l = this->_layerList[i];
                                    break;
                                }
                            }
                            if (l == nullptr) {
                                l = std::make_shared<Layer>();
                                l->Id = layer;
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
                            sf::Vector2i srcPos(((tile % (tlsSize.x + 1)) * this->_tileSize.x) - (tile <= tlsSize.x ? this->_tileSize.x : 0), tile <= tlsSize.x ? 0 : (tile - 1)  / tlsSize.x *  this->_tileSize.y);
                            sf::Vector2f destPos((posX - 1) * this->_tileSize.x * std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")),
                                                 (posY - 1) * this->_tileSize.y * std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")));
                            l->Tiles.push_back(std::make_shared<Tile>(this->_graphics, tlsPath, srcPos, this->_tileSize, destPos, tileset, layer));
                            pTile = pTile->NextSiblingElement("tile");
                        }
                    }
                    pPos = pPos->NextSiblingElement("pos");
                }
            }
            pTiles = pTiles->NextSiblingElement("tiles");
        }
    }
    //Objects
    tx2::XMLElement* pObjects = pMap->FirstChildElement("objects");
    if (pObjects != nullptr) {
        while (pObjects) {
            //Lights
            tx2::XMLElement* pLights = pObjects->FirstChildElement("lights");
            if (pLights != nullptr) {
                while (pLights) {
                    //Ambient light
                    tx2::XMLElement* pAmbientLight = pLights->FirstChildElement("ambient");
                    if (pAmbientLight != nullptr) {
                        this->_ambientColor = sf::Color(static_cast<sf::Uint32>(pAmbientLight->IntAttribute("color")));
                        this->_ambientIntensity = pAmbientLight->FloatAttribute("intensity");
                    }
                    pLights = pLights->NextSiblingElement("lights");
                }
            }
            //Shapes
            tx2::XMLElement* pShapes = pObjects->FirstChildElement("shapes");
            if (pShapes != nullptr) {
                while (pShapes) {
                    //Rectangles
                    tx2::XMLElement* pRectangles = pShapes->FirstChildElement("rectangles");
                    if (pRectangles != nullptr) {
                        while (pRectangles) {
                            tx2::XMLElement* pRectangle = pRectangles->FirstChildElement("rectangle");
                            if (pRectangle != nullptr) {
                                while (pRectangle) {
                                    std::string name = pRectangle->Attribute("name");
                                    sf::Color color = sf::Color(
                                            static_cast<sf::Uint32>(pRectangle->IntAttribute("color")));
                                    l2d_internal::ObjectTypes type = static_cast<l2d_internal::ObjectTypes>(pRectangle->IntAttribute(
                                            "type"));
                                    sf::Vector2f position;
                                    sf::Vector2f size;
                                    tx2::XMLElement* pRectanglePosition = pRectangle->FirstChildElement("pos");
                                    if (pRectanglePosition != nullptr) {
                                        position.x = pRectanglePosition->FloatAttribute("x");
                                        position.y = pRectanglePosition->FloatAttribute("y");
                                    }
                                    tx2::XMLElement* pRectangleSize = pRectangle->FirstChildElement("size");
                                    if (pRectangleSize != nullptr) {
                                        size.x = pRectangleSize->FloatAttribute("w");
                                        size.y = pRectangleSize->FloatAttribute("h");
                                    }
                                    sf::RectangleShape rect;
                                    rect.setPosition(position);
                                    rect.setSize(size);
                                    rect.setFillColor(color);
                                    rect.setOutlineThickness(2.0f);
                                    rect.setOutlineColor(sf::Color(color.r, color.g, color.b, 160));
                                    this->_shapeList.push_back(std::make_shared<l2d_internal::Rectangle>(name, color, type, rect));
                                    pRectangle = pRectangle->NextSiblingElement("rectangle");
                                }
                                pRectangles = pRectangles->NextSiblingElement("rectangles");
                            }
                        }
                    }
                    //Points
                    tx2::XMLElement* pPoints = pShapes->FirstChildElement("points");
                    if (pPoints != nullptr) {
                        while (pPoints) {
                            tx2::XMLElement* pPoint = pPoints->FirstChildElement("point");
                            if (pPoint != nullptr) {
                                while (pPoint) {
                                    std::string name = pPoint->Attribute("name");
                                    sf::Color color = sf::Color(static_cast<sf::Uint32>(pPoint->IntAttribute("color")));
                                    sf::Vector2f position;
                                    tx2::XMLElement* pPointPosition = pPoint->FirstChildElement("pos");
                                    if (pPointPosition != nullptr) {
                                        position.x = pPointPosition->FloatAttribute("x");
                                        position.y = pPointPosition->FloatAttribute("y");
                                    }
                                    sf::CircleShape dot;
                                    dot.setPosition(position);
                                    dot.setFillColor(sf::Color(color.r, color.g, color.b, 80));
                                    dot.setOutlineColor(sf::Color(color.r, color.g, color.b, 160));
                                    dot.setOutlineThickness(2.0f);
                                    dot.setRadius(6.0f);
                                    this->_shapeList.push_back(std::make_shared<l2d_internal::Point>(name, color, dot));
                                    pPoint = pPoint->NextSiblingElement("point");
                                }
                                pPoints = pPoints->NextSiblingElement("points");
                            }
                        }
                    }
                    pShapes = pShapes->NextSiblingElement("shapes");
                }
            }
            pObjects = pObjects->NextSiblingElement("objects");
        }
    }
    return "";
}

void l2d_internal::Level::saveMap(std::string name) {
    tx2::XMLDocument document;
    std::stringstream ss;
    ss << l2d_internal::utils::getConfigValue("map_path") << name << ".xml";
    document.LoadFile(ss.str().c_str());
    document.Clear();
    tx2::XMLDeclaration* pDeclaration = document.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
    document.InsertFirstChild(pDeclaration);

    //Map node
    tx2::XMLElement* pMap = document.NewElement("map");
    pMap->SetAttribute("name", name.c_str());
    pMap->SetAttribute("width", this->_size.x);
    pMap->SetAttribute("height", this->_size.y);
    pMap->SetAttribute("tileWidth", this->_tileSize.x);
    pMap->SetAttribute("tileHeight", this->_tileSize.y);

    //Tilesets
    for (Tileset &t : this->_tilesetList) {
        tx2::XMLElement* pTileset = document.NewElement("tileset");
        pTileset->SetAttribute("id", t.Id);
        pTileset->SetAttribute("path", t.Path.c_str());
        pTileset->SetAttribute("width", t.Size.x);
        pTileset->SetAttribute("height", t.Size.y);
        pMap->InsertEndChild(pTileset);
    }

    //Tiles
    tx2::XMLElement* pTiles = document.NewElement("tiles");

    //Pos nodes
    std::vector<std::shared_ptr<Tile>> allTiles;
    for (std::shared_ptr<Layer> &layer : this->_layerList) {
        allTiles.insert(allTiles.end(), layer.get()->Tiles.begin(), layer.get()->Tiles.end());
    }
    //Sort allTiles by y, then by x, then by layer
    std::sort(allTiles.begin(), allTiles.end(), [&](const std::shared_ptr<Tile> &a, const std::shared_ptr<Tile> &b) {
        return ((a.get()->getSprite().getPosition().y < b.get()->getSprite().getPosition().y) ||
                ((a.get()->getSprite().getPosition().y == b.get()->getSprite().getPosition().y) &&
                 a.get()->getSprite().getPosition().x < b.get()->getSprite().getPosition().x) ||
                (((a.get()->getSprite().getPosition().y == b.get()->getSprite().getPosition().y) &&
                  a.get()->getSprite().getPosition().x == b.get()->getSprite().getPosition().x) &&
                 a.get()->getLayer() < b.get()->getLayer()));
    });
    for (std::shared_ptr<Tile> &tile : allTiles) {
        float x = tile.get()->getSprite().getPosition().x / this->_tileSize.x / std::stof(l2d_internal::utils::getConfigValue("tile_scale_x")) + 1;
        float y = tile.get()->getSprite().getPosition().y / this->_tileSize.y / std::stof(l2d_internal::utils::getConfigValue("tile_scale_y")) + 1;

        //If the pos was already created previously, use it instead of creating a new one
        //This will group all tiles from all layers that exist in one pos together
        tx2::XMLElement* lastPosX = pTiles->LastChildElement("pos");
        tx2::XMLElement* lastPosY = pTiles->LastChildElement("pos");
        tx2::XMLElement* pPos;
        if (lastPosX == nullptr || lastPosY == nullptr) {
            pPos = document.NewElement("pos");
        }
        else {
            pPos = pTiles->LastChildElement("pos")->FloatAttribute("x") == x &&
                   pTiles->LastChildElement("pos")->FloatAttribute("y") == y ?
                   pTiles->LastChildElement("pos") : document.NewElement("pos");
        }
        pPos->SetAttribute("x", x);
        pPos->SetAttribute("y", y);

        //Tile elements
        tx2::XMLElement *pTile = document.NewElement("tile");
        pTile->SetAttribute("layer", tile.get()->getLayer());
        pTile->SetAttribute("tileset", tile.get()->getTilesetId());
        int tileNumber;
        if (tile.get()->getSprite().getTextureRect().top == 0) {
            //First row in tileset
            tileNumber = (tile.get()->getSprite().getTextureRect().left / this->_tileSize.x) + 1;
        }
        else {
            auto tileset = std::find_if(this->_tilesetList.begin(), this->_tilesetList.end(), [&](Tileset t) {
                return t.Id == tile.get()->getTilesetId();
            });
            tileNumber = (tileset->Size.x) * (tile.get()->getSprite().getTextureRect().top / this->_tileSize.y) + 1 + (tile.get()->getSprite().getTextureRect().left / this->_tileSize.x);
        }
        pTile->SetText(tileNumber);
        pPos->InsertEndChild(pTile);
        pTiles->InsertEndChild(pPos);
    }

    pMap->InsertEndChild(pTiles);

    //Save objects
    tx2::XMLElement* pObjects = document.NewElement("objects");
    //Lights
    tx2::XMLElement* pLights = document.NewElement("lights");
    if (this->_ambientColor != sf::Color::White && this->_ambientIntensity != 1.0f) {
        tx2::XMLElement* pAmbientLight = document.NewElement("ambient");
        pAmbientLight->SetAttribute("color", this->_ambientColor.toInteger());
        pAmbientLight->SetAttribute("intensity", this->_ambientIntensity);
        pLights->InsertEndChild(pAmbientLight);
    }
    pObjects->InsertEndChild(pLights);

    //Shapes
    tx2::XMLElement* pShapes = document.NewElement("shapes");
    tx2::XMLElement* pRectangles = document.NewElement("rectangles");
    tx2::XMLElement* pPoints = document.NewElement("points");
    for (std::shared_ptr<l2d_internal::Shape> &shape: this->_shapeList) {
        std::shared_ptr<l2d_internal::Rectangle> r = std::dynamic_pointer_cast<l2d_internal::Rectangle>(shape);
        if (r != nullptr) {
            tx2::XMLElement* pRectangle = document.NewElement("rectangle");
            pRectangle->SetAttribute("name", r->getName().c_str());
            pRectangle->SetAttribute("color", r->getColor().toInteger());
            pRectangle->SetAttribute("type", static_cast<int>(r->getObjectType()));
            tx2::XMLElement* pRectanglePosition = document.NewElement("pos");
            pRectanglePosition->SetAttribute("x", r->getRectangle().getPosition().x);
            pRectanglePosition->SetAttribute("y", r->getRectangle().getPosition().y);
            pRectangle->InsertEndChild(pRectanglePosition);
            tx2::XMLElement* pRectangleSize = document.NewElement("size");
            pRectangleSize->SetAttribute("w", r->getRectangle().getSize().x);
            pRectangleSize->SetAttribute("h", r->getRectangle().getSize().y);
            pRectangle->InsertEndChild(pRectangleSize);
            pRectangles->InsertEndChild(pRectangle);
            continue;
        }
        std::shared_ptr<l2d_internal::Point> p = std::dynamic_pointer_cast<l2d_internal::Point>(shape);
        if (p != nullptr) {
            tx2::XMLElement* pPoint = document.NewElement("point");
            pPoint->SetAttribute("name", p->getName().c_str());
            pPoint->SetAttribute("color", p->getColor().toInteger());
            tx2::XMLElement* pPointPosition = document.NewElement("pos");
            pPointPosition->SetAttribute("x", p->getCircle().getPosition().x);
            pPointPosition->SetAttribute("y", p->getCircle().getPosition().y);
            pPoint->InsertEndChild(pPointPosition);
            pPoints->InsertEndChild(pPoint);
            continue;
        }
    }
    pShapes->InsertEndChild(pRectangles);
    pShapes->InsertEndChild(pPoints);
    pObjects->InsertEndChild(pShapes);
    pMap->InsertEndChild(pObjects);
    document.InsertAfterChild(pDeclaration, pMap);

    //Save the document
    document.SaveFile(ss.str().c_str());
}

void l2d_internal::Level::updateTile(std::string newTilesetPath, sf::Vector2i newTilesetSize, sf::Vector2i srcPos,
                                     sf::Vector2f destPos, int tilesetId, int layer) {

    static auto layerExists = [&]()->std::shared_ptr<Layer> {
        for (unsigned int i = 0; i < this->_layerList.size(); ++i) {
            if (this->_layerList[i]->Id == layer) {
                return this->_layerList[i];
            }
        }
        return nullptr;
    };

    sf::Vector2f newDestPos((destPos.x - 1) * this->_tileSize.x * static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))),
                            (destPos.y - 1) * this->_tileSize.y * static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))));
    //First do a check to see if the tile is identical on the same layer. If so, don't do any of this
    std::shared_ptr<Layer> cl = layerExists();
    if (cl != nullptr) {
        //Layer exists. Now check if there's anything on the tile
        std::shared_ptr<Tile> t = nullptr;
        for (unsigned int i = 0; i < cl.get()->Tiles.size(); ++i) {
            if (cl.get()->Tiles[i].get()->getSprite().getPosition().x == newDestPos.x && cl.get()->Tiles[i].get()->getSprite().getPosition().y == newDestPos.y) {
                t = cl.get()->Tiles[i];
                break;
            }
        }
        if (t != nullptr) {
            //Something's on the tile. Check if it's the same tile trying to be drawn. If so, quit.
            if (t.get()->getSprite().getTextureRect().left == srcPos.x && t.get()->getSprite().getTextureRect().top == srcPos.y) {
                //Check if the tileset is the same. If not, continue through function
                if (t->getTilesetId() == tilesetId) {
                    //Same tile. Stop the function.
                    return;
                }
            }
        }
    }

    //Set oldLayerList for Undo
    std::vector<std::shared_ptr<l2d_internal::Layer>> tmpList;
    for (unsigned int i = 0; i < this->_layerList.size(); ++i) {
        l2d_internal::Layer l;
        l.Id = this->_layerList.at(i).get()->Id;
        for (unsigned int j = 0; j < this->_layerList[i].get()->Tiles.size(); ++j) {
            l.Tiles.push_back(this->_layerList[i].get()->Tiles.at(j));
        }
        tmpList.push_back(std::make_shared<l2d_internal::Layer>(l));
    }
    this->_oldLayerList.push(tmpList);

    //Add the tileset to the map if it isn't already
    std::shared_ptr<Tileset> tls = nullptr;
    for (unsigned int i = 0; i < this->_tilesetList.size(); ++i) {
        if (this->_tilesetList[i].Id == tilesetId) {
            tls = std::make_shared<Tileset>(this->_tilesetList[i]);
            break;
        }
    }
    int newId = -1;
    if (tls == nullptr) {
        //Create a new tilesetId (max existing tilesetid + 1)
        for (const l2d_internal::Tileset &t : this->_tilesetList) {
            if (t.Id >= newId) {
                newId = t.Id + 1;
            }
        }
        this->_tilesetList.push_back(Tileset(newId, newTilesetPath, sf::Vector2i(newTilesetSize.x / this->_tileSize.x, newTilesetSize.y / this->_tileSize.y)));
    }

    std::shared_ptr<Tile> t = nullptr;
    //Check if the layer exists. If not, create it
    std::shared_ptr<Layer> l = layerExists();
    if (l == nullptr) {
        l = std::make_shared<Layer>();
        l->Id = layer;
        this->_layerList.push_back(l);
    }

    for (unsigned int i = 0; i < l.get()->Tiles.size(); ++i) {
        auto tile = l.get()->Tiles[i];
        int tileLayer = tile.get()->getLayer();
        sf::Vector2f tilePos(tile.get()->getSprite().getPosition().x / this->_tileSize.x /
                             static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) + 1,
                             tile.get()->getSprite().getPosition().y / this->_tileSize.y /
                             static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))) + 1);
        if (tileLayer == layer && tilePos.x == static_cast<int>(destPos.x) &&
            tilePos.y == static_cast<int>(destPos.y)) {
            t = tile;
            break;
        }
    }
    if (t != nullptr) {
        //Remove the existing tile from the layer
        l.get()->Tiles.erase(
                std::remove(l.get()->Tiles.begin(),
                            l.get()->Tiles.end(), t),
                l.get()->Tiles.end());
    }

    //Place the new one
    l.get()->Tiles.push_back(std::make_shared<Tile>(this->_graphics, newTilesetPath, srcPos, this->_tileSize, newDestPos, newId == -1 ? tilesetId : newId, layer));
}

bool l2d_internal::Level::tileExists(int layer, sf::Vector2i pos) const {
    auto l = std::find_if(this->_layerList.begin(), this->_layerList.end(), [&](std::shared_ptr<Layer> t) {
        return t.get()->Id == layer;
    });
    if (l == this->_layerList.end()) {
        //The layer doesn't even exist
        return false;
    }
    auto tile = std::find_if(l->get()->Tiles.begin(), l->get()->Tiles.end(), [&](std::shared_ptr<Tile> t) {
        return this->globalToLocalCoordinates(t.get()->getSprite().getPosition()) == pos;
    });
    return !(tile == l->get()->Tiles.end());
}

void l2d_internal::Level::removeTile(int layer, sf::Vector2f pos) {
    if (!this->tileExists(layer, this->globalToLocalCoordinates(pos))) {
        return;
    }
    //Set oldLayerList for Undo
    std::vector<std::shared_ptr<l2d_internal::Layer>> tmpList;
    for (unsigned int i = 0; i < this->_layerList.size(); ++i) {
        l2d_internal::Layer l;
        l.Id = this->_layerList.at(i).get()->Id;
        for (unsigned int j = 0; j < this->_layerList[i].get()->Tiles.size(); ++j) {
            l.Tiles.push_back(this->_layerList[i].get()->Tiles.at(j));
        }
        tmpList.push_back(std::make_shared<l2d_internal::Layer>(l));
    }
    this->_oldLayerList.push(tmpList);

    std::shared_ptr<Tile> t = nullptr;
    //Check if the layer exists. If not, create it
    std::shared_ptr<Layer> l = nullptr;
    for (unsigned int i = 0; i < this->_layerList.size(); ++i) {
        if (this->_layerList[i]->Id == layer) {
            l = this->_layerList[i];
            break;
        }
    }

    for (unsigned int i = 0; i < l.get()->Tiles.size(); ++i) {
        auto tile = l.get()->Tiles[i];
        int tileLayer = tile.get()->getLayer();
        sf::Vector2f tilePos(tile.get()->getSprite().getPosition().x / this->_tileSize.x /
                             static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) + 1,
                             tile.get()->getSprite().getPosition().y / this->_tileSize.y /
                             static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))) + 1);
        if (tileLayer == layer && tilePos.x == static_cast<int>(pos.x / this->_tileSize.x /
                                                                static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) + 1) &&
            tilePos.y == static_cast<int>(pos.y / this->_tileSize.y /
                                          static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))) + 1)) {
            t = tile;
            break;
        }
    }
    if (t != nullptr) {
        //Remove the existing tile from the layer
        l.get()->Tiles.erase(
                std::remove(l.get()->Tiles.begin(),
                            l.get()->Tiles.end(), t),
                l.get()->Tiles.end());
    }
}

int l2d_internal::Level::getTilesetID(const std::string &path) const {
    for (const l2d_internal::Tileset &t : this->_tilesetList) {
        if (t.Path == path) {
            return t.Id;
        }
    }
    return -1;
}

void l2d_internal::Level::undo() {
    if (!this->isUndoListEmpty()) {
        std::vector<std::shared_ptr<l2d_internal::Layer>> tmpList;
        for (unsigned int i = 0; i < this->_oldLayerList.top().size(); ++i) {
            l2d_internal::Layer l;
            l.Id = this->_oldLayerList.top().at(i).get()->Id;
            for (unsigned int j = 0; j < this->_oldLayerList.top()[i].get()->Tiles.size(); ++j) {
                l.Tiles.push_back(this->_oldLayerList.top()[i].get()->Tiles.at(j));
            }
            tmpList.push_back(std::make_shared<l2d_internal::Layer>(l));
        }

        //Set up redo list
        std::vector<std::shared_ptr<l2d_internal::Layer>> tmpRedoList;
        for (unsigned int i = 0; i < this->_layerList.size(); ++i) {
            l2d_internal::Layer l;
            l.Id = this->_layerList.at(i).get()->Id;
            for (unsigned int j = 0; j < this->_layerList[i].get()->Tiles.size(); ++j) {
                l.Tiles.push_back(this->_layerList[i].get()->Tiles.at(j));
            }
            tmpRedoList.push_back(std::make_shared<l2d_internal::Layer>(l));
        }
        this->_redoList.push(tmpRedoList);

        this->_layerList = tmpList;
        this->_oldLayerList.pop();
    }
}

bool l2d_internal::Level::isUndoListEmpty() const {
    return this->_oldLayerList.empty();
}

void l2d_internal::Level::redo() {
    if (!this->isRedoListEmpty()) {
        std::vector<std::shared_ptr<l2d_internal::Layer>> tmpList;
        for (unsigned int i = 0; i < this->_redoList.top().size(); ++i) {
            l2d_internal::Layer l;
            l.Id = this->_redoList.top().at(i).get()->Id;
            for (unsigned int j = 0; j < this->_redoList.top()[i].get()->Tiles.size(); ++j) {
                l.Tiles.push_back(this->_redoList.top()[i].get()->Tiles.at(j));
            }
            tmpList.push_back(std::make_shared<l2d_internal::Layer>(l));
        }

        //Set up undo list
        std::vector<std::shared_ptr<l2d_internal::Layer>> tmpUndoList;
        for (unsigned int i = 0; i < this->_layerList.size(); ++i) {
            l2d_internal::Layer l;
            l.Id = this->_layerList.at(i).get()->Id;
            for (unsigned int j = 0; j < this->_layerList[i].get()->Tiles.size(); ++j) {
                l.Tiles.push_back(this->_layerList[i].get()->Tiles.at(j));
            }
            tmpUndoList.push_back(std::make_shared<l2d_internal::Layer>(l));
        }
        this->_oldLayerList.push(tmpUndoList);

        this->_layerList = tmpList;
        this->_redoList.pop();
    }
}

bool l2d_internal::Level::isRedoListEmpty() const {
    return this->_redoList.empty();
}

void l2d_internal::Level::updateShape(std::shared_ptr<l2d_internal::Shape> oldShape, std::shared_ptr<l2d_internal::Shape> newShape) {
    for (unsigned int i = 0; i < this->_shapeList.size(); ++i) {
        if (oldShape->equals(this->_shapeList[i])) {
            this->_shapeList[i] = newShape;
            return;
        }
    }
}

void l2d_internal::Level::removeShape(std::shared_ptr<l2d_internal::Shape> shape) {
    this->_shapeList.erase(std::remove(this->_shapeList.begin(), this->_shapeList.end(), shape), this->_shapeList.end());
}

sf::Vector2i l2d_internal::Level::globalToLocalCoordinates(sf::Vector2f coords) const {
    return sf::Vector2i(static_cast<int>(coords.x) / this->_tileSize.x / static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) + 1,
                        static_cast<int>(coords.y) / this->_tileSize.y / static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))) + 1);
}

void l2d_internal::Level::draw(sf::Shader* ambientLight) {
    for (auto &layer : this->_layerList) {
        layer->draw(ambientLight);
    }
}

void l2d_internal::Level::update(float elapsedTime) {
    (void)elapsedTime;
}


/*
 * Shape
 */
l2d_internal::Shape::Shape(std::string name, sf::Color color) {
    this->_name = name;
    this->_color = color;
    this->_selected = false;
}

std::string l2d_internal::Shape::getName() {
    return this->_name;
}

sf::Color l2d_internal::Shape::getColor() const {
    return this->_color;
}

void l2d_internal::Shape::setName(std::string name) {
    this->_name = name;
}



void l2d_internal::Shape::setColor(sf::Color color) {
    this->_color = color;
}

/*
 * Point
 */

l2d_internal::Point::Point(std::string name, sf::Color color, sf::CircleShape dot) :
    Shape(name, color)
{
    this->_dot = dot;
}

sf::CircleShape l2d_internal::Point::getCircle() {
    return this->_dot;
}

sf::Color l2d_internal::Point::getColor() const {
    return this->_color;
}

void l2d_internal::Point::setColor(sf::Color color) {
    this->_color = color;
    this->_dot.setFillColor(sf::Color(color.r, color.g, color.b, 80));
    this->_dot.setOutlineColor(sf::Color(color.r, color.g, color.b, 160));
}

void l2d_internal::Point::select() {
    if (!this->_selected) {
        this->_selected = true;
        this->_dot.setOutlineThickness(this->_dot.getOutlineThickness() + 1);
        this->_dot.setFillColor(sf::Color(std::min(255, this->_dot.getFillColor().r + 16),
                                           std::min(255, this->_dot.getFillColor().g + 16),
                                           std::min(255, this->_dot.getFillColor().b + 16),
                                           std::min(255, this->_dot.getFillColor().a + 16)
        ));
    }
}

void l2d_internal::Point::unselect() {
    if (this->_selected) {
        this->_selected = false;
        this->_dot.setOutlineThickness(this->_dot.getOutlineThickness() - 1);
        this->_dot.setFillColor(sf::Color(std::max(0, this->_dot.getFillColor().r - 16),
                                           std::max(0, this->_dot.getFillColor().g - 16),
                                           std::max(0, this->_dot.getFillColor().b - 16),
                                           std::max(0, this->_dot.getFillColor().a - 16)
        ));
    }
}

void l2d_internal::Point::setPosition(sf::Vector2f pos) {
    this->_dot.setPosition(pos);
}

bool l2d_internal::Point::isPointInside(sf::Vector2f point) {
    sf::Vector2f center = sf::Vector2f(this->_dot.getGlobalBounds().left + (this->_dot.getGlobalBounds().width / 2),
                                       this->_dot.getGlobalBounds().top + (this->_dot.getGlobalBounds().height / 2));
    auto x = pow(point.x - center.x, 2);
    auto y = pow(point.y - center.y, 2);
    return pow(this->_dot.getRadius(), 2) > (x + y);
}

void l2d_internal::Point::setSize(sf::Vector2f size) {
    this->_dot.setRadius(size.x);
}

void l2d_internal::Point::draw(sf::RenderWindow *window) {
    window->draw(this->_dot);
}

bool l2d_internal::Point::equals(std::shared_ptr<Shape> other) {
    return false;
}

/*
 * Line
 */

l2d_internal::Line::Line(std::string name, sf::Color color, std::vector<l2d_internal::Point> points) :
    Shape(name, color)
{
    this->_points = points;
}

std::vector<l2d_internal::Point> l2d_internal::Line::getPoints() {
    return this->_points;
}

sf::Color l2d_internal::Line::getColor() const {
    return this->_color;
}

void l2d_internal::Line::setColor(sf::Color color) {
    this->_color = color;
    for (l2d_internal::Point &p : this->_points) {
        p.setColor(color);
    }
}

bool l2d_internal::Line::isPointInside(sf::Vector2f point) {
    //TODO
    throw utils::NotImplementedException();
    return false;
}

void l2d_internal::Line::select() {
    //TODO
    throw utils::NotImplementedException();
}

void l2d_internal::Line::unselect() {
    //TODO
    throw utils::NotImplementedException();
}

void l2d_internal::Line::setPosition(sf::Vector2f pos) {
    //TODO
    throw utils::NotImplementedException();
}

void l2d_internal::Line::setSize(sf::Vector2f size) {
    //TODO
    throw utils::NotImplementedException();
}

void l2d_internal::Line::draw(sf::RenderWindow* window) {
    //TODO
    throw utils::NotImplementedException();
}

bool l2d_internal::Line::equals(std::shared_ptr<Shape> other) {
    return false;
}



/*
 * Rectangle
 */

l2d_internal::Rectangle::Rectangle(std::string name, sf::Color color, l2d_internal::ObjectTypes objectType,
                                   sf::RectangleShape rect) :
    Shape(name, color)
{
    this->_rect = rect;
    this->_objectType = objectType;
}

sf::RectangleShape l2d_internal::Rectangle::getRectangle() {
    return this->_rect;
}

void l2d_internal::Rectangle::setObjectType(l2d_internal::ObjectTypes objectType) {
    this->_objectType = objectType;
}

l2d_internal::ObjectTypes l2d_internal::Rectangle::getObjectType() {
    return this->_objectType;
}

sf::Color l2d_internal::Rectangle::getColor() const {
    return this->_rect.getFillColor();
}

void l2d_internal::Rectangle::setColor(sf::Color color) {
    this->_color = color;
    this->_rect.setFillColor(sf::Color(color.r, color.g, color.b, 80));
    this->_rect.setOutlineColor(sf::Color(color.r, color.g, color.b, 160));
}

bool l2d_internal::Rectangle::isPointInside(sf::Vector2f point) {
    sf::Rect<float> r(this->_rect.getPosition().x, this->_rect.getPosition().y, this->_rect.getSize().x, this->_rect.getSize().y);
    sf::Rect<float> r2(point.x, point.y, 1, 1);
    return r.intersects(r2);
}

void l2d_internal::Rectangle::select() {
    if (!this->_selected) {
        this->_selected = true;
        this->_rect.setOutlineThickness(this->_rect.getOutlineThickness() + 1);
        this->_rect.setFillColor(sf::Color(std::min(255, this->_rect.getFillColor().r + 16),
                                           std::min(255, this->_rect.getFillColor().g + 16),
                                           std::min(255, this->_rect.getFillColor().b + 16),
                                           std::min(255, this->_rect.getFillColor().a + 16)
        ));
    }
}

void l2d_internal::Rectangle::unselect() {
    if (this->_selected) {
        this->_selected = false;
        this->_rect.setOutlineThickness(this->_rect.getOutlineThickness() - 1);
        this->_rect.setFillColor(sf::Color(std::max(0, this->_rect.getFillColor().r - 16),
                                           std::max(0, this->_rect.getFillColor().g - 16),
                                           std::max(0, this->_rect.getFillColor().b - 16),
                                           std::max(0, this->_rect.getFillColor().a - 16)
        ));
    }
}

void l2d_internal::Rectangle::setPosition(sf::Vector2f pos) {
    this->_rect.setPosition(pos);
}

void l2d_internal::Rectangle::setSize(sf::Vector2f size) {
    this->_rect.setSize(size);
}

void l2d_internal::Rectangle::draw(sf::RenderWindow *window) {
    window->draw(this->_rect);
}

bool l2d_internal::Rectangle::equals(std::shared_ptr<Shape> other) {
    return false;
}


/*
 * LuaScript
 */
l2d_internal::LuaScript::LuaScript(const std::string &filePath) {
    this->L = luaL_newstate();
    if (luaL_loadfile(this->L, filePath.c_str()) || lua_pcall(this->L, 0, 0, 0)) {
        std::cerr << "Unable to load Lua script." << std::endl;
        this->L = nullptr;
    }
    if (this->L != nullptr) {
        luaL_openlibs(this->L);
        this->_fileName = filePath;
    }
}

l2d_internal::LuaScript::~LuaScript() {
    if (this->L != nullptr) {
        lua_close(this->L);
    }
}

void l2d_internal::LuaScript::printError(const std::string &variable, const std::string error) {
    std::cerr << "Error: Unable to get [" << variable << "]" << std::endl << error << std::endl;
}

bool l2d_internal::LuaScript::lua_getVariable(const std::string &variable) {
    int level = 0;
    std::string var = "";
    for (unsigned int i = 0; i < variable.length(); ++i) {
        if (variable.at(i) == '.') {
            if (level == 0) {
                lua_getglobal(this->L, var.c_str());
            }
            else {
                lua_getfield(this->L, -1, var.c_str());
            }
            if (lua_isnil(this->L, -1)) {
                this->printError(variable, var + " is not defined.");
                return false;
            }
            var = "";
            ++level;
        }
        else {
            var += variable.at(i);
        }
    }
    if (level == 0) {
        lua_getglobal(this->L, var.c_str());
    }
    else {
        lua_getfield(this->L, -1, var.c_str());
    }
    if (lua_isnil(L, -1)) {
        this->printError(variable, var + " is not defined.");
        return false;
    }
    return true;
}

std::vector<std::string> l2d_internal::LuaScript::getTableKeys(const std::string &variable) {
    std::string code =
            "function __genOrderedIndex(t) "
                    "local orderedIndex = {} "
                    "for key in pairs(t) do "
                    "table.insert(orderedIndex, key) "
                    "end "
                    "table.sort(orderedIndex) "
                    "return orderedIndex "
                    "end "
                    "function orderedNext(t, state) "
                    "key = nil "
                    "if state == nil then "
                    "t.__orderedIndex = __genOrderedIndex(t) "
                    "key = t.__orderedIndex[1] "
                    "else "
                    "for i = 1, #t.__orderedIndex do "
                    "if t.__orderedIndex[i] == state then "
                    "key = t.__orderedIndex[i + 1] "
                    "end "
                    "end "
                    "end "
                    "if key then "
                    "return key, t[key] "
                    "end "
                    "t.__orderedIndex = nil "
                    "return "
                    "end "
                    "function orderedPairs(t) "
                    "return orderedNext, t, nil "
                    "end "
                    "function getKeys(variable) "
                    "s = \"\" "
                    "local v = _G "
                    "for w in string.gmatch(variable, \"[%w_]+\") do "
                    "v = v[w] "
                    "end "
                    "for key, value in orderedPairs(v) do "
                    "s = s..key..\",\" "
                    "end "
                    "return s "
                    "end  ";
    luaL_loadstring(L, code.c_str()); //Put the new getKeys function on the top of the Lua stack
    lua_pcall(L, 0, 0, 0);
    lua_getglobal(L, "getKeys");
    lua_pushstring(L, variable.c_str()); //Push the variable name onto the stack
    lua_pcall(L, 1, 1, 0); //Call getKeys with one argument: variable
    std::string test = lua_tostring(L, -1); //Get the comma separated key string from the top of the stack
    std::vector<std::string> strings = l2d_internal::utils::split(test, ',');
    if (strings.size() > 0) {
        if (strings[0].find("[string \"function __genOrderedIndex(t) local") != std::string::npos) {
            strings.clear();
        }
    }
    this->clean();
    return strings;
}

void l2d_internal::LuaScript::lua_save(std::string globalKey) {
    std::ofstream os(this->_fileName);
    long level = 0;
    auto tab = [&level, &os]() {
        for (int a = 0; a < level; ++a) {
            os << "\t";
        }
    };
    if (os.is_open()) {
        std::function<void(std::vector<std::string>, std::string)> doSubKeys = [&](std::vector<std::string> keys, std::string currentKey) {
            level = std::count(currentKey.begin(), currentKey.end(), '.') + 1;
            for (unsigned int i = 0; i < keys.size(); ++i) {
                tab();
                os << keys[i] << " = ";
                if (this->getTableKeys(currentKey + "." + keys[i]).size() > 0) {
                    os << "{" << std::endl;
                    doSubKeys(this->getTableKeys(currentKey + "." + keys[i]), currentKey + "." + keys[i]);
                    level = std::count(currentKey.begin(), currentKey.end(), '.') + 1;
                    tab();
                    os << "" << (level == 2 ? "__" : "") << "}," << std::endl;
                }
                else {
                    os << "\"" << this->get<std::string>(currentKey + "." + keys[i]) + "\"" + (keys[i] != "sprite_path" ? "," : "") << std::endl;
                }
            }
        };
        os << globalKey << " = {" << std::endl;
        doSubKeys(this->getTableKeys(globalKey), globalKey);
        os << "}";
        os.close();
    }
}

void l2d_internal::LuaScript::updateKeyName(std::string oldValue, std::string newValue) {
    std::ifstream in(this->_fileName);
    std::stringstream ss;
    bool done = false;
    for (std::string line; std::getline(in, line); ) {
        size_t pos = line.find(oldValue);
        if (pos != std::string::npos) {
            if (!done) {
                line.replace(pos, oldValue.length(), newValue);
                done = true;
            }
        }
        ss << line << std::endl;
    }
    in.close();
    std::ofstream out(this->_fileName, std::ios_base::trunc);
    out << ss.str() << std::endl;
    out.close();
}

void l2d_internal::LuaScript::clean() {
    int n = lua_gettop(this->L);
    lua_pop(this->L, n);
}
