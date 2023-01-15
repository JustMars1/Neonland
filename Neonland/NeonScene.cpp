#include "NeonScene.hpp"

#include <iostream>
#include <random>
#include <atomic>

NeonScene::NeonScene(size_t maxInstanceCount, double timestep, GameClock clock)
: _maxInstanceCount{maxInstanceCount}
, _timestep{timestep}
, _instances(maxInstanceCount)
, clock(clock)
, _nextTickTime{clock.Time()}
, _prevRenderTime{clock.Time()} {
    
    _scene.CreateEntity(Transform(float3{0, 0, 0}, float3{0, -90, 0}, float3{1, mapSize.y, mapSize.x}), Mesh(Mesh::PLANE));
    
    auto tf = Transform();
    player = _scene.CreateEntity(Physics(tf), std::move(tf), Mesh(Mesh::PLAYER), HP());
    
    cam = _scene.CreateEntity<Camera>();
    crosshair = _scene.CreateEntity(Transform(), Mesh(Mesh::CROSSHAIR));
    
    _scene.Get<Camera>(cam).SetPosition({0, 0, -camDistance});
    _scene.Get<Camera>(cam).SetFarClipPlane(200);
    
    size_t enemyCount = 10'000;
    int columns = sqrt(enemyCount), rows = sqrt(enemyCount);
    
    float gap = 2.0f;
    
    float width = (rows - 1) * (1 + gap);
    float height = (columns - 1) * (1 + gap);
    
    std::random_device device;
    std::default_random_engine engine(device());
    std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);
    
    for (int x = 0; x < columns; x++) {
        for (int y = 0; y < rows; y++) {
            float3 pos = {x * (1 + gap) - width / 2, y * (1 + gap) - height / 2};
            Transform tf(pos);
            Entity enemy = _scene.CreateEntity(Physics(tf), std::move(tf), Mesh(Mesh::ENEMY), Enemy(), HP());
            _scene.Get<Physics>(enemy).angularVelocity.z = 30 * distribution(engine);
        }
    }
}

void NeonScene::Update(float aspectRatio) {
    double time = clock.Time();
    double dt = time - _prevRenderTime;
    
    if (aspectRatio != _scene.Get<Camera>(cam).GetAspectRatio()) {
        _scene.Get<Camera>(cam).SetAspectRatio(aspectRatio);
    }
    
    moveDir += {directionalInput.x, directionalInput.y, 0};
    float length = simd_length(moveDir);
    if (length > 0.0f) {
        moveDir /= length;
    }
    
    mousePressed |= !prevMouseState && mouseDown;
    
    EarlyRender(time, dt);
    
    bool didTick = time >= _nextTickTime;
    while (time >= _nextTickTime) {
        Tick(time);
        _nextTickTime += _timestep;
    }
    
    LateRender(time, dt);
    
    _prevRenderTime = time;
    mouseDelta = {0, 0};
    
    if (didTick) {
        moveDir = {0, 0, 0};
        prevMouseState = mouseDown;
        mousePressed = false;
    }
}

void NeonScene::EarlyRender(double time, double dt) {
    _scene.Get<Physics>(player).velocity = moveDir * 5;
}

void NeonScene::Tick(double time) {
    _scene.GetGroup<Transform, Physics>()->UpdateParallel([timestep = _timestep](auto entity, auto& tf, auto& physics) {
        Physics::Update(physics, tf, timestep);
    });
    
    {
        float3 pos = _scene.Get<Physics>(player).position;
        pos.x = std::clamp(pos.x, -mapSize.x / 2, mapSize.x / 2);
        pos.y = std::clamp(pos.y, -mapSize.y / 2, mapSize.y / 2);
        _scene.Get<Physics>(player).position = pos;
    }
    
    if (mouseDown && shotCooldownEndTime < time) {
        float3 playerPos = _scene.Get<Transform>(player).position;
        float3 aimPos = _scene.Get<Transform>(crosshair).position;
        
        float3 dir = aimPos - playerPos;
        
        float length = simd_length(dir);
        if (length > 0.0f) {
            dir /= length;
        }
        
        float r = std::atan2f(dir.y, dir.x) * RadToDeg;
        
        float3 spawnPos = playerPos + dir * 0.5f;
        
        auto tf = Transform(spawnPos, float3{0, 0, r}, float3{1.0f, 0.25f, 0.25f});
        _scene.CreateEntity(Physics(tf, dir * 20.0f), std::move(tf), Mesh(Mesh::PROJECTILE), PlayerProjectile(1, time + 1.0f));
        shotCooldownEndTime = time + shotCooldown;
    }
    
    auto& playerTf = _scene.Get<Transform>(player);
    auto& playerPhysics = _scene.Get<Physics>(player);
    
    std::atomic<int> totalDamage = 0;
    _scene.GetGroup<Transform, Physics, Enemy>()->UpdateParallel([&, t = time](auto entity,
                                                                               auto& tf,
                                                                               auto& physics,
                                                                               auto& enemy) {
        if (Physics::Overlapping(physics, playerPhysics, tf, playerTf)) {
            if (enemy.cooldownEndTime < t) {
                totalDamage += enemy.attackDamage;
                enemy.cooldownEndTime = t + enemy.attackCooldown;
            }
        }
    });
    _scene.Get<HP>(player).currentHP -= totalDamage;
    
    _scene.GetGroup<Transform, Physics, PlayerProjectile>()->Update([&, t = time](auto projectileEntity,
                                                                                  auto& projectileTf,
                                                                                  auto& projectilePhysics,
                                                                                  auto& projectile) {
        std::atomic<bool> didHit = false;
        _scene.GetGroup<Transform, Physics, Enemy, HP>()->UpdateParallel([&](auto enemyEntity,
                                                                             auto& enemyTf,
                                                                             auto& enemyPhysics,
                                                                             auto& enemy,
                                                                             auto& enemyHP) {
            if (Physics::Overlapping(projectilePhysics, enemyPhysics, projectileTf, enemyTf)) {
                enemyHP.currentHP -= projectile.damage;
                didHit = true;
            }
        });
        
        if (didHit || projectile.despawnTime < t) {
            _scene.DestroyEntity(projectileEntity);
        }
    });
    
    
    _scene.GetGroup<HP>()->UpdateParallel([&](auto entity,
                                              auto& hp) {
        if (hp.currentHP < 0) {
            
            if (entity == player) {
                gameOver = true;
            }
            else {
                _scene.DestroyEntity(entity);
            }
        }
    });
}

void NeonScene::LateRender(double time, double dt) {
    const double interpolation = std::clamp((_timestep - (_nextTickTime - time)) / _timestep, 0.0, 1.0);
    
    _scene.GetGroup<Transform, Physics>()->UpdateParallel([interpolation](auto entity,
                                                                          auto& tf,
                                                                          auto& physics) {
        if (!tf.teleported) {
            tf.position = physics.GetInterpolatedPosition(interpolation);
        }
        
        if (!tf.rotationSet) {
            tf.rotation = physics.GetInterpolatedRotation(interpolation);
        }
    });
    
    {
        float3 playerPos = _scene.Get<Physics>(player).GetInterpolatedPosition(interpolation);
        _scene.Get<Camera>(cam).SetPosition({0.0f, 0.0f, playerPos.z - camDistance});
        float3 topRightPos = _scene.Get<Camera>(cam).ScreenPointToWorld({1.0f, 1.0f}, playerPos.z);
        float hWidth = topRightPos.x;
        float hHeight = topRightPos.y;
        
        float3 camPos = playerPos;
        camPos.x = std::clamp(camPos.x, std::min(-mapSize.x / 2 + hWidth, 0.0f), std::max(mapSize.x / 2 - hWidth, 0.0f));
        camPos.y = std::clamp(camPos.y, std::min(-mapSize.y / 2 + hHeight, 0.0f), std::max(mapSize.y  / 2 - hHeight, 0.0f));
        camPos.z -= camDistance;
        
        _scene.Get<Camera>(cam).SetPosition(camPos);
    }
    
    {
        float3 playerPos = _scene.Get<Physics>(player).position;
        float depth = playerPos.z;
        float3 crosshairPos = _scene.Get<Camera>(cam).ScreenPointToWorld(mousePos, depth);
        _scene.Get<Transform>(crosshair).position = crosshairPos;
        
        float3 dir = crosshairPos - playerPos;
        
        float length = simd_length(dir);
        if (length > 0.0f) {
            dir /= length;
        }
        
        float r = std::atan2f(dir.y, dir.x) * RadToDeg;
        _scene.Get<Transform>(player).SetRotation({0, 0, r});
    }
    
    constexpr static auto xAxis = float3{1, 0, 0};
    constexpr static auto yAxis = float3{0, 1, 0};
    constexpr static auto zAxis = float3{0, 0, 1};
    
    _scene.GetGroup<Transform, Mesh>()->UpdateParallel([interpolation](auto entity,
                                                                       auto& tf,
                                                                       auto& mesh) {
        float4x4 S = ScaleMatrix(tf.scale);
        float4x4 T = TranslationMatrix(tf.position);
        
        float4x4 rX = RotationMatrix(xAxis, tf.rotation.x);
        float4x4 rY = RotationMatrix(yAxis, tf.rotation.y);
        float4x4 rZ = RotationMatrix(zAxis, tf.rotation.z);
        
        float4x4 R = rZ * rY * rX;
        
        mesh.modelMatrix =  T * R * S;
    });
}

FrameData NeonScene::GetFrameData() {
    GlobalUniforms uniforms;
    uniforms.projMatrix = _scene.Get<Camera>(cam).GetProjectionMatrix();
    uniforms.viewMatrix = _scene.Get<Camera>(cam).GetViewMatrix();
    
    FrameData frameData;
    frameData.globalUniforms = uniforms;
    frameData.clearColor = _scene.Get<Camera>(cam).clearColor;
    
    _groupSizes.clear();
    _instances.clear();
    auto meshPool = _scene.GetPool<Mesh>();
    meshPool->Sort();
    
    if (meshPool->size() > 0) {
        _groupSizes.resize(meshPool->back().type + 1, 0);
        
        for (auto& mesh : *meshPool) {
            Instance instance;
            instance.transform = mesh.modelMatrix;
            _instances.emplace_back(instance);
            _groupSizes[mesh.type]++;
        }
    }
    
    assert(_instances.size() < MAX_INSTANCE_COUNT && "Number of instances must be less than MAX_ENTITY_COUNT");
    frameData.instanceCount = _instances.size();
    frameData.instances = _instances.data();
    
    frameData.groupCount = static_cast<uint32_t>(_groupSizes.size());
    frameData.groupSizes = _groupSizes.data();
    
    return frameData;
}

double NeonScene::Timestep() const {
    return _timestep;
}

size_t NeonScene::MaxInstanceCount() const {
    return _maxInstanceCount;
}
