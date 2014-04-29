 /*********************************************************************************************************\
 *                                                                                                        *
 *   This file is part of Minor Planet Mayhem                                                             *
 *                                                                                                        *
 *   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
 *                                                                                                        *
 *   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
 *                                                                                                        *
 \*********************************************************************************************************/

#include "Moon.h"

#include "Locus/Rendering/Mesh.h"
#include "Locus/Rendering/RenderingState.h"
#include "Locus/Rendering/DrawUtility.h"

#include "Locus/Common/Random.h"

#include <math.h>

namespace MPM
{

Moon::Moon(unsigned int t, float radius, float distanceFromPlanetCenter, float rotationSpeed, Locus::Mesh* mesh)
   : CelestialObject(t, mesh), rotationSpeed(rotationSpeed)
{
   Locus::Random random;

   rotationAxis.x = static_cast<float>(random.randomDouble(-1.0, 1.0));
   rotationAxis.y = static_cast<float>(random.randomDouble(-1.0, 1.0));
   rotationAxis.z = static_cast<float>(random.randomDouble(-1.0, 1.0));

   rotationAxis.normalize();

   Locus::Vector3 axisOrthogonalToRotationAxis;
   axisOrthogonalToRotationAxis.x = static_cast<float>(random.randomDouble(-1.0, 1.0));
   axisOrthogonalToRotationAxis.y = static_cast<float>(random.randomDouble(-1.0, 1.0));

   //set z such that the dot product is zero
   axisOrthogonalToRotationAxis.z = (-rotationAxis.x * axisOrthogonalToRotationAxis.x - rotationAxis.y * axisOrthogonalToRotationAxis.y) / rotationAxis.z;
   axisOrthogonalToRotationAxis.normalize();

   Locus::Vector3 position = axisOrthogonalToRotationAxis * distanceFromPlanetCenter;

   Translate(position);
   Scale( Locus::Vector3(radius, radius, radius) );
}

void Moon::tick(double DT)
{
   Locus::Vector3 currentPosition = CurrentTranslation();
   currentPosition.rotateAround(rotationAxis, rotationSpeed * static_cast<float>(DT));

   Translate(currentPosition - CurrentTranslation());
}

void Moon::Draw(Locus::RenderingState& renderingState)
{
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, CurrentModelTransformation());
   mesh->Draw(renderingState);
}

}