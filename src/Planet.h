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

#include "Locus/Geometry/Moveable.h"

#include <vector>
#include <memory>

namespace Locus
{

class RenderingState;
class Mesh;

}

namespace MPM
{

class TextureManager;
class Moon;

class Planet : public Locus::Moveable
{
public:
   Planet(const Locus::Mesh* mesh, float radius, unsigned int textureIndex);

   float GetRadius() const;
   unsigned int GetTextureIndex() const;

   void AddMoon(std::unique_ptr<Moon> moon);

   void SetRandomTextures(const MPM::TextureManager& textureManager);

   void Tick(double DT);
   void Draw(Locus::RenderingState& renderingState, const MPM::TextureManager& textureManager) const;

private:
   const Locus::Mesh* mesh;
   std::vector< std::unique_ptr<Moon> > moons;
   float radius;
   unsigned int textureIndex;
};

}