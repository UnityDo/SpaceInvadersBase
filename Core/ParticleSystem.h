#pragma once
#include <vector>
#include <SDL3/SDL.h>

struct Particle {
    float x, y;         // Posición
    float vx, vy;       // Velocidad
    float life;         // Tiempo de vida restante
    float r, g, b, a;   // Color y opacidad
};

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    // Crear explosión de partículas en una posición
    void CreateExplosion(float x, float y, int cantidad = 15);
    
    void Update(float dt);
    void Render(SDL_Renderer* renderer);
    void Clear();

private:
    std::vector<Particle> particles;
};
