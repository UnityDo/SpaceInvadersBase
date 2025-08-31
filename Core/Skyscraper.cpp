#include "Skyscraper.h"
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <cstring>
#include <algorithm>

// Helper to create a procedural surface matching given size
static SDL_Surface* CreateProceduralSurface(int w, int h) {
    SDL_Surface* surf = SDL_CreateSurface(w, h, SDL_PIXELFORMAT_RGBA32);
    if (!surf) return nullptr;
    const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surf->format);
    Uint32 fill = SDL_MapRGBA(details, nullptr, 20, 160, 160, 255);
    Uint32* pixels = (Uint32*)surf->pixels;
    int pitch = surf->pitch / 4;
    for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) pixels[y * pitch + x] = fill;

    Uint32 winCol = SDL_MapRGBA(details, nullptr, 255, 230, 120, 255);
    int wx = 6, wy = 8; // window size
    int gutterX = 6, gutterY = 10;
    for (int yy = 10; yy + wy < h - 10; yy += wy + gutterY) {
        for (int xx = 6; xx + wx < w - 6; xx += wx + gutterX) {
            for (int wy_i = 0; wy_i < wy; ++wy_i) {
                for (int wx_i = 0; wx_i < wx; ++wx_i) {
                    int px = xx + wx_i;
                    int py = yy + wy_i;
                    if (px >= 0 && px < w && py >= 0 && py < h) pixels[py * pitch + px] = winCol;
                }
            }
        }
    }
    return surf;
}

// Helper: scale a surface to target size using nearest-neighbor sampling
static SDL_Surface* ScaleSurfaceNearest(SDL_Surface* src, int dstW, int dstH) {
    if (!src) return nullptr;
    SDL_Surface* dst = SDL_CreateSurface(dstW, dstH, SDL_PIXELFORMAT_RGBA32);
    if (!dst) return nullptr;

    const SDL_PixelFormatDetails* srcDetails = SDL_GetPixelFormatDetails(src->format);
    const SDL_PixelFormatDetails* dstDetails = SDL_GetPixelFormatDetails(dst->format);

    Uint8* srcPixels = (Uint8*)src->pixels;
    Uint32* dstPixels = (Uint32*)dst->pixels;
    int srcPitch = src->pitch; // bytes per row
    int dstPitch = dst->pitch / 4; // dst is RGBA32 -> 4 bytes per pixel
    int srcBpp = srcPitch / src->w;

    for (int y = 0; y < dstH; ++y) {
        int sy = (y * src->h) / dstH;
        if (sy < 0) sy = 0; if (sy >= src->h) sy = src->h - 1;
        for (int x = 0; x < dstW; ++x) {
            int sx = (x * src->w) / dstW;
            if (sx < 0) sx = 0; if (sx >= src->w) sx = src->w - 1;

            Uint8* p = srcPixels + sy * srcPitch + sx * srcBpp;
            Uint32 pixel = 0;
            memcpy(&pixel, p, std::min(4, srcBpp));

            Uint8 r=0,g=0,b=0,a=255;
            SDL_GetRGBA(pixel, srcDetails, nullptr, &r, &g, &b, &a);
            dstPixels[y * dstPitch + x] = SDL_MapRGBA(dstDetails, nullptr, r, g, b, a);
        }
    }

    return dst;
}

