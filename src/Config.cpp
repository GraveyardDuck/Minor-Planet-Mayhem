/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "Config.h"

#include "Locus/Common/Parsing.h"
#include "Locus/Common/Exception.h"

#include "Locus/FileSystem/MountedFilePath.h"

#include "Locus/XML/XMLParsing.h"
#include "Locus/XML/XMLTag.h"

#include <map>
#include <type_traits>

namespace MPM
{

static const std::string Default_Model_File = "demo.sap";
static const int Default_Num_Asteroids = 600;
static const int Default_Num_Stars = 10000;
static const int Default_Num_Shots = 100;
static const float Default_Player_Translation_Speed = 80.0f;
static const float Default_Shot_Speed = 100;
static const float Default_Asteroids_Boundary = 130.0f;
static const float Default_Player_Collision_Radius = 8.0f;
static const float Default_Min_Asteroid_Speed = 10.0f;
static const float Default_Max_Asteroid_Speed = 20.0f;
static const float Default_Min_Asteroid_Rotation_Speed = 1.0f;
static const float Default_Max_Asteroid_Rotation_Speed = 3.0f;
static const int Default_Min_Planets = 10;
static const int Default_Max_Planets = 15;
static const float Default_Min_Planet_Radius = 30.0f;
static const float Default_Max_Planet_Radius = 50.0f;

std::string Config::modelFile = Default_Model_File;
int Config::numAsteroids = Default_Num_Asteroids;
unsigned int Config::numStars = Default_Num_Stars;
unsigned int Config::numShots = Default_Num_Shots;
float Config::playerTranslationSpeed = Default_Player_Translation_Speed;
float Config::shotSpeed = Default_Shot_Speed;
float Config::asteroidsBoundary = Default_Asteroids_Boundary;
float Config::playerCollisionRadius = Default_Player_Collision_Radius;
float Config::minAsteroidSpeed = Default_Min_Asteroid_Speed;
float Config::maxAsteroidSpeed = Default_Max_Asteroid_Speed;
float Config::minAsteroidRotationSpeed = Default_Min_Asteroid_Rotation_Speed;
float Config::maxAsteroidRotationSpeed = Default_Max_Asteroid_Rotation_Speed;
int Config::minPlanets = Default_Min_Planets;
int Config::maxPlanets = Default_Max_Planets;
float Config::minPlanetRadius = Default_Min_Planet_Radius;
float Config::maxPlanetRadius = Default_Max_Planet_Radius;

namespace OptionsXML
{

static const std::string XML_Root = "Options";
static const std::string Model_File = "Model_File";
static const std::string Num_Asteroids = "Num_Asteroids";
static const std::string Num_Stars = "Num_Stars";
static const std::string Num_Shots = "Num_Shots";
static const std::string Player_Translation_Speed = "Player_Speed";
static const std::string Shot_Speed = "Shot_Speed";
static const std::string Asteroids_Boundary = "Game_Boundary_Length";
static const std::string Player_Collision_Radius = "Player_Collision_Radius";
static const std::string Asteroid_Speed = "Asteroid_Speed";
static const std::string Asteroid_Rotation_Speed = "Rotation_Speed";
static const std::string Num_Planets = "Num_Planets";
static const std::string Planet_Radius = "Planet_Radius";

static const std::string Minimum = "Min";
static const std::string Maximum = "Max";

}

namespace LightingXML
{

static const std::string XML_Root = "Lighting";
static const std::string Shot_Lights = "ShotLights";
static const std::string Light = "Light";
static const std::string R = "R";
static const std::string G = "G";
static const std::string B = "B";
static const std::string A = "A";
static const std::string Index = "index";
static const std::string Attenuation = "Attenuation";
static const std::string ConstantAttenuation = "Constant";
static const std::string LinearAttenuation = "Linear";
static const std::string QuadraticAttenuation = "Quadratic";

}

template <typename T>
void LoadNumeric(T& valueToLoad, const Locus::XMLTag& rootTag, const std::string& tagName, float minValue)
{
   const Locus::XMLTag* xmlTag = rootTag.FindSubTag(tagName, 0);
   if (xmlTag != nullptr)
   {
      std::string xmlTagValue;

      xmlTagValue = xmlTag->value;
      Locus::TrimString(xmlTagValue);

      if (Locus::IsType<T>(xmlTagValue))
      {
         float potentialValue = std::stof(xmlTagValue);

         if (potentialValue >= minValue)
         {
            valueToLoad = static_cast<T>(potentialValue);
         }
      }
   }
}

template <typename T>
void LoadMinMaxPair(T& minValueToLoad, T& maxValueToLoad, const Locus::XMLTag& rootTag, const std::string& tagName, float minValue)
{
   const Locus::XMLTag* xmlTag = rootTag.FindSubTag(tagName, 0);
   if (xmlTag != nullptr)
   {
      const Locus::XMLTag* minTag = xmlTag->FindSubTag(OptionsXML::Minimum, 0);
      if (minTag != nullptr)
      {
         std::string xmlTagValue;

         xmlTagValue = minTag->value;
         Locus::TrimString(xmlTagValue);

         if (Locus::IsType<T>(xmlTagValue))
         {
            float potentialMinValue = std::stof(xmlTagValue);

            if (potentialMinValue >= minValue)
            {
               const Locus::XMLTag* maxTag = xmlTag->FindSubTag(OptionsXML::Maximum, 0);
               if (maxTag != nullptr)
               {
                  xmlTagValue = maxTag->value;
                  Locus::TrimString(xmlTagValue);

                  if (Locus::IsType<T>(xmlTagValue))
                  {
                     float potentialMaxValue = std::stof(xmlTagValue);

                     if (potentialMaxValue >= potentialMinValue)
                     {
                        minValueToLoad = static_cast<T>(potentialMinValue);
                        maxValueToLoad = static_cast<T>(potentialMaxValue);
                     }
                  }
               }
            }
         }
      }
   }
}

void Config::Set()
{
   modelFile = Default_Model_File;
   numAsteroids = Default_Num_Asteroids;
   numStars = Default_Num_Stars;
   numShots = Default_Num_Shots;
   playerTranslationSpeed = Default_Player_Translation_Speed;
   shotSpeed = Default_Shot_Speed;
   asteroidsBoundary = Default_Asteroids_Boundary;
   playerCollisionRadius = Default_Player_Collision_Radius;

   minAsteroidSpeed = Default_Min_Asteroid_Speed;
   maxAsteroidSpeed = Default_Max_Asteroid_Speed;
   minAsteroidRotationSpeed = Default_Min_Asteroid_Rotation_Speed;
   maxAsteroidRotationSpeed = Default_Max_Asteroid_Rotation_Speed;
   minPlanets = Default_Min_Planets;
   maxPlanets = Default_Max_Planets;
   minPlanetRadius = Default_Min_Planet_Radius;
   maxPlanetRadius = Default_Max_Planet_Radius;

   Locus::XMLTag rootTag;

   try
   {
      Locus::ParseXMLFile(Locus::MountedFilePath("config/options.config.xml"), rootTag);
   }
   catch (Locus::Exception&)
   {
      return;
   }

   Locus::XMLTag* modelFileTag = rootTag.FindSubTag(OptionsXML::Model_File, 0);
   if (modelFileTag != nullptr)
   {
      modelFile = modelFileTag->value;
      Locus::TrimString(modelFile);
   }

   LoadNumeric<int>(numAsteroids, rootTag, OptionsXML::Num_Asteroids, 1.0f);
   LoadNumeric<unsigned int>(numStars, rootTag, OptionsXML::Num_Stars, 0.0f);
   LoadNumeric<unsigned int>(numShots, rootTag, OptionsXML::Num_Shots, 1.0f);
   LoadNumeric<float>(playerTranslationSpeed, rootTag, OptionsXML::Player_Translation_Speed, 0.0f);
   LoadNumeric<float>(shotSpeed, rootTag, OptionsXML::Shot_Speed, 1.0f);
   LoadNumeric<float>(asteroidsBoundary, rootTag, OptionsXML::Asteroids_Boundary, 1.0f);
   LoadNumeric<float>(playerCollisionRadius, rootTag, OptionsXML::Player_Collision_Radius, 0.01f);

   LoadMinMaxPair<float>(minAsteroidSpeed, maxAsteroidSpeed, rootTag, OptionsXML::Asteroid_Speed, 0.0f);
   LoadMinMaxPair<float>(minAsteroidRotationSpeed, maxAsteroidRotationSpeed, rootTag, OptionsXML::Asteroid_Rotation_Speed, 0.0f);
   LoadMinMaxPair<int>(minPlanets, maxPlanets, rootTag, OptionsXML::Num_Planets, 0.0f);
   LoadMinMaxPair<float>(minPlanetRadius, maxPlanetRadius, rootTag, OptionsXML::Planet_Radius, 0.01f);
}

static bool ReadInt(const std::string& str, int& value)
{
   if (!Locus::IsType<int>(str))
   {
      return false;
   }

   value = std::stoi(str);

   return true;
}

static bool ReadFloat(const std::string& str, float& value)
{
   if (!Locus::IsType<float>(str))
   {
      return false;
   }

   value = std::stof(str);

   return true;
}

bool LoadShotLights(const Locus::XMLTag& containingTag, std::vector<Locus::Color>& lightColors)
{
   #define CHECK_XML(tagOrAttribute) if (tagOrAttribute == nullptr) return false

   std::map<int, Locus::Color> loadedColorMap;

   Locus::Color loadedColor;

   for (const Locus::XMLTag& lightTag : containingTag.subTags)
   {
      if (lightTag.name != LightingXML::Light)
      {
         return false;
      }

      const Locus::XMLAttribute* indexAttribute = lightTag.FindAttribute(LightingXML::Index, 0);
      CHECK_XML(indexAttribute);

      const Locus::XMLTag* redTag = lightTag.FindSubTag(LightingXML::R, 0);
      CHECK_XML(redTag);

      const Locus::XMLTag* greenTag = lightTag.FindSubTag(LightingXML::G, 0);
      CHECK_XML(greenTag);

      const Locus::XMLTag* blueTag = lightTag.FindSubTag(LightingXML::B, 0);
      CHECK_XML(blueTag);

      const Locus::XMLTag* alphaTag = lightTag.FindSubTag(LightingXML::A, 0);
      CHECK_XML(alphaTag);

      int index = 0;
      if (!ReadInt(indexAttribute->value, index))
      {
         return false;
      }

      int redAsInt = 0;
      if (!ReadInt(redTag->value, redAsInt))
      {
         return false;
      }
      loadedColor.r = static_cast<unsigned char>(redAsInt);

      int greenAsInt = 0;
      if (!ReadInt(greenTag->value, greenAsInt))
      {
         return false;
      }
      loadedColor.g = static_cast<unsigned char>(greenAsInt);

      int blueAsInt = 0;
      if (!ReadInt(blueTag->value, blueAsInt))
      {
         return false;
      }
      loadedColor.b = static_cast<unsigned char>(blueAsInt);

      int alphaAsInt = 0;
      if (!ReadInt(alphaTag->value, alphaAsInt))
      {
         return false;
      }
      loadedColor.a = static_cast<unsigned char>(alphaAsInt);

      loadedColorMap[index] = loadedColor;
   }

   lightColors.reserve(lightColors.size() + loadedColorMap.size());

   for (const std::pair<int, Locus::Color>& indexColorPair : loadedColorMap)
   {
      lightColors.push_back(indexColorPair.second);
   }

   return true;
}

static void ReadFloatFromTag(const Locus::XMLTag& containingTag, const std::string& subTagName, float& floatValue)
{
   const Locus::XMLTag* subTag = containingTag.FindSubTag(subTagName, 0);
   if (subTag != nullptr)
   {
      ReadFloat(subTag->value, floatValue);
   }
}

Config::LightingOptions Config::ReadLightingOptions()
{
   static const std::vector<Locus::Color> defaultColors = 
   {
      Locus::Color(255, 0, 0, 255),
      Locus::Color(255, 132, 0, 255),
      Locus::Color(255, 255, 0, 255),
      Locus::Color(0, 255, 0, 255),
      Locus::Color(0, 0, 255, 255),
      Locus::Color(128, 0, 255, 255),
      Locus::Color(255, 0, 255, 255)
   };

   static const float Default_Constant_Attenuation = 0.009f;
   static const float Default_Linear_Attenuation = 0.009f;
   static const float Default_Quadratic_Attenuation = 0.05f;

   bool problemWithColors = true;

   Config::LightingOptions lightingOptions;
   lightingOptions.constantAttenuation = Default_Constant_Attenuation;
   lightingOptions.linearAttenuation = Default_Linear_Attenuation;
   lightingOptions.quadraticAttenuation = Default_Quadratic_Attenuation;

   bool problemLoadingFile = false;

   Locus::XMLTag rootTag;

   try
   {
      Locus::ParseXMLFile(Locus::MountedFilePath("config/lighting.config.xml"), rootTag);
   }
   catch (Locus::Exception&)
   {
      problemLoadingFile = true;
   }

   if (!problemLoadingFile)
   {
      Locus::XMLTag* shotLightsTag = rootTag.FindSubTag(LightingXML::Shot_Lights, 0);
      if (shotLightsTag != nullptr)
      {
         problemWithColors = !LoadShotLights(*shotLightsTag, lightingOptions.lightColors);
      }

      Locus::XMLTag* attenuationTag = rootTag.FindSubTag(LightingXML::Attenuation, 0);
      if (attenuationTag != nullptr)
      {
         ReadFloatFromTag(*attenuationTag, LightingXML::ConstantAttenuation, lightingOptions.constantAttenuation);
         ReadFloatFromTag(*attenuationTag, LightingXML::LinearAttenuation, lightingOptions.linearAttenuation);
         ReadFloatFromTag(*attenuationTag, LightingXML::QuadraticAttenuation, lightingOptions.quadraticAttenuation);
      }
   }

   if (problemWithColors)
   {
      lightingOptions.lightColors = defaultColors;
   }

   return lightingOptions;
}

std::string Config::GetModelFile()
{
   return modelFile;
}

int Config::GetNumAsteroids()
{
   return numAsteroids;
}

unsigned int Config::GetNumStars()
{
   return numStars;
}

unsigned int Config::GetNumShots()
{
   return numShots;
}

float Config::GetPlayerTranslationSpeed()
{
   return playerTranslationSpeed;
}

float Config::GetShotSpeed()
{
   return shotSpeed;
}

float Config::GetAsteroidsBoundary()
{
   return asteroidsBoundary;
}

float Config::GetPlayerCollisionRadius()
{
   return playerCollisionRadius;
}

float Config::GetMinAsteroidSpeed()
{
   return minAsteroidSpeed;
}

float Config::GetMaxAsteroidSpeed()
{
   return maxAsteroidSpeed;
}

float Config::GetMinAsteroidRotationSpeed()
{
   return minAsteroidRotationSpeed;
}

float Config::GetMaxAsteroidRotationSpeed()
{
   return maxAsteroidRotationSpeed;
}

int Config::GetMinPlanets()
{
   return minPlanets;
}

int Config::GetMaxPlanets()
{
   return maxPlanets;
}

float Config::GetMinPlanetRadius()
{
   return minPlanetRadius;
}

float Config::GetMaxPlanetRadius()
{
   return maxPlanetRadius;
}

}