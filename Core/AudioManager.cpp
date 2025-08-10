#include "AudioManager.h"
#include <cstdio>

AudioManager::AudioManager() 
    : isInitialized(false), audioDevice(0), masterVolume(0.7f) {
}

AudioManager::~AudioManager() {
    Shutdown();
}

bool AudioManager::Initialize() {
    if (isInitialized) {
        return true;
    }
    
    std::cout << "🔊 Inicializando AudioManager SDL3..." << std::endl;
    
    // En SDL3, simplemente inicializamos el subsistema de audio
    if (!SDL_Init(SDL_INIT_AUDIO)) {
        std::cout << "❌ Error al inicializar audio: " << SDL_GetError() << std::endl;
        return false;
    }
    
    audioDevice = 1; // Marcador que está inicializado
    isInitialized = true;
    
    std::cout << "✅ AudioManager inicializado correctamente" << std::endl;
    std::cout << "   Modo: Reproducción básica SDL3" << std::endl;
    
    return true;
}

void AudioManager::Shutdown() {
    if (!isInitialized) {
        return;
    }
    
    std::cout << "🔊 Cerrando AudioManager..." << std::endl;
    
    // Limpiar registro de sonidos
    soundEffects.clear();
    
    audioDevice = 0;
    isInitialized = false;
    std::cout << "✅ AudioManager cerrado" << std::endl;
}

bool AudioManager::LoadSound(const std::string& name, const std::string& filepath) {
    if (!isInitialized) {
        std::cout << "⚠️ AudioManager no inicializado" << std::endl;
        return false;
    }
    
    std::cout << "📂 Cargando sonido: " << name << " desde " << filepath << std::endl;
    
    // Para esta implementación simplificada, solo verificamos que el archivo existe
    FILE* file = fopen(filepath.c_str(), "rb");
    if (!file) {
        std::cout << "❌ Error al abrir archivo " << filepath << std::endl;
        return false;
    }
    
    // Obtener tamaño del archivo
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    fseek(file, 0, SEEK_SET);
    fclose(file);
    
    // Crear entrada de sonido
    SoundEffect sound;
    sound.isLoaded = true;
    sound.length = fileSize;
    sound.buffer = nullptr; // Simplificado - no cargamos el buffer real
    
    soundEffects[name] = sound;
    
    std::cout << "✅ Sonido '" << name << "' registrado correctamente" << std::endl;
    std::cout << "   Tamaño archivo: " << fileSize << " bytes" << std::endl;
    
    return true;
}

void AudioManager::adjustVolume(Uint8* buffer, Uint32 length, float volume) {
    if (volume >= 1.0f) return; // No necesita ajuste
    
    // Interpretar como muestras de 16-bit signed
    Sint16* samples = (Sint16*)buffer;
    Uint32 sampleCount = length / sizeof(Sint16);
    
    for (Uint32 i = 0; i < sampleCount; i++) {
        samples[i] = (Sint16)(samples[i] * volume);
    }
}

void AudioManager::PlaySound(const std::string& name, float volume) {
    if (!isInitialized) {
        std::cout << "⚠️ AudioManager no inicializado" << std::endl;
        return;
    }
    
    auto it = soundEffects.find(name);
    if (it == soundEffects.end()) {
        std::cout << "⚠️ Sonido '" << name << "' no encontrado" << std::endl;
        return;
    }
    
    const SoundEffect& sound = it->second;
    if (!sound.isLoaded || !sound.buffer) {
        std::cout << "⚠️ Sonido '" << name << "' no está cargado correctamente" << std::endl;
        return;
    }
    
    // Calcular volumen final
    float finalVolume = masterVolume * volume;
    
    // Para SDL3, usamos un método más simple
    // Solo reproducimos si el volumen es suficiente
    if (finalVolume > 0.1f) {
        std::cout << "🔊 Reproduciendo: " << name << " (vol: " << (int)(finalVolume * 100) << "%)" << std::endl;
    }
    
    // Nota: En esta implementación simplificada, el sonido se "reproduce" conceptualmente
    // En una implementación completa, necesitaríamos un callback de audio o usar SDL_mixer
}

void AudioManager::SetMasterVolume(float volume) {
    masterVolume = SDL_clamp(volume, 0.0f, 1.0f);
    std::cout << "🔊 Volumen maestro: " << (int)(masterVolume * 100) << "%" << std::endl;
}

float AudioManager::GetMasterVolume() const {
    return masterVolume;
}

bool AudioManager::IsInitialized() const {
    return isInitialized;
}

void AudioManager::ShowDebugInfo() const {
    std::cout << "\n🔊 === AUDIO DEBUG INFO ===" << std::endl;
    std::cout << "Inicializado: " << (isInitialized ? "✅" : "❌") << std::endl;
    std::cout << "Dispositivo ID: " << audioDevice << std::endl;
    std::cout << "Volumen maestro: " << (int)(masterVolume * 100) << "%" << std::endl;
    std::cout << "Sonidos cargados: " << soundEffects.size() << std::endl;
    
    for (const auto& pair : soundEffects) {
        std::cout << "  - " << pair.first << " (" << pair.second.length << " bytes)" << std::endl;
    }
    
    // Estado del dispositivo
    if (audioDevice != 0) {
        // En SDL3, no hay SDL_GetQueuedAudioSize equivalente directo
        // Podemos mostrar otra información útil
        std::cout << "Dispositivo activo: ID " << audioDevice << std::endl;
    }
    
    std::cout << "=========================" << std::endl;
}
