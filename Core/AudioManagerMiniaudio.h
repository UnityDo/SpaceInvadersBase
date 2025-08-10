#pragma once
#include <string>
#include <unordered_map>

class AudioManagerMiniaudio {
public:
    AudioManagerMiniaudio();
    ~AudioManagerMiniaudio();
    bool Initialize();
    void Shutdown();
    bool LoadSound(const std::string& name, const std::string& filepath);
    void PlaySoundManager(const std::string& name, float volume = 1.0f);
private:
    struct SoundData;
    std::unordered_map<std::string, SoundData*> sounds;
    void* pEngine; // miniaudio engine
};
