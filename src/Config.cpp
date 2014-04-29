 /*********************************************************************************************************\
 *                                                                                                        *
 *   This file is part of Minor Planet Mayhem                                                             *
 *                                                                                                        *
 *   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
 *                                                                                                        *
 *   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
 *                                                                                                        *
 \*********************************************************************************************************/

#include "Config.h"

#include "Locus/FileSystem/FileSystemUtil.h"

#include "Locus/Common/Parsing.h"

#include "RapidXML/rapidxml.hpp"

#include "XML.h"

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
static const int Default_Min_Moons = 1;
static const int Default_Max_Moons = 3;
static const float Default_Min_Moon_Orbital_Speed = 1.0f;
static const float Default_Max_Moon_Orbital_Speed = 1.5f;

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
int Config::minMoons = Default_Min_Moons;
int Config::maxMoons = Default_Max_Moons;
float Config::minMoonOrbitalSpeed = Default_Min_Moon_Orbital_Speed;
float Config::maxMoonOrbitalSpeed = Default_Max_Moon_Orbital_Speed;

namespace OptionsXML
{

static const char* XML_Root = "Options";
static const char* Model_File = "Model_File";
static const char* Num_Asteroids = "Num_Asteroids";
static const char* Num_Stars = "Num_Stars";
static const char* Num_Shots = "Num_Shots";
static const char* Player_Translation_Speed = "Player_Speed";
static const char* Shot_Speed = "Shot_Speed";
static const char* Asteroids_Boundary = "Game_Boundary_Length";
static const char* Player_Collision_Radius = "Player_Collision_Radius";
static const char* Asteroid_Speed = "Asteroid_Speed";
static const char* Asteroid_Rotation_Speed = "Rotation_Speed";
static const char* Num_Planets = "Num_Planets";
static const char* Planet_Radius = "Planet_Radius";
static const char* Num_Moons = "Num_Moons";
static const char* Moon_Orbital_Speed = "Moon_Orbital_Speed";

static const char* Minimum = "Min";
static const char* Maximum = "Max";

}

namespace LightingXML
{

static const char* XML_Root = "Lighting";
static const char* Shot_Lights = "ShotLights";
static const char* Light = "Light";
static const char* R = "R";
static const char* G = "G";
static const char* B = "B";
static const char* A = "A";
static const char* Index = "index";
static const char* Attenuation = "Attenuation";
static const char* ConstantAttenuation = "Constant";
static const char* LinearAttenuation = "Linear";
static const char* QuadraticAttenuation = "Quadratic";

}

template <typename T>
void LoadNumeric(T& valueToLoad, rapidxml::xml_node<>* rootNode, const char* tagName, float minValue)
{
   rapidxml::xml_node<>* xmlNode = rootNode->first_node(tagName);
   if (xmlNode != nullptr)
   {
      std::string xmlNodeValue;

      xmlNodeValue = xmlNode->value();
      Locus::TrimString(xmlNodeValue);

      if (Locus::IsType<T>(xmlNodeValue))
      {
         float potentialValue = std::stof(xmlNodeValue);

         if (potentialValue >= minValue)
         {
            valueToLoad = static_cast<T>(potentialValue);
         }
      }
   }
}

