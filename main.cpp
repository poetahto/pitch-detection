#include <iostream>
#include <cassert>
#include <windows.h>
#include <mmdeviceapi.h>


#define CHECK_SUCCESS(result) if (FAILED(result)) { goto Exit; }
#define SAFE_FREE(object) if ((object) != nullptr) { (object)->Release(); (object) = nullptr; }

class AudioSource {
public:
    virtual void Play() = 0;
};

class SinAudioSource : public AudioSource {
public:
    void Play() override {
        printf("Playing sin wave\n");
    }
};

class WavAudioSource : public AudioSource {
public:
    void Play() override {
        printf("Playing wav file!\n");
    }
};


void PlayAudioStream(AudioSource &source) {
    HRESULT hr;

    IMMDeviceEnumerator enumerator;
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IID_IMMDeviceEnumerator), (void**) &enumerator);
    CHECK_SUCCESS(hr);

    source.Play();
Exit:
    SAFE_FREE(enumerator);
}

int main() {
    SinAudioSource sinSource {};
    //WavAudioSource wavSource {};
    PlayAudioStream(sinSource);
}
