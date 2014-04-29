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

#include "Locus/Geometry/Moveable.h"

namespace Locus
{

class Mesh;

}

namespace MPM
{

class CelestialObject : public Locus::Moveable
{
public:
   CelestialObject(unsigned int t, Locus::Mesh* mesh);

   unsigned int textureIndex;

protected:
   Locus::Mesh* mesh;
};

}