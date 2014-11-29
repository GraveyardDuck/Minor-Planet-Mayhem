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

#include "Locus/Geometry/Vector3.h"
#include "Locus/Geometry/Collidable.h"
#include "Locus/Geometry/MotionProperties.h"
#include "Locus/Geometry/OrientedBox.h"

#include "Locus/Rendering/Color.h"
#include "Locus/Rendering/DefaultSingleDrawable.h"

//TODO: Remove magic numbers, either by putting in data files or use a scripting interface
#define SHOT_RADIUS 0.5f

namespace Locus
{

class Mesh;

}

namespace MPM
{

class Asteroid;

class Shot : public Locus::Collidable, public Locus::DefaultSingleDrawable
{
public:
   Shot(const Locus::Vector3& direction, const Locus::Vector3& position, Locus::Mesh* mesh);

   const Locus::Vector3& GetPosition() const;

   virtual void UpdateBroadCollisionExtent();

   virtual bool CollidesWith(Collidable& collidable) const override;
   virtual void ResolveCollision(Collidable& collidable) override;
   void ResolveCollision(Asteroid& asteroid);

   bool IsValid() const;

   void MoveAlongDirection(float units);

   virtual void UpdateGPUVertexData() override;
   virtual void Draw(Locus::RenderingState& renderingState) const override;

   //temp
   Locus::Color color;

private:
   bool valid;
   Locus::MotionProperties motionProerties;
   Locus::Vector3 position;
   Locus::OrientedBox collisionBox;

   Locus::Mesh* mesh;
};

}