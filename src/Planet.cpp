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

namespace MPM
{

Planet::Planet(float r, unsigned int t, Locus::Mesh* mesh) : CelestialObject(t, mesh), radius(r)
{
}

float Planet::getRadius() const
{
   return radius;
}

void Planet::addMoon(std::unique_ptr<Moon> moon)
{
   moons.push_back(std::move(moon));
}

void Planet::SetRandomTextures(const MPM::TextureManager& textureManager)
{
   Locus::Random random;

   textureIndex = static_cast<unsigned int>( random.RandomInt(0, static_cast<int>(textureManager.NumPlanetTextures()) - 1) );

   for (std::unique_ptr<Moon>& moon : moons)
   {
      moon->textureIndex = static_cast<unsigned int>( random.RandomInt(0, static_cast<int>(textureManager.NumMoonTextures()) - 1) );
   }
}

void Planet::tick(double DT)
{
   for (std::unique_ptr<Moon>& moon : moons)
   {
      moon->tick(DT);
   }
}

void Planet::Draw(Locus::RenderingState& renderingState, const MPM::TextureManager& textureManager)
{
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, CurrentModelTransformation());
   mesh->Draw(renderingState);

   renderingState.transformationStack.Translate(Position());

   for (std::unique_ptr<Moon>& moon : moons)
   {
      textureManager.GetTexture(MPM::TextureManager::MakeMoonTextureName(moon->textureIndex))->Bind();
      renderingState.shaderController.SetTextureUniform(Locus::ShaderSource::Map_Diffuse, 0);

      moon->Draw(renderingState);
   }
}

}