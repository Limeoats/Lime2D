/*
 * Lime2d_internal.h
 * By: Mark Guerra
 * Created on: 6/10/2016
 * Copyright (c) 2016 Limeoats
 */

#ifndef LIME2D_LIME2D_INTERNAL_H
#define LIME2D_LIME2D_INTERNAL_H

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
};


#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>
#include <sstream>
#include <cstring>

namespace l2d_internal {

    /*
     * Forward declares
     */
    class Camera;
    class Shape;

    /*
     * Enumerations
     */
    enum class Features {
        None, Map, Animation
    };
    enum class MapEditorMode {
        None, Tile, Object
    };
    enum class LightType {
        None, Ambient, Point
    };
    enum class DrawShapes {
        None, Rectangle, Point, Line
    };
    enum class ObjectTypes {
        None, Collision, Other
    };
    enum class WindowTypes {
        None, TilesetWindow, NewMapWindow, ConfigWindow, MapSelectWindow, AboutWindow, LightEditorWindow,
        NewAnimatedSpriteWindow, NewAnimationWindow, RemoveAnimationWindow, EntityListWindow, EntityPropertiesWindow, ShapeColorWindow,
        ConfigureMapWindow, ConfigureBackgroundColorWindow
    };

    namespace utils {
        template<class C, class T>
        inline bool contains(const C &v, const T &x) {
            return std::end(v) != std::find(std::begin(v), std::end(v), x);
        };
        std::vector<std::string> split(std::string str, char c);
        std::vector<std::string> split(const std::string& str, const std::string& delim, int count = -1);
        std::vector<const char*> getFilesInDirectory(std::string directory);
        std::string getConfigValue(std::string key);
        void createNewAnimationFile(std::string name, std::string spriteSheetPath);
        void addNewAnimationToAnimationFile(std::string fileName, std::string animationName);
        void removeAnimationFromAnimationFile(std::string fileName, std::string animationName);
        std::vector<const char*> getObjectTypesForList();

        class NotImplementedException : public std::logic_error {
        public:
            NotImplementedException(std::string method = "") : std::logic_error("Function not yet implemented." + (method.length() > 0 ? " Method: " + method : "")) {};
        };
    }

    /*
     * The internal graphics class for Lime2D.
     * Handles the loading, storage, and drawing of all sprites, tiles, and effects
     */
    class Graphics {
    public:
        Graphics(sf::RenderWindow* window);
        void draw(sf::Drawable &drawable, sf::Shader* ambientLight = nullptr);
        void draw(const sf::Vertex* vertices, unsigned int vertexCount, sf::PrimitiveType type, const sf::RenderStates &states = sf::RenderStates::Default);
        sf::Texture loadImage(const std::string &filePath);
        void zoom(float n, sf::Vector2i pixel);
        void update(float elapsedTime, sf::Vector2f tileSize, bool windowHasFocus);
        sf::View getView() const;
        float getZoomPercentage() const;
    private:
        std::map<std::string, sf::Texture> _spriteSheets;
        sf::RenderWindow* _window;
        sf::View _view;
        float _zoomPercentage;
    };

    /*
     * The internal Sprite class for Lime2D
     * A wrapper around a texture and sf::Sprite used for easy updating and rendering
     */
    class Sprite {
    public:
        Sprite() = default;
        Sprite(std::shared_ptr<Graphics>, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos);
        Sprite(const Sprite&) = delete;
        Sprite& operator=(const Sprite&) = delete;
        virtual void update(float elapsedTime);
        virtual void draw(sf::Shader* ambientLight = nullptr);
    protected:
        sf::Texture _texture;
        sf::Sprite _sprite;
        std::shared_ptr<Graphics> _graphics;
    };

    /*
     * The internal AnimatedSprite class for Lime2D
     * Extends off of the base Sprite class. Handles animations.
     */
    class AnimatedSprite : public Sprite {
    public:
        AnimatedSprite(std::shared_ptr<Graphics> graphics, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos, float timeToUpdate);
        void playAnimation(std::string animation, bool once = false);
        virtual void update(float elapsedTime) override;
        virtual void draw(sf::Shader* ambientLight = nullptr) override;
        void addAnimation(int frames, sf::Vector2i srcPos, std::string name, sf::Vector2i size, sf::Vector2i offset);
        void updateAnimation(int frames, sf::Vector2i srcPos, std::string name, sf::Vector2i size, sf::Vector2i offset, float timeToUpdate);
        void removeAnimation(std::string name);
        void setVisible(bool visible);
        sf::Sprite getSprite() const;
    protected:
        float _timeToUpdate;
        bool _currentAnimationOnce;
        std::string _currentAnimation;

