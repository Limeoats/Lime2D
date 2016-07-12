/*!
 * \file lime2d_internal.cpp
 * \author Mark Guerra
 * \date 6/10/2016
 * \copyright 2016 Limeoats
 */

#include <glob.h>

#include <sstream>
#include <fstream>
#include <tuple>
#include <iostream>
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

l2d_internal::Sprite::Sprite() { }

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

void l2d_internal::Tile::draw() {
    Sprite::draw();
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
    this->_oldLayerList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_redoList = std::stack<std::vector<std::shared_ptr<Layer>>>();
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

void l2d_internal::Level::createMap(std::string name, sf::Vector2i size, sf::Vector2i tileSize) {
    this->_layerList.clear();
    this->_oldLayerList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_redoList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_tilesetList.clear();
    this->_name = name;
    this->_size = size;
    this->_tileSize = tileSize;
    this->saveMap(name);
    this->loadMap(name);
}

void l2d_internal::Level::loadMap(std::string &name) {
    this->_name = name;
    if (name == "l2dSTART") {
        return;
    }

    this->_layerList.clear();
    this->_tilesetList.clear();
    this->_oldLayerList = std::stack<std::vector<std::shared_ptr<Layer>>>();
    this->_redoList = std::stack<std::vector<std::shared_ptr<Layer>>>();

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
            tsPath = pTileset->Attribute("path");
            this->_tilesetList.push_back(Tileset(tsId, tsPath, sf::Vector2i(tsWidth, tsHeight)));
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
}

void l2d_internal::Level::saveMap(std::string name) {
    XMLDocument document;
    std::stringstream ss;
    ss << l2d_internal::utils::getConfigValue("map_path") << name << ".xml";
    document.LoadFile(ss.str().c_str());
    document.Clear();
    XMLDeclaration* pDeclaration = document.NewDeclaration("xml version=\"1.0\" encoding=\"UTF-8\"");
    document.InsertFirstChild(pDeclaration);

    //Map node
    XMLElement* pMap = document.NewElement("map");
    pMap->SetAttribute("name", name.c_str());
    pMap->SetAttribute("width", this->_size.x);
    pMap->SetAttribute("height", this->_size.y);
    pMap->SetAttribute("tileWidth", this->_tileSize.x);
    pMap->SetAttribute("tileHeight", this->_tileSize.y);

    //Tilesets
    for (Tileset &t : this->_tilesetList) {
        XMLElement* pTileset = document.NewElement("tileset");
        pTileset->SetAttribute("id", t.Id);
        pTileset->SetAttribute("path", t.Path.c_str());
        pTileset->SetAttribute("width", t.Size.x);
        pTileset->SetAttribute("height", t.Size.y);
        pMap->InsertEndChild(pTileset);
    }

    //Tiles
    XMLElement* pTiles = document.NewElement("tiles");

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
        XMLElement* lastPosX = pTiles->LastChildElement("pos");
        XMLElement* lastPosY = pTiles->LastChildElement("pos");
        XMLElement* pPos;
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
        XMLElement *pTile = document.NewElement("tile");
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
    document.InsertAfterChild(pDeclaration, pMap);

    //Save the document
    document.SaveFile(ss.str().c_str());
}

void l2d_internal::Level::updateTile(std::string newTilesetPath, sf::Vector2i newTilesetSize, sf::Vector2i srcPos, sf::Vector2i size,
                                     sf::Vector2f destPos, int tilesetId, int layer) {
    //Set oldLayerList for Undo
    std::vector<std::shared_ptr<l2d_internal::Layer>> tmpList;
    for (int i = 0; i < this->_layerList.size(); ++i) {
        l2d_internal::Layer l;
        l.Id = this->_layerList.at(i).get()->Id;
        for (int j = 0; j < this->_layerList[i].get()->Tiles.size(); ++j) {
            l.Tiles.push_back(this->_layerList[i].get()->Tiles.at(j));
        }
        tmpList.push_back(std::make_shared<l2d_internal::Layer>(l));
    }
    this->_oldLayerList.push(tmpList);

    //Add the tileset to the map if it isn't already
    std::shared_ptr<Tileset> tls = nullptr;
    for (int i = 0; i < this->_tilesetList.size(); ++i) {
        if (this->_tilesetList[i].Id == tilesetId) {
            tls = std::make_shared<Tileset>(this->_tilesetList[i]);
            break;
        }
    }
    if (tls == nullptr) {
        this->_tilesetList.push_back(Tileset(tilesetId, newTilesetPath, sf::Vector2i(newTilesetSize.x / this->_tileSize.x, newTilesetSize.y / this->_tileSize.y)));
    }

    std::shared_ptr<Tile> t = nullptr;
    //Check if the layer exists. If not, create it
    std::shared_ptr<Layer> l = nullptr;
    for (int i = 0; i < this->_layerList.size(); ++i) {
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

    for (int i = 0; i < l.get()->Tiles.size(); ++i) {
        auto tile = l.get()->Tiles[i];
        int tileLayer = tile.get()->getLayer();
        sf::Vector2f tilePos(tile.get()->getSprite().getPosition().x / static_cast<int>(this->_tileSize.x) /
                             static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) + 1,
                             tile.get()->getSprite().getPosition().y / static_cast<int>(this->_tileSize.y) /
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
    sf::Vector2f newDestPos((destPos.x - 1) * this->_tileSize.x * static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))),
                            (destPos.y - 1) * this->_tileSize.y * static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))));
    l.get()->Tiles.push_back(std::make_shared<Tile>(this->_graphics, newTilesetPath, srcPos, this->_tileSize, newDestPos, tilesetId, layer));
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
    if (tile == l->get()->Tiles.end()) {
        //The tile does not exist on the layer
        return false;
    }
    return true;
}

