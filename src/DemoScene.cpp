 /*********************************************************************************************************\
 *                                                                                                        *
 *   This file is part of Minor Planet Mayhem                                                             *
 *                                                                                                        *
 *   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
 *                                                                                                        *
 *   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
 *                                                                                                        *
 \*********************************************************************************************************/

#include "DemoScene.h"
#include "Config.h"
#include "Moon.h"
#include "Asteroid.h"
#include "ShaderNames.h"
#include "Shot.h"
#include "Planet.h"
#include "PauseScene.h"

#include "Locus/Common/Random.h"
#include "Locus/FileSystem/FileSystemUtil.h"

#include "Locus/Geometry/Geometry.h"
#include "Locus/Geometry/Line.h"
#include "Locus/Geometry/Frustum.h"

#include "Locus/Rendering/SAPReading.h"
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

#include <Locus/Rendering/Locus_glew.h>

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
   : Scene(sceneManager), dieOnNextFrame(false), maxLights(1), resolutionX(resolutionX), resolutionY(resolutionY),
     score(0), lives(3), level(1), crosshairsX(resolutionX/2), crosshairsY(resolutionY/2), skyBox(SKY_BOX_RADIUS)
{
   if (!Locus::ParseSAPFile(Locus::MountedFilePath("data/" + Config::GetModelFile()), asteroidMeshes))
   {
      throw std::runtime_error("Failed to load models");
   }

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
   initializeMeshes();
   initializeStars();
   initializePlanets();
   initializeAsteroids();
}

void DemoScene::LoadRenderingState()
{
   //throws an exception if GLSL can't be loaded, or if the requested GLSL version isn't supported
   renderingState.reset( new Locus::RenderingState(Locus::GLInfo::GLSLVersion::V_110, true) );

   textureManager.reset( new MPM::TextureManager(renderingState->glInfo) );

   LoadShaderPrograms();
   LoadLights();
}

