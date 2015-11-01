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
#include "Moon.h"
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

void Planet::AddMoon(std::unique_ptr<Moon> moon)
{
   assert(moon != nullptr);

   moons.push_back(std::move(moon));
}

void Planet::SetRandomTextures(const MPM::TextureManager& textureManager)
{
   Locus::Random random;

   textureIndex = static_cast<unsigned int>( random.RandomInt(0, static_cast<int>(textureManager.NumPlanetTextures()) - 1) );

   for (std::unique_ptr<Moon>& moon : moons)
   {
      moon->SetRandomTexture(textureManager);
   }
}

void Planet::Tick(double DT)
{
   for (std::unique_ptr<Moon>& moon : moons)
   {
      moon->Tick(DT);
   }
}

void Planet::Draw(Locus::RenderingState& renderingState, const MPM::TextureManager& textureManager) const
{
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, CurrentModelTransformation());
   mesh->Draw(renderingState);

   renderingState.transformationStack.Translate(Position());

   for (const std::unique_ptr<Moon>& moon : moons)
   {
      textureManager.GetTexture(MPM::TextureManager::MakeMoonTextureName(moon->GetTextureIndex()))->Bind();
      renderingState.shaderController.SetTextureUniform(Locus::ShaderSource::Map_Diffuse, 0);

      moon->Draw(renderingState);
   }
}

}