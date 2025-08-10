#include "ParticleSystem.h"
#include <algorithm>
#include <cstdlib>
#include <cmath>

ParticleSystem::ParticleSystem() {}

ParticleSystem::~ParticleSystem() {}

void ParticleSystem::CreateExplosion(float x, float y, int cantidad) {
    for (int i = 0; i < cantidad; ++i) {
        // Ángulo aleatorio en todas las direcciones
        float angle = (float)(rand() % 360) * 3.14159f / 180.0f;
        
        // Velocidad aleatoria
        float speed = 30.0f + rand() % 70;
        float vx = cos(angle) * speed;
        float vy = sin(angle) * speed;
        
        // Tiempo de vida aleatorio
        float life = 0.3f + (rand() % 50) / 100.0f;
        
        // Color amarillo-naranja para explosión
        float r = 1.0f;  // Rojo completo
        float g = 0.5f + (rand() % 50) / 100.0f;  // Verde variable (amarillo-naranja)
        float b = 0.0f;  // Sin azul
        float a = 1.0f;  // Opacidad completa
        
        particles.push_back({x, y, vx, vy, life, r, g, b, a});
    }
}

void ParticleSystem::Update(float dt) {
    for (auto& p : particles) {
        // Actualizar posición
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        
        // Aplicar gravedad leve
        p.vy += 50.0f * dt;
        
        // Reducir tiempo de vida
        p.life -= dt;
        
        // Desvanecer partícula
        p.a = p.life / 0.8f; // Se desvanece basado en el tiempo de vida
        if (p.a < 0) p.a = 0;
    }
    
    // Eliminar partículas muertas
    particles.erase(
        std::remove_if(particles.begin(), particles.end(), 
            [](const Particle& p) { return p.life <= 0; }),
        particles.end()
    );
}

void ParticleSystem::Render(SDL_Renderer* renderer) {
    for (const auto& p : particles) {
        // Establecer color con transparencia
        SDL_SetRenderDrawColor(renderer, 
            (Uint8)(p.r * 255), 
            (Uint8)(p.g * 255), 
            (Uint8)(p.b * 255), 
            (Uint8)(p.a * 255));
        
        // Dibujar partícula como pequeño rectángulo
        SDL_FRect rect = { p.x - 1, p.y - 1, 2.0f, 2.0f };
        SDL_RenderFillRect(renderer, &rect);
    }
}

void ParticleSystem::Clear() {
    particles.clear();
}