void DemoScene::LoadShaderPrograms()
{
   Locus::GLInfo::GLSLVersion activeGLSLVersion = renderingState->shaderController.GetActiveGLSLVersion();

   std::vector<std::string> basicAttributes(2);
   basicAttributes[0] = Locus::ShaderSource::Color;
   basicAttributes[1] = Locus::ShaderSource::Vert_Pos;

   std::vector<std::string> basicUniforms(1);
   basicUniforms[0] = Locus::ShaderSource::Mat_MVP;

   renderingState->shaderController.LoadShaderProgram
   (
     ShaderNames::NotTexturedNotLit,
     Locus::Shader(Locus::Shader::ShaderType::Vertex, Locus::ShaderSource::Vert(activeGLSLVersion, false, 0)),
     Locus::Shader(Locus::Shader::ShaderType::Fragment, Locus::ShaderSource::Frag(activeGLSLVersion, false, 0)),
     false,
     false,
     basicAttributes,
     basicUniforms
   );

   std::vector<std::string> texturedAttributes(3);
   texturedAttributes[0] = Locus::ShaderSource::Color;
   texturedAttributes[1] = Locus::ShaderSource::Vert_Pos;
   texturedAttributes[2] = Locus::ShaderSource::Vert_Tex;

   std::vector<std::string> texturedUniforms(2);
   texturedUniforms[0] = Locus::ShaderSource::Mat_MVP;
   texturedUniforms[1] = Locus::ShaderSource::Map_Diffuse;

   renderingState->shaderController.LoadShaderProgram
   (
     ShaderNames::TexturedNotLit,
     Locus::Shader(Locus::Shader::ShaderType::Vertex, Locus::ShaderSource::Vert(activeGLSLVersion, true, 0)),
     Locus::Shader(Locus::Shader::ShaderType::Fragment, Locus::ShaderSource::Frag(activeGLSLVersion, true, 0)),
     true,
     false,
     texturedAttributes,
     texturedUniforms
   );

   std::vector<std::string> litAndTexturedAttributes(4);
   litAndTexturedAttributes[0] = Locus::ShaderSource::Color;
   litAndTexturedAttributes[1] = Locus::ShaderSource::Vert_Pos;
   litAndTexturedAttributes[2] = Locus::ShaderSource::Vert_Tex;
   litAndTexturedAttributes[3] = Locus::ShaderSource::Vert_Normal;

   static const int Max_Lights_Guess = 8;

   std::vector<std::string> litAndTexturedUniforms;
   litAndTexturedUniforms.reserve(4 + Max_Lights_Guess * 5);

   litAndTexturedUniforms.push_back(Locus::ShaderSource::Mat_MVP);
   litAndTexturedUniforms.push_back(Locus::ShaderSource::Map_Diffuse);
   litAndTexturedUniforms.push_back(Locus::ShaderSource::Mat_MV);
   litAndTexturedUniforms.push_back(Locus::ShaderSource::Mat_Normal);

   unsigned int lightIndex = 0;

   for (;;)
   {
      try
      {
         litAndTexturedUniforms.push_back( Locus::ShaderSource::GetMultiVariableName(Locus::ShaderSource::Light_EyePos, lightIndex) );
         litAndTexturedUniforms.push_back( Locus::ShaderSource::GetMultiVariableName(Locus::ShaderSource::Light_Attenuation, lightIndex) );
         litAndTexturedUniforms.push_back( Locus::ShaderSource::GetMultiVariableName(Locus::ShaderSource::Light_LinearAttenuation, lightIndex) );
         litAndTexturedUniforms.push_back( Locus::ShaderSource::GetMultiVariableName(Locus::ShaderSource::Light_QuadraticAttenuation, lightIndex) );
         litAndTexturedUniforms.push_back( Locus::ShaderSource::GetMultiVariableName(Locus::ShaderSource::Light_Diffuse, lightIndex) );

         renderingState->shaderController.LoadShaderProgram
         (
           ShaderNames::TexturedAndLit_1 + lightIndex,
           Locus::Shader(Locus::Shader::ShaderType::Vertex, Locus::ShaderSource::Vert(activeGLSLVersion, true, lightIndex + 1)),
           Locus::Shader(Locus::Shader::ShaderType::Fragment, Locus::ShaderSource::Frag(activeGLSLVersion, true, lightIndex + 1)),
           true,
           true,
           litAndTexturedAttributes,
           litAndTexturedUniforms
         );
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

   renderingState->shaderController.UseProgram(ShaderNames::TexturedNotLit);
}

void DemoScene::DestroyRenderingState()
{
   renderingState.reset();
   textureManager.reset();

   planetMesh.DeleteGPUVertexData();
   moonMesh.DeleteGPUVertexData();
   shotMesh.DeleteGPUVertexData();
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
   soundState.reset(new Locus::SoundState());

   shotSoundEffect.reset(new Locus::SoundEffect());
   shotSoundEffect->Load(Locus::MountedFilePath("audio/shot.ogg"));

   asteroidShotCollisionSoundEffect.reset(new Locus::SoundEffect());
   asteroidShotCollisionSoundEffect->Load(Locus::MountedFilePath("audio/asteroid_shot_collision.ogg"));

   //player.LoadCollisionSoundEffect("audio/asteroid_ship_collision.ogg");

   //Asteroid::asteroidAsteroidCollsionSoundEffect.reset(new Locus::SoundEffect());
   //Asteroid::asteroidAsteroidCollsionSoundEffect->Load(Locus::MountedFilePath("audio/asteroid_asteroid_collision.ogg"));
}

void DemoScene::initializeMeshes()
{
   planetMesh = Locus::MeshUtility::MakeSphere(1.0, 3);
   planetMesh.CreateGPUVertexData();
   planetMesh.UpdateGPUVertexData();

   moonMesh = Locus::MeshUtility::MakeSphere(1.0, 2);
   moonMesh.CreateGPUVertexData();
   moonMesh.UpdateGPUVertexData();

   shotMesh = Locus::MeshUtility::MakeIcosahedron(SHOT_RADIUS);
   shotMesh.gpuVertexDataTransferInfo.sendColors = false;
   shotMesh.gpuVertexDataTransferInfo.sendNormals = false;

   shotMesh.CreateGPUVertexData();
   shotMesh.UpdateGPUVertexData();

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

void DemoScene::initializeStars()
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
         x = static_cast<float>(random.randomDouble(-starDistance, starDistance));
         y = static_cast<float>(random.randomDouble(-starDistance, starDistance));
         goodPair = ((x*x + y*y) <= (starDistance*starDistance));
      }

      float z = sqrt(starDistance*starDistance - (x*x + y*y)) - 10;

      if (random.flipCoin(0.5))
      {
         z = (-1) * z;
      }

      unsigned char r = 255;
      unsigned char g = 255;
      unsigned char b = 255;

      if (random.flipCoin(0.15))
      {
         r = static_cast<unsigned char>(random.randomInt(0, 255));
         g = static_cast<unsigned char>(random.randomInt(0, 255));
         b = static_cast<unsigned char>(random.randomInt(0, 255));
      }

      starPositions.push_back( Locus::Vector3(x, y, z) );
      starColors.push_back( Locus::Color(r, g, b, 255) );
   }

   stars.Set(starPositions, starColors);
   stars.CreateGPUVertexData();
   stars.UpdateGPUVertexData();
}