        void resetAnimation();
        void stopAnimation();
    private:
        std::map<std::string, std::vector<sf::IntRect>> _animations;
        std::map<std::string, sf::Vector2i> _offsets;
        unsigned int _frameIndex;
        float _timeElapsed;
        bool _visible;
    };

    /*
     * The internal Tile class for Lime2D.
     * Extends off of the sprite class. Represents tiles drawn on a level
     */
    class Tile : public Sprite {
    public:
        Tile(std::shared_ptr<Graphics> graphics, std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos, int tilesetId, int layer);
        Tile(const Tile& tile);
        virtual ~Tile();
        sf::Sprite getSprite() const;
        sf::Texture getTexture() const;
        int getTilesetId() const;
        int getLayer() const;
        virtual void update(float elapsedTime);
        virtual void draw(sf::Shader* ambientLight);
    private:
        int _tilesetId;
        int _layer;
    };

    /*
     * The internal Tileset class for Lime2D
     * This is the image where the level's tiles come from
     */
    class Tileset {
    public:
        int Id;
        std::string Path;
        sf::Vector2i Size;
        Tileset(int id, std::string path, sf::Vector2i size);
    };

    /*
     * The internal Layer class for Lime2D
     */
    class Layer {
    public:
        int Id;
        std::vector<std::shared_ptr<Tile>> Tiles;
        Layer();
        void draw(sf::Shader* ambientLight);
    };

    /*
     * The internal Level class for Lime2D
     */
    class Level {
    public:
        Level(std::shared_ptr<Graphics> graphics, std::string name);
        ~Level();
        void createMap(std::string name, sf::Vector2i size, sf::Vector2i tileSize);
        std::string loadMap(std::string &name);
        void saveMap(std::string name);
        void draw(sf::Shader* ambientLight);
        void update(float elapsedTime);
        std::string getName() const;
        sf::Vector2i getSize() const;
        void setSize(sf::Vector2i size);
        void updateTileList();
        sf::Vector2i getTileSize() const;
        float getAmbientIntensity() const;
        sf::Color getAmbientColor() const;
        void setAmbientIntensity(float intensity);
        void setAmbientColor(sf::Color color);
        std::vector<Tileset> getTilesetList();
        std::vector<std::shared_ptr<Layer>> getLayerList();
        void addShape(std::shared_ptr<l2d_internal::Shape> shape);
        std::vector<std::shared_ptr<l2d_internal::Shape>> getShapeList();
        void removeTile(int layer, sf::Vector2f pos, bool fromResize = false);
        void updateTile(std::string newTilesetPath, sf::Vector2i newTilesetSize, sf::Vector2i srcPos, sf::Vector2f destPos, int tilesetId, int layer);
        void updateShape(std::shared_ptr<l2d_internal::Shape> oldShape, std::shared_ptr<l2d_internal::Shape> newShape);
        void removeShape(std::shared_ptr<l2d_internal::Shape> shape);
        bool tileExists(int layer, sf::Vector2i pos) const;
        int getTilesetID(const std::string &path) const;
        void undo();
        bool isUndoListEmpty() const;
        void redo();
        bool isRedoListEmpty() const;
        sf::Vector2i globalToLocalCoordinates(sf::Vector2f coords) const;
    private:
        std::string _name;
        sf::Vector2i _size;
        sf::Vector2i _tileSize;
        std::vector<Tileset> _tilesetList;
        std::vector<std::shared_ptr<Layer>> _layerList;
        std::vector<std::shared_ptr<l2d_internal::Shape>> _shapeList;
        std::shared_ptr<Graphics> _graphics;
        std::stack<std::vector<std::shared_ptr<Layer>>> _oldLayerList;
        std::stack<std::vector<std::shared_ptr<Layer>>> _redoList;
        float _ambientIntensity = 1.0f;
        sf::Color _ambientColor = sf::Color::White;
    };

