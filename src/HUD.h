 /*********************************************************************************************************\
 *                                                                                                        *
 *   This file is part of Minor Planet Mayhem                                                             *
 *                                                                                                        *
 *   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
 *                                                                                                        *
 *   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
 *                                                                                                        *
 \*********************************************************************************************************/

#pragma once

#include "Locus/Rendering/Drawable.h"
#include "Locus/Rendering/LineCollection.h"
#include "Locus/Rendering/Quad.h"

#include "TextureManager.h"

namespace MPM
{

class HUD : public Locus::Drawable
{
public:
   HUD();

   void Initialize(MPM::TextureManager* textureManager, std::size_t maxShots, unsigned int resolutionX, unsigned int resolutionY);
   void Update(int score, int level, int lives, std::size_t currentShots, int crosshairsX, int crosshairsY, int fps);

   virtual void CreateGPUVertexData() override;
   virtual void DeleteGPUVertexData() override;
   virtual void UpdateGPUVertexData() override;

   virtual void Draw(Locus::RenderingState& renderingState) const override;

private:
   MPM::TextureManager* textureManager;

   unsigned int resolutionX;
   unsigned int resolutionY;

   int score;
   int level;
   int lives;
   std::size_t currentShots;
   int crosshairsX;
   int crosshairsY;
   std::size_t maxShots;

   //object positions
   float topStripY;
   float bottomStripY;
   float scoreX;
   float levelX;
   float ammoBoxX;
   float digitWidth;
   float ammoWidth;
   float fpsX;

   int fps;

   static const Locus::Color QuadTextureColor;

   static const float ammoPadding;

   Locus::LineCollection crosshairs;
   Locus::Quad livesIcon;
   Locus::Quad livesTimes;
   Locus::Quad livesQuad;
   Locus::Quad scoreLabel;
   Locus::Quad levelLabel;
   Locus::Quad digit;
   Locus::Quad ammo;

   void DrawDigits(Locus::RenderingState& renderingState, int value, int numDigits, float x, float y) const;
   void DrawAmmo(Locus::RenderingState& renderingState) const;
};

}