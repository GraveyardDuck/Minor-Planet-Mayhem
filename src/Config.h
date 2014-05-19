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

#include "Locus/Rendering/Color.h"

#include <string>
#include <vector>

namespace MPM
{

class Config
{
public:
   static void Set();

   static std::string GetModelFile();
   static int GetNumAsteroids();
   static unsigned int GetNumStars();
   static unsigned int GetNumShots();
   static float GetPlayerTranslationSpeed();
   static float GetShotSpeed();
   static float GetAsteroidsBoundary();
   static float GetPlayerCollisionRadius();
   static float GetMinAsteroidSpeed();
   static float GetMaxAsteroidSpeed();
   static float GetMinAsteroidRotationSpeed();
   static float GetMaxAsteroidRotationSpeed();
   static int GetMinPlanets();
   static int GetMaxPlanets();
   static float GetMinPlanetRadius();
   static float GetMaxPlanetRadius();
   static int GetMinMoons();
   static int GetMaxMoons();
   static float GetMinMoonOrbitalSpeed();
   static float GetMaxMoonOrbitalSpeed();

   struct LightingOptions
   {
      std::vector<Locus::Color> lightColors;
      float constantAttenuation;
      float linearAttenuation;
      float quadraticAttenuation;
   };

   static LightingOptions ReadLightingOptions();

private:
   static std::string modelFile;
   static int numAsteroids;
   static unsigned int numStars;
   static unsigned int numShots;
   static float playerTranslationSpeed;
   static float shotSpeed;
   static float asteroidsBoundary;
   static float playerCollisionRadius;
   static float minAsteroidSpeed;
   static float maxAsteroidSpeed;
   static float minAsteroidRotationSpeed;
   static float maxAsteroidRotationSpeed;
   static int minPlanets;
   static int maxPlanets;
   static float minPlanetRadius;
   static float maxPlanetRadius;
   static int minMoons;
   static int maxMoons;
   static float minMoonOrbitalSpeed;
   static float maxMoonOrbitalSpeed;
};

}