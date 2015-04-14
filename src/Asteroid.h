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

#include "Locus/Math/Vectors.h"

#include "Locus/Geometry/Moveable.h"
#include "Locus/Geometry/Collidable.h"
#include "Locus/Geometry/MotionProperties.h"
#include "Locus/Geometry/TriangleFwd.h"
#include "Locus/Geometry/BoundingVolumeHierarchy.h"

#include "Locus/Rendering/Mesh.h"

#include <chrono>

namespace Locus
{

class RenderingState;
class Texture;
class SoundEffect;

}

namespace MPM
{

class Asteroid : public Locus::Mesh, public Locus::Collidable
{
public:
   bool visible;

   Locus::MotionProperties motionProperties;

   //HACK: avoiding interpenetration
   Locus::Collidable* lastCollision;
   std::chrono::high_resolution_clock::time_point lastCollisionTime;

   static std::unique_ptr< Locus::SoundEffect > asteroidAsteroidCollsionSoundEffect;

   Asteroid();
   Asteroid(int h);
   Asteroid(const Asteroid& other);
   Asteroid& operator=(const Asteroid& other);

   //getters
   Locus::Texture* GetTexture();
   int getHitsLeft();

   const Locus::SphereTree_t& GetBoundingVolumeHierarchy() const;

   //setters
   void SetTexture(Locus::Texture* texture);

   void GrabMesh(const Mesh& mesh);
   void GrabMeshAndCollidable(const Asteroid& other);

   virtual void UpdateBroadCollisionExtent();

   void CreateBoundingVolumeHierarchy();

   bool GetAsteroidIntersection(Asteroid& other, Locus::Triangle3D_t& intersectingTriangle1, Locus::Triangle3D_t& intersectingTriangle2);

   virtual void ResolveCollision(Collidable& collidable) override;
   void ResolveCollision(Asteroid& otherAsteroid);

   //asteroid logic functions

   bool WasHit() const;
   void RegisterHit(const Locus::FVector3& hitLocation);
   const Locus::FVector3& GetHitLocation() const;

   void decreaseHitsLeft();
   void negateDirection();
   void negateXDirection();
   void negateYDirection();
   void negateZDirection();

   void tick(double DT);

private:
   Locus::Texture* texture;
   int hitsLeft;

   bool hit;
   Locus::FVector3 hitLocation;

   std::unique_ptr< Locus::SphereTree_t > boundingVolumeHierarchy;
};

}