void DemoScene::initializePlanets()
{
   //randomly place a certain amount of planets (between MIN_PLANETS and MAX_PLANETS) a certain distance
   //away (between MIN_PLANET_DISTANCE and MAX_PLANET_DISTANCE) from the origin.  Also place a certain
   //amount of moons to orbit the planet (between MIN_MOONS and MAX_MOONS) at a random location MOON_ORBIT_DISTANCE
   //from the planet and give each a random angular velocity.

   Locus::Random r;

   int maxTries = 10;

   int numPlanets = r.randomInt(Config::GetMinPlanets(), Config::GetMaxPlanets());

   for (int i = 0; i < numPlanets; ++i)
   {
      Locus::Vector3 planetLocation;
      bool goodLocation = false;
      bool goodPair = false;

      int numTries = 0;

      while (!goodLocation)
      {
         goodLocation = true;

         float distance = static_cast<float>(r.randomDouble(minPlanetDistance, maxPlanetDistance));

         while (!goodPair)
         {
            planetLocation.x = static_cast<float>(r.randomDouble(-distance, distance));
            planetLocation.y = static_cast<float>(r.randomDouble(-distance, distance));
            goodPair = ((planetLocation.x * planetLocation.x + planetLocation.y * planetLocation.y) <= (distance*distance));
         }

         planetLocation.z = sqrt(distance*distance - (planetLocation.x * planetLocation.x + planetLocation.y * planetLocation.y)) - 5;

         if (r.flipCoin(0.5))
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

      int planetTextureIndex = r.randomInt(0, static_cast<int>(textureManager->NumPlanetTextures()) - 1);
      float planetRadius = static_cast<float>(r.randomDouble(Config::GetMinPlanetRadius(), Config::GetMaxPlanetRadius()));

      std::unique_ptr<Planet> planet( new Planet(planetRadius, planetTextureIndex, &planetMesh) );
      planet->Translate(planetLocation);
      planet->Scale( Locus::Vector3(planetRadius, planetRadius, planetRadius) );

      int numMoons = r.randomInt(Config::GetMinMoons(), Config::GetMaxMoons());

      float moonDistance = planetRadius + MOON_ORBIT_DISTANCE;

      for (int j = 0; j < numMoons; ++j)
      {
         int moonTextureIndex = r.randomInt(0, static_cast<int>(textureManager->NumMoonTextures()) - 1);

         float rotationSpeed = static_cast<float>( r.randomDouble(Config::GetMinMoonOrbitalSpeed(), Config::GetMaxMoonOrbitalSpeed()) );

         std::unique_ptr<Moon> moon( new Moon(moonTextureIndex, MOON_RADIUS, moonDistance, rotationSpeed, &moonMesh) );

         planet->addMoon(std::move(moon));
      }

      planets.push_back(std::move(planet));
   }
}

void DemoScene::initializeAsteroids()
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
      asteroidTemplates[asteroidTemplateIndex].GrabMesh(asteroidMeshes[asteroidTemplateIndex]);
      asteroidTemplates[asteroidTemplateIndex].CreateBoundingVolumeHierarchy();
   }

   std::size_t numAsteroidTemplates = asteroidTemplates.size();

   collisionManager.StartAddRemoveBatch();

   player.SetModel(Config::GetPlayerCollisionRadius());
   collisionManager.Add(player);

   float minAsteroidDistance = 0.0f;
   float maxAsteroidDistance = Config::GetAsteroidsBoundary() - 5.0f;

   std::size_t whichMesh = 0;

   for (int i = 0; i < Config::GetNumAsteroids(); ++i)
   {
      //get asteroid type
      //std::size_t whichMesh = r.randomInt(0, numAsteroidTemplates - 1);

      asteroids[i].reset( new Asteroid(MAX_ASTEROID_HITS) );
      asteroids[i]->GrabMeshAndCollidable(asteroidTemplates[whichMesh]);

      whichMesh = (whichMesh + 1) % numAsteroidTemplates;

      //randomize direction
      float xDirection = static_cast<float>(r.randomDouble(-1, 1));
      float yDirection = static_cast<float>(r.randomDouble(-1, 1));
      float zDirection = static_cast<float>(r.randomDouble(-1, 1));

      asteroids[i]->motionProperties.direction.set(xDirection, yDirection, zDirection);
      asteroids[i]->motionProperties.direction.normalize();

      //randomize speed
      asteroids[i]->motionProperties.speed = static_cast<float>(r.randomDouble(Config::GetMinAsteroidSpeed(), Config::GetMaxAsteroidSpeed()));

      //randomize rotation direction
      xDirection = static_cast<float>(r.randomDouble(-1, 1));
      yDirection = static_cast<float>(r.randomDouble(-1, 1));
      zDirection = static_cast<float>(r.randomDouble(-1, 1));

      asteroids[i]->motionProperties.rotation.set(xDirection, yDirection, zDirection);
      
      //randomize rotation speed
      asteroids[i]->motionProperties.angularSpeed = static_cast<float>(r.randomDouble(Config::GetMinAsteroidRotationSpeed(), Config::GetMaxAsteroidRotationSpeed()));

      //randomize size
      float scale = static_cast<float>(r.randomDouble(MIN_ASTEROID_SCALE, MAX_ASTEROID_SCALE));
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
            asteroidPosition.x = static_cast<float>(r.randomDouble(-maxAsteroidDistance, maxAsteroidDistance));
            asteroidPosition.y = static_cast<float>(r.randomDouble(-maxAsteroidDistance, maxAsteroidDistance));
            asteroidPosition.z = static_cast<float>(r.randomDouble(-maxAsteroidDistance, maxAsteroidDistance));

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

      collisionManager.Add(*asteroids[i]);
   }

   collisionManager.FinishAddRemoveBatch();
}

