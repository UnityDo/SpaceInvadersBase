#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>

class TextRenderer {
public:
    TextRenderer();
    ~TextRenderer();
    
    bool Init();
    void Shutdown();
    
    void RenderText(SDL_Renderer* renderer, const std::string& text, int x, int y, SDL_Color color = {255, 255, 255, 255});
    
private:
    TTF_Font* font;
    bool initialized;
};
