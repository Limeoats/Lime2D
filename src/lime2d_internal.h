
/*!
 * \file lime2d_internal.h
 * \author Mark Guerra
 * \date 6/10/2016
 * \copyright 2016 Limeoats
 */

#ifndef LIME2D_LIME2D_INTERNAL_H
#define LIME2D_LIME2D_INTERNAL_H


#include <SFML/Graphics.hpp>
#include <memory>
#include <stack>

namespace l2d_internal {

    /*!
     * Forward declares
     */
    class Camera;

    /*!
     * Enumerations
     */
    enum class LightType {
        None, Ambient, Point
    };

    namespace utils {
        /*!
         * Checks if an element exist in a container
         * @param v The container
         * @param x The element being looked for
         * @return True if element exists, otherwise false
         */
        template<class C, class T>
        inline bool contains(const C &v, const T &x) {
            return std::end(v) != std::find(std::begin(v), std::end(v), x);
        };
        /*!
         * Split a string by a delimiting character
         * @param str The string being split
         * @param c The delimiting character
         * @return A vector containing each of the strings after the split
         */
        std::vector<std::string> split(std::string str, char c);
        /*!
         * Get the names of all files in a given directory
         * @param directory The location of the files
         * @return A vector containing the names of the files in the directory
         */
        std::vector<const char*> getFilesInDirectory(std::string directory);
        /*!
         * Get configuration values from lime2d.config
         * @param key The name of the configuration being retrieved
         * @return The value of the configuration being retrieved
         */
        std::string getConfigValue(std::string key);
    }

    /*!
     * The internal graphics class for Lime2D.
     * Handles the loading, storage, and drawing of all sprites, tiles, and effects
     */
    class Graphics {
    public:
        /*!
         * Graphics constructor
         * Stores the game window and initializes the camera
         * @param window The game's window
         */
        Graphics(sf::RenderWindow* window);
        /*!
         * Draws any drawable object to the window using the camera's current view
         * @param drawable Any drawable SFML object
         * @param ambientLight The ambient light to be drawn
         */
        void draw(sf::Drawable &drawable, sf::Shader* ambientLight = nullptr);
        /*!
         * Draws a primitive shape based on the vertices provided
         * @param vertices The vertices for the shape to be drawn
         * @param vertexCount The number of vertices
         * @param type The type of primitive shape to be drawn
         * @param states The render states for drawing
         */
        void draw(const sf::Vertex* vertices, unsigned int vertexCount, sf::PrimitiveType type, const sf::RenderStates &states = sf::RenderStates::Default);
        /*!
         * Loads an image from disk.
         * Stores the loaded image in the _spriteSheets map so that no image is loaded more than once
         * @param filePath The path to the image being loaded
         * @return An sf::Texture containing the image
         */
        sf::Texture loadImage(const std::string &filePath);
        /*!
         * The update function for the Graphics class
         * @param elapsedTime The amount of time the previous frame took to execute
         * @param tileSize The current map's tile size; used for the camera
         * @param windowHasFocus Whether or not the game window has focus; used for the camera
         */
        void update(float elapsedTime, sf::Vector2f tileSize, bool windowHasFocus);
        /*!
         * Gets a pointer to the camera object
         * @return A pointer to the camera object
         */
        std::shared_ptr<Camera> getCamera();
    private:
        std::map<std::string, sf::Texture> _spriteSheets; /*!< A map of all loaded textures in memory*/
        sf::RenderWindow* _window; /*!< A pointer to the game window*/
        std::shared_ptr<Camera> _camera; /*!< A pointer to the camera object*/
    };

    /*!
     * The internal Sprite class for Lime2D
     * A wrapper around a texture and sf::Sprite used for easy updating and rendering
     */
    class Sprite {
    public:
        /*!
         * The default Sprite constructor.
         * Currently unimplemented
         */
        Sprite();
        /*!
         * The main Sprite constructor
         * @param filePath The path to the spritesheet
         * @param srcPos The coordinates of the sprite on the spritesheet
         * @param size The size of the sprite (in pixels)
         * @param destPos The location on the map to draw the sprite
         */
        Sprite(std::shared_ptr<Graphics>, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos);
        /*!
         * The copy constructor.
         * Copying of sprites is disabled
         */
        Sprite(const Sprite&) = delete;
        /*!
         * Assignment operator overload
         * Assignment of sprites is disabled
         */
        Sprite& operator=(const Sprite&) = delete;
        /*!
         * The update function for the Sprite class
         * @param elapsedTime The amount of time the previous frame took to execute
         */
        virtual void update(float elapsedTime);
        /*!
         * Passes the internal sf::Sprite to the graphics class for drawing
         */
        virtual void draw(sf::Shader* ambientLight = nullptr);
    protected:
        sf::Texture _texture; /*!< The sprite's texture after being loaded*/
        sf::Sprite _sprite;  /*!< The internal sf::Sprite. Holds position, size, and more*/
        std::shared_ptr<Graphics> _graphics; /*!< A pointer to the internal graphics object*/
    };

