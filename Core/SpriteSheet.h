#pragma once
#if defined(__has_include)
#  if __has_include(<SDL3/SDL.h>)
#    include <SDL3/SDL.h>
#  elif __has_include(<SDL.h>)
#    include <SDL.h>
#  else
#    error "SDL header not found; adjust include paths or install SDL3/SDL2 development files."
#  endif
#else
#  include <SDL.h>
#endif
#include <string>

class SpriteSheet {
public:
    SpriteSheet();
    ~SpriteSheet();

    // Carga la textura desde 'path' usando el renderer. tileW/tileH por defecto 8x8.
    bool Load(SDL_Renderer* renderer, const std::string& path, int tileW = 8, int tileH = 8);
    SDL_Texture* GetTexture() const { return texture; }
    SDL_Rect GetSrcRect(int index) const;
    int Columns() const { return cols; }
    int Rows() const { return rows; }
    int TileW() const { return tileW; }
    int TileH() const { return tileH; }

private:
    SDL_Texture* texture = nullptr;
    int texW = 0, texH = 0;
    int tileW = 8, tileH = 8;
    int cols = 0, rows = 0;
};