void Skyscraper::Initialize(SDL_Renderer* rend) {
    Destroy();
    surfW = (int)originalRect.w;
    surfH = (int)originalRect.h;

    if (!imagePath.empty()) {
        SDL_Surface* loaded = IMG_Load(imagePath.c_str());
        if (loaded) {
            // if size differs, scale by blit scaled fallback to create surface of correct size
            if (loaded->w != surfW || loaded->h != surfH) {
                SDL_Surface* scaled = SDL_CreateSurface(surfW, surfH, SDL_PIXELFORMAT_RGBA32);
                if (scaled) {
                    // Use nearest-neighbor scaler helper instead of SDL_BlitScaled to avoid API rename issues
                    SDL_Surface* scaled2 = ScaleSurfaceNearest(loaded, surfW, surfH);
                    SDL_DestroySurface(loaded);
                    if (scaled2) {
                        SDL_DestroySurface(scaled);
                        surface = scaled2;
                    } else {
                        surface = scaled; // fallback
                    }
                } else {
                    surface = loaded;
                }
            } else {
                surface = loaded;
            }
        } else {
            std::cout << "[Skyscraper] IMG_Load failed for " << imagePath << " : " << SDL_GetError() << std::endl;
        }
    }

    if (!surface) {
        surface = CreateProceduralSurface(surfW, surfH);
    }

    if (surface) {
        // Ensure surface is in RGBA32 so alpha channel modifications work reliably
        if (surface->format != SDL_PIXELFORMAT_RGBA32) {
            SDL_Surface* conv = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
            if (conv) {
                SDL_DestroySurface(surface);
                surface = conv;
            }
        }
        // Create texture only if a renderer was provided. Allow Initialize(nullptr)
        // to prepare the mutable surface for collision checks without creating a texture.
        if (rend) {
            texture = SDL_CreateTextureFromSurface(rend, surface);
            if (texture) {
                SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
                SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
            }
        } else {
            texture = nullptr;
        }
    }
}

void Skyscraper::ApplyExplosion(int cx, int cy, int radius) {
    if (!surface) return;
    
    std::cout << "[Skyscraper] ApplyExplosion at (" << cx << "," << cy << ") radius=" << radius << std::endl;
    
    Uint32* pixels = (Uint32*)surface->pixels;
    int pitch = surface->pitch / 4;
    int r2 = radius * radius;
    int y0 = std::max(0, cy - radius);
    int y1 = std::min(surfH - 1, cy + radius);
    int x0 = std::max(0, cx - radius);
    int x1 = std::min(surfW - 1, cx + radius);
    
    int pixelsCleared = 0;
    const SDL_PixelFormatDetails* d = SDL_GetPixelFormatDetails(surface->format);
    for (int y = y0; y <= y1; ++y) {
        int dy = y - cy;
        for (int x = x0; x <= x1; ++x) {
            int dx = x - cx;
            if (dx*dx + dy*dy <= r2) {
                pixels[y * pitch + x] = SDL_MapRGBA(d, nullptr, 0,0,0,0);
                pixelsCleared++;
            }
        }
    }
    
    std::cout << "[Skyscraper] Cleared " << pixelsCleared << " pixels in explosion" << std::endl;
    
    // mark texture for update; texture will be recreated lazily when Render is called
    if (texture) { SDL_DestroyTexture(texture); texture = nullptr; }
}

void Skyscraper::TakeBulletHit(float wx, float wy, int radius) {
    if (!surface) return;
    
    // Log the hit for debugging
    std::cout << "[Skyscraper] TakeBulletHit at world coords (" << wx << "," << wy << ") radius=" << radius << std::endl;
    
    // Convert world coords to local surface coords
    float lx = (wx - rect.x) * (surfW / rect.w);
    float ly = (wy - rect.y) * (surfH / rect.h);
    int cx = (int)lx;
    int cy = (int)ly;
    
    std::cout << "[Skyscraper] Local surface coords: (" << cx << "," << cy << ") surface size: " << surfW << "x" << surfH << std::endl;
    
    // Save snapshot of current surface to history (undo point) before applying damage
    if ((int)history.size() >= 8) {
        // limit history depth to 8: free oldest
        SDL_DestroySurface(history.front());
        history.erase(history.begin());
    }
    // Create a copy of the surface
    SDL_Surface* snap = SDL_CreateSurface(surface->w, surface->h, SDL_PIXELFORMAT_RGBA32);
    if (snap) {
        // copy pixels
        int bytes = surface->pitch * surface->h;
        memcpy(snap->pixels, surface->pixels, bytes);
        history.push_back(snap);
    }

    ApplyExplosion(cx, cy, radius);
    // store last impact (world coords) for debug visualization
    lastImpactX = wx;
    lastImpactY = wy;
    
    // After modifying surface, destroy texture so it will be recreated with new pixels
    if (texture) { SDL_DestroyTexture(texture); texture = nullptr; }
    
    // Update alive flag: if most pixels are transparent, mark dead.
    Uint32* pixels = (Uint32*)surface->pixels;
    int pitch = surface->pitch / 4;
    int total = surfW * surfH;
    int opaque = 0;
    for (int i = 0; i < total; ++i) {
        Uint32 p = pixels[i];
        Uint8 r,g,b,a;
        SDL_GetRGBA(p, SDL_GetPixelFormatDetails(surface->format), nullptr, &r, &g, &b, &a);
        if (a > 16) opaque++;
    }
    
    std::cout << "[Skyscraper] After explosion: " << opaque << "/" << total << " opaque pixels (" << (100.0f * opaque / total) << "%)" << std::endl;
    
    if (opaque < total/20) { // less than 5% opaque
        alive = false;
        std::cout << "[Skyscraper] Building destroyed!" << std::endl;
    }
}

