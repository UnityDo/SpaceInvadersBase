#pragma once
#include <vector>
#include <functional>
#include <optional>
#include <SDL3/SDL_rect.h>

namespace Core {
namespace Raycast {

struct HitResult {
    int index = -1;            // índice del objeto golpeado en la colección (o -1)
    SDL_FPoint point{0,0};     // punto de intersección en coordenadas del mundo
    float t = 0.0f;            // parámetro [0,1] a lo largo del segmento desde el origen
};

// Intersecta un segmento (p0->p1) con un SDL_FRect. Si hay intersección, devuelve true y opcional HitResult parcial (t y point).
bool IntersectSegmentRect(const SDL_FPoint &p0, const SDL_FPoint &p1, const SDL_FRect &rect, SDL_FPoint &outPoint, float &outT);

// Raycast básico: devuelve el índice del primer rect golpeado (el más cercano al origen), -1 si ninguno.
// outHit será llenado si se produce un impacto.
int RaycastRects(const SDL_FPoint &p0, const SDL_FPoint &p1, const std::vector<SDL_FRect> &rects, HitResult &outHit);

// Versión genérica: acepta una colección de T y un getter que retorna SDL_FRect desde T.
// Esto permite pasar vectores de enemigos, powerups, etc., sin copiar rects.
template<typename T>
int Raycast(const SDL_FPoint &p0, const SDL_FPoint &p1, const std::vector<T> &items, std::function<SDL_FRect(const T&)> getter, HitResult &outHit) {
    std::vector<SDL_FRect> tmp;
    tmp.reserve(items.size());
    for (const auto &it : items) tmp.push_back(getter(it));
    return RaycastRects(p0, p1, tmp, outHit);
}

} // namespace Raycast
} // namespace Core
