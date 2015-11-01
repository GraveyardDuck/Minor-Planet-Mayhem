/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "Moon.h"
#include "TextureManager.h"

#include "Locus/Rendering/Mesh.h"
#include "Locus/Rendering/RenderingState.h"
#include "Locus/Rendering/DrawUtility.h"

#include "Locus/Common/Random.h"

#include <cassert>

namespace MPM
{

Moon::Moon(const Locus::Mesh* mesh, float radius, unsigned int textureIndex, float distanceFromPlanetCenter, float rotationSpeed)
   : mesh(mesh), radius(radius), textureIndex(textureIndex), rotationSpeed(rotationSpeed)
{
   assert(mesh != nullptr);

   Locus::Random random;

   rotationAxis.x = static_cast<float>(random.RandomDouble(-1.0, 1.0));
   rotationAxis.y = static_cast<float>(random.RandomDouble(-1.0, 1.0));
   rotationAxis.z = static_cast<float>(random.RandomDouble(-1.0, 1.0));

   Normalize(rotationAxis);

   Locus::FVector3 axisOrthogonalToRotationAxis;
   axisOrthogonalToRotationAxis.x = static_cast<float>(random.RandomDouble(-1.0, 1.0));
   axisOrthogonalToRotationAxis.y = static_cast<float>(random.RandomDouble(-1.0, 1.0));

   //set z such that the dot product is zero
   axisOrthogonalToRotationAxis.z = (-rotationAxis.x * axisOrthogonalToRotationAxis.x - rotationAxis.y * axisOrthogonalToRotationAxis.y) / rotationAxis.z;
   Normalize(axisOrthogonalToRotationAxis);

   Locus::FVector3 position = axisOrthogonalToRotationAxis * distanceFromPlanetCenter;

   Translate(position);
   Scale( Locus::FVector3(radius, radius, radius) );
}

unsigned int Moon::GetTextureIndex() const
{
   return textureIndex;
}

void Moon::SetRandomTexture(const MPM::TextureManager& textureManager)
{
   textureIndex = static_cast<unsigned int>( Locus::Random().RandomInt(0, static_cast<int>(textureManager.NumMoonTextures()) - 1) );
}

void Moon::Tick(double DT)
{
   Locus::FVector3 currentPosition = CurrentTranslation();
   RotateAround(currentPosition, rotationAxis, rotationSpeed * static_cast<float>(DT));

   Translate(currentPosition - CurrentTranslation());
}

void Moon::Draw(Locus::RenderingState& renderingState) const
{
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, CurrentModelTransformation());
   mesh->Draw(renderingState);
}

}