void l2d_internal::Level::removeTile(int layer, sf::Vector2f pos) {
    if (!this->tileExists(layer, this->globalToLocalCoordinates(pos))) {
        return;
    }
    //Set oldLayerList for Undo
    std::vector<std::shared_ptr<l2d_internal::Layer>> tmpList;
    for (int i = 0; i < this->_layerList.size(); ++i) {
        l2d_internal::Layer l;
        l.Id = this->_layerList.at(i).get()->Id;
        for (int j = 0; j < this->_layerList[i].get()->Tiles.size(); ++j) {
            l.Tiles.push_back(this->_layerList[i].get()->Tiles.at(j));
        }
        tmpList.push_back(std::make_shared<l2d_internal::Layer>(l));
    }
    this->_oldLayerList.push(tmpList);

    std::shared_ptr<Tile> t = nullptr;
    //Check if the layer exists. If not, create it
    std::shared_ptr<Layer> l = nullptr;
    for (int i = 0; i < this->_layerList.size(); ++i) {
        if (this->_layerList[i]->Id == layer) {
            l = this->_layerList[i];
            break;
        }
    }

    for (int i = 0; i < l.get()->Tiles.size(); ++i) {
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

void l2d_internal::Level::undo() {
    if (!this->isUndoListEmpty()) {
        std::vector<std::shared_ptr<l2d_internal::Layer>> tmpList;
        for (int i = 0; i < this->_oldLayerList.top().size(); ++i) {
            l2d_internal::Layer l;
            l.Id = this->_oldLayerList.top().at(i).get()->Id;
            for (int j = 0; j < this->_oldLayerList.top()[i].get()->Tiles.size(); ++j) {
                l.Tiles.push_back(this->_oldLayerList.top()[i].get()->Tiles.at(j));
            }
            tmpList.push_back(std::make_shared<l2d_internal::Layer>(l));
        }

        //Set up redo list
        std::vector<std::shared_ptr<l2d_internal::Layer>> tmpRedoList;
        for (int i = 0; i < this->_layerList.size(); ++i) {
            l2d_internal::Layer l;
            l.Id = this->_layerList.at(i).get()->Id;
            for (int j = 0; j < this->_layerList[i].get()->Tiles.size(); ++j) {
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
        for (int i = 0; i < this->_redoList.top().size(); ++i) {
            l2d_internal::Layer l;
            l.Id = this->_redoList.top().at(i).get()->Id;
            for (int j = 0; j < this->_redoList.top()[i].get()->Tiles.size(); ++j) {
                l.Tiles.push_back(this->_redoList.top()[i].get()->Tiles.at(j));
            }
            tmpList.push_back(std::make_shared<l2d_internal::Layer>(l));
        }

        //Set up undo list
        std::vector<std::shared_ptr<l2d_internal::Layer>> tmpUndoList;
        for (int i = 0; i < this->_layerList.size(); ++i) {
            l2d_internal::Layer l;
            l.Id = this->_layerList.at(i).get()->Id;
            for (int j = 0; j < this->_layerList[i].get()->Tiles.size(); ++j) {
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

sf::Vector2i l2d_internal::Level::globalToLocalCoordinates(sf::Vector2f coords) const {
    return sf::Vector2i(static_cast<int>(coords.x) / this->_tileSize.x / static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_x"))) + 1,
                        static_cast<int>(coords.y) / this->_tileSize.y / static_cast<int>(std::stof(l2d_internal::utils::getConfigValue("tile_scale_y"))) + 1);
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
}