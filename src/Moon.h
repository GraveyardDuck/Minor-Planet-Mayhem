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

#include "Locus/Geometry/Vector3.h"

#include "CelestialObject.h"

namespace Locus
{

class RenderingState;
class Mesh;

}

namespace MPM
{

class Moon : public CelestialObject
{
public:
   Moon(unsigned int t, float radius, float distanceFromPlanetCenter, float rotationSpeed, Locus::Mesh* mesh);

   void tick(double DT);
   void Draw(Locus::RenderingState& renderingState);

private:
   Locus::Vector3 rotationAxis;
   float rotationSpeed; //radians
};

}