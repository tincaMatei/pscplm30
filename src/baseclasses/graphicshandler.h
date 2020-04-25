#ifndef __GRAPHICSHANDLER_H
#define __GRAPHICSHANDLER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <string>
#include "geometry.h"

// Draw the contour of the polygon with the last renderer color
void drawPolygon(SDL_Renderer* renderer, Polygon* poly);

// Draw text on the screen
class FontRenderer {
private:
  // Given font
  TTF_Font* usedFont;
public:
  // Load the font with the given size
  FontRenderer(const char* filename, int fontsize);
  ~FontRenderer();
  
  // Convert an int to a string
  static std::string inttostring(int x);
  
  // Render text on the screen at the given position
  void renderText(SDL_Renderer* renderer, float _x, float _y, 
                  std::string text);
};

// Render different textures from tilesets on the screen
class TileSetRenderer {
private:
  // The loaded texture
  int textureWidth, textureHeight;

  // The loaded texture
  SDL_Texture* texture;
  
  // Tile width and height
  int tileWidth, tileHeight;

  // Rotation angle
  double rotationAngle;
  
  // Rotation center
  SDL_Point* rotationCenter;
  
  // Texture flip
  SDL_RendererFlip textureFlip;
public:
  // Create a tileset from the filename.
  // Pass renderer for the renderer
  // The size of a tile will be (_tw and _th)
  TileSetRenderer(const char* filename, SDL_Renderer* renderer,
                  int _tw = 0, int _th = 0);
  
  // Unload text
  ~TileSetRenderer();
  
  // Get the rendering target if we want to render a texture with center
  // in (x, y)
  SDL_Rect getCenteredPosition(int x, int y);
  
  // Set the rotation angle to x
  // Rotation is in clockwise order
  void setRotationAngle(double x);
  
  // Set the rotation center to x
  void setRotationCenter(SDL_Point x);
  
  // Set the texture to rotate by it's center
  void setCentroidRotation();
  
  // Set the texture to be rendered by the flip
  void setFlip(SDL_RendererFlip x);
  
  // Render the tile from the l'th line and c'th column from the texture pack
  // on the given target
  void renderTexture(SDL_Renderer* renderer, int l, int c, SDL_Rect target);

  // Render the entire texture on the given target
  void renderTexture(SDL_Renderer* renderer, SDL_Rect target);
};

#endif
