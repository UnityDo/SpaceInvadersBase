#if defined(__has_include)
#  if __has_include(<SDL3/SDL.h>)
#    include <SDL3/SDL.h>
#  elif __has_include(<SDL.h>)
#    include <SDL.h>
#  else
#    error "SDL header not found; adjust include paths"
#  endif
#else
#  include <SDL.h>
#endif

#if defined(__has_include)
#  if __has_include(<SDL3_image/SDL_image.h>)
#    include <SDL3_image/SDL_image.h>
#include <cmath>
#  elif __has_include(<SDL_image.h>)
#    include <SDL_image.h>
#  else
#    error "SDL_image header not found; install SDL_image or adjust include paths"
#  endif
#else
#  include <SDL_image.h>
#endif
#include <iostream>
#include <thread>
#include <atomic>
#include <string>
#include <chrono>
#include <algorithm>
#include <fstream>
#include <vector>
#include <filesystem>
#include <cstdlib>
#include <sstream>
#include "..\..\Core\SpriteSheet.h"
#include "..\..\Core\Renderer.h"
// Detect console availability on Windows
#ifdef _WIN32
#include <io.h>
#include <stdio.h>
#include <windows.h>
#endif

#ifdef _WIN32
static std::string GetLastErrorAsString()
{
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) return std::string(); //No error
    LPSTR messageBuffer = nullptr;
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    std::string message(messageBuffer, size);
    LocalFree(messageBuffer);
    return message;
}
#endif

static void AppendDiag(const std::string &s) {
    try {
        std::string path = "spritetest_diag.txt";
        std::ofstream f(path, std::ios::app);
        if (f.is_open()) {
            f << s << std::endl;
            f.close();
        }
    } catch (...) {}
}

static void WriteStartupDiag() {
    // Remove existing diagnostic file
    try {
        std::filesystem::remove("spritetest_diag.txt");
    } catch (...) {}
    
    AppendDiag("--- SpriteTest diagnostic ---");
    try {
        AppendDiag(std::string("cwd: ") + std::filesystem::current_path().string());
    } catch(...) { AppendDiag("cwd: (error)"); }
    const char* pathEnv = std::getenv("PATH");
    AppendDiag(std::string("PATH: ") + (pathEnv?pathEnv:"(null)"));
    // Record loaded module filenames if available
#ifdef _WIN32
    HMODULE hSDL = GetModuleHandleA("SDL3.dll");
    if (hSDL) {
        char buf[MAX_PATH];
        if (GetModuleFileNameA(hSDL, buf, MAX_PATH)) AppendDiag(std::string("Loaded SDL3 module: ")+buf);
    } else AppendDiag("Loaded SDL3 module: (none)");
    HMODULE hIMG = GetModuleHandleA("SDL3_image.dll");
    if (hIMG) {
        char buf[MAX_PATH];
        if (GetModuleFileNameA(hIMG, buf, MAX_PATH)) AppendDiag(std::string("Loaded SDL3_image module: ")+buf);
    } else AppendDiag("Loaded SDL3_image module: (none)");
#endif
    // SDL runtime version: not checked here (to avoid compile-time differences between SDL versions)
    AppendDiag("SDL runtime: (not checked)");
    // Check for DLLs in cwd
    std::vector<std::string> dlls = {"SDL3.dll","SDL3_image.dll","SDL3_ttf.dll"};
    for (auto &d : dlls) {
        bool exists = std::filesystem::exists(d);
        AppendDiag(std::string(d + " exists in cwd: ") + (exists?"yes":"no"));
#ifdef _WIN32
        if (exists) {
            HMODULE h = LoadLibraryA(d.c_str());
            if (h) {
                AppendDiag(d + " LoadLibrary: OK");
                FreeLibrary(h);
            } else {
                AppendDiag(d + std::string(" LoadLibrary failed: ") + GetLastErrorAsString());
            }
        }
#endif
    }
    AppendDiag("--- end diag ---\n");
}

// Config
static const int WIN_W = 640;
static const int WIN_H = 480;