    struct CustomProperty {
    public:
        CustomProperty() : Id(-1) {
            std::strcpy(Name, "");
            std::strcpy(Value, "");
        }
        CustomProperty(int id, std::string name, std::string value) : Id(id) {
            std::strcpy(Name, name.c_str());
            std::strcpy(Value, value.c_str());
        }
        int Id;
        char Name[100];
        char Value[100];
    };

    /*
     * The internal Shape class for Lime2D
     */
    class Shape {
    public:
        Shape(std::string name, sf::Color color);
        std::string getName();

        virtual sf::Color getColor() const;
        void setName(std::string name);
        virtual void setColor(sf::Color color);
        std::vector<l2d_internal::CustomProperty> getCustomProperties();
        void addCustomProperty(std::string name, std::string value);
        void removeCustomProperty(int id);
        void setCustomProperties(std::vector<l2d_internal::CustomProperty> &others);
        void clearCustomProperties();
        virtual void fixPosition(sf::Vector2i levelSize, sf::Vector2i tileSize, sf::Vector2f tileScale) = 0;
        virtual bool isPointInside(sf::Vector2f point) = 0;
        virtual void select() = 0;
        virtual void unselect() = 0;
        virtual void setPosition(sf::Vector2f pos) = 0;
        virtual void setSize(sf::Vector2f size) = 0;
        virtual void draw(sf::RenderWindow* window) = 0;
        virtual bool equals(std::shared_ptr<Shape> other) = 0;
    protected:
        std::string _name;
        sf::Color _color = sf::Color::White;
        bool _selected;
        std::vector<l2d_internal::CustomProperty> _customProperties;
    };


    /*
     * The internal Point class for Lime2D
     * Extends off of Shape
     */
    class Point : public Shape {
    public:
        Point(std::string name, sf::Color color, sf::CircleShape dot);
        sf::CircleShape getCircle();
        virtual sf::Color getColor() const override;
        virtual void setColor(sf::Color color) override;
        virtual void fixPosition(sf::Vector2i levelSize, sf::Vector2i tileSize, sf::Vector2f tileScale) override;
        virtual bool isPointInside(sf::Vector2f point) override;
        virtual void select() override;
        virtual void unselect() override;
        virtual void setPosition(sf::Vector2f pos) override;
        virtual void setSize(sf::Vector2f size) override;
        virtual void draw(sf::RenderWindow* window) override;
        virtual bool equals(std::shared_ptr<Shape> other) override;
    private:
        sf::CircleShape _dot;
    };

    /*
     * The internal Line class for Lime2D
     * Extends off of shape
     */
    class Line : public Shape {
    public:
        Line(std::string name, sf::Color color, std::vector<std::shared_ptr<l2d_internal::Point>> points);
        std::vector<std::shared_ptr<l2d_internal::Point>> getPoints();
        std::shared_ptr<Point> getSelectedPoint(sf::Vector2f mousePos);
        void deletePoint(std::shared_ptr<l2d_internal::Point> p);
        virtual sf::Color getColor() const override;
        virtual void setColor(sf::Color color) override;
        virtual void fixPosition(sf::Vector2i levelSize, sf::Vector2i tileSize, sf::Vector2f tileScale) override;
        virtual bool isPointInside(sf::Vector2f point) override;
        virtual void select() override;
        virtual void unselect() override;
        virtual void setPosition(sf::Vector2f pos) override;
        virtual void setSize(sf::Vector2f size) override;
        virtual void draw(sf::RenderWindow* window) override;
        virtual bool equals(std::shared_ptr<Shape> other) override;
    private:
        std::vector<std::shared_ptr<l2d_internal::Point>> _points;
    };

