/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "Shot.h"
#include "Asteroid.h"
#include "CollidableTypes.h"

#include "Locus/Geometry/Quaternion.h"

#include "Locus/Rendering/Mesh.h"
#include "Locus/Rendering/DefaultGPUVertexData.h"
#include "Locus/Rendering/RenderingState.h"

#include <Locus/Rendering/Locus_glew.h>

namespace MPM
{

Shot::Shot(const Locus::Vector3& direction, const Locus::Vector3& position, Locus::Mesh* mesh)
   : valid(true), position(position), collisionBox(position, 2 * SHOT_RADIUS, 2 * SHOT_RADIUS, 2 * SHOT_RADIUS), mesh(mesh)
{
   collidableType = CollidableType_Shot;

   motionProerties.direction = direction;

   const Locus::Vector3& zAxis = Locus::Vector3::ZAxis();
   Locus::Quaternion inverseRotation(direction.cross(zAxis), zAxis.angleBetweenRadians(direction));

   collisionBox.SetRotationInverse( inverseRotation.ToTransformation() );
}

const Locus::Vector3& Shot::GetPosition() const
{
   return position;
}

bool Shot::IsValid() const
{
   return valid;
}

void Shot::MoveAlongDirection(float units)
{
   Locus::Vector3 oldPosition = position;

   position += units * motionProerties.direction;

   collisionBox.centroid = (position + oldPosition) / 2 ;
   collisionBox.SetZLength( (position - oldPosition).norm() + 2 * SHOT_RADIUS );
}

void Shot::UpdateBroadCollisionExtent()
{
   Collidable::UpdateBroadCollisionExtent(position, collisionBox.DiagonalLength() / 2);
}

bool Shot::CollidesWith(Collidable& collidable) const
{
   return (collidable.GetCollidableType() == CollidableType_Asteroid);
}

void Shot::ResolveCollision(Collidable& collidable)
{
   ResolveCollision( dynamic_cast<Asteroid&>(collidable) );
}

void Shot::ResolveCollision(Asteroid& asteroid)
{
   std::unordered_set<std::size_t> asteroidIntersectionSet;

   asteroid.GetBoundingVolumeHierarchy().GetIntersection(asteroid, collisionBox, asteroidIntersectionSet);

   if (asteroidIntersectionSet.size() > 0)
   {
      Locus::Transformation asteroidTransformation = asteroid.CurrentModelTransformation();

      for (std::size_t triangleIndex : asteroidIntersectionSet)
      {
         if (collisionBox.Intersects(asteroid.GetFaceTriangle(triangleIndex, asteroidTransformation)))
         {
            asteroid.RegisterHit(position);
            valid = false;

            return;
         }
      }
   }
}

void Shot::UpdateGPUVertexData()
{
   if ((defaultGPUVertexData != nullptr) && (mesh != nullptr))
   {
      std::size_t numTotalVertices = mesh->NumFaces() * Locus::Triangle3D_t::NumPointsOnATriangle;

      defaultGPUVertexData->Bind();
      defaultGPUVertexData->Buffer(numTotalVertices, GL_STATIC_DRAW);

      Locus::GPUVertexDataStorage colorAsGPUVertexDataStorage;
      Locus::ZeroFill(colorAsGPUVertexDataStorage);

      colorAsGPUVertexDataStorage.color[0] = color.r;
      colorAsGPUVertexDataStorage.color[1] = color.g;
      colorAsGPUVertexDataStorage.color[2] = color.b;
      colorAsGPUVertexDataStorage.color[3] = color.a;

      std::vector<Locus::GPUVertexDataStorage> vertData(numTotalVertices, colorAsGPUVertexDataStorage);

      defaultGPUVertexData->BufferSub(0, numTotalVertices, vertData.data());

      defaultGPUVertexData->transferInfo.sendPositions = false;
      defaultGPUVertexData->transferInfo.sendColors = true;
      defaultGPUVertexData->transferInfo.sendNormals = false;
      defaultGPUVertexData->transferInfo.sendTexCoords = false;

      defaultGPUVertexData->drawMode = GL_TRIANGLES;
   }
}

void Shot::Draw(Locus::RenderingState& renderingState) const
{
   if ((gpuVertexData != nullptr) && (mesh != nullptr))
   {
      gpuVertexData->PreDraw(renderingState.shaderController);
      mesh->Draw(renderingState);
   }
}

}