int main(int argc, char** argv) {
    std::string sheetPath = "assets/sprites/SpaceShooterAssetPack_Ships.png";
    int scale = 5;
    if (argc >= 2) sheetPath = argv[1];
    if (argc >= 3) {
    int temp = std::stoi(argv[2]);
    if (temp < 1) temp = 1;
    scale = temp;
}

    // Write diagnostic file to help debug silent init failures
    WriteStartupDiag();

    // Add more diagnostic about DLLs
    AppendDiag("--- DLL Verification ---");
#ifdef _WIN32
    HMODULE hSDL = LoadLibraryA("SDL3.dll");
    if (hSDL) {
        AppendDiag("SDL3.dll can be loaded explicitly");
        FreeLibrary(hSDL);
    } else {
        DWORD err = GetLastError();
        AppendDiag(std::string("SDL3.dll cannot be loaded explicitly, error: ") + std::to_string(err));
    }
    
    HMODULE hSDLImg = LoadLibraryA("SDL3_image.dll");
    if (hSDLImg) {
        AppendDiag("SDL3_image.dll can be loaded explicitly");
        FreeLibrary(hSDLImg);
    } else {
        DWORD err = GetLastError();
        AppendDiag(std::string("SDL3_image.dll cannot be loaded explicitly, error: ") + std::to_string(err));
    }
    
    // Check graphics capabilities
    AppendDiag("--- Graphics System Check ---");
    HDC hdc = GetDC(NULL);
    if (hdc) {
        int colorDepth = GetDeviceCaps(hdc, BITSPIXEL);
        int width = GetDeviceCaps(hdc, HORZRES);
        int height = GetDeviceCaps(hdc, VERTRES);
        AppendDiag(std::string("Desktop resolution: ") + std::to_string(width) + "x" + std::to_string(height));
        AppendDiag(std::string("Color depth: ") + std::to_string(colorDepth) + " bits");
        ReleaseDC(NULL, hdc);
    } else {
        AppendDiag("Failed to get desktop DC");
    }
#endif

    // Try different SDL init approaches
    AppendDiag("--- SDL Init Tests ---");
    
    // Test 1: Try with no subsystems first (minimal init)
    if (SDL_Init(0) != 0) {
        const char* err = SDL_GetError();
        AppendDiag(std::string("SDL_Init(0) failed: ") + (err?err:"(no message)"));
    } else {
        AppendDiag("SDL_Init(0) succeeded");
        SDL_Quit();
    }
    
    // Test 2: Try with VIDEO
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        const char* err = SDL_GetError();
        AppendDiag(std::string("SDL_Init(SDL_INIT_VIDEO) failed: ") + (err?err:"(no message)"));
        std::cerr << "SDL_Init(SDL_INIT_VIDEO) failed: " << (err?err:"(no message)") << "\n";
        #ifdef _WIN32
        std::string extra = GetLastErrorAsString();
        std::string msg = std::string("SDL_Init(SDL_INIT_VIDEO) failed: ") + (err?err:"(no message)");
        if (!extra.empty()) msg += std::string("\n\nWin32: ") + extra;
        MessageBoxA(NULL, (std::string("SDL_Init(SDL_INIT_VIDEO) failed. Diagnostic written to spritetest_diag.txt\n\n") + msg).c_str(), "SpriteTest - SDL_Init error", MB_OK | MB_ICONERROR);
        #endif
        return 1;
    }
    AppendDiag("SDL_Init(SDL_INIT_VIDEO) succeeded");
    
    // Create window directly (not via Renderer class)
    SDL_Window* testWin = SDL_CreateWindow("SpriteTest", WIN_W, WIN_H, 0);
    if (!testWin) {
        const char* err = SDL_GetError();
        AppendDiag(std::string("SDL_CreateWindow failed: ") + (err?err:"(no message)"));
        std::cerr << "SDL_CreateWindow failed: " << (err?err:"(no message)") << "\n";
        #ifdef _WIN32
        std::string extra = GetLastErrorAsString();
        std::string msg = std::string("SDL_CreateWindow failed: ") + (err?err:"(no message)");
        if (!extra.empty()) msg += std::string("\n\nWin32: ") + extra;
        MessageBoxA(NULL, (std::string("SDL_CreateWindow failed. Diagnostic written to spritetest_diag.txt\n\n") + msg).c_str(), "SpriteTest - Window error", MB_OK | MB_ICONERROR);
        #endif
        SDL_Quit();
        return 1;
    }
    AppendDiag("SDL_CreateWindow succeeded");
    
    // Create renderer directly
    SDL_Renderer* rend = SDL_CreateRenderer(testWin, nullptr);
    if (!rend) {
        const char* err = SDL_GetError();
        AppendDiag(std::string("SDL_CreateRenderer failed: ") + (err?err:"(no message)"));
        std::cerr << "SDL_CreateRenderer failed: " << (err?err:"(no message)") << "\n";
        #ifdef _WIN32
        std::string extra = GetLastErrorAsString();
        std::string msg = std::string("SDL_CreateRenderer failed: ") + (err?err:"(no message)");
        if (!extra.empty()) msg += std::string("\n\nWin32: ") + extra;
        MessageBoxA(NULL, (std::string("SDL_CreateRenderer failed. Diagnostic written to spritetest_diag.txt\n\n") + msg).c_str(), "SpriteTest - Renderer error", MB_OK | MB_ICONERROR);
        #endif
        SDL_DestroyWindow(testWin);
        SDL_Quit();
        return 1;
    }
    AppendDiag("SDL_CreateRenderer succeeded");
    AppendDiag("--- All SDL Init Steps Completed Successfully ---");

    // Test sprite loading separately after SDL is working
    AppendDiag("--- Testing Sprite Loading ---");
    
    // Check if sprite file exists before trying to load
    if (!std::filesystem::exists(sheetPath)) {
        AppendDiag(std::string("Sprite file does not exist: ") + sheetPath);
        std::cerr << "Sprite file not found: " << sheetPath << "\n";
        std::cout << "Continuing without sprite for SDL testing...\n";
        sheetPath = ""; // Clear path to skip loading
    } else {
        AppendDiag(std::string("Sprite file exists: ") + sheetPath);
    }

    SpriteSheet sheet;
    bool sheetLoaded = false;
    if (!sheetPath.empty()) {
        if (!sheet.Load(rend, sheetPath, 8, 8)) {
            AppendDiag(std::string("Failed to load spritesheet: ") + sheetPath);
            std::cerr << "Failed to load spritesheet: " << sheetPath << "\n";
            std::cout << "Continuing without sprite...\n";
        } else {
            AppendDiag(std::string("Successfully loaded spritesheet: ") + sheetPath);
            std::cout << "Loaded sheet: " << sheetPath << " (" << sheet.Columns() << " cols x " << sheet.Rows() << " rows)\n";
            sheetLoaded = true;
            // Force nearest-neighbor (pixel-perfect) sampling for this texture
            if (sheet.GetTexture()) {
                bool ok = SDL_SetTextureScaleMode(sheet.GetTexture(), SDL_SCALEMODE_NEAREST);
                if (!ok) {
                    std::cerr << "Warning: SDL_SetTextureScaleMode(..., SDL_SCALEMODE_NEAREST) not supported or failed.\n";
                }
            }
        }
    }

    std::atomic<int> currentIndex(-1);
    std::atomic<bool> running(true);

    // Decide whether stdin is a real console (tty). If not, don't spawn the console thread
    bool hasConsole = false;
#ifdef _WIN32
    hasConsole = (_isatty(_fileno(stdin)) != 0);
#else
    // fallback: assume console exists
    hasConsole = true;
#endif

    std::thread consoleThread;
    if (hasConsole) {
        // Thread de consola: leer índices y comandos
        consoleThread = std::thread([&](){
            std::string line;
            std::cout << "Introduce índice (o 'q' para salir). Ej: 0\n";
            while (running) {
                if (!std::getline(std::cin, line)) { std::this_thread::sleep_for(std::chrono::milliseconds(50)); continue; }
                if (line == "q" || line == "quit") { running = false; break; }
                try {
                    int idx = std::stoi(line);
                    currentIndex.store(idx);
                } catch (...) {
                    std::cout << "Índice inválido\n";
                }
            }
        });
    } else {
        // No console: instruct user to use keyboard controls
        currentIndex.store(0);
        std::cout << "No console detected. Use arrow keys to change index, numbers to set digit, Q or ESC to quit.\n";
    }

    // Cámara simple: render centrado
    float camX = WIN_W / 2.0f;
    float camY = WIN_H / 2.0f;

    // Loop principal: render a 60Hz y lee currentIndex
    while (running) {
        SDL_Event ev;
        while (SDL_PollEvent(&ev)) {
            if (ev.type == SDL_EVENT_QUIT) running = false;
            if (ev.type == SDL_EVENT_KEY_DOWN) {
                int sym = ev.key.key;
                if (sym == SDLK_Q || sym == SDLK_ESCAPE) { running = false; }
                else if (sym == SDLK_RIGHT) { currentIndex.fetch_add(1); }
                else if (sym == SDLK_LEFT) { int v = currentIndex.load(); if (v>0) currentIndex.store(v-1); }
                else if (sym >= SDLK_0 && sym <= SDLK_9) { int digit = sym - SDLK_0; currentIndex.store(digit); }
            }
        }

        SDL_SetRenderDrawColor(rend, 0, 0, 0, 255);
        SDL_RenderClear(rend);

        int idx = currentIndex.load();
        if (idx >= 0 && sheetLoaded && sheet.GetTexture()) {
                SDL_Rect srcI = sheet.GetSrcRect(idx);
                // dibujamos en el centro, escalado por 'scale' (usar SDL_FRect para SDL3 API)
                int dstW = srcI.w * scale;
                int dstH = srcI.h * scale;
                int dstX = (int)std::floor(camX - dstW / 2.0f);
                int dstY = (int)std::floor(camY - dstH / 2.0f);
                SDL_FRect srcF = {(float)srcI.x, (float)srcI.y, (float)srcI.w, (float)srcI.h};
                SDL_FRect dstF = {(float)dstX, (float)dstY, (float)dstW, (float)dstH};
                SDL_RenderTexture(rend, sheet.GetTexture(), &srcF, &dstF);
        } else if (!sheetLoaded) {
            // Draw a simple rectangle as fallback when no sprite is loaded
            SDL_SetRenderDrawColor(rend, 255, 255, 255, 255);
            SDL_FRect rect = {camX - 50, camY - 50, 100, 100};
            SDL_RenderFillRect(rend, &rect);
            SDL_SetRenderDrawColor(rend, 255, 0, 0, 255);
            SDL_FRect border = {camX - 52, camY - 52, 104, 104};
            SDL_RenderRect(rend, &border);
        }

        SDL_RenderPresent(rend);
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }

    // cleanup
    running = false;
    if (consoleThread.joinable()) consoleThread.join();
    // Clean up manually created window and renderer
    if (rend) SDL_DestroyRenderer(rend);
    if (testWin) SDL_DestroyWindow(testWin);
    SDL_Quit();
    return 0;
}