void DemoScene::shotFired()
{
   if (shots.size() < Config::GetNumShots())
   {
      std::unique_ptr<Shot> shot( new Shot(player.viewpoint.GetForward(), player.viewpoint.GetPosition() + player.viewpoint.GetForward(), &shotMesh) );
      shot->UpdateBroadCollisionExtent();

      std::size_t numLightColors = lightColors.size();

      if (currentLightColorIndex >= numLightColors)
      {
         currentLightColorIndex = 0;
      }

      shot->color = lightColors[currentLightColorIndex];
      currentLightColorIndex = (currentLightColorIndex + 1) % numLightColors;

      collisionManager.Add(*shot);

      shot->CreateGPUVertexData();
      shot->UpdateGPUVertexData();

      shots.push_back( std::move(shot) );

      shotSoundEffect->Play();
   }
}

void DemoScene::updateShotPositions(double DT)
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

            collisionManager.Update(*shots[shotIndex]);
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

         collisionManager.Remove(*shots[indexOfShotToRemove]);

         shots.erase(shots.begin() + indexOfShotToRemove);

      } while (!shotsToRemove.empty());

      collisionManager.FinishAddRemoveBatch();
   }
}

Locus::Plane DemoScene::makeHalfSplitPlane(const Locus::Vector3& shotPosition, const Locus::Vector3& asteroidCentroid)
{
   Locus::Plane orthogonalPlane(asteroidCentroid, player.viewpoint.GetForward());
   Locus::Vector3 shotProjection = orthogonalPlane.getProjection(shotPosition);

   Locus::Vector3 normal = shotProjection - asteroidCentroid;
   normal.rotateAround(-player.viewpoint.GetForward(), Locus::PI/2);

   return Locus::Plane(asteroidCentroid, normal);
}

