#include "Neonland.h"

#include "NeonScene.hpp"

namespace {

auto scene = NeonScene(MAX_INSTANCE_COUNT, TIMESTEP);
    
}

void Neon_Start() {
    scene.Start();
}

#ifdef _WIN64
void Neon_SetSaveFilePath(const wchar_t* path, size_t len) {
    scene.saveFilePath = std::wstring(path, len);
}
#endif

FrameData Neon_Render(float aspectRatio) {
    scene.Update(aspectRatio);
    
    return scene.GetFrameData();
}

void Neon_UpdateCursorPosition(float x, float y) {
    scene.prevMousePos = scene.mousePos;
    
    scene.mousePos.x = std::clamp(x, -1.0f, 1.0f);
    scene.mousePos.y = std::clamp(y, -1.0f, 1.0f);
    scene.mouseDelta += scene.mousePos - scene.prevMousePos;
}

void Neon_UpdateMouseDown(bool down) {
    scene.mouseDown = down;
}

void Neon_UpdateDirectionalInput(float x, float y) {
    scene.directionalInput = VecNormalize(float2{x, y});
}

void Neon_UpdateNumberKeyPressed(int num) {
    if (num == 0) {
        num = 10;
    }
    
    scene.SelectWeapon(num - 1);
}

void Neon_EscapePressed() {
    scene.TogglePause();
}

void Neon_UpdateTextureSize(TextureType tex, TexSize size) {
    scene.textureSizes[tex] = size;
}

bool Neon_IsMusic(AudioType audio) {
    return audio == MUSIC_AUDIO;
}

float Neon_MusicVolume() {
    return 0.1f;
}

float Neon_SFXVolume() {
    return scene.RandomBetween(0.1f, 0.15f);
}

bool Neon_AppShouldQuit() {
    return scene.appShouldQuit;
}