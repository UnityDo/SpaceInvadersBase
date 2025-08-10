#ifndef AUDIOMANAGER_BEEP_H
#define AUDIOMANAGER_BEEP_H

#include <SDL3/SDL.h>
#include <string>
#include <map>
#include <iostream>
#include <windows.h>

class AudioManagerBeep {
private:
    bool isInitialized;
    float masterVolume;
    std::map<std::string, bool> soundEffects;
    
public:
    AudioManagerBeep();
    ~AudioManagerBeep();
    
    // Inicialización
    bool Initialize();
    void Shutdown();
    
    // Cargar sonidos (solo registrar nombres)
    bool LoadSound(const std::string& name, const std::string& filepath);
    
    // Reproducir sonidos usando Beep de Windows
    void PlaySound(const std::string& name, float volume = 1.0f);
    
    // Control de volumen
    void SetMasterVolume(float volume);
    float GetMasterVolume() const;
    
    // Estado
    bool IsInitialized() const;
    
    // Debug
    void ShowDebugInfo() const;
};

#endif // AUDIOMANAGER_BEEP_H