void DemoScene::splitAsteroid(std::size_t splitIndex, const Locus::Vector3& shotPosition)
{
   //split an asteroid in two. If it has no more hits left,
   //simply remove the asteroid from the game

   ++score;

   asteroids[splitIndex]->decreaseHitsLeft();

   if (asteroids[splitIndex]->getHitsLeft() == 0)
   {
      //asteroid is destroyed
      asteroids[splitIndex]->DeleteGPUVertexData();
      collisionManager.Remove(*asteroids[splitIndex]);

      asteroids.erase(asteroids.begin() + splitIndex);
   }
   else
   {
      asteroids.push_back( std::unique_ptr<Asteroid>( new Asteroid(asteroids[splitIndex]->getHitsLeft()) ) );
      asteroids.push_back( std::unique_ptr<Asteroid>( new Asteroid(asteroids[splitIndex]->getHitsLeft()) ) );

      std::size_t newAsteroidIndex1 = asteroids.size() - 2;
      std::size_t newAsteroidIndex2 = asteroids.size() - 1;

      Locus::Random random;

      float xRotationDirection = static_cast<float>( random.randomDouble(-1, 1) );
      float yRotationDirection = static_cast<float>( random.randomDouble(-1, 1) );
      float zRotationDirection = static_cast<float>( random.randomDouble(-1, 1) );
      asteroids[newAsteroidIndex1]->motionProperties.rotation.set(xRotationDirection, yRotationDirection, zRotationDirection);
      asteroids[newAsteroidIndex2]->motionProperties.rotation.set(xRotationDirection, yRotationDirection, zRotationDirection);

      asteroids[newAsteroidIndex1]->motionProperties.angularSpeed = asteroids[splitIndex]->motionProperties.angularSpeed;
      asteroids[newAsteroidIndex2]->motionProperties.angularSpeed = asteroids[splitIndex]->motionProperties.angularSpeed;
      asteroids[newAsteroidIndex1]->SetTexture(asteroids[splitIndex]->GetTexture());
      asteroids[newAsteroidIndex2]->SetTexture(asteroids[splitIndex]->GetTexture());

      Locus::Plane splitPlane = makeHalfSplitPlane(shotPosition, asteroids[splitIndex]->Position());

      asteroids[splitIndex]->DetermineSplit(splitPlane, asteroids[splitIndex]->CurrentModelTransformation(), *asteroids[newAsteroidIndex1], *asteroids[newAsteroidIndex2]);

      if ((asteroids[newAsteroidIndex1]->NumFaces() > 0) && (asteroids[newAsteroidIndex2]->NumFaces() > 0))
      {
         asteroids[newAsteroidIndex1]->Reset(asteroids[newAsteroidIndex1]->centroid);
         asteroids[newAsteroidIndex2]->Reset(asteroids[newAsteroidIndex2]->centroid);

         asteroids[newAsteroidIndex1]->motionProperties.speed = asteroids[splitIndex]->motionProperties.speed;
         asteroids[newAsteroidIndex2]->motionProperties.speed = asteroids[splitIndex]->motionProperties.speed;

         asteroids[newAsteroidIndex1]->motionProperties.direction = splitPlane.getNormal();
         asteroids[newAsteroidIndex1]->motionProperties.direction.normalize();

         asteroids[newAsteroidIndex2]->motionProperties.direction = -asteroids[newAsteroidIndex1]->motionProperties.direction;

         //avoiding immediate interpenetration
         asteroids[newAsteroidIndex1]->lastCollision = asteroids[newAsteroidIndex2].get();
         asteroids[newAsteroidIndex2]->lastCollision = asteroids[newAsteroidIndex1].get();

         asteroids[newAsteroidIndex1]->lastCollisionTime = asteroids[newAsteroidIndex2]->lastCollisionTime = std::chrono::high_resolution_clock::now();

         asteroids[newAsteroidIndex1]->AssignNormals();
         asteroids[newAsteroidIndex2]->AssignNormals();

         asteroids[newAsteroidIndex1]->CreateGPUVertexData();
         asteroids[newAsteroidIndex1]->UpdateGPUVertexData();
         asteroids[newAsteroidIndex1]->UpdateMaxDistanceToCenter();
         asteroids[newAsteroidIndex1]->UpdateBroadCollisionExtent();
         asteroids[newAsteroidIndex1]->CreateBoundingVolumeHierarchy();

         asteroids[newAsteroidIndex2]->CreateGPUVertexData();
         asteroids[newAsteroidIndex2]->UpdateGPUVertexData();
         asteroids[newAsteroidIndex2]->UpdateMaxDistanceToCenter();
         asteroids[newAsteroidIndex2]->UpdateBroadCollisionExtent();
         asteroids[newAsteroidIndex2]->CreateBoundingVolumeHierarchy();

         collisionManager.Add(*asteroids[newAsteroidIndex1]);
         collisionManager.Add(*asteroids[newAsteroidIndex2]);
      }
      else
      {
         asteroids.pop_back();
         asteroids.pop_back();
      }

      asteroids[splitIndex]->DeleteGPUVertexData();
      collisionManager.Remove(*asteroids[splitIndex]);

      asteroids.erase(asteroids.begin() + splitIndex);
   }
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
         initializeAsteroids();
         break;

      case KEY_LIGHTS:
         LoadLights();
         break;

      case KEY_PAUSE:
         sceneManager.AddScene( std::unique_ptr<Locus::Scene>(new PauseScene(sceneManager, *this, KEY_PAUSE)) );
         break;

      case Locus::Key_ESCAPE:
         DestroyRenderingState();
         dieOnNextFrame = true;
         break;

      case KEY_FORWARD:
         moveCameraForward();
         break;

      case KEY_BACKWARD:
         moveCameraBackward();
         break;

      case KEY_LEFT:
         moveCameraLeft();
         break;

      case KEY_RIGHT:
         moveCameraRight();
         break;

      case KEY_UP:
         moveCameraUp();
         break;

      case KEY_DOWN:
         moveCameraDown();
         break;
   }
}

