
/*!
 * \file lime2d_internal.h
 * \author Mark Guerra
 * \date 6/10/2016
 * \copyright 2016 Limeoats
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

namespace l2d_internal {
  
  /*!
   * Forward declares
   */
  class Camera;
  
  /*!
   * Enumerations
   */
  enum class Features {
    None, Map, Animation
      };
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
     * Split a string by a delimiting string
     * @param str The string being split
     * @param delim The delimiting string
     * @param count The number of times to do the split. Default is -1 (all)
     * @return A vector containing each of the strings after the split
     */
    std::vector<std::string> split(const std::string& str, const std::string& delim, int count = -1);
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
    /*!
     * Create a new animation Lua file
     * @param name The name of the new sprite
     */
    void createNewAnimationFile(std::string name, std::string spriteSheetPath);
    /*!
     * Add a new animation to the Lua file
     * @param fileName The name of the Lua file
     * @param animationName The name of the animation being added
     */
    void addNewAnimationToAnimationFile(std::string fileName, std::string animationName);
    /*!
     * Remove an animation from the Lua file
     * @param fileName The name of the Lua file
     * @param animationName The name of the animation being removed
     */
    void removeAnimationFromAnimationFile(std::string fileName, std::string animationName);
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
     * @param ambientLight The ambient light shader. Defaults to null.
     */
    virtual void draw(sf::Shader* ambientLight = nullptr);
  protected:
    sf::Texture _texture; /*!< The sprite's texture after being loaded*/
    sf::Sprite _sprite;  /*!< The internal sf::Sprite. Holds position, size, and more*/
    std::shared_ptr<Graphics> _graphics; /*!< A pointer to the internal graphics object*/
  };
  
  /*!
   * The internal AnimatedSprite class for Lime2D
   * Extends off of the base Sprite class. Handles animations.
   */
  class AnimatedSprite : public Sprite {
  public:
    /*!
     * The main AnimatedSprite constructor
     * @param filePath The path to the spritesheet
     * @param srcPos The coordinates of the first frame of the sprite on the spritesheet
     * @param size The size of the sprite (in pixels)
     * @param destPos The location on the map to draw the animated sprite
     * @param timeToUpdate The amount of time between animation frames
     */
    AnimatedSprite(std::shared_ptr<Graphics> graphics, const std::string &filePath, sf::Vector2i srcPos, sf::Vector2i size, sf::Vector2f destPos, float timeToUpdate);
    /*!
     * Plays the specified animation
     * @param animation The animation to play
     * @param once Whether or not to only play the animation once
     */
    void playAnimation(std::string animation, bool once = false);
    /*!
     * The update function for the animated sprite class
     * @param elapsedTime The amount of time the previous frame took to execute
     */
    virtual void update(float elapsedTime) override;
    /*!
     * Draws the animated sprite at the current frame
     * @param ambientLight The ambient light shader. Defaults to null.
     */
    virtual void draw(sf::Shader* ambientLight = nullptr) override;
    /*!
     * Adds a new animation to the animated sprite
     * @param frames The number of frames in the new animation
     * @param srcPos The coordinates of the first frame of the animation on the spritesheet
     * @param name The name of the new animation
     * @param size The size of the sprite/animation (in pixels)
     * @param offset The offset (if there is one)
     */
    void addAnimation(int frames, sf::Vector2i srcPos, std::string name, sf::Vector2i size, sf::Vector2i offset);
    /*!
     * Update an animation
     * @param frames New number of frames
     * @param srcPos New src pos
     * @param name New name
     * @param description New description
     * @param filePath New file path
     * @param size New size
     * @param offset New offset
     * @param timeToUpdate New time to update
     */
    void updateAnimation(int frames, sf::Vector2i srcPos, std::string name, sf::Vector2i size, sf::Vector2i offset, float timeToUpdate);
    /*!
     * Remove an animation for the animated sprite
     * @param name The name of the animation to be removed
     */
    void removeAnimation(std::string name);
    /*!
     * Sets the visibility of the animated sprite
     * @param visible Whether or not the animated sprite is visible
     */
    void setVisible(bool visible);
    /*!
     * Gets the internal sprite
     * @return The internal sprite
     */
    sf::Sprite getSprite() const;
  protected:
    float _timeToUpdate; /*!< The amount of time between animation frames*/
    bool _currentAnimationOnce; /*!< Whether or not this animation should only be played once*/
    std::string _currentAnimation; /*!< The current animation being played for the sprite*/
    /*!
     * Resets the current animation
     */
    void resetAnimation();
    /*!
     * Stop the current animation
     */
    void stopAnimation();
  private:
    std::map<std::string, std::vector<sf::IntRect>> _animations; /*!< A list of the animations in the form of rectangles on the spritesheet*/
    std::map<std::string, sf::Vector2i> _offsets; /*!< A list of the offsets for the animations*/
    int _frameIndex; /*!< The current frame the animation is on*/
    float _timeElapsed; /*!< The amount of time that has elapsed since the last animation frame*/
    bool _visible; /*!< Whether or not the animation is currently visible*/
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
     * @return Any error messages
     */
    std::string loadMap(std::string &name);
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
     * @param destPos The location on the map to draw the new tile
     * @param tilesetId The id number of the tileset
     * @param layer The layer the updating is happening on
     */
    void updateTile(std::string newTilesetPath, sf::Vector2i newTilesetSize, sf::Vector2i srcPos, sf::Vector2f destPos, int tilesetId, int layer);
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

  /*!
   * The internal LuaScript class
   * Handles the reading and writing of Lua scripts
   */
  class LuaScript {
  public:
    /*!
     * The LuaScript constructor
     * @param filePath  The path to the lua script
     */
    LuaScript(const std::string &filePath);
    /*!
     * The LuaScript destructor. Frees memory
     */
    ~LuaScript();
    /*!
     * Prints the error and the variable if something goes wrong
     * @param variable The variable with the error
     * @param error An error message describing what went wrong
     */
    void printError(const std::string &variable, const std::string error);
    /*!
     * A default for string variables
     * @return Default value for string variables
     */
    template<typename T>
      T lua_getDefault() {
      return {};
    }
    /*!
     * Default lua_get function.
     * This function is overloaded with different data types below
     * @param variable The variable being gotten from the script
     * @return Default = 0
     */
    template<typename T>
      T lua_get(const std::string &variable) {
      (void)variable;
      return 0;
    }
    /*!
     * Get the value of the variable from the script
     * @param variable The variable being gotten from the script
     * @return The value of the variable
     */
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
    /*!
     * Get all of the keys under a variable
     * @param variable The variable being checked
     * @return A list of the names of the keys
     */
    std::vector<std::string> getTableKeys(const std::string &variable);

    /*!
     * Set a new value for the given key
     * @param key The key
     * @param value The new value
     */
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
      for (int i = 0; i < key.length(); ++i) {
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
    /*!
     * Save the lua file in its current state
     */
    void lua_save(std::string globalKey);
    /*!
     * Update a key name in the Lua file
     * @param oldValue The old value of the key name
     * @param newValue The new value of the key name
     */
    void updateKeyName(std::string oldValue, std::string newValue);
  private:
    lua_State* L; /*!< The current lua state*/
    std::string _fileName; /*!< The current lua script file name*/

    /*!
     * The internal function for getting the variable value
     * @param variable The variable being gotten from the script
     * @return The value of the variable
     */
    bool lua_getVariable(const std::string &variable);
    /*!
     * Removes the top item from the Lua stack
     */
    void clean();
  };

  /*!
   * Boolean overload for lua_get
   * @param variable The variable being checked
   * @return The boolean value of the variable
   */
  template<>
    inline bool LuaScript::lua_get<bool>(const std::string &variable) {
    if (!lua_isboolean(this->L, -1)) {
      this->printError(variable, "Not a boolean");
    }
    return static_cast<bool>(lua_toboolean(this->L, -1));
  }
  /*!
   * Float overload for lua_get
   * @param variable The variable being checked
   * @return The float value of the variable
   */
  template<>
    inline float LuaScript::lua_get<float>(const std::string &variable) {
    if (!lua_isnumber(this->L, -1)) {
      this->printError(variable, "Not a number");
    }
    return static_cast<float>(lua_tonumber(this->L, -1));
  }
  /*!
   * Integer overload for lua_get
   * @param variable The variable being checked
   * @return The integer value of the variable
   */
  template<>
    inline int LuaScript::lua_get<int>(const std::string &variable) {
    if (!lua_isnumber(this->L, -1)) {
      this->printError(variable, "Not a number");
    }
    return static_cast<int>(lua_tonumber(this->L, -1));
  }
  /*!
   * Uint8 overload for lua_get
   * @param variable The variable being checked
   * @return The Uint8 value of the variable
   */
  template<>
    inline sf::Uint8 LuaScript::lua_get<sf::Uint8>(const std::string &variable) {
    if (!lua_isnumber(this->L, -1)) {
      this->printError(variable, "Not a number");
    }
    return static_cast<sf::Uint8>(lua_tonumber(this->L, -1));
  }
  /*!
   * String overload for lua_get
   * @param variable The variable being checked
   * @return The string value of the variable
   */
  template<>
    inline std::string LuaScript::lua_get<std::string>(const std::string &variable) {
    std::string s = "null";
    if (lua_isstring(this->L, -1)) {
      s = std::string(lua_tostring(this->L, -1));
    }
    else {
      this->printError(variable, "Not a string");
    }
    return s;
  }
  /*!
   * Default overload for lua_get
   * @return The null string
   */
  template<>
    inline std::string LuaScript::lua_getDefault<std::string>() {
    return "null";
  }
}




#endif //LIME2D_LIME2D_INTERNAL_H
