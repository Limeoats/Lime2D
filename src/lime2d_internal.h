/*
 * lime2d_internal.h
 * By: Mark Guerra
 * Created on 6/10/2016
 */

#ifndef LIME2D_LIME2D_INTERNAL_H
#define LIME2D_LIME2D_INTERNAL_H


#include <SFML/Graphics.hpp>
#include <memory>

namespace l2d_internal {

    //Forward declares
    class Camera;


    //Internal util functions
    namespace utils {

        template<class C, class T>
        inline bool contains(const C &v, const T &x) {
            return std::end(v) != std::find(std::begin(v), std::end(v), x);
        };

        std::vector<std::string> split(std::string str, char c);

        std::vector<const char*> getFilesInDirectory(std::string directory);

        std::string getConfigValue(std::string key);
    }

    //Internal graphics class
    class Graphics {
    public:
        Graphics(sf::RenderWindow* window);
        void draw(sf::Drawable &drawable);
        void draw(const sf::Vertex* vertices, unsigned int vertexCount, sf::PrimitiveType type, const sf::RenderStates &states = sf::RenderStates::Default);
        sf::Texture loadImage(const std::string &filePath);
        void update(float elapsedTime, sf::Vector2f tileSize, bool windowHasFocus);
        std::shared_ptr<Camera> getCamera();
    private:
        std::map<std::string, sf::Texture> _spriteSheets;
        sf::RenderWindow* _window;
        std::shared_ptr<Camera> _camera;
    };

    //Internal sprite class
    class Sprite {
    public:
        Sprite();
        Sprite(std::shared_ptr<Graphics>, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos);
        Sprite(const Sprite&) = delete;
        Sprite& operator=(const Sprite&) = delete;
        virtual void update(float elapsedTime);
        virtual void draw();
    protected:
        sf::Texture _texture;
        sf::Sprite _sprite;
        std::shared_ptr<Graphics> _graphics;
    };

    //Internal Tile class
    class Tile : public Sprite {
    public:
        Tile(std::shared_ptr<Graphics> graphics, std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos, int tilesetId, int layer);
        Tile(const Tile& tile);
        sf::Sprite getSprite() const;
        sf::Texture getTexture() const;
        int getTilesetId() const;
        int getLayer() const;
        virtual ~Tile();
        virtual void update(float elapsedTime);
        virtual void draw();
    private:
        int _tilesetId;
        int _layer;
    };

    //Internal Tileset class
    class Tileset {
    public:
        int Id;
        std::string Name;
        std::string Path;
        sf::Vector2i Size;
        Tileset(int id, std::string name, std::string path, sf::Vector2i size);
    };

    //Internal Layer class
    class Layer {
    public:
        int Id;
        std::vector<std::shared_ptr<Tile>> Tiles;
        void draw();
    };

    //Internal Level class
    class Level {
    public:
        Level(std::shared_ptr<Graphics> graphics, std::string name);
        ~Level();
        void createMap(std::string name, sf::Vector2i size, sf::Vector2i tileSize);
        void loadMap(std::string &name);
        void saveMap(std::string name);
        void draw();
        void update(float elapsedTime);

        std::string getName() const;
        sf::Vector2i getSize() const;
        sf::Vector2i getTileSize() const;

        std::vector<Tileset> getTilesetList();
        std::vector<std::shared_ptr<Layer>> getLayerList();
    private:
        std::string _name;
        sf::Vector2i _size;
        sf::Vector2i _tileSize;
        std::vector<Tileset> _tilesetList;
        std::vector<std::shared_ptr<Layer>> _layerList;

        std::shared_ptr<Graphics> _graphics;
    };

    //Internal Camera class
    class Camera {
    public:
        Camera();
        sf::FloatRect getRect();
        void update(float elapsedTime, sf::Vector2f tileSize, bool windowHasFocus);
    private:
        sf::FloatRect _rect;
        std::shared_ptr<Level> _level;
    };
}


#endif //LIME2D_LIME2D_INTERNAL_H
