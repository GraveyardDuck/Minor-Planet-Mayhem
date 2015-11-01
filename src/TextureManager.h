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

#include "Locus/Rendering/TextureManager.h"

#include <cstddef>

namespace Locus
{

struct MountedFilePath;

}

namespace MPM
{

class TextureManager : public Locus::TextureManager
{
public:
   TextureManager(const Locus::GLInfo& glInfo);

   static const std::string Skybox_Front;
   static const std::string Skybox_Back;
   static const std::string Skybox_Left;
   static const std::string Skybox_Right;
   static const std::string Skybox_Up;
   static const std::string Skybox_Down;
   static const std::string Shot_TextureName;
   static const std::string Lives_Icon_TextureName;
   static const std::string Lives_Times_TextureName;
   static const std::string Score_Label_TextureName;
   static const std::string Level_Label_TextureName;
   static const std::string Ammo_TextureName;

   static std::string MakeAsteroidTextureName(std::size_t index);
   static std::string MakePlanetTextureName(std::size_t index);
   static std::string MakeDigitTextureName(std::size_t index);

   virtual void UnLoad() override;
   void LoadAllTextures();

   std::size_t NumAsteroidTextures() const;
   std::size_t NumPlanetTextures() const;

private:
   static const std::string Asteroid_Base_TextureName;
   static const std::string Planet_Base_TextureName;
   static const std::string Digit_Base_TextureName;

   std::size_t numAsteroidTextures;
   std::size_t numPlanetTextures;

   void LoadAsteroidTexture(const std::string& textureLocation);
   void LoadPlanetTexture(const std::string& textureLocation);

   void LoadAsteroidTexture(const Locus::MountedFilePath& textureLocation);
   void LoadPlanetTexture(const Locus::MountedFilePath& textureLocation);
};

}