    /*
     * The internal Rectangle class for Lime2D
     * Extends off of Shape
     */
    class Rectangle : public Shape {
    public:
        Rectangle(std::string name, sf::Color color, l2d_internal::ObjectTypes objectType, sf::RectangleShape rect);
        sf::RectangleShape getRectangle();
        virtual sf::Color getColor() const override;
        virtual void setColor(sf::Color color) override;
        virtual void fixPosition(sf::Vector2i levelSize, sf::Vector2i tileSize, sf::Vector2f tileScale) override;
        l2d_internal::ObjectTypes getObjectType();
        void setObjectType(l2d_internal::ObjectTypes objectType);
        virtual bool isPointInside(sf::Vector2f point) override;
        virtual void select() override;
        virtual void unselect() override;
        virtual void setPosition(sf::Vector2f pos) override;
        virtual void setSize(sf::Vector2f size) override;
        virtual void draw(sf::RenderWindow* window) override;
        virtual bool equals(std::shared_ptr<Shape> other) override;
    private:
        sf::RectangleShape _rect;
        l2d_internal::ObjectTypes _objectType;
    };

    /*
     * The internal LuaScript class
     * Handles the reading and writing of Lua scripts
     */
    class LuaScript {
    public:
        LuaScript(const std::string &filePath);
        ~LuaScript();
        void printError(const std::string &variable, const std::string error);

        template<typename T>
        T lua_getDefault() {
            return {};
        }

        template<typename T>
        T lua_get(const std::string &variable) {
            (void)variable;
            return 0;
        }

        template<typename T>
        T get(const std::string &variable) {
            if (this->L == nullptr) {
                this->printError(variable, "Lua script is not loaded.");
                return lua_getDefault<T>();
            }
            T result;
            if (lua_getVariable(variable)) {
                result = lua_get<T>(variable);
            }
            else {
                result = lua_getDefault<T>();
            }
            this->clean();
            return result;
        }
        std::vector<std::string> getTableKeys(const std::string &variable);

        template<typename T>
        void lua_set(const std::string &key, T value) {
            if (this->L == nullptr) {
                this->printError(key, "Lua script is not loaded.");
                return;
            }
            std::stringstream ss;
            ss << value;

            int level = 0;
            std::string var = "";
            for (unsigned long i = 0; i < key.length(); ++i) {
                if (key.at(i) == '.') {
                    if (level == 0) {
                        lua_getglobal(this->L, var.c_str());
                    }
                    else {
                        lua_getfield(this->L, -1, var.c_str());
                    }
                    if (lua_isnil(this->L, -1)) {
                        this->printError(key, var + " is not defined.");
                    }
                    else {
                        var = "";
                        ++level;
                    }
                }
                else {
                    var += key.at(i);
                }
            }
            lua_getfield(this->L, -1, var.c_str());
            lua_pop(this->L, 1);
            lua_pushstring(this->L, ss.str().c_str());
            lua_setfield(this->L, -2, var.c_str());
            this->clean();
        }
        void lua_save(std::string globalKey);
        void updateKeyName(std::string oldValue, std::string newValue);
    private:
        lua_State* L;
        std::string _fileName;

        bool lua_getVariable(const std::string &variable);
        void clean();
    };

    template<>
    inline bool LuaScript::lua_get<bool>(const std::string &variable) {
        if (!lua_isboolean(this->L, -1)) {
            this->printError(variable, "Not a boolean");
        }
        return static_cast<bool>(lua_toboolean(this->L, -1));
    }

    template<>
    inline float LuaScript::lua_get<float>(const std::string &variable) {
        if (!lua_isnumber(this->L, -1)) {
            this->printError(variable, "Not a number");
        }
        return static_cast<float>(lua_tonumber(this->L, -1));
    }

    template<>
    inline int LuaScript::lua_get<int>(const std::string &variable) {
        if (!lua_isnumber(this->L, -1)) {
            this->printError(variable, "Not a number");
        }
        return static_cast<int>(lua_tonumber(this->L, -1));
    }

    template<>
    inline sf::Uint8 LuaScript::lua_get<sf::Uint8>(const std::string &variable) {
        if (!lua_isnumber(this->L, -1)) {
            this->printError(variable, "Not a number");
        }
        return static_cast<sf::Uint8>(lua_tonumber(this->L, -1));
    }

    template<>
    inline std::string LuaScript::lua_get<std::string>(const std::string &variable) {
        std::string s = "null";
        if (lua_isstring(this->L, -1)) {
            s = std::string(lua_tostring(this->L, -1));
        } else {
            this->printError(variable, "Not a string");
        }
        return s;
    }

    template<>
    inline std::string LuaScript::lua_getDefault<std::string>() {
        return "null";
    }
}




#endif //LIME2D_LIME2D_INTERNAL_H