void Skyscraper::RestoreFromHistory(int levels, SDL_Renderer* rend) {
    if (levels <= 0) return;
    if (history.empty()) return;
    // If building is dead, do not resurrect via history
    if (!alive) return;

    int take = std::min((int)history.size(), levels);
    // pick the snapshot 'take' steps back from the end
    SDL_Surface* target = history[history.size() - take];
    if (!target) return;

    // Replace current surface with a copy of target
    SDL_Surface* newSurf = SDL_CreateSurface(target->w, target->h, SDL_PIXELFORMAT_RGBA32);
    if (!newSurf) return;
    int bytes = target->pitch * target->h;
    memcpy(newSurf->pixels, target->pixels, bytes);

    // swap surfaces
    if (surface) SDL_DestroySurface(surface);
    surface = newSurf;

    // Update texture immediately if renderer provided
    if (rend) {
        if (texture) SDL_DestroyTexture(texture);
        texture = SDL_CreateTextureFromSurface(rend, surface);
        if (texture) {
            SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
        }
    }

    // Trim history: remove the last 'take' entries
    for (int i = 0; i < take; ++i) {
        SDL_DestroySurface(history.back());
        history.pop_back();
    }

    // Recompute alive flag conservatively
    Uint32* pixels = (Uint32*)surface->pixels;
    int pitch = surface->pitch / 4;
    int total = surfW * surfH;
    int opaque = 0;
    for (int i = 0; i < total; ++i) {
        Uint32 p = pixels[i];
        Uint8 r,g,b,a;
        SDL_GetRGBA(p, SDL_GetPixelFormatDetails(surface->format), nullptr, &r, &g, &b, &a);
        if (a > 16) opaque++;
    }
    if (opaque < total/20) {
        alive = false;
    } else {
        alive = true;
    }
}

bool Skyscraper::IsOpaqueAtWorld(float wx, float wy, Uint8 alphaThreshold) const {
    if (!surface) return false;
    // Convert world coords to local surface coords
    float lx = (wx - rect.x) * (surfW / rect.w);
    float ly = (wy - rect.y) * (surfH / rect.h);
    int x = (int)lx;
    int y = (int)ly;
    if (x < 0 || y < 0 || x >= surfW || y >= surfH) return false;
    const Uint32* pixels = (const Uint32*)surface->pixels;
    int pitch = surface->pitch / 4;
    Uint32 p = pixels[y * pitch + x];
    Uint8 r,g,b,a;
    SDL_GetRGBA(p, SDL_GetPixelFormatDetails(surface->format), nullptr, &r, &g, &b, &a);
    return a > alphaThreshold;
}

void Skyscraper::Restore(SDL_Renderer* rend) {
    Initialize(rend);
}

void Skyscraper::UpdateTexture(SDL_Renderer* rend) {
    if (!surface) return;
    if (texture) SDL_DestroyTexture(texture);
    texture = SDL_CreateTextureFromSurface(rend, surface);
    if (texture) {
        SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST);
        SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    }
}

void Skyscraper::Render(SDL_Renderer* rend) {
    if (!alive) return;
    if (!texture) UpdateTexture(rend);
    if (!texture) return;
    SDL_FRect dst = rect;
    SDL_RenderTexture(rend, texture, nullptr, &dst);
}

void Skyscraper::Destroy() {
    if (texture) { SDL_DestroyTexture(texture); texture = nullptr; }
    if (surface) { SDL_DestroySurface(surface); surface = nullptr; }
    // free history
    for (SDL_Surface* s : history) {
        if (s) SDL_DestroySurface(s);
    }
    history.clear();
}
