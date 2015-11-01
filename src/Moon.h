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

#include "Locus/Math/Vectors.h"

#include "Locus/Geometry/Moveable.h"

namespace Locus
{

class RenderingState;
class Mesh;

}

namespace MPM
{

class TextureManager;

class Moon : public Locus::Moveable
{
public:
   Moon(const Locus::Mesh* mesh, float radius, unsigned int textureIndex, float distanceFromPlanetCenter, float rotationSpeed);

   unsigned int GetTextureIndex() const;

   void SetRandomTexture(const MPM::TextureManager& textureManager);

   void Tick(double DT);
   void Draw(Locus::RenderingState& renderingState) const;

private:
   const Locus::Mesh* mesh;
   float radius;
   unsigned int textureIndex;
   Locus::FVector3 rotationAxis;
   float rotationSpeed; //radians
};

}