void DemoScene::KeyReleased(Locus::Key_t key)
{
   switch (key)
   {
      case KEY_FORWARD:
         stopMoveCameraForward();
         break;

      case KEY_BACKWARD:
         stopMoveCameraBackward();
         break;

      case KEY_LEFT:
         stopMoveCameraLeft();
         break;

      case KEY_RIGHT:
         stopMoveCameraRight();
         break;

      case KEY_UP:
         stopMoveCameraUp();
         break;

      case KEY_DOWN:
         stopMoveCameraDown();
         break;
   }
}

void DemoScene::MousePressed(Locus::MouseButton_t button, int /*x*/, int /*y*/)
{
   if (button == Locus::Mouse_Button_Left)
   {
      shotFired();
   }
}

void DemoScene::MouseReleased(Locus::MouseButton_t /*button*/, int /*x*/, int /*y*/)
{
}

void DemoScene::MouseMoved(int x, int y)
{
   Locus::Vector3 difference(static_cast<float>(x), static_cast<float>(y), 0.0f);
   Locus::Vector3 rotation((-difference.y/resolutionY) * FIELD_OF_VIEW * Locus::TO_RADIANS, (-difference.x/resolutionX)* FIELD_OF_VIEW * Locus::TO_RADIANS, 0.0f);

   rotateCamera(rotation);

   sceneManager.CenterMouse();
}

void DemoScene::Resized(int width, int height)
{
   glViewport(0, 0, width, height);

   hud.Initialize(textureManager.get(), Config::GetNumShots(), width, height);
   hud.UpdateGPUVertexData();

   resolutionX = width;
   resolutionY = height;
}

