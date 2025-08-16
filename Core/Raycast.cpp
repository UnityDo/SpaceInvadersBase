#include "Raycast.h"
#include <algorithm>
#include <cmath>

namespace Core { namespace Raycast {

// https://stackoverflow.com/questions/306316/determine-if-two-rectangles-overlap-each-other
// Usaremos la técnica del parámetro t en cada eje (slab method) para segment vs AABB (rect)

bool IntersectSegmentRect(const SDL_FPoint &p0, const SDL_FPoint &p1, const SDL_FRect &rect, SDL_FPoint &outPoint, float &outT) {
    // Transformar rect a min/max
    float rminx = rect.x;
    float rmaxx = rect.x + rect.w;
    float rminy = rect.y;
    float rmaxy = rect.y + rect.h;

    float dx = p1.x - p0.x;
    float dy = p1.y - p0.y;

    float tmin = 0.0f;
    float tmax = 1.0f;

    auto update = [&](float num, float den)->bool{
        if (std::abs(den) < 1e-6f) {
            if (num < 0.0f) return false; // paralelo e fuera del slab
            return true; // paralelo y dentro
        }
        float t = num / den;
        if (den > 0.0f) {
            if (t > tmax) return false;
            if (t > tmin) tmin = t;
        } else {
            if (t < tmin) return false;
            if (t < tmax) tmax = t;
        }
        return true;
    };

    // X slabs
    if (!update(rminx - p0.x, dx)) return false;
    if (!update(rmaxx - p0.x, dx)) return false;
    // Y slabs
    if (!update(rminy - p0.y, dy)) return false;
    if (!update(rmaxy - p0.y, dy)) return false;

    // Intersección si tmin <= tmax y existe en [0,1]
    float tHit = tmin;
    if (tHit < 0.0f || tHit > 1.0f) return false;
    outT = tHit;
    outPoint.x = p0.x + dx * outT;
    outPoint.y = p0.y + dy * outT;
    return true;
}

int RaycastRects(const SDL_FPoint &p0, const SDL_FPoint &p1, const std::vector<SDL_FRect> &rects, HitResult &outHit) {
    int bestIndex = -1;
    float bestT = 1e9f;
    SDL_FPoint tmpPoint{0,0};
    float tmpT = 0.0f;
    for (size_t i=0;i<rects.size();++i) {
        if (IntersectSegmentRect(p0,p1,rects[i],tmpPoint,tmpT)) {
            if (tmpT >= 0.0f && tmpT <= 1.0f && tmpT < bestT) {
                bestT = tmpT;
                bestIndex = (int)i;
                outHit.index = (int)i;
                outHit.point = tmpPoint;
                outHit.t = tmpT;
            }
        }
    }
    return bestIndex;
}

}} // namespace