template <typename T>
void LoadMinMaxPair(T& minValueToLoad, T& maxValueToLoad, rapidxml::xml_node<>* rootNode, const char* tagName, float minValue)
{
   rapidxml::xml_node<>* xmlNode = rootNode->first_node(tagName);
   if (xmlNode != nullptr)
   {
      rapidxml::xml_node<>* minNode = xmlNode->first_node(OptionsXML::Minimum);
      if (minNode != nullptr)
      {
         std::string xmlNodeValue;

         xmlNodeValue = minNode->value();
         Locus::TrimString(xmlNodeValue);

         if (Locus::IsType<T>(xmlNodeValue))
         {
            float potentialMinValue = std::stof(xmlNodeValue);

            if (potentialMinValue >= minValue)
            {
               rapidxml::xml_node<>* maxNode = xmlNode->first_node(OptionsXML::Maximum);
               if (maxNode != nullptr)
               {
                  xmlNodeValue = maxNode->value();
                  Locus::TrimString(xmlNodeValue);

                  if (Locus::IsType<T>(xmlNodeValue))
                  {
                     float potentialMaxValue = std::stof(xmlNodeValue);

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
   minMoons = Default_Min_Moons;
   maxMoons = Default_Max_Moons;
   minMoonOrbitalSpeed = Default_Min_Moon_Orbital_Speed;
   maxMoonOrbitalSpeed = Default_Max_Moon_Orbital_Speed;

   rapidxml::xml_document<> xmlDocument;
   std::vector<char> xmlContents;

   if(!ParseXMLFile(Locus::MountedFilePath("config/options.config.xml"), xmlDocument, xmlContents))
   {
      return;
   }

   rapidxml::xml_node<>* rootNode = xmlDocument.first_node(OptionsXML::XML_Root);
   if (rootNode != nullptr)
   {
      rapidxml::xml_node<>* modelFileNode = rootNode->first_node(OptionsXML::Model_File);
      if (modelFileNode != nullptr)
      {
         modelFile = modelFileNode->value();
         Locus::TrimString(modelFile);
      }

      LoadNumeric<int>(numAsteroids, rootNode, OptionsXML::Num_Asteroids, 1.0f);
      LoadNumeric<unsigned int>(numStars, rootNode, OptionsXML::Num_Stars, 0.0f);
      LoadNumeric<unsigned int>(numShots, rootNode, OptionsXML::Num_Shots, 1.0f);
      LoadNumeric<float>(playerTranslationSpeed, rootNode, OptionsXML::Player_Translation_Speed, 0.0f);
      LoadNumeric<float>(shotSpeed, rootNode, OptionsXML::Shot_Speed, 1.0f);
      LoadNumeric<float>(asteroidsBoundary, rootNode, OptionsXML::Asteroids_Boundary, 1.0f);
      LoadNumeric<float>(playerCollisionRadius, rootNode, OptionsXML::Player_Collision_Radius, 0.01f);

      LoadMinMaxPair<float>(minAsteroidSpeed, maxAsteroidSpeed, rootNode, OptionsXML::Asteroid_Speed, 0.0f);
      LoadMinMaxPair<float>(minAsteroidRotationSpeed, maxAsteroidRotationSpeed, rootNode, OptionsXML::Asteroid_Rotation_Speed, 0.0f);
      LoadMinMaxPair<int>(minPlanets, maxPlanets, rootNode, OptionsXML::Num_Planets, 0.0f);
      LoadMinMaxPair<float>(minPlanetRadius, maxPlanetRadius, rootNode, OptionsXML::Planet_Radius, 0.01f);
      LoadMinMaxPair<int>(minMoons, maxMoons, rootNode, OptionsXML::Num_Moons, 0.0f);
      LoadMinMaxPair<float>(minMoonOrbitalSpeed, maxMoonOrbitalSpeed, rootNode, OptionsXML::Moon_Orbital_Speed, 0.0f);
   }
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

bool LoadShotLights(rapidxml::xml_node<>* containingNode, std::vector<Locus::Color>& lightColors)
{
   rapidxml::xml_node<>* lightNode = containingNode->first_node(LightingXML::Light);

   #define CHECK_NODE(node) if (node == nullptr) return false;

   CHECK_NODE(lightNode)

   std::map<int, Locus::Color> loadedColorMap;

   Locus::Color loadedColor;

   do
   {
      rapidxml::xml_attribute<>* indexAttribute = lightNode->first_attribute(LightingXML::Index);
      CHECK_NODE(indexAttribute)

      rapidxml::xml_node<>* redNode = lightNode->first_node(LightingXML::R);
      CHECK_NODE(redNode)

      rapidxml::xml_node<>* greenNode = lightNode->first_node(LightingXML::G);
      CHECK_NODE(greenNode)

      rapidxml::xml_node<>* blueNode = lightNode->first_node(LightingXML::B);
      CHECK_NODE(blueNode)

      rapidxml::xml_node<>* alphaNode = lightNode->first_node(LightingXML::A);
      CHECK_NODE(alphaNode)

      int index = 0;
      if (!ReadInt(indexAttribute->value(), index))
      {
         return false;
      }

      int redAsInt = 0;
      if (!ReadInt(redNode->value(), redAsInt))
      {
         return false;
      }
      loadedColor.r = static_cast<unsigned char>(redAsInt);

      int greenAsInt = 0;
      if (!ReadInt(greenNode->value(), greenAsInt))
      {
         return false;
      }
      loadedColor.g = static_cast<unsigned char>(greenAsInt);

      int blueAsInt = 0;
      if (!ReadInt(blueNode->value(), blueAsInt))
      {
         return false;
      }
      loadedColor.b = static_cast<unsigned char>(blueAsInt);

      int alphaAsInt = 0;
      if (!ReadInt(alphaNode->value(), alphaAsInt))
      {
         return false;
      }
      loadedColor.a = static_cast<unsigned char>(alphaAsInt);

      loadedColorMap[index] = loadedColor;

      lightNode = lightNode->next_sibling(LightingXML::Light);

   } while (lightNode != nullptr);

#undef CHECK_NODE

   lightColors.reserve(lightColors.size() + loadedColorMap.size());

   for (const std::pair<int, Locus::Color>& indexColorPair : loadedColorMap)
   {
      lightColors.push_back(indexColorPair.second);
   }

   return true;
}

static void ReadFloatFromNode(rapidxml::xml_node<>* containingNode, const char* nodeName, float& value)
{
   rapidxml::xml_node<>* xmlNode = containingNode->first_node(nodeName);
   if (xmlNode != nullptr)
   {
      ReadFloat(xmlNode->value(), value);
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

   rapidxml::xml_document<> xmlDocument;
   std::vector<char> xmlContents;

   bool problemWithColors = true;

   Config::LightingOptions lightingOptions;
   lightingOptions.constantAttenuation = Default_Constant_Attenuation;
   lightingOptions.linearAttenuation = Default_Linear_Attenuation;
   lightingOptions.quadraticAttenuation = Default_Quadratic_Attenuation;

   bool problemLoadingFile = false;

   if (!ParseXMLFile(Locus::MountedFilePath("config/lighting.config.xml"), xmlDocument, xmlContents))
   {
      problemLoadingFile = true;
   }

   if (!problemLoadingFile)
   {
      rapidxml::xml_node<>* rootNode = xmlDocument.first_node(LightingXML::XML_Root);
      if (rootNode != nullptr)
      {
         rapidxml::xml_node<>* shotLightsNode = rootNode->first_node(LightingXML::Shot_Lights);
         if (shotLightsNode != nullptr)
         {
            problemWithColors = !LoadShotLights(shotLightsNode, lightingOptions.lightColors);
         }

         rapidxml::xml_node<>* attenuationNode = rootNode->first_node(LightingXML::Attenuation);
         if (attenuationNode != nullptr)
         {
            ReadFloatFromNode(attenuationNode, LightingXML::ConstantAttenuation, lightingOptions.constantAttenuation);
            ReadFloatFromNode(attenuationNode, LightingXML::LinearAttenuation, lightingOptions.linearAttenuation);
            ReadFloatFromNode(attenuationNode, LightingXML::QuadraticAttenuation, lightingOptions.quadraticAttenuation);
         }
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

int Config::GetMinMoons()
{
   return minMoons;
}

int Config::GetMaxMoons()
{
   return maxMoons;
}

float Config::GetMinMoonOrbitalSpeed()
{
   return minMoonOrbitalSpeed;
}

float Config::GetMaxMoonOrbitalSpeed()
{
   return maxMoonOrbitalSpeed;
}

}