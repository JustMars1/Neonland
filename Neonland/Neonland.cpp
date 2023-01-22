#include "Neonland.h"

#include "NeonScene.hpp"

#include <iostream>

namespace {

auto scene = NeonScene(MAX_INSTANCE_COUNT, TIMESTEP);
    
}

void Neon_Start() {
    scene.Start();
}

FrameData Neon_Render(float aspectRatio) {
    scene.Update(aspectRatio);
    
    return scene.GetFrameData();
}

void Neon_UpdateCursorPosition(vector_float2 newPos) {
    scene.prevMousePos = scene.mousePos;
    
    scene.mousePos.x = std::clamp(newPos.x, -1.0f, 1.0f);
    scene.mousePos.y = std::clamp(newPos.y, -1.0f, 1.0f);
    scene.mouseDelta += scene.mousePos - scene.prevMousePos;
}

void Neon_UpdateMouseDown(bool down) {
    scene.mouseDown = down;
}

void Neon_UpdateDirectionalInput(vector_float2 newDir) {
    scene.directionalInput = VecNormalize(newDir);
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
