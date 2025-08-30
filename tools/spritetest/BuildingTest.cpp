// BuildingTest.cpp - render 6 procedural buildings (43x140) on 800x600
#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>
#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <algorithm>
#include <cstring>

int main(int argc, char** argv) {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
        return -1;
    }

    const int WIN_W = 800;
    const int WIN_H = 600;
    SDL_Window* win = SDL_CreateWindow("BuildingTest", WIN_W, WIN_H, 0);
    if (!win) { std::cerr << "CreateWindow failed: " << SDL_GetError() << std::endl; SDL_Quit(); return -1; }
    SDL_Renderer* rend = SDL_CreateRenderer(win, nullptr);
    if (!rend) { std::cerr << "CreateRenderer failed: " << SDL_GetError() << std::endl; SDL_DestroyWindow(win); SDL_Quit(); return -1; }

    // Prepare per-building surfaces/textures and initial damage
    const int count = 6;
    const int bw = 43;
    const int bh = 140;
    const int maxHits = 4; // resisten hasta 4 impactos, 5 destruye

    auto applyExplosion = [&](SDL_Surface* surf, int cx, int cy, int radius){
        if (!surf) return;
        Uint32* pixels = (Uint32*)surf->pixels;
        int pitch = surf->pitch / 4;
        int r2 = radius * radius;
        int y0 = std::max(0, cy - radius);
        int y1 = std::min(bh - 1, cy + radius);
        int x0 = std::max(0, cx - radius);
        int x1 = std::min(bw - 1, cx + radius);
        for (int y = y0; y <= y1; ++y) {
            int dy = y - cy;
            for (int x = x0; x <= x1; ++x) {
                int dx = x - cx;
                if (dx*dx + dy*dy <= r2) {
                    const SDL_PixelFormatDetails* d = SDL_GetPixelFormatDetails(surf->format);
                    pixels[y * pitch + x] = SDL_MapRGBA(d, nullptr, 0, 0, 0, 0);
                }
            }
        }
    };

    // initial hits: 0,1,2,3,4,5 (six buildings)
    std::vector<int> hits = {0,1,2,3,4,5};
    std::vector<SDL_Surface*> surfs(count, nullptr);
    std::vector<SDL_Texture*> texs(count, nullptr);

    // Helper: scale a surface to target size using nearest-neighbor sampling
    auto scaleSurfaceNearest = [&](SDL_Surface* src, int dstW, int dstH) -> SDL_Surface* {
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
                // copy up to 4 bytes into pixel (works for common formats)
                std::memcpy(&pixel, p, std::min(4, srcBpp));

                Uint8 r=0,g=0,b=0,a=255;
                SDL_GetRGBA(pixel, srcDetails, nullptr, &r, &g, &b, &a);
                dstPixels[y * dstPitch + x] = SDL_MapRGBA(dstDetails, nullptr, r, g, b, a);
            }
        }

        return dst;
    };

    // candidate asset names for each building (relative to working dir)
    std::vector<std::string> assetPaths = {
        "assets/sprites/build_01.png",
        "assets/sprites/build_02.png",
        "assets/sprites/build_03.png",
        "assets/sprites/build_04.png",
        "assets/sprites/build_05.png",
        "assets/sprites/build_06.png"
    };

    for (int i = 0; i < count; ++i) {
        if (hits[i] > maxHits) {
            // considered destroyed already
            surfs[i] = nullptr;
            texs[i] = nullptr;
            continue;
        }

        SDL_Surface* surf = nullptr;

        // Try to load an image for this building
        if (i < (int)assetPaths.size()) {
            const std::string& path = assetPaths[i];
            SDL_Surface* loaded = IMG_Load(path.c_str());
            if (loaded) {
                // If size differs, scale to target bw x bh using NN helper
                if (loaded->w != bw || loaded->h != bh) {
                    SDL_Surface* scaled = scaleSurfaceNearest(loaded, bw, bh);
                    SDL_DestroySurface(loaded);
                    if (scaled) surf = scaled; else surf = loaded; // if scaling failed, keep loaded
                } else {
                    surf = loaded;
                }
            } else {
                SDL_Log("IMG_Load failed for %s: %s", path.c_str(), SDL_GetError());
            }
        }

        // If no image, generate procedurally
        if (!surf) {
            surf = SDL_CreateSurface(bw, bh, SDL_PIXELFORMAT_RGBA32);
            if (!surf) { SDL_Log("Failed to create surface: %s", SDL_GetError()); continue; }

            // Fill base color
            const SDL_PixelFormatDetails* details = SDL_GetPixelFormatDetails(surf->format);
            Uint32 fill = SDL_MapRGBA(details, nullptr, 20, 160, 160, 255);
            Uint32* pixels = (Uint32*)surf->pixels;
            int pitch = surf->pitch / 4;
            for (int y = 0; y < bh; ++y) for (int x = 0; x < bw; ++x) pixels[y * pitch + x] = fill;

            // Draw windows procedurally onto surface
            Uint32 winCol = SDL_MapRGBA(details, nullptr, 255, 230, 120, 255);
            int wx = 6, wy = 8; // window size
            int gutterX = 6, gutterY = 10;
            for (int yy = 10; yy + wy < bh - 10; yy += wy + gutterY) {
                for (int xx = 6; xx + wx < bw - 6; xx += wx + gutterX) {
                    for (int wy_i = 0; wy_i < wy; ++wy_i) {
                        for (int wx_i = 0; wx_i < wx; ++wx_i) {
                            int px = xx + wx_i;
                            int py = yy + wy_i;
                            if (px >= 0 && px < bw && py >= 0 && py < bh) pixels[py * pitch + px] = winCol;
                        }
                    }
                }
            }
        }

        // Apply initial impact stamps (spread vertically)
        int hitsToApply = hits[i];
        int radius = 36;
        for (int k = 0; k < hitsToApply; ++k) {
            // positions distributed: x across width, y random-ish across height
            int cx = (bw * (k + 1)) / (hitsToApply + 1);
            int cy = bh/3 + (k % 3) * 14;
            applyExplosion(surf, cx, cy, radius);
        }

        SDL_Texture* tex = SDL_CreateTextureFromSurface(rend, surf);
        if (tex) {
            SDL_SetTextureScaleMode(tex, SDL_SCALEMODE_NEAREST);
            SDL_SetTextureBlendMode(tex, SDL_BLENDMODE_BLEND);
        } else {
            SDL_Log("Failed to create texture from surface: %s", SDL_GetError());
        }

        surfs[i] = surf;
        texs[i] = tex;
    }

    bool running = true;
    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_EVENT_QUIT) running = false;
            if (e.type == SDL_EVENT_KEY_DOWN) {
                if (e.key.key == SDLK_ESCAPE) running = false;
            }
        }

    SDL_SetRenderDrawColor(rend, 16, 16, 20, 255);
    SDL_RenderClear(rend);

        // place buildings horizontally centered using prepared textures
        const int spacing = 20;
        int totalW = count * bw + (count - 1) * spacing;
        int startX = (WIN_W - totalW) / 2;
        int baseY = WIN_H - 80; // ground offset

        for (int i = 0; i < count; ++i) {
            int bx = startX + i * (bw + spacing);
            int by = baseY - bh; // top
            SDL_FRect dst = { (float)bx, (float)by, (float)bw, (float)bh };

            if (hits[i] > maxHits) {
                // fully destroyed, skip rendering
                continue;
            }

            if (texs[i]) {
                SDL_RenderTexture(rend, texs[i], nullptr, &dst);
            } else if (surfs[i]) {
                // create texture on the fly if missing
                SDL_Texture* t = SDL_CreateTextureFromSurface(rend, surfs[i]);
                if (t) {
                    SDL_SetTextureScaleMode(t, SDL_SCALEMODE_NEAREST);
                    SDL_SetTextureBlendMode(t, SDL_BLENDMODE_BLEND);
                    SDL_RenderTexture(rend, t, nullptr, &dst);
                    SDL_DestroyTexture(t);
                }
            } else {
                // fallback solid rect
                SDL_SetRenderDrawColor(rend, 40, 40, 48, 255);
                SDL_RenderFillRect(rend, &dst);
            }
        }

        // ground
        SDL_SetRenderDrawColor(rend, 30, 80, 30, 255);
        SDL_FRect ground = { 0.0f, (float)(WIN_H - 60), (float)WIN_W, 60.0f };
        SDL_RenderFillRect(rend, &ground);

        SDL_RenderPresent(rend);
        SDL_Delay(16);
    }

    for (size_t i = 0; i < texs.size(); ++i) {
        if (texs[i]) SDL_DestroyTexture(texs[i]);
    }
    for (size_t i = 0; i < surfs.size(); ++i) {
        if (surfs[i]) SDL_DestroySurface(surfs[i]);
    }
    SDL_DestroyRenderer(rend);
    SDL_DestroyWindow(win);
    SDL_Quit();
    return 0;
}
