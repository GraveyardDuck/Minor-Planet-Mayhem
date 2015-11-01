/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "Planet.h"
#include "TextureManager.h"

#include "Locus/Rendering/Mesh.h"
#include "Locus/Rendering/RenderingState.h"
#include "Locus/Rendering/DrawUtility.h"
#include "Locus/Rendering/ShaderVariables.h"
#include "Locus/Rendering/Texture.h"

#include "Locus/Common/Random.h"

#include <cassert>

namespace MPM
{

Planet::Planet(const Locus::Mesh* mesh, float radius, unsigned int textureIndex)
   : mesh(mesh), radius(radius), textureIndex(textureIndex)
{
   assert(mesh != nullptr);
}

float Planet::GetRadius() const
{
   return radius;
}

unsigned int Planet::GetTextureIndex() const
{
   return textureIndex;
}

void Planet::RandomizeTexture(const MPM::TextureManager& textureManager)
{
   textureIndex = static_cast<unsigned int>( Locus::Random().RandomInt(0, static_cast<int>(textureManager.NumPlanetTextures()) - 1) );
}

void Planet::Draw(Locus::RenderingState& renderingState) const
{
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, CurrentModelTransformation());
   mesh->Draw(renderingState);
}

}