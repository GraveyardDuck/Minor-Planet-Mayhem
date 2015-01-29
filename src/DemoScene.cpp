/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "DemoScene.h"
#include "Config.h"
#include "Moon.h"
#include "Asteroid.h"
#include "Shot.h"
#include "Planet.h"
#include "PauseScene.h"
#include "SAPReading.h"

#include "Locus/Common/Random.h"

#include "Locus/FileSystem/FileSystemUtil.h"
#include "Locus/FileSystem/MountedFilePath.h"

#include "Locus/Geometry/Geometry.h"
#include "Locus/Geometry/Line.h"
#include "Locus/Geometry/Frustum.h"

#include "Locus/Rendering/MeshUtility.h"
#include "Locus/Rendering/DrawUtility.h"
#include "Locus/Rendering/ShaderVariables.h"
#include "Locus/Rendering/ShaderSourceStore.h"
#include "Locus/Rendering/ShaderLinkException.h"
#include "Locus/Rendering/RenderingState.h"
#include "Locus/Rendering/Texture.h"

#include "Locus/Audio/SoundState.h"
#include "Locus/Audio/SoundEffect.h"

#include "Locus/Simulation/UserEvents.h"
#include "Locus/Simulation/SceneManager.h"

#include "Locus/Rendering/Locus_glew.h"

#include <stack>
#include <unordered_map>
#include <stdexcept>
#include <fstream>

#include <math.h>

//TODO: Remove magic numbers, either by putting in data files or use a scripting interface

#define PLANET_SPACING_THRESHOLD 30
#define MOON_ORBIT_DISTANCE 0.6f
#define MOON_RADIUS 0.5f

#define SKY_BOX_RADIUS 100

#define ASTEROID_SPACING_THRESHOLD 0
#define MAX_ASTEROID_HITS 20
#define MIN_ASTEROID_SCALE 10
#define MAX_ASTEROID_SCALE 30

#define FIELD_OF_VIEW 30
#define Z_NEAR 0.01f

#define FRUSTUM_VERTICAL_FIELD_OF_VIEW (1.9f * FIELD_OF_VIEW)
#define FRUSTUM_NEAR_DISTANCE 0.01f
#define FRUSTUM_FAR_DISTANCE (STAR_DISTANCE * 4)

namespace MPM
{

static const Locus::Key_t KEY_FORWARD = Locus::Key_W;
static const Locus::Key_t KEY_BACKWARD = Locus::Key_S;
static const Locus::Key_t KEY_LEFT = Locus::Key_A;
static const Locus::Key_t KEY_RIGHT = Locus::Key_F;
static const Locus::Key_t KEY_UP = Locus::Key_E;
static const Locus::Key_t KEY_DOWN = Locus::Key_D;

static const Locus::Key_t KEY_PAUSE = Locus::Key_P;
static const Locus::Key_t KEY_CONTROLS = Locus::Key_C;
static const Locus::Key_t KEY_TEXTURIZE = Locus::Key_T;
static const Locus::Key_t KEY_INITIALIZE = Locus::Key_I;
static const Locus::Key_t KEY_LIGHTS = Locus::Key_L;

DemoScene::DemoScene(Locus::SceneManager& sceneManager, unsigned int resolutionX, unsigned int resolutionY)
   : Scene(sceneManager),
     dieOnNextFrame(false),
     maxLights(1),
     notTexturedNotLitProgramID(Locus::BAD_ID),
     texturedNotLitProgramID(Locus::BAD_ID),
     resolutionX(resolutionX),
     resolutionY(resolutionY),
     lastMouseX(0),
     lastMouseY(0),
     score(0),
     lives(3),
     level(1),
     crosshairsX(resolutionX/2),
     crosshairsY(resolutionY/2),
     skyBox(SKY_BOX_RADIUS)
{
   ParseSAPFile(Locus::MountedFilePath("data/" + Config::GetModelFile()), asteroidMeshes);

   Load();
}

DemoScene::~DemoScene()
{
   DestroyRenderingState();
}

void DemoScene::Load()
{
   minPlanetDistance = 2 * Config::GetAsteroidsBoundary() * 1.414213562373f + Config::GetMaxPlanetRadius() + 5;
   maxPlanetDistance = minPlanetDistance + 10.0f;
   starDistance = maxPlanetDistance + 50.0f;
   z_far = 4.0f * starDistance;

   LoadRenderingState();
   InitializeRenderingState();
   LoadTextures();

   LoadAudioState();

   Initialize();
}

void DemoScene::Initialize()
{
   InitializeMeshes();
   InitializeStars();
   InitializePlanets();
   InitializeAsteroids();
}

void DemoScene::LoadRenderingState()
{
   //throws an exception if GLSL can't be loaded, or if the requested GLSL version isn't supported
   renderingState = std::make_unique<Locus::RenderingState>(Locus::GLInfo::GLSLVersion::V_110, true);

   textureManager = std::make_unique<MPM::TextureManager>(renderingState->glInfo);

   LoadShaderPrograms();
   LoadLights();
}

void DemoScene::LoadShaderPrograms()
{
   Locus::GLInfo::GLSLVersion activeGLSLVersion = renderingState->shaderController.GetActiveGLSLVersion();

   notTexturedNotLitProgramID = renderingState->shaderController.LoadShaderProgram(activeGLSLVersion, false, 0);

   texturedNotLitProgramID = renderingState->shaderController.LoadShaderProgram(activeGLSLVersion, true, 0);

   hud.SetProgramIDs(notTexturedNotLitProgramID, texturedNotLitProgramID);

   unsigned int lightIndex = 0;

   for (;;)
   {
      try
      {
         Locus::ID_t thisLitProgramID = renderingState->shaderController.LoadShaderProgram(activeGLSLVersion, true, lightIndex + 1);

         litProgramIDs.push_back(thisLitProgramID);
      }
      catch(Locus::ShaderLinkException& shaderLinkException)
      {
         if (lightIndex == 0)
         {
            throw shaderLinkException;
         }
         else
         {
            break;
         }
      }

      ++lightIndex;
   }

   maxLights = lightIndex;
}

void DemoScene::LoadLights()
{
   lights.resize(maxLights);

   Config::LightingOptions lightingOptions = Config::ReadLightingOptions();

   lightColors = lightingOptions.lightColors;

   currentLightColorIndex = 0;

   for (unsigned int lightIndex = 0; lightIndex < maxLights; ++lightIndex)
   {
      lights[lightIndex].diffuseColor.r = 255;
      lights[lightIndex].diffuseColor.g = 0;
      lights[lightIndex].diffuseColor.b = 0;
      lights[lightIndex].diffuseColor.a = 255;

      lights[lightIndex].attenuation = lightingOptions.constantAttenuation;
      lights[lightIndex].linearAttenuation = lightingOptions.linearAttenuation;
      lights[lightIndex].quadraticAttenuation = lightingOptions.quadraticAttenuation;
   }
}

void DemoScene::InitializeRenderingState()
{
   renderingState->SetOpenGLStateToDefault();

   renderingState->transformationStack.SetTransformationMode(Locus::TransformationStack::Projection);
   renderingState->transformationStack.Load( Locus::Transformation::Perspective(FIELD_OF_VIEW, static_cast<float>(resolutionX)/resolutionY, Z_NEAR, z_far) );

   renderingState->transformationStack.SetTransformationMode(Locus::TransformationStack::ModelView);
   renderingState->transformationStack.LoadIdentity();

   renderingState->shaderController.UseProgram(texturedNotLitProgramID);
}

void DemoScene::DestroyRenderingState()
{
   renderingState.reset();
   textureManager.reset();

   planetMesh->DeleteGPUVertexData();
   moonMesh->DeleteGPUVertexData();
   shotMesh->DeleteGPUVertexData();
   skyBox.DeleteGPUVertexData();
   hud.DeleteGPUVertexData();

   stars.DeleteGPUVertexData();

   for (std::unique_ptr<Asteroid>& asteroid : asteroids)
   {
      asteroid->DeleteGPUVertexData();
   }

   for (std::unique_ptr<Shot>& shot : shots)
   {
      shot->DeleteGPUVertexData();
   }
}

void DemoScene::LoadTextures()
{
   textureManager->LoadAllTextures();

   GLuint textureIndex = 0;
   for (std::unique_ptr<Asteroid>& asteroid : asteroids)
   {
      textureIndex = (textureIndex + 1) % static_cast<GLuint>(textureManager->NumAsteroidTextures());
      asteroid->SetTexture( textureManager->GetTexture(MPM::TextureManager::MakeAsteroidTextureName(textureIndex)) );
   }

   for (std::unique_ptr<Planet>& planet : planets)
   {
      planet->SetRandomTextures(*textureManager);
   }
}

void DemoScene::LoadAudioState()
{
   soundState = std::make_unique<Locus::SoundState>();

   shotSoundEffect = std::make_unique<Locus::SoundEffect>();
   shotSoundEffect->Load(Locus::MountedFilePath("audio/shot.ogg"));

   asteroidShotCollisionSoundEffect = std::make_unique<Locus::SoundEffect>();
   asteroidShotCollisionSoundEffect->Load(Locus::MountedFilePath("audio/asteroid_shot_collision.ogg"));

   //player.LoadCollisionSoundEffect("audio/asteroid_ship_collision.ogg");

   //Asteroid::asteroidAsteroidCollsionSoundEffect = std::make_unique<Locus::SoundEffect>();
   //Asteroid::asteroidAsteroidCollsionSoundEffect->Load(Locus::MountedFilePath("audio/asteroid_asteroid_collision.ogg"));
}

void DemoScene::InitializeMeshes()
{
   planetMesh = Locus::MeshUtility::MakeSphere(1.0, 3);
   planetMesh->CreateGPUVertexData();
   planetMesh->UpdateGPUVertexData();

   moonMesh = Locus::MeshUtility::MakeSphere(1.0, 2);
   moonMesh->CreateGPUVertexData();
   moonMesh->UpdateGPUVertexData();

   shotMesh = Locus::MeshUtility::MakeIcosahedron(SHOT_RADIUS);
   shotMesh->gpuVertexDataTransferInfo.sendColors = false;
   shotMesh->gpuVertexDataTransferInfo.sendNormals = false;

   shotMesh->CreateGPUVertexData();
   shotMesh->UpdateGPUVertexData();

   InitializeSkyBoxAndHUD();
}

void DemoScene::InitializeSkyBoxAndHUD()
{
   skyBox.SetTextures( textureManager->GetTexture(MPM::TextureManager::Skybox_Front),
                       textureManager->GetTexture(MPM::TextureManager::Skybox_Back),
                       textureManager->GetTexture(MPM::TextureManager::Skybox_Left),
                       textureManager->GetTexture(MPM::TextureManager::Skybox_Right),
                       textureManager->GetTexture(MPM::TextureManager::Skybox_Up),
                       textureManager->GetTexture(MPM::TextureManager::Skybox_Down) );

   skyBox.CreateGPUVertexData();
   skyBox.UpdateGPUVertexData();

   hud.Initialize(textureManager.get(), Config::GetNumShots(), resolutionX, resolutionY);
   hud.CreateGPUVertexData();
   hud.UpdateGPUVertexData();
}

void DemoScene::InitializeStars()
{
   //randomly place a set amount of stars on the surface of a sphere
   //of radius STAR_DISTANCE centered at the origin

   Locus::Random random;

   std::vector<Locus::Vector3> starPositions(Config::GetNumStars());
   std::vector<Locus::Color> starColors(Config::GetNumStars());

   for (std::size_t i = 0; i < Config::GetNumStars(); ++i)
   {
      float x = 0;
      float y = 0;

      bool goodPair = false;

      while (!goodPair)
      {
         x = static_cast<float>(random.RandomDouble(-starDistance, starDistance));
         y = static_cast<float>(random.RandomDouble(-starDistance, starDistance));
         goodPair = ((x*x + y*y) <= (starDistance*starDistance));
      }

      float z = sqrt(starDistance*starDistance - (x*x + y*y)) - 10;

      if (random.FlipCoin(0.5))
      {
         z = (-1) * z;
      }

      unsigned char r = 255;
      unsigned char g = 255;
      unsigned char b = 255;

      if (random.FlipCoin(0.15))
      {
         r = static_cast<unsigned char>(random.RandomInt(0, 255));
         g = static_cast<unsigned char>(random.RandomInt(0, 255));
         b = static_cast<unsigned char>(random.RandomInt(0, 255));
      }

      starPositions.push_back( Locus::Vector3(x, y, z) );
      starColors.push_back( Locus::Color(r, g, b, 255) );
   }

   stars.Set(starPositions, starColors);
   stars.CreateGPUVertexData();
   stars.UpdateGPUVertexData();
}

void DemoScene::InitializePlanets()
{
   //randomly place a certain amount of planets (between MIN_PLANETS and MAX_PLANETS) a certain distance
   //away (between MIN_PLANET_DISTANCE and MAX_PLANET_DISTANCE) from the origin.  Also place a certain
   //amount of moons to orbit the planet (between MIN_MOONS and MAX_MOONS) at a random location MOON_ORBIT_DISTANCE
   //from the planet and give each a random angular velocity.

   Locus::Random r;

   int maxTries = 10;

   int numPlanets = r.RandomInt(Config::GetMinPlanets(), Config::GetMaxPlanets());

   for (int i = 0; i < numPlanets; ++i)
   {
      Locus::Vector3 planetLocation;
      bool goodLocation = false;
      bool goodPair = false;

      int numTries = 0;

      while (!goodLocation)
      {
         goodLocation = true;

         float distance = static_cast<float>(r.RandomDouble(minPlanetDistance, maxPlanetDistance));

         while (!goodPair)
         {
            planetLocation.x = static_cast<float>(r.RandomDouble(-distance, distance));
            planetLocation.y = static_cast<float>(r.RandomDouble(-distance, distance));
            goodPair = ((planetLocation.x * planetLocation.x + planetLocation.y * planetLocation.y) <= (distance*distance));
         }

         planetLocation.z = sqrt(distance*distance - (planetLocation.x * planetLocation.x + planetLocation.y * planetLocation.y)) - 5;

         if (r.FlipCoin(0.5))
         {
            planetLocation.z = (-1) * planetLocation.z;
         }

         if (numTries < maxTries)
         {
            //planets shouldn't be touching each other or each other's moons
            for (const std::unique_ptr<Planet>& planet : planets)
            {
               if (planet->Position().distanceTo(planetLocation) <= (planet->getRadius() + MOON_ORBIT_DISTANCE + PLANET_SPACING_THRESHOLD))
               {
                  goodLocation = false;
                  ++numTries;
                  break;
               }
            }
         }
      }

      int planetTextureIndex = r.RandomInt(0, static_cast<int>(textureManager->NumPlanetTextures()) - 1);
      float planetRadius = static_cast<float>(r.RandomDouble(Config::GetMinPlanetRadius(), Config::GetMaxPlanetRadius()));

      std::unique_ptr<Planet> planet( std::make_unique<Planet>(planetRadius, planetTextureIndex, planetMesh.get()) );
      planet->Translate(planetLocation);
      planet->Scale( Locus::Vector3(planetRadius, planetRadius, planetRadius) );

      int numMoons = r.RandomInt(Config::GetMinMoons(), Config::GetMaxMoons());

      float moonDistance = planetRadius + MOON_ORBIT_DISTANCE;

      for (int j = 0; j < numMoons; ++j)
      {
         int moonTextureIndex = r.RandomInt(0, static_cast<int>(textureManager->NumMoonTextures()) - 1);

         float rotationSpeed = static_cast<float>( r.RandomDouble(Config::GetMinMoonOrbitalSpeed(), Config::GetMaxMoonOrbitalSpeed()) );

         planet->addMoon( std::make_unique<Moon>(moonTextureIndex, MOON_RADIUS, moonDistance, rotationSpeed, moonMesh.get()) );
      }

      planets.push_back(std::move(planet));
   }
}

void DemoScene::InitializeAsteroids()
{
   for (std::unique_ptr<Asteroid>& asteroid : asteroids)
   {
      asteroid->DeleteGPUVertexData();
   }

   asteroids.clear();

   collisionManager.Clear();

   shots.clear();

   //////////////////////////////////////////////////////////////////////

   Locus::Random r;

   GLuint textureIndex = 0;

   asteroids.resize(Config::GetNumAsteroids());

   std::size_t numAsteroidMeshes = asteroidMeshes.size();

   std::vector<Asteroid> asteroidTemplates(numAsteroidMeshes);
   for (std::size_t asteroidTemplateIndex = 0; asteroidTemplateIndex < numAsteroidMeshes; ++asteroidTemplateIndex)
   {
      asteroidTemplates[asteroidTemplateIndex].GrabMesh(*asteroidMeshes[asteroidTemplateIndex]);
      asteroidTemplates[asteroidTemplateIndex].CreateBoundingVolumeHierarchy();
   }

   std::size_t numAsteroidTemplates = asteroidTemplates.size();

   collisionManager.StartAddRemoveBatch();

   player.SetModel(Config::GetPlayerCollisionRadius());
   collisionManager.Add(&player);

   float minAsteroidDistance = 0.0f;
   float maxAsteroidDistance = Config::GetAsteroidsBoundary() - 5.0f;

   std::size_t whichMesh = 0;

   for (int i = 0; i < Config::GetNumAsteroids(); ++i)
   {
      //get asteroid type
      //std::size_t whichMesh = r.randomInt(0, numAsteroidTemplates - 1);

      asteroids[i] = std::make_unique<Asteroid>(MAX_ASTEROID_HITS);
      asteroids[i]->GrabMeshAndCollidable(asteroidTemplates[whichMesh]);

      whichMesh = (whichMesh + 1) % numAsteroidTemplates;

      //randomize direction
      float xDirection = static_cast<float>(r.RandomDouble(-1, 1));
      float yDirection = static_cast<float>(r.RandomDouble(-1, 1));
      float zDirection = static_cast<float>(r.RandomDouble(-1, 1));

      asteroids[i]->motionProperties.direction.set(xDirection, yDirection, zDirection);
      asteroids[i]->motionProperties.direction.normalize();

      //randomize speed
      asteroids[i]->motionProperties.speed = static_cast<float>(r.RandomDouble(Config::GetMinAsteroidSpeed(), Config::GetMaxAsteroidSpeed()));

      //randomize rotation direction
      xDirection = static_cast<float>(r.RandomDouble(-1, 1));
      yDirection = static_cast<float>(r.RandomDouble(-1, 1));
      zDirection = static_cast<float>(r.RandomDouble(-1, 1));

      asteroids[i]->motionProperties.rotation.set(xDirection, yDirection, zDirection);
      
      //randomize rotation speed
      asteroids[i]->motionProperties.angularSpeed = static_cast<float>(r.RandomDouble(Config::GetMinAsteroidRotationSpeed(), Config::GetMaxAsteroidRotationSpeed()));

      //randomize size
      float scale = static_cast<float>(r.RandomDouble(MIN_ASTEROID_SCALE, MAX_ASTEROID_SCALE));
      asteroids[i]->Scale( Locus::Vector3(scale, scale, scale) );

      //randomize position (centroid)
      Locus::Vector3 asteroidPosition;
      bool goodLocation = false;

      while (!goodLocation)
      {
         goodLocation = true;

         bool goodPoint = false;

         while (!goodPoint)
         {
            asteroidPosition.x = static_cast<float>(r.RandomDouble(-maxAsteroidDistance, maxAsteroidDistance));
            asteroidPosition.y = static_cast<float>(r.RandomDouble(-maxAsteroidDistance, maxAsteroidDistance));
            asteroidPosition.z = static_cast<float>(r.RandomDouble(-maxAsteroidDistance, maxAsteroidDistance));

            goodPoint = asteroidPosition.distanceTo(Locus::Vector3::ZeroVector()) >= minAsteroidDistance;
         }

         for (int previousAsteroidIndex = 0; previousAsteroidIndex < i; ++previousAsteroidIndex)
         {
            if (asteroids[previousAsteroidIndex]->Position().distanceTo(asteroidPosition) <= ASTEROID_SPACING_THRESHOLD)
            {
               goodLocation = false;
               break;
            }
         }
      } 

      asteroids[i]->Translate(asteroidPosition);

      textureIndex = (textureIndex + 1) % static_cast<GLuint>(textureManager->NumAsteroidTextures());
      asteroids[i]->SetTexture( textureManager->GetTexture(MPM::TextureManager::MakeAsteroidTextureName(textureIndex)) );

      asteroids[i]->CreateGPUVertexData();
      asteroids[i]->UpdateGPUVertexData();
      asteroids[i]->UpdateMaxDistanceToCenter();
      asteroids[i]->UpdateBroadCollisionExtent();

      collisionManager.Add(asteroids[i].get());
   }

   collisionManager.FinishAddRemoveBatch();
}

void DemoScene::ShotFired()
{
   if (shots.size() < Config::GetNumShots())
   {
      std::unique_ptr<Shot> shot( std::make_unique<Shot>(player.viewpoint.GetForward(), player.viewpoint.GetPosition() + player.viewpoint.GetForward(), shotMesh.get()) );
      shot->UpdateBroadCollisionExtent();

      std::size_t numLightColors = lightColors.size();

      shot->color = lightColors[currentLightColorIndex];

      currentLightColorIndex = (currentLightColorIndex + 1) % numLightColors;

      collisionManager.Add(shot.get());

      shot->CreateGPUVertexData();
      shot->UpdateGPUVertexData();

      shots.push_back( std::move(shot) );

      shotSoundEffect->Play();
   }
}

void DemoScene::UpdateShotPositions(double DT)
{
   std::size_t numShots = shots.size();

   std::stack<std::size_t> shotsToRemove;

   float boundary = Config::GetAsteroidsBoundary();

   float shotDistance = Locus::Vector3(boundary, boundary, boundary).norm();

   //check if shots go beyond the boundary. If they do, remove them
   for (std::size_t shotIndex = 0; shotIndex < numShots; ++shotIndex)
   {
      if (shots[shotIndex]->IsValid())
      {
         shots[shotIndex]->MoveAlongDirection(static_cast<float>(DT) * Config::GetShotSpeed());

         if (shots[shotIndex]->GetPosition().distanceTo(Locus::Vector3::ZeroVector()) >= shotDistance)
         {
            shotsToRemove.push(shotIndex);
         }
         else
         {
            shots[shotIndex]->UpdateBroadCollisionExtent();

            collisionManager.Update(shots[shotIndex].get());
         }
      }
      else
      {
         shotsToRemove.push(shotIndex);
      }
   }

   if (!shotsToRemove.empty())
   {
      collisionManager.StartAddRemoveBatch();

      //remove the shots in backwards order as this is more efficient for vectors
      do
      {
         std::size_t indexOfShotToRemove = shotsToRemove.top();
         shotsToRemove.pop();

         collisionManager.Remove(shots[indexOfShotToRemove].get());

         shots.erase(shots.begin() + indexOfShotToRemove);

      } while (!shotsToRemove.empty());

      collisionManager.FinishAddRemoveBatch();
   }
}

Locus::Plane DemoScene::MakeHalfSplitPlane(const Locus::Vector3& shotPosition, const Locus::Vector3& asteroidCentroid)
{
   Locus::Plane orthogonalPlane(asteroidCentroid, player.viewpoint.GetForward());
   Locus::Vector3 shotProjection = orthogonalPlane.getProjection(shotPosition);

   Locus::Vector3 normal = shotProjection - asteroidCentroid;
   normal.rotateAround(-player.viewpoint.GetForward(), Locus::PI/2);

   return Locus::Plane(asteroidCentroid, normal);
}

void DemoScene::SplitAsteroid(std::size_t splitIndex, const Locus::Vector3& shotPosition)
{
   //split an asteroid in two. If it has no more hits left,
   //simply remove the asteroid from the game

   ++score;

   asteroids[splitIndex]->decreaseHitsLeft();

   int hitsLeft = asteroids[splitIndex]->getHitsLeft();

   if (hitsLeft > 0)
   {
      std::unique_ptr<Asteroid> splitAsteroid1( std::make_unique<Asteroid>(hitsLeft) );
      std::unique_ptr<Asteroid> splitAsteroid2( std::make_unique<Asteroid>(hitsLeft) );

      Locus::Random random;

      float xRotationDirection = static_cast<float>( random.RandomDouble(-1, 1) );
      float yRotationDirection = static_cast<float>( random.RandomDouble(-1, 1) );
      float zRotationDirection = static_cast<float>( random.RandomDouble(-1, 1) );

      splitAsteroid1->motionProperties.rotation.set(xRotationDirection, yRotationDirection, zRotationDirection);
      splitAsteroid2->motionProperties.rotation.set(xRotationDirection, yRotationDirection, zRotationDirection);

      std::unique_ptr<Asteroid>& asteroidToSplit = asteroids[splitIndex];

      splitAsteroid1->motionProperties.angularSpeed = asteroidToSplit->motionProperties.angularSpeed;
      splitAsteroid2->motionProperties.angularSpeed = asteroidToSplit->motionProperties.angularSpeed;

      splitAsteroid1->SetTexture(asteroidToSplit->GetTexture());
      splitAsteroid2->SetTexture(asteroidToSplit->GetTexture());

      Locus::Plane splitPlane = MakeHalfSplitPlane(shotPosition, asteroidToSplit->Position());

      asteroidToSplit->DetermineSplit(splitPlane, asteroidToSplit->CurrentModelTransformation(), *splitAsteroid1, *splitAsteroid2);

      if ((splitAsteroid1->NumFaces() > 0) && (splitAsteroid2->NumFaces() > 0))
      {
         splitAsteroid1->Reset(splitAsteroid1->centroid);
         splitAsteroid2->Reset(splitAsteroid2->centroid);

         splitAsteroid1->motionProperties.speed = asteroidToSplit->motionProperties.speed;
         splitAsteroid2->motionProperties.speed = asteroidToSplit->motionProperties.speed;

         splitAsteroid1->motionProperties.direction = splitPlane.getNormal();
         splitAsteroid1->motionProperties.direction.normalize();

         splitAsteroid2->motionProperties.direction = -(splitAsteroid1->motionProperties.direction);

         //avoiding immediate interpenetration
         splitAsteroid1->lastCollision = splitAsteroid2.get();
         splitAsteroid2->lastCollision = splitAsteroid1.get();

         splitAsteroid1->lastCollisionTime = splitAsteroid2->lastCollisionTime = std::chrono::high_resolution_clock::now();

         splitAsteroid1->AssignNormals();
         splitAsteroid2->AssignNormals();

         splitAsteroid1->CreateGPUVertexData();
         splitAsteroid1->UpdateGPUVertexData();
         splitAsteroid1->UpdateMaxDistanceToCenter();
         splitAsteroid1->UpdateBroadCollisionExtent();
         splitAsteroid1->CreateBoundingVolumeHierarchy();

         splitAsteroid2->CreateGPUVertexData();
         splitAsteroid2->UpdateGPUVertexData();
         splitAsteroid2->UpdateMaxDistanceToCenter();
         splitAsteroid2->UpdateBroadCollisionExtent();
         splitAsteroid2->CreateBoundingVolumeHierarchy();

         collisionManager.Add(splitAsteroid1.get());
         collisionManager.Add(splitAsteroid2.get());

         asteroids.emplace_back( std::move(splitAsteroid1) );
         asteroids.emplace_back( std::move(splitAsteroid2) );
      }
   }

   asteroids[splitIndex]->DeleteGPUVertexData();

   collisionManager.Remove(asteroids[splitIndex].get());

   asteroids.erase(asteroids.begin() + splitIndex);
}

//////////////////////////////////////Events//////////////////////////////////////////

void DemoScene::KeyPressed(Locus::Key_t key)
{
   switch (key)
   {
      case KEY_TEXTURIZE:
         LoadTextures();
         break;

      case KEY_INITIALIZE:
         InitializeAsteroids();
         break;

      case KEY_LIGHTS:
         LoadLights();
         break;

      case KEY_PAUSE:
         sceneManager.AddScene( std::make_unique<PauseScene>(sceneManager, *this, KEY_PAUSE) );
         break;

      case Locus::Key_ESCAPE:
         DestroyRenderingState();
         dieOnNextFrame = true;
         break;

      case KEY_FORWARD:
         player.translateAhead = true;
         break;

      case KEY_BACKWARD:
         player.translateBack = true;
         break;

      case KEY_LEFT:
         player.translateLeft = true;
         break;

      case KEY_RIGHT:
         player.translateRight = true;
         break;

      case KEY_UP:
         player.translateUp = true;
         break;

      case KEY_DOWN:
         player.translateDown = true;
         break;
   }
}

void DemoScene::KeyReleased(Locus::Key_t key)
{
   switch (key)
   {
      case KEY_FORWARD:
         player.translateAhead = false;
         break;

      case KEY_BACKWARD:
         player.translateBack = false;
         break;

      case KEY_LEFT:
         player.translateLeft = false;
         break;

      case KEY_RIGHT:
         player.translateRight = false;
         break;

      case KEY_UP:
         player.translateUp = false;
         break;

      case KEY_DOWN:
         player.translateDown = false;
         break;
   }
}

void DemoScene::MousePressed(Locus::MouseButton_t button)
{
   if (button == Locus::Mouse_Button_Left)
   {
      ShotFired();
   }
}

void DemoScene::MouseMoved(int x, int y)
{
   int diffX = x - lastMouseX;
   int diffY = y - lastMouseY;

   Locus::Vector3 difference(static_cast<float>(diffX), static_cast<float>(diffY), 0.0f);

   Locus::Vector3 rotation((-difference.y/resolutionY) * FIELD_OF_VIEW * Locus::TO_RADIANS, (-difference.x/resolutionX)* FIELD_OF_VIEW * Locus::TO_RADIANS, 0.0f);

   player.Rotate(rotation);

   sceneManager.CenterMouse();

   UpdateLastMousePosition();
}

void DemoScene::Resized(int width, int height)
{
   glViewport(0, 0, width, height);

   hud.Initialize(textureManager.get(), Config::GetNumShots(), width, height);
   hud.UpdateGPUVertexData();

   resolutionX = width;
   resolutionY = height;

   crosshairsX = (resolutionX / 2);
   crosshairsY = (resolutionY / 2);

   hud.Update(score, level, lives, shots.size(), crosshairsX, crosshairsY, 1);
}

void DemoScene::Activate()
{
   sceneManager.MakeFullScreen();
   sceneManager.HideMouse();
   sceneManager.CenterMouse();

   UpdateLastMousePosition();
}

void DemoScene::UpdateLastMousePosition()
{
   sceneManager.GetMousePosition(lastMouseX, lastMouseY);
}

bool DemoScene::Update(double DT)
{
   if (dieOnNextFrame)
   {
      return false;
   }

   player.tick(DT);
   collisionManager.Update(&player);

   TickAsteroids(DT);
   UpdateShotPositions(DT);

   for (std::unique_ptr<Planet>& planet : planets)
   {
      planet->tick(DT);
   }

   collisionManager.UpdateCollisions();
   collisionManager.TransmitCollisions();

   CheckForAsteroidHits();

   hud.Update(score, level, lives, shots.size(), crosshairsX, crosshairsY, static_cast<int>(1 / DT));

   return true;
}

void DemoScene::TickAsteroids(double DT)
{
   //this function updates all asteroids' positions. If an asteroid is
   //about to go beyond the asteroid boundary, it bounces off the side.
   //This function also updates the visible asteroids and checks and responds 
   //to asteroid-to-asteroid collisions

   //update asteroid positions
   for (std::unique_ptr<Asteroid>& asteroid : asteroids)
   {
      Locus::Vector3 nextPosition = asteroid->Position() + ((asteroid->motionProperties.speed * asteroid->motionProperties.direction) * static_cast<float>(DT));

      if (abs(nextPosition.x) >= Config::GetAsteroidsBoundary())
      {
         asteroid->negateXDirection();
      }
      if (abs(nextPosition.y) >= Config::GetAsteroidsBoundary())
      {
         asteroid->negateYDirection();
      }
      if (abs(nextPosition.z) >= Config::GetAsteroidsBoundary())
      {
         asteroid->negateZDirection();
      }

      asteroid->tick(DT);
      asteroid->UpdateBroadCollisionExtent();

      collisionManager.Update(asteroid.get());
   }

   //update asteroid visibility with camera frustum

   Locus::Vector3 forward = player.viewpoint.GetForward();
   Locus::Vector3 up = player.viewpoint.GetUp();

   Locus::Vector3 point = player.viewpoint.GetPosition();

   //TODO: Fix fudging going on here
   Locus::Frustum viewFrustum(point, forward, up, 2.5f * FIELD_OF_VIEW * (static_cast<float>(resolutionX)/resolutionY), FRUSTUM_VERTICAL_FIELD_OF_VIEW, FRUSTUM_NEAR_DISTANCE, starDistance * 4.0f);

   for (std::unique_ptr<Asteroid>& asteroid : asteroids)
   {
      asteroid->visible = viewFrustum.Within(asteroid->Position(), asteroid->GetMaxDistanceToCenter());
   }
}

void DemoScene::CheckForAsteroidHits()
{
   bool hadAnyHits = false;

   for (int asteroidIndex = static_cast<int>(asteroids.size() - 1); asteroidIndex >= 0; --asteroidIndex)
   {
      if (asteroids[asteroidIndex]->WasHit())
      {
         if (!hadAnyHits)
         {
            collisionManager.StartAddRemoveBatch();
         }

         SplitAsteroid(asteroidIndex, asteroids[asteroidIndex]->GetHitLocation());

         hadAnyHits = true;
      }
   }

   if (hadAnyHits)
   {
      asteroidShotCollisionSoundEffect->Play();
      collisionManager.FinishAddRemoveBatch();
   }
}

void DemoScene::Draw()
{
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   DrawSkyBox();
   DrawStars();
   DrawPlanets();

   DrawAsteroids();
   DrawShots();

   DrawHUD();
}

void DemoScene::DrawSkyBox()
{
   //the sky box is a cube of length 2*SKY_BOX_RADIUS centered at
   //the camera. In the asteroids game, it shows a bunch of super novae

   glDisable(GL_CULL_FACE);
   glDisable(GL_DEPTH_TEST);

   player.viewpoint.Activate(renderingState->transformationStack);

   renderingState->transformationStack.Translate(player.viewpoint.GetPosition());
   renderingState->UploadTransformations();

   skyBox.Draw(*renderingState);

   player.viewpoint.Deactivate(renderingState->transformationStack);

   glEnable(GL_CULL_FACE);
   glEnable(GL_DEPTH_TEST);
}

void DemoScene::DrawStars()
{
   renderingState->shaderController.UseProgram(notTexturedNotLitProgramID);

   glDisable(GL_DEPTH_TEST);

   player.viewpoint.Activate(renderingState->transformationStack);

   renderingState->transformationStack.Translate(player.viewpoint.GetPosition());
   renderingState->UploadTransformations();

   stars.Draw(*renderingState);

   player.viewpoint.Deactivate(renderingState->transformationStack);

   renderingState->shaderController.UseProgram(texturedNotLitProgramID);

   glEnable(GL_DEPTH_TEST);
}

void DemoScene::DrawPlanets()
{
   for (std::unique_ptr<Planet>& planet : planets)
   {
      textureManager->GetTexture(MPM::TextureManager::MakePlanetTextureName(planet->textureIndex))->Bind();
      renderingState->shaderController.SetTextureUniform(Locus::ShaderSource::Map_Diffuse, 0);

      player.viewpoint.Activate(renderingState->transformationStack);

         renderingState->transformationStack.Translate(player.viewpoint.GetPosition());
         planet->Draw(*renderingState, *textureManager);

      player.viewpoint.Deactivate(renderingState->transformationStack);
   }
}

void DemoScene::DrawAsteroids()
{
   bool shaderChanged = false;

   struct ShotPositionAndDistance
   {
      const Shot* shot;
      Locus::Vector3 position;
      float squaredDistance;

      bool operator <(const ShotPositionAndDistance& other) const
      {
         return squaredDistance < other.squaredDistance;
      }
   };

   ShotPositionAndDistance singleShotPositionAndDistance;

   std::vector<ShotPositionAndDistance> shotPositionsAndDistances;
   shotPositionsAndDistances.reserve(shots.size());

   for (const std::unique_ptr<Shot>& shot : shots)
   {
      if (shot->IsValid())
      {
         singleShotPositionAndDistance.shot = shot.get();
         singleShotPositionAndDistance.position = shot->GetPosition();
         singleShotPositionAndDistance.squaredDistance = (singleShotPositionAndDistance.position - player.viewpoint.GetPosition()).squaredNorm();

         shotPositionsAndDistances.push_back(singleShotPositionAndDistance);
      }
   }

   std::size_t numShotsAndPositions = shotPositionsAndDistances.size();
   unsigned int numShotPositionsToUse = 0;

   if (numShotsAndPositions > 0)
   {
      std::sort(shotPositionsAndDistances.begin(), shotPositionsAndDistances.end());

      numShotPositionsToUse = (numShotsAndPositions > maxLights) ? maxLights : numShotsAndPositions;

      renderingState->shaderController.UseProgram(litProgramIDs[numShotPositionsToUse - 1]);

      for (unsigned int shotPositionIndex = 0; shotPositionIndex < numShotPositionsToUse; ++shotPositionIndex)
      {
         lights[shotPositionIndex].eyePosition = player.viewpoint.ToEyePosition(shotPositionsAndDistances[shotPositionIndex].position);
         lights[shotPositionIndex].diffuseColor = shotPositionsAndDistances[shotPositionIndex].shot->color;

         renderingState->shaderController.SetLightUniforms(shotPositionIndex, lights[shotPositionIndex]);
      }

      shaderChanged = true;
   }

   renderingState->shaderController.SetTextureUniform(Locus::ShaderSource::Map_Diffuse, 0);

   player.viewpoint.Activate(renderingState->transformationStack);

   for (const std::unique_ptr<Asteroid>& asteroid : asteroids)
   {
      if (asteroid->visible)
      {
         asteroid->GetTexture()->Bind();

         renderingState->transformationStack.UploadTransformations(renderingState->shaderController, asteroid->CurrentModelTransformation());
         asteroid->Draw(*renderingState);
      }
   }

   player.viewpoint.Deactivate(renderingState->transformationStack);

   if (shaderChanged)
   {
      renderingState->shaderController.UseProgram(texturedNotLitProgramID);
   }
}

void DemoScene::DrawShots()
{
   textureManager->GetTexture(MPM::TextureManager::Shot_TextureName)->Bind();

   renderingState->shaderController.SetTextureUniform(Locus::ShaderSource::Map_Diffuse, 0);

   for (const std::unique_ptr<Shot>& shot : shots)
   {
      if (shot->IsValid())
      {
         player.viewpoint.Activate(renderingState->transformationStack);

            renderingState->transformationStack.Translate(shot->GetPosition());
            renderingState->UploadTransformations();

            shot->Draw(*renderingState);

         player.viewpoint.Deactivate(renderingState->transformationStack);
      }
   }
}

void DemoScene::DrawHUD()
{
   Locus::DrawUtility::BeginDrawing2D(*renderingState, resolutionX, resolutionY);

   hud.Draw(*renderingState);

   Locus::DrawUtility::EndDrawing2D(*renderingState);
}

}