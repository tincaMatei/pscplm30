#include "baseclasses/graphicshandler.h"
#include <algorithm>

FontRenderer::FontRenderer(const char* filename, int fontsize) {
  usedFont = TTF_OpenFont(filename, fontsize);
  if(usedFont == NULL)
    SDL_Log("Failed to open font: %s\n", TTF_GetError());
}

FontRenderer::~FontRenderer() {
  if(usedFont != NULL) {
    TTF_CloseFont(usedFont);
    usedFont = NULL;
  }
}

std::string FontRenderer::inttostring(int x) {
  bool negative = (x < 0);
  std::string rez;
  
  if(x < 0)
    x = -x;
  
  if(x == 0)
    rez.push_back('0');
  
  while(x > 0) {
    rez.push_back(x % 10 + '0');
    x /= 10;
  }
  
  if(negative)
    rez.push_back('-');
  reverse(rez.begin(), rez.end());
  return rez;
}

void FontRenderer::renderText(SDL_Renderer* renderer, float _x, float _y,
                             std::string text) {
  SDL_Color color;
  SDL_GetRenderDrawColor(renderer, &color.r, &color.g, &color.b, &color.a);
  
  SDL_Surface* surface = TTF_RenderText_Solid(usedFont, text.c_str(), color);
  if(surface == NULL)
    SDL_Log("Unable to create font surface: %s\n", TTF_GetError());
  else {
    SDL_Texture* texture;
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(texture == NULL)
      SDL_Log("Unable to create texture from surface: %s\n", SDL_GetError());
    else {
      int textureWidth = surface->w; 
      int textureHeight = surface->h;
      
      SDL_Rect rect = {(int)floor(_x - textureWidth / 2.0f),
                       (int)floor(_y - textureHeight / 2.0f),
                       textureWidth, textureHeight};
      SDL_RenderCopy(renderer, texture, NULL, &rect);
      
      SDL_DestroyTexture(texture);
      texture = NULL;
    }
    
    SDL_FreeSurface(surface);
    surface = NULL;
  }
}

TileSetRenderer::TileSetRenderer(const char* filename, 
                                 SDL_Renderer* renderer,
                                 int _tw, int _th) {
  SDL_Surface* surface = IMG_Load(filename);
  texture = NULL;
  
  textureWidth = surface->w; 
  textureHeight = surface->h;
  
  tileWidth = _tw;
  tileHeight = _th;
  if(_tw == 0 || _th == 0) {
    tileWidth = textureWidth;
    tileHeight = textureHeight;
  }
  
  if(surface == NULL) {
    SDL_Log("Unable to load image %s: %s\n", filename, SDL_GetError());
  } else {
    texture = SDL_CreateTextureFromSurface(renderer, surface);
    if(texture == NULL)
      SDL_Log("Unable to create texture %s: %s\n", filename, SDL_GetError());
    SDL_FreeSurface(surface);
    surface = NULL;
  }
  
  rotationAngle = 0.0f;
  rotationCenter = NULL;
  textureFlip = SDL_FLIP_NONE;
}

TileSetRenderer::~TileSetRenderer() {
  if(texture != NULL) {
    SDL_DestroyTexture(texture);
    texture = NULL;
  }
}

void TileSetRenderer::setFlip(SDL_RendererFlip x) {
  textureFlip = x;
}

void TileSetRenderer::setRotationAngle(double x) {
  rotationAngle = x;
}

void TileSetRenderer::setRotationCenter(SDL_Point x) {
  rotationCenter = &x;
}

void TileSetRenderer::setCentroidRotation() {
  rotationCenter = NULL;
}

void TileSetRenderer::renderTexture(SDL_Renderer* renderer, int l, int c, 
                                    SDL_Rect target) {
  if(texture != NULL) {
    SDL_Rect from = {c * tileWidth, l * tileHeight, tileWidth, tileHeight};
    SDL_RenderCopyEx(renderer, texture, &from, &target, rotationAngle, rotationCenter, textureFlip);
  }
}

void TileSetRenderer::renderTexture(SDL_Renderer* renderer, SDL_Rect target) {
  if(texture != NULL)
    SDL_RenderCopyEx(renderer, texture, NULL, &target, rotationAngle, rotationCenter, textureFlip);
}


SDL_Rect TileSetRenderer::getCenteredPosition(int x, int y) {
  SDL_Rect rez = {x - tileWidth / 2, y - tileHeight / 2, tileWidth, tileHeight};
  return rez;
}

void drawPolygon(SDL_Renderer* renderer, Polygon* poly) {
  if(poly->circleRadius >= 0.0f) {
    int prec = (int)floor(12 * poly->circleRadius);
    float prevX, prevY;
    
    float angleGrowth = 2 * PI / prec, angle = angleGrowth;
    
    prevX = poly->centroidPos.x + poly->circleRadius;
    prevY = poly->centroidPos.y;
    
    for(int i = 0; i < prec; ++i) {
      float newX = poly->centroidPos.x + cos(angle) * poly->circleRadius, 
            newY = poly->centroidPos.y + sin(angle) * poly->circleRadius;
      SDL_RenderDrawLine(renderer, (int)floor(prevX),
                                   (int)floor(prevY),
                                   (int)floor(newX),
                                   (int)floor(newY));
      
      prevX = newX;
      prevY = newY;
      
      angle += angleGrowth;
    }
  } else {
    for(unsigned int i = 0; i < poly->polygon.size(); ++i) {
      int i2 = (i + 1) % poly->polygon.size();
      SDL_RenderDrawLine(renderer, (int)poly->polygon[i].x,  
                                   (int)poly->polygon[i].y, 
                                   (int)poly->polygon[i2].x, 
                                   (int)poly->polygon[i2].y);
    }
    
    if(poly->polygon.size() == 1)
      SDL_RenderDrawPoint(renderer, (int)poly->polygon[0].x, 
                                    (int)poly->polygon[0].y);
  }
}

