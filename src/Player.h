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

#include "Locus/Geometry/Collidable.h"
#include "Locus/Geometry/Model.h"
#include "Locus/Geometry/MotionProperties.h"

#include "Locus/Rendering/Viewpoint.h"

#include "Locus/Audio/SoundEffect.h"

#include "Locus/Geometry/BoundingVolumeHierarchy.h"

#include <memory>
#include <string>

namespace MPM
{

class Asteroid;

class Player : public Locus::Collidable
{
public:
   Player();

   void tick(double DT);

   bool translateAhead;
   bool translateBack;
   bool translateRight;
   bool translateLeft;
   bool translateUp;
   bool translateDown;

   void SetModel(float radius);

   virtual void Rotate(const Locus::Vector3& rotation);

   virtual void UpdateBroadCollisionExtent();

   virtual bool CollidesWith(Locus::Collidable& collidable) const override;
   virtual void ResolveCollision(Locus::Collidable& collidable) override;
   void ResolveCollision(Asteroid& asteroid);

   void LoadCollisionSoundEffect(const std::string& pathToSoundEffect);

   Locus::Viewpoint viewpoint;

private:
   Locus::Model_t model;

   std::unique_ptr< Locus::SphereTree_t > boundingVolumeHierarchy;

   Locus::MotionProperties motionProperties;

   std::unique_ptr< Locus::SoundEffect > collisionSoundEffect;
};

}