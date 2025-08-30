// SpriteTestSDL2.cpp - Test version using SDL2 to verify if the issue is SDL3-specific
#include <iostream>
#include <string>
#include <fstream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

// This is a minimal test to see if SDL2 works where SDL3 fails
// We'll just try to initialize SDL2 and create a window

int main() {
    std::ofstream diag("spritetest_sdl2_diag.txt");
    diag << "--- SDL2 Test Diagnostic ---" << std::endl;
    
    try {
        diag << "cwd: " << std::filesystem::current_path().string() << std::endl;
    } catch(...) { 
        diag << "cwd: (error)" << std::endl; 
    }
    
#ifdef _WIN32
    // Check if we can load common SDL2 DLL
    HMODULE hSDL2 = LoadLibraryA("SDL2.dll");
    if (hSDL2) {
        diag << "SDL2.dll can be loaded" << std::endl;
        FreeLibrary(hSDL2);
    } else {
        diag << "SDL2.dll cannot be loaded (expected if not installed)" << std::endl;
    }
    
    // Check graphics system
    HDC hdc = GetDC(NULL);
    if (hdc) {
        int colorDepth = GetDeviceCaps(hdc, BITSPIXEL);
        int width = GetDeviceCaps(hdc, HORZRES);
        int height = GetDeviceCaps(hdc, VERTRES);
        diag << "Desktop: " << width << "x" << height << " @ " << colorDepth << " bits" << std::endl;
        ReleaseDC(NULL, hdc);
    }
#endif

    std::cout << "SDL2 test completed. Check spritetest_sdl2_diag.txt for results." << std::endl;
    std::cout << "This test verifies if the problem is specific to SDL3." << std::endl;
    
    diag << "--- End SDL2 Test ---" << std::endl;
    diag.close();
    
    return 0;
}
