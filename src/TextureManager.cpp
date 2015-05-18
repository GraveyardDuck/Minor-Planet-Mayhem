/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "TextureManager.h"

#include "Locus/Common/Parsing.h"

#include "Locus/FileSystem/MountedFilePath.h"

#include "Locus/XML/XMLParsing.h"
#include "Locus/XML/XMLTag.h"

#include <utility>

namespace MPM
{

const std::string TextureManager::Skybox_Front = "Skybox_Front";
const std::string TextureManager::Skybox_Back = "Skybox_Back";
const std::string TextureManager::Skybox_Left = "Skybox_Left";
const std::string TextureManager::Skybox_Right = "Skybox_Right";
const std::string TextureManager::Skybox_Up = "Skybox_Up";
const std::string TextureManager::Skybox_Down = "Skybox_Down";
const std::string TextureManager::Shot_TextureName = "Shot";
const std::string TextureManager::Lives_Icon_TextureName = "LivesIcon";
const std::string TextureManager::Lives_Times_TextureName = "LivesTimes";
const std::string TextureManager::Score_Label_TextureName = "ScoreLabel";
const std::string TextureManager::Level_Label_TextureName = "LevelLabel";
const std::string TextureManager::Ammo_TextureName = "Ammo";

const std::string TextureManager::Asteroid_Base_TextureName = "Asteroid";
const std::string TextureManager::Planet_Base_TextureName = "Planet";
const std::string TextureManager::Moon_Base_TextureName = "Moon";
const std::string TextureManager::Digit_Base_TextureName = "Digit";

static const char* Textures_Config_XML_Root = "Textures";
static const char* Asteroids_XML_Node = "Asteroids";
static const char* Planets_XML_Node = "Planets";
static const char* Moons_XML_Node = "Moons";
static const char* Single_Texture_XML_Node = "Texture";

TextureManager::TextureManager(const Locus::GLInfo& glInfo)
   : Locus::TextureManager(glInfo), numAsteroidTextures(0), numPlanetTextures(0), numMoonTextures(0)
{
}

std::size_t TextureManager::NumAsteroidTextures() const
{
   return numAsteroidTextures;
}

std::size_t TextureManager::NumPlanetTextures() const
{
   return numPlanetTextures;
}

std::size_t TextureManager::NumMoonTextures() const
{
   return numMoonTextures;
}

std::string TextureManager::MakeAsteroidTextureName(std::size_t index)
{
   return TextureManager::Asteroid_Base_TextureName + "_" + std::to_string(index);
}

std::string TextureManager::MakePlanetTextureName(std::size_t index)
{
   return TextureManager::Planet_Base_TextureName + "_" + std::to_string(index);
}

std::string TextureManager::MakeMoonTextureName(std::size_t index)
{
   return TextureManager::Moon_Base_TextureName + "_" + std::to_string(index);
}

std::string TextureManager::MakeDigitTextureName(std::size_t index)
{
   return TextureManager::Digit_Base_TextureName + "_" + std::to_string(index);
}

void TextureManager::LoadAsteroidTexture(const std::string& textureLocation)
{
   Load(TextureManager::MakeAsteroidTextureName(numAsteroidTextures), textureLocation, Locus::Texture::MipmapGeneration::GLGenerateMipMap, Locus::TextureFiltering::Linear, false);
}

void TextureManager::LoadPlanetTexture(const std::string& textureLocation)
{
   Load(TextureManager::MakePlanetTextureName(numPlanetTextures), textureLocation, Locus::Texture::MipmapGeneration::GLGenerateMipMap, Locus::TextureFiltering::Linear, false);
}

void TextureManager::LoadMoonTexture(const std::string& textureLocation)
{
   Load(TextureManager::MakeMoonTextureName(numMoonTextures), textureLocation, Locus::Texture::MipmapGeneration::GLGenerateMipMap, Locus::TextureFiltering::Linear, false);
}

void TextureManager::LoadAsteroidTexture(const Locus::MountedFilePath& textureLocation)
{
   Load(TextureManager::MakeAsteroidTextureName(numAsteroidTextures), textureLocation, Locus::Texture::MipmapGeneration::GLGenerateMipMap, Locus::TextureFiltering::Linear, false);
}

void TextureManager::LoadPlanetTexture(const Locus::MountedFilePath& textureLocation)
{
   Load(TextureManager::MakePlanetTextureName(numPlanetTextures), textureLocation, Locus::Texture::MipmapGeneration::GLGenerateMipMap, Locus::TextureFiltering::Linear, false);
}

void TextureManager::LoadMoonTexture(const Locus::MountedFilePath& textureLocation)
{
   Load(TextureManager::MakeMoonTextureName(numMoonTextures), textureLocation, Locus::Texture::MipmapGeneration::GLGenerateMipMap, Locus::TextureFiltering::Linear, false);
}

void TextureManager::UnLoad()
{
   Locus::TextureManager::UnLoad();

   numAsteroidTextures = 0;
   numPlanetTextures = 0;
   numMoonTextures = 0;
}

void TextureManager::LoadAllTextures()
{
   UnLoad();

   Locus::XMLTag rootTag;

   Locus::ParseXMLFile(Locus::MountedFilePath("config/textures.config.xml"), rootTag);

   Locus::MountedFilePath texturesPath("textures/");

   #define CHECK_TAG(node, name) if (node == nullptr) throw std::runtime_error(std::string("Failed to parse textures.config.xml. ") + name + " not found")
   #define CHECK_TAG_NAME(tagName, expectedName) if (tagName != expectedName) throw std::runtime_error(std::string("Failed to parse textures.config.xml. Unexpected tag found: ") + tagName)

   //add asteroid textures

   Locus::XMLTag* asteroidsTag = rootTag.FindSubTag(Asteroids_XML_Node, 0);
   CHECK_TAG(asteroidsTag, Asteroids_XML_Node);

   std::string imageFile;

   for (Locus::XMLTag& asteroidsTextureTag : asteroidsTag->subTags)
   {
      CHECK_TAG_NAME(asteroidsTextureTag.name, Single_Texture_XML_Node);

      imageFile = asteroidsTextureTag.value;
      Locus::TrimString(imageFile);

      LoadAsteroidTexture(texturesPath + imageFile);

      ++numAsteroidTextures;
   }

   //add planet textures

   Locus::XMLTag* planetsTag = rootTag.FindSubTag(Planets_XML_Node, 0);
   CHECK_TAG(planetsTag, Planets_XML_Node);

   for (Locus::XMLTag& planetTextureTag : planetsTag->subTags)
   {
      CHECK_TAG_NAME(planetTextureTag.name, Single_Texture_XML_Node);

      imageFile = planetTextureTag.value;
      Locus::TrimString(imageFile);

      LoadPlanetTexture(texturesPath + imageFile);

      ++numPlanetTextures;
   }

   //add moon textures

   Locus::XMLTag* moonsTag = rootTag.FindSubTag(Moons_XML_Node, 0);
   CHECK_TAG(moonsTag, Moons_XML_Node);

   for (Locus::XMLTag& moonTextureTag : moonsTag->subTags)
   {
      CHECK_TAG_NAME(moonTextureTag.name, Single_Texture_XML_Node);

      imageFile = moonTextureTag.value;
      Locus::TrimString(imageFile);

      LoadMoonTexture(texturesPath + imageFile);

      ++numMoonTextures;
   }

   //add skybox and Hud textures

   typedef std::pair<std::string, bool> TextureNameAndClamp_t;

   TextureNameAndClamp_t skyboxAndHudTextures[] = { TextureNameAndClamp_t(TextureManager::Skybox_Front, true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Back, true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Left, true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Right, true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Up, true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Down, true),
                                                    TextureNameAndClamp_t(TextureManager::Shot_TextureName, false),
                                                    TextureNameAndClamp_t(TextureManager::Lives_Icon_TextureName, false),
                                                    TextureNameAndClamp_t(TextureManager::Lives_Times_TextureName, false),
                                                    TextureNameAndClamp_t(TextureManager::Score_Label_TextureName, false),
                                                    TextureNameAndClamp_t(TextureManager::Level_Label_TextureName, false),
                                                    TextureNameAndClamp_t(TextureManager::Ammo_TextureName, false) };

   for (const TextureNameAndClamp_t& textureNameAndClampValue : skyboxAndHudTextures)
   {
      Locus::XMLTag* skyboxOrHudTextureTag = rootTag.FindSubTag(textureNameAndClampValue.first, 0);
      CHECK_TAG(skyboxOrHudTextureTag, textureNameAndClampValue.first);

      imageFile = skyboxOrHudTextureTag->value;
      Locus::TrimString(imageFile);

      Load(textureNameAndClampValue.first, texturesPath + imageFile, Locus::Texture::MipmapGeneration::GLGenerateMipMap, Locus::TextureFiltering::Linear, textureNameAndClampValue.second);
   }

   //add digits textures

   std::string digitTextureName;

   for (unsigned int digit = 0; digit <= 9; ++digit)
   {
      digitTextureName = TextureManager::MakeDigitTextureName(digit);

      Locus::XMLTag* digitTextureTag = rootTag.FindSubTag(digitTextureName, 0);
      CHECK_TAG(digitTextureTag, digitTextureName);

      imageFile = digitTextureTag->value;
      Locus::TrimString(imageFile);

      Load(digitTextureName, texturesPath + imageFile, Locus::Texture::MipmapGeneration::GLGenerateMipMap, Locus::TextureFiltering::Linear, false);
   }

   #undef CHECK_TAG
   #undef CHECK_TAG_NAME
}

}