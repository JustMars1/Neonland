#pragma once

#include "Scene.hpp"
#include "GameClock.hpp"
#include "NeonConstants.h"

#include "Transform.hpp"
#include "Physics.hpp"
#include "Camera.hpp"
#include "Mesh.hpp"
#include "HP.hpp"
#include "Enemy.hpp"
#include "PlayerProjectile.hpp"

#include "FrameData.h"

class NeonScene {
public:
    GameClock clock;
    
    float2 mouseDelta = {0, 0};
    float2 mousePos = {0, 0};
    float2 prevMousePos = {0, 0};
    
    bool mouseDown = false;
    bool prevMouseState = false;
    bool mousePressed = false;
    
    double shotCooldown = 0.1f;
    double shotCooldownEndTime = 0;
    
    float2 directionalInput = {0, 0};
    float3 moveDir = {0, 0, 0};
    
    Entity player;
    Entity cam;
    Entity crosshair;
    
    float2 mapSize = {20, 20};
    
    bool gameOver = false;
    
    float camDistance = 20;
    
    NeonScene(size_t maxInstanceCount, double timestep, GameClock clock);
    
    void Update(float aspectRatio);
    
    void EarlyRender(double time, double dt);
    void Tick(double time);
    void LateRender(double time, double dt);
    
    FrameData GetFrameData();
    
    double Timestep() const;
    
    size_t MaxInstanceCount() const;
private:
    Scene _scene;
    
    std::vector<Instance> _instances;
    std::vector<size_t> _groupSizes;
    
    size_t _maxInstanceCount;
    
    double _timestep;
    double _nextTickTime;
    double _prevRenderTime;
};
