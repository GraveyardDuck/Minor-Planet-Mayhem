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

class Planet : public Locus::Moveable
{
public:
   Planet(const Locus::Mesh* mesh, float radius, unsigned int textureIndex);

   float GetRadius() const;
   unsigned int GetTextureIndex() const;

   void RandomizeTexture(const MPM::TextureManager& textureManager);

   void Draw(Locus::RenderingState& renderingState) const;

private:
   const Locus::Mesh* mesh;
   float radius;
   unsigned int textureIndex;
};

}