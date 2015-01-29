/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#pragma once

#include "Locus/Common/IDType.h"

#include "Locus/Simulation/Scene.h"

#include "Locus/Rendering/DrawablePointCloud.h"
#include "Locus/Rendering/SkyBox.h"
#include "Locus/Rendering/Light.h"

#include "Locus/Geometry/CollisionManager.h"

#include "Player.h"
#include "HUD.h"

#include <memory>

#include <cstddef>

namespace Locus
{

class SceneManager;
class SAPReader;
class SoundEffect;
class SoundState;
class RenderingState;

}

namespace MPM
{

class Asteroid;
class Planet;
class Shot;
class TextureManager;

class DemoScene : public Locus::Scene
{
public:
   DemoScene(Locus::SceneManager& sceneManager, unsigned int resolutionX, unsigned int resolutionY);
   ~DemoScene();

   virtual void Activate() override;

   virtual bool Update(double DT) override;
   virtual void Draw() override;

   virtual void InitializeRenderingState() override;

   virtual void KeyPressed(Locus::Key_t key) override;
   virtual void KeyReleased(Locus::Key_t key) override;

   virtual void MousePressed(Locus::MouseButton_t button) override;
   virtual void MouseMoved(int x, int y) override;

   virtual void Resized(int width, int height) override;

private:
   std::unique_ptr< Locus::RenderingState > renderingState;
   std::unique_ptr< Locus::SoundState > soundState;

   std::unique_ptr< MPM::TextureManager > textureManager;
   Locus::CollisionManager collisionManager;

   std::unique_ptr< Locus::SoundEffect > shotSoundEffect;
   std::unique_ptr< Locus::SoundEffect > asteroidShotCollisionSoundEffect;

   bool dieOnNextFrame;

   Player player;

   unsigned int maxLights;

   std::vector<Locus::Light> lights;
   std::vector<Locus::Color> lightColors;

   std::size_t currentLightColorIndex;

   Locus::ID_t notTexturedNotLitProgramID;
   Locus::ID_t texturedNotLitProgramID;
   std::vector<Locus::ID_t> litProgramIDs;

   unsigned int resolutionX;
   unsigned int resolutionY;

   int lastMouseX;
   int lastMouseY;

   int score;
   int lives;
   int level;

   int crosshairsX;
   int crosshairsY;

   float minPlanetDistance;
   float maxPlanetDistance;
   float starDistance;
   float z_far;

   std::unique_ptr<Locus::Mesh> planetMesh;
   std::unique_ptr<Locus::Mesh> moonMesh;

   std::vector<std::unique_ptr<Planet>> planets;
   Locus::DrawablePointCloud stars;

   std::vector<std::unique_ptr<Shot>> shots;
   std::unique_ptr<Locus::Mesh> shotMesh;
   Locus::SkyBox skyBox;

   std::vector<std::unique_ptr<Locus::Mesh>> asteroidMeshes;
   std::vector<std::unique_ptr<Asteroid>> asteroids;

   HUD hud;

   void Initialize();
   void InitializeStars();
   void InitializeAsteroids();
   void InitializeMeshes();
   void InitializePlanets();
   void InitializeSkyBoxAndHUD();

   void Load();
   void LoadRenderingState();
   void LoadShaderPrograms();

   void LoadAudioState();
   void LoadLights();
   void LoadTextures();

   void DestroyRenderingState();

   void UpdateLastMousePosition();

   void TickAsteroids(double DT);
   void CheckForAsteroidHits();

   Locus::Plane MakeHalfSplitPlane(const Locus::Vector3& shotPosition, const Locus::Vector3& asteroidCentroid);
   void SplitAsteroid(std::size_t splitIndex, const Locus::Vector3& shotPosition);
   void UpdateShotPositions(double DT);
   void ShotFired();

   void DrawShots();
   void DrawAsteroids();
   void DrawHUD();
   void DrawSkyBox();
   void DrawStars();
   void DrawPlanets();
};

}