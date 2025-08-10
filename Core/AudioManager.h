#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <SDL3/SDL.h>
#include <string>
#include <map>
#include <iostream>

struct SoundEffect {
    Uint8* buffer;
    Uint32 length;
    SDL_AudioSpec spec;
    bool isLoaded;
    
    SoundEffect() : buffer(nullptr), length(0), isLoaded(false) {}
};

class AudioManager {
private:
    bool isInitialized;
    SDL_AudioDeviceID audioDevice;
    std::map<std::string, SoundEffect> soundEffects;
    float masterVolume;
    
    // Ajustar volumen de un buffer de audio
    void adjustVolume(Uint8* buffer, Uint32 length, float volume);
    
public:
    AudioManager();
    ~AudioManager();
    
    // Inicialización
    bool Initialize();
    void Shutdown();
    
    // Cargar sonidos WAV
    bool LoadSound(const std::string& name, const std::string& filepath);
    
    // Reproducir sonidos
    void PlaySound(const std::string& name, float volume = 1.0f);
    
    // Control de volumen
    void SetMasterVolume(float volume); // 0.0 - 1.0
    float GetMasterVolume() const;
    
    // Estado
    bool IsInitialized() const;
    
    // Debug
    void ShowDebugInfo() const;
};

#endif // AUDIOMANAGER_H
