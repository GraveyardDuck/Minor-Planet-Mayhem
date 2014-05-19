/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "CelestialObject.h"

namespace MPM
{

CelestialObject::CelestialObject(unsigned int t, Locus::Mesh* mesh)
   : mesh(mesh)
{
   textureIndex = t;
}

}