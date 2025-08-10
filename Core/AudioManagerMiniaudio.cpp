#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#include "AudioManagerMiniaudio.h"
#include <iostream>
#include <vector>
#include <cstring>


struct AudioManagerMiniaudio::SoundData {
    ma_sound sound;
    ma_decoder decoder;
    bool loaded = false;
};

AudioManagerMiniaudio::AudioManagerMiniaudio() : pEngine(nullptr) {}
AudioManagerMiniaudio::~AudioManagerMiniaudio() { Shutdown(); }

bool AudioManagerMiniaudio::Initialize() {
    ma_result result;
    ma_engine* engine = new ma_engine();
    result = ma_engine_init(NULL, engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Error al inicializar miniaudio engine" << std::endl;
        delete engine;
        return false;
    }
    // Arrancar el engine explícitamente
    result = ma_engine_start(engine);
    if (result != MA_SUCCESS) {
        std::cerr << "Error al arrancar miniaudio engine" << std::endl;
        ma_engine_uninit(engine);
        delete engine;
        return false;
    }
    pEngine = engine;
    return true;
}

void AudioManagerMiniaudio::Shutdown() {
    for (auto& pair : sounds) {
        if (pair.second->loaded) {
            ma_sound_uninit(&pair.second->sound);
            ma_decoder_uninit(&pair.second->decoder);
        }
        delete pair.second;
    }
    sounds.clear();
    if (pEngine) {
        ma_engine_uninit((ma_engine*)pEngine);
        delete (ma_engine*)pEngine;
        pEngine = nullptr;
    }
}
// ...existing code...
bool AudioManagerMiniaudio::LoadSound(const std::string& name, const std::string& filepath) {
    if (sounds.count(name)) return true;
    SoundData* data = new SoundData();
    ma_result result = ma_decoder_init_file(filepath.c_str(), NULL, &data->decoder);
    if (result != MA_SUCCESS) {
        std::cerr << "No se pudo cargar el sonido: " << filepath << std::endl;
        delete data;
        return false;
    }
    ma_engine* engine = (ma_engine*)pEngine;
    result = ma_sound_init_from_data_source(engine, &data->decoder, 0, NULL, &data->sound);
    if (result != MA_SUCCESS) {
        std::cerr << "No se pudo inicializar el sonido: " << filepath << std::endl;
        ma_decoder_uninit(&data->decoder);
        delete data;
        return false;
    }
    data->loaded = true;
    sounds[name] = data;
    return true;
}
// Nota: El nombre PlaySoundManager se usa para evitar conflicto con la macro PlaySound de windows.h
void AudioManagerMiniaudio::PlaySoundManager(const std::string& name, float volume) {
    auto it = sounds.find(name);
    if (it == sounds.end()) return;
    ma_sound_set_volume(&it->second->sound, volume);
    ma_sound_start(&it->second->sound);
}