    /*!
     * The internal Tile class for Lime2D.
     * Extends off of the sprite class.
     */
    class Tile : public Sprite {
    public:
        /*!
         * The Tile constructor
         * @param graphics A pointer to Lime2d's graphics object
         * @param filePath The path to the tileset
         * @param srcPos The location of the tile on the tileset
         * @param size The size of the tile (in pixels)
         * @param destPos The location on the map to draw the tile
         * @param tilesetId The unique tileset id that the tile belongs to
         * @param layer The layer that the tile is on
         */
        Tile(std::shared_ptr<Graphics> graphics, std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos, int tilesetId, int layer);
        /*!
         * The tile copy constructor
         * @param tile The other tile being copied
         */
        Tile(const Tile& tile);
        /*!
         * The destructor for the tile class. Cleans up resources
         */
        virtual ~Tile();
        /*!
         * Gets the internal sprite for the tile
         * @return The internal sprite for the tile
         */
        sf::Sprite getSprite() const;
        /*!
         * Gets the internal texture for the tile
         * @return The internal texture for the tile
         */
        sf::Texture getTexture() const;
        /*!
         * Gets the unique tileset id that the tile belongs to
         * @return The tileset id
         */
        int getTilesetId() const;
        /*!
         * Gets the layer that the tile is on
         * @return The layer number
         */
        int getLayer() const;
        /*!
         * The update function for the Tile class
         * @param elapsedTime The amount of time the previous frame took to execute
         */
        virtual void update(float elapsedTime);
        /*!
         * Calls the base Sprite class draw method
         * @param ambientLight The ambient light shader for the tile
         */
        virtual void draw(sf::Shader* ambientLight);
    private:
        int _tilesetId; /*!< The id for the tileset the tile belongs to*/
        int _layer; /*!< The layer number the tile is on*/
    };

    /*!
     * The internal Tileset class for Lime2D
     */
    class Tileset {
    public:
        int Id; /*!< The tileset Id*/
        std::string Path; /*!< The path to the tileset*/
        sf::Vector2i Size; /*!< The size of the tileset (in pixels)*/
        /*!
         * The Tileset constructor
         * @param id The tileset id
         * @param path The path to the tileset
         * @param size The size of the tileset (in pixels)
         */
        Tileset(int id, std::string path, sf::Vector2i size);
    };

    /*!
     * The internal Layer class for Lime2D
     */
    class Layer {
    public:
        int Id; /*!< The layer id number*/
        std::vector<std::shared_ptr<Tile>> Tiles; /*!< The list of tile pointers in the layer*/
        /*!
         * Default constructor for Layer
         */
        Layer();
        /*!
         * Loops through all of the tiles on the layer and calls their draw method
         * @param ambientLight The ambient light shader for the layer
         */
        void draw(sf::Shader* ambientLight);
    };

