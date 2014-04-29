 /*********************************************************************************************************\
 *                                                                                                        *
 *   This file is part of Minor Planet Mayhem                                                             *
 *                                                                                                        *
 *   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
 *                                                                                                        *
 *   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
 *                                                                                                        *
 \*********************************************************************************************************/

#include "TextureManager.h"

#include "Locus/Common/Parsing.h"

#include "Locus/FileSystem/FileSystemUtil.h"

#include "RapidXML/rapidxml.hpp"

#include "XML.h"

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
static const char* Moons_XML_Node = "Planets";
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
   Load(TextureManager::MakeAsteroidTextureName(numAsteroidTextures), textureLocation, false);
}

void TextureManager::LoadPlanetTexture(const std::string& textureLocation)
{
   Load(TextureManager::MakePlanetTextureName(numPlanetTextures), textureLocation, false);
}

void TextureManager::LoadMoonTexture(const std::string& textureLocation)
{
   Load(TextureManager::MakeMoonTextureName(numMoonTextures), textureLocation, false);
}

void TextureManager::LoadAsteroidTexture(const Locus::MountedFilePath& textureLocation)
{
   Load(TextureManager::MakeAsteroidTextureName(numAsteroidTextures), textureLocation, false);
}

void TextureManager::LoadPlanetTexture(const Locus::MountedFilePath& textureLocation)
{
   Load(TextureManager::MakePlanetTextureName(numPlanetTextures), textureLocation, false);
}

void TextureManager::LoadMoonTexture(const Locus::MountedFilePath& textureLocation)
{
   Load(TextureManager::MakeMoonTextureName(numMoonTextures), textureLocation, false);
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

   rapidxml::xml_document<> xmlDocument;
   std::vector<char> xmlContents;

   if (!ParseXMLFile(Locus::MountedFilePath("config/textures.config.xml"), xmlDocument, xmlContents))
   {
      throw std::runtime_error("Failed to read textures.config.xml");
   }

   Locus::MountedFilePath texturesPath("textures/");

   #define CHECK_NODE(node, name) if (node == nullptr) throw std::runtime_error(std::string("Failed to parse textures.config.xml. ") + name + " not found");

   rapidxml::xml_node<>* rootNode = xmlDocument.first_node(Textures_Config_XML_Root);
   CHECK_NODE(rootNode, Textures_Config_XML_Root)

   //add asteroid textures

   rapidxml::xml_node<>* asteroidsNode = rootNode->first_node(Asteroids_XML_Node);
   CHECK_NODE(asteroidsNode, Asteroids_XML_Node)

   rapidxml::xml_node<>* asteroidsTextureNode = asteroidsNode->first_node(Single_Texture_XML_Node);
   CHECK_NODE(asteroidsTextureNode, "Asteroids texture")

   std::string imageFile;

   do
   {
      imageFile = asteroidsTextureNode->value();
      Locus::TrimString(imageFile);

      LoadAsteroidTexture(texturesPath + imageFile);

      ++numAsteroidTextures;

      asteroidsTextureNode = asteroidsTextureNode->next_sibling(Single_Texture_XML_Node);

   } while (asteroidsTextureNode != nullptr);

   //add planet textures

   rapidxml::xml_node<>* planetsNode = rootNode->first_node(Planets_XML_Node);
   CHECK_NODE(planetsNode, Planets_XML_Node)

   rapidxml::xml_node<>* planetsTextureNode = planetsNode->first_node(Single_Texture_XML_Node);
   CHECK_NODE(planetsTextureNode, "Planets texture")

   do
   {
      imageFile = planetsTextureNode->value();
      Locus::TrimString(imageFile);

      LoadPlanetTexture(texturesPath + imageFile);

      ++numPlanetTextures;

      planetsTextureNode = planetsTextureNode->next_sibling(Single_Texture_XML_Node);

   } while (planetsTextureNode != nullptr);

   //add moon textures

   rapidxml::xml_node<>* moonsNode = rootNode->first_node(Moons_XML_Node);
   CHECK_NODE(moonsNode, Moons_XML_Node)

   rapidxml::xml_node<>* moonsTextureNode = moonsNode->first_node(Single_Texture_XML_Node);
   CHECK_NODE(moonsTextureNode, "Moons texture")

   do
   {
      imageFile = moonsTextureNode->value();
      Locus::TrimString(imageFile);

      LoadMoonTexture(texturesPath + imageFile);

      ++numMoonTextures;

      moonsTextureNode = moonsTextureNode->next_sibling(Single_Texture_XML_Node);

   } while (moonsTextureNode != nullptr);

   //add skybox and Hud textures

   typedef std::pair<const char*, bool> TextureNameAndClamp_t;

   TextureNameAndClamp_t skyboxAndHudTextures[] = { TextureNameAndClamp_t(TextureManager::Skybox_Front.c_str(), true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Back.c_str(), true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Left.c_str(), true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Right.c_str(), true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Up.c_str(), true),
                                                    TextureNameAndClamp_t(TextureManager::Skybox_Down.c_str(), true),
                                                    TextureNameAndClamp_t(TextureManager::Shot_TextureName.c_str(), false),
                                                    TextureNameAndClamp_t(TextureManager::Lives_Icon_TextureName.c_str(), false),
                                                    TextureNameAndClamp_t(TextureManager::Lives_Times_TextureName.c_str(), false),
                                                    TextureNameAndClamp_t(TextureManager::Score_Label_TextureName.c_str(), false),
                                                    TextureNameAndClamp_t(TextureManager::Level_Label_TextureName.c_str(), false),
                                                    TextureNameAndClamp_t(TextureManager::Ammo_TextureName.c_str(), false) };

   for (const TextureNameAndClamp_t& textureNameAndClampValue : skyboxAndHudTextures)
   {
      rapidxml::xml_node<>* skyboxOrHudTextureNode = rootNode->first_node(textureNameAndClampValue.first);
      CHECK_NODE(skyboxOrHudTextureNode, textureNameAndClampValue.first)

      imageFile = skyboxOrHudTextureNode->value();
      Locus::TrimString(imageFile);

      Load(textureNameAndClampValue.first, texturesPath + imageFile, textureNameAndClampValue.second);
   }

   //add digits textures

   std::string digitTextureName;

   for (unsigned int digit = 0; digit <= 9; ++digit)
   {
      digitTextureName = TextureManager::MakeDigitTextureName(digit);

      rapidxml::xml_node<>* digitTextureNode = rootNode->first_node(digitTextureName.c_str());
      CHECK_NODE(digitTextureNode, digitTextureName)

      imageFile = digitTextureNode->value();
      Locus::TrimString(imageFile);

      Load(digitTextureName, texturesPath + imageFile, false);
   }

   #undef CHECK_NODE
}

}