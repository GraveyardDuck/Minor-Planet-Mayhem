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

#include "CelestialObject.h"

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

class Planet : public CelestialObject
{
public:
   Planet(float r, unsigned int t, Locus::Mesh* mesh);

   float getRadius() const;

   void addMoon(std::unique_ptr<Moon> moon);

   void SetRandomTextures(const MPM::TextureManager& textureManager);

   void tick(double DT);
   void Draw(Locus::RenderingState& renderingState, const MPM::TextureManager& textureManager);

private:
   std::vector< std::unique_ptr<Moon> > moons;
   float radius;
};

}