//////////////////////////////////////Camera Functions//////////////////////////////////////////

//////////Begin Translating////////////////////////
void DemoScene::moveCameraForward()
{
   player.translateAhead = true;
}

void DemoScene::moveCameraBackward()
{
   player.translateBack = true;
}

void DemoScene::moveCameraRight()
{
   player.translateRight = true;
}

void DemoScene::moveCameraLeft()
{
   player.translateLeft = true;
}

void DemoScene::moveCameraUp()
{
   player.translateUp = true;
}

void DemoScene::moveCameraDown()
{
   player.translateDown = true;
}

/////////////Stop Translating/////////////
void DemoScene::stopMoveCameraForward()
{
   player.translateAhead = false;
}

void DemoScene::stopMoveCameraBackward()
{
   player.translateBack = false;
}

void DemoScene::stopMoveCameraRight()
{
   player.translateRight = false;
}

void DemoScene::stopMoveCameraLeft()
{
   player.translateLeft = false;
}

void DemoScene::stopMoveCameraUp()
{
   player.translateUp = false;
}

void DemoScene::stopMoveCameraDown()
{
   player.translateDown = false;
}

void DemoScene::rotateCamera(const Locus::Vector3& rotation)
{
   player.Rotate(rotation);
}

//////////////////////////////////////Game Main Loop Functions//////////////////////////////////////////

void DemoScene::Activate()
{
   sceneManager.MakeFullScreen();
   sceneManager.HideMouse();
   sceneManager.CenterMouse();
}

bool DemoScene::Update(double DT)
{
   if (dieOnNextFrame)
   {
      return false;
   }

   player.tick(DT);
   collisionManager.Update(player);

   tickAsteroids(DT);
   updateShotPositions(DT);

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

void DemoScene::tickAsteroids(double DT)
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

      collisionManager.Update(*asteroid);
   }

   //update asteroid visibility with camera frustum

   Locus::Vector3 forward = player.viewpoint.GetForward();
   Locus::Vector3 up = player.viewpoint.GetUp();

   Locus::Vector3 point = player.viewpoint.GetPosition();

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

         splitAsteroid(asteroidIndex, asteroids[asteroidIndex]->GetHitLocation());

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

   drawBackground();

   drawAsteroids();
   drawShots();

   drawHUD();
}

void DemoScene::drawBackground()
{
   drawSkyBox();
   drawStars();
   drawPlanets();
}

void DemoScene::drawSkyBox()
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

void DemoScene::drawStars()
{
   renderingState->shaderController.UseProgram(ShaderNames::NotTexturedNotLit);

   glDisable(GL_DEPTH_TEST);

   player.viewpoint.Activate(renderingState->transformationStack);

   renderingState->transformationStack.Translate(player.viewpoint.GetPosition());
   renderingState->UploadTransformations();

   stars.Draw(*renderingState);

   player.viewpoint.Deactivate(renderingState->transformationStack);

   renderingState->shaderController.UseProgram(ShaderNames::TexturedNotLit);

   glEnable(GL_DEPTH_TEST);
}

void DemoScene::drawPlanets()
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

void DemoScene::drawAsteroids()
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

      renderingState->shaderController.UseProgram(ShaderNames::TexturedAndLit_1 + (numShotPositionsToUse - 1));

      for (unsigned int shotPositionIndex = 0; shotPositionIndex < numShotPositionsToUse; ++shotPositionIndex)
      {
         lights[shotPositionIndex].eyePosition = player.viewpoint.GetTransformation().GetInverse().MultVertex(shotPositionsAndDistances[shotPositionIndex].position);
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
      renderingState->shaderController.UseProgram(ShaderNames::TexturedNotLit);
   }
}

void DemoScene::drawShots()
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

void DemoScene::drawHUD()
{
   Locus::DrawUtility::BeginDrawing2D(*renderingState, resolutionX, resolutionY);

   hud.Draw(*renderingState);

   Locus::DrawUtility::EndDrawing2D(*renderingState);
}

}