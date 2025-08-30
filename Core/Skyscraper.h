#pragma once
#include <SDL3/SDL.h>
#include <string>

// Skyscraper: representa un edificio destructible por impactos
// Implementa una superficie mutable con máscara alpha donde se borran píxeles
struct Skyscraper {
    SDL_FRect rect; // posición/escala en pantalla
    SDL_FRect originalRect;
    bool alive = true; // si todo el edificio está destruido, alive = false

    // imagePath opcional: si se proporciona, se carga la imagen de textura (relative path)
    std::string imagePath;

    // Surface y texture usadas internamente (surface mutable, texture para render)
    SDL_Surface* surface = nullptr;
    SDL_Texture* texture = nullptr;

    // altura/anchura en px de la surface interna
    int surfW = 0;
    int surfH = 0;
    // Debug: last impact point in world coords (for visual debug), negative if none
    float lastImpactX = -1.0f;
    float lastImpactY = -1.0f;

    Skyscraper(float x=0, float y=0, float w=60, float h=140, const std::string& img="") {
        rect = { x, y, w, h };
        originalRect = rect;
        imagePath = img;
        surface = nullptr;
        texture = nullptr;
        surfW = (int)w;
        surfH = (int)h;
        alive = true;
    }

    // Initialize the internal surface/texture. Must pass an SDL_Renderer* to create the texture.
    void Initialize(SDL_Renderer* rend);

    // Apply a circular explosion at local surface coords
    void ApplyExplosion(int cx, int cy, int radius);

    // Handle a bullet impact given world coordinates
    void TakeBulletHit(float wx, float wy, int radius);

    // Restore the building to full health (recreate surface from image or procedural)
    void Restore(SDL_Renderer* rend);

    // Recreate texture from surface (call after modifying surface)
    void UpdateTexture(SDL_Renderer* rend);

    // Render
    void Render(SDL_Renderer* rend);

    // Query: returns true if the surface at world coordinates (wx,wy) is opaque (> alphaThreshold)
    bool IsOpaqueAtWorld(float wx, float wy, Uint8 alphaThreshold = 16) const;

    // Destroy resources
    void Destroy();
};
