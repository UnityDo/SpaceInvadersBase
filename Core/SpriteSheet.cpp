#include "SpriteSheet.h"
#if defined(__has_include)
#  if __has_include(<SDL3/SDL.h>)
#    include <SDL3/SDL.h>
#    include <SDL3/SDL_render.h>
#  elif __has_include(<SDL.h>)
#    include <SDL.h>
#    include <SDL_render.h>
#  else
#    error "SDL header not found"
#  endif
#else
#  include <SDL.h>
#  include <SDL_render.h>
#endif

#if defined(__has_include)
#  if __has_include(<SDL3_image/SDL_image.h>)
#    include <SDL3_image/SDL_image.h>
#  elif __has_include(<SDL_image.h>)
#    include <SDL_image.h>
#  else
#    error "SDL_image header not found"
#  endif
#else
#  include <SDL_image.h>
#endif
#include <cmath>
#include <iostream>

SpriteSheet::SpriteSheet() = default;

SpriteSheet::~SpriteSheet() {
    if (texture) SDL_DestroyTexture(texture);
}

bool SpriteSheet::Load(SDL_Renderer* renderer, const std::string& path, int tW, int tH) {
    if (!renderer) return false;
    tileW = tW; tileH = tH;
    texture = IMG_LoadTexture(renderer, path.c_str());
    if (!texture) {
        std::cerr << "[SpriteSheet] failed to load: " << path << " (" << SDL_GetError() << ")\n";
        return false;
    }
    // Prefer nearest-neighbor scaling for pixel-art sprite sheets
    // SDL3 exposes SDL_SetTextureScaleMode to control per-texture scaling.
    SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
    // SDL3: SDL_QueryTexture was removed; use SDL_GetTextureSize to obtain width/height
    float fw = 0.0f, fh = 0.0f;
    if (!SDL_GetTextureSize(texture, &fw, &fh)) {
        std::cerr << "[SpriteSheet] SDL_GetTextureSize failed: " << SDL_GetError() << "\n";
        SDL_DestroyTexture(texture);
        texture = nullptr;
        return false;
    }
    // Convert float sizes to ints (textures are pixel sizes, floats should be integral)
    texW = (int)std::lroundf(fw);
    texH = (int)std::lroundf(fh);

    if (tileW <= 0 || tileH <= 0) {
        std::cerr << "[SpriteSheet] invalid tile size: " << tileW << "x" << tileH << "\n";
        SDL_DestroyTexture(texture);
        texture = nullptr;
        return false;
    }

    cols = texW / tileW;
    rows = texH / tileH;

    if (cols <= 0 || rows <= 0) {
        std::cerr << "[SpriteSheet] texture too small for given tile size: tex=" << texW << "x" << texH
                  << " tile=" << tileW << "x" << tileH << "\n";
        SDL_DestroyTexture(texture);
        texture = nullptr;
        return false;
    }

    if ((texW % tileW) != 0 || (texH % tileH) != 0) {
        std::cerr << "[SpriteSheet] Warning: texture dimensions (" << texW << "x" << texH
                  << ") are not an exact multiple of tile size (" << tileW << "x" << tileH << ")."
                  << " Partial tiles will be ignored.\n";
    }

    return true;
}

SDL_Rect SpriteSheet::GetSrcRect(int index) const {
    if (cols <= 0) return SDL_Rect{0,0,tileW,tileH};
    int col = index % cols;
    int row = index / cols;
    return SDL_Rect{ col * tileW, row * tileH, tileW, tileH };
}