    /*!
     * The internal Level class for Lime2D
     */
    class Level {
    public:
        /*!
         * Constructor
         * @param graphics A pointer to the internal graphics object
         * @param name The name of the first map being loaded
         */
        Level(std::shared_ptr<Graphics> graphics, std::string name);
        /*!
         * Destructor
         */
        ~Level();
        /*!
         * Creates a brand new map, saves it, and then loads it for immediate use
         * @param name The name of the new map
         * @param size The size of the new map (in tiles)
         * @param tileSize The size of the tiles on the new map
         */
        void createMap(std::string name, sf::Vector2i size, sf::Vector2i tileSize);
        /*!
         * Loads an existing map into memory
         * @param name The name of the map to load
         */
        void loadMap(std::string &name);
        /*!
         * Saves the currently loaded map
         * @param name The file name for the map being saved
         */
        void saveMap(std::string name);
        /*!
         * Loops through the layers/tiles and draws everything on the map
         * @param ambientLight The ambient light shader for the level
         */
        void draw(sf::Shader* ambientLight);
        /*!
         * Updates anything on the map that needs updating.
         * @param elapsedTime The amount of time the previous frame took to execute
         */
        void update(float elapsedTime);
        /*!
         * Gets the name of the currently loaded map
         * @return The name of the currently loaded map
         */
        std::string getName() const;
        /*!
         * Gets the size of the currently loaded map (in tiles)
         * @return The size of the currently loaded map (in tiles)
         */
        sf::Vector2i getSize() const;
        /*!
         * Get the map's tile size
         * @return The map's tile size
         */
        sf::Vector2i getTileSize() const;
        /*!
         * Get the ambient light intensity
         * @return The intensity of the ambient light
         */
        float getAmbientIntensity() const;
        /*!
         * Get the ambient light color
         * @return The ambient light color
         */
        sf::Color getAmbientColor() const;
        /*!
         * Set the ambient intensity
         * @param intensity The new intensity
         */
        void setAmbientIntensity(float intensity);
        /*!
         * Set the ambient color
         * @param color The new color
         */
        void setAmbientColor(sf::Color color);
        /*!
         * Get a list of all of the tilesets being used on the currently loaded map
         * @return A list of all of the tilesets being used on the currently loaded map
         */
        std::vector<Tileset> getTilesetList();
        /*!
         * Get a list of all of the layers on the currently loaded map
         * @return A list of all of the layers on the currently loaded map
         */
        std::vector<std::shared_ptr<Layer>> getLayerList();
        /*!
         * Removes the tile at a given position and layer
         * @param layer The layer containing the tile to remove
         * @param pos The position on the map of the tile to remove (in pixels)
         */
        void removeTile(int layer, sf::Vector2f pos);
        /*!
         * Updates a tile on the map by first removing the existing one and then adding the new one
         * @param newTilesetPath The tileset containing the new tile to be drawn
         * @param newTilesetSize The size of the new tileset
         * @param srcPos The location on the tileset where the tile is
         * @param size The size of the tile (in pixels)
         * @param destPos The location on the map to draw the new tile
         * @param tilesetId The id number of the tileset
         * @param layer The layer the updating is happening on
         */
        void updateTile(std::string newTilesetPath, sf::Vector2i newTilesetSize, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos, int tilesetId, int layer);
        /*!
         * Checks if a tile exists
         * @param layer The layer the tile would be on
         * @param pos The position of the tile in local coordinates
         */
        bool tileExists(int layer, sf::Vector2i pos) const;
        /*!
         * Undo the previous action. Currently goes back until the undo list is empty
         */
        void undo();
        /*!
         * Get whether or not the undo list is empty
         * @return True if the undo list is empty, otherwise false
         */
        bool isUndoListEmpty() const;
        /*!
         * Redo the previous undo.
         */
        void redo();
        /*!
         * Get whether or not the redo list is empty
         * @return True if the redo list is empty, otherwise false
         */
        bool isRedoListEmpty() const;
        /*!
         * Converts global coordinates (pixels) to local coordinates (tiles)
         * @param coords The global coordinates in pixels
         * @return The local coordinates in tiles
         */
        sf::Vector2i globalToLocalCoordinates(sf::Vector2f coords) const;
    private:
        std::string _name; /*!< The name of the map*/
        sf::Vector2i _size; /*!< The size of the map (in tile coordinates)*/
        sf::Vector2i _tileSize; /*!< The size of the tiles on the map*/
        std::vector<Tileset> _tilesetList; /*!< The list of tilesets being used on the map*/
        std::vector<std::shared_ptr<Layer>> _layerList; /*!< The list of layer pointers that exist on the map*/
        std::shared_ptr<Graphics> _graphics; /*!< A pointer to the internal graphics object*/
        std::stack<std::vector<std::shared_ptr<Layer>>> _oldLayerList; /*!< A stack of all layers/tiles for undo*/
        std::stack<std::vector<std::shared_ptr<Layer>>> _redoList; /*!< A stack of all layers/tiles for redo*/
        float _ambientIntensity = 1.0f;
        sf::Color _ambientColor = sf::Color::White;
    };

    /*!
     * The internal camera class for Lime2D
     */
    class Camera {
    public:
        /*!
         * The default constructor.
         * Initializes the position and size of the camera
         */
        Camera();
        /*!
         * Gets the rectangle that represents the camera's position and size
         * @return The rectangle representing the camera's position and size
         */
        sf::FloatRect getRect();
        /*!
         * Update method for the camera. Updates position, checks for key inputs for movement, etc
         * @param elapsedTime The amount of time the previous frame took to execute
         * @param tileSize The current map's tile size
         * @param windowHasFocus Whether or not the game window has focus
         */
        void update(float elapsedTime, sf::Vector2f tileSize, bool windowHasFocus);
    private:
        sf::FloatRect _rect; /*!< The rectangle representing the position and size of the camera*/
        std::shared_ptr<Level> _level; /*!< A pointer to the currently loaded level*/
    };
}


#endif //LIME2D_LIME2D_INTERNAL_H
