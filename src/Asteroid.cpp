/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "Asteroid.h"
#include "Player.h"
#include "CollidableTypes.h"
#include "Shot.h"
#include "Config.h"

#include "Locus/Geometry/Geometry.h"
#include "Locus/Geometry/Plane.h"
#include "Locus/Geometry/Triangle.h"

#include "Locus/Common/Random.h"

#include "Locus/Rendering/RenderingState.h"

namespace MPM
{

std::unique_ptr< Locus::SoundEffect > Asteroid::asteroidAsteroidCollsionSoundEffect;

Asteroid::Asteroid()
   : Asteroid(0)
{
}

Asteroid::Asteroid(int h)
   : visible(false), lastCollision(nullptr), texture(nullptr), hitsLeft(h), hit(false)
{
   collidableType = CollidableType_Asteroid;
}

Asteroid::Asteroid(const Asteroid& other)
   :
   visible(other.visible),
   lastCollision(other.lastCollision),
   texture(other.texture),
   hitsLeft(other.hitsLeft),
   hit(other.hit),
   hitLocation(other.hitLocation),
   boundingVolumeHierarchy( std::make_unique<Locus::SphereTree_t>(*other.boundingVolumeHierarchy) )
{
}

Asteroid& Asteroid::operator=(const Asteroid& other)
{
   if (this != &other)
   {
      texture = other.texture;
      hitsLeft = other.hitsLeft;

      hit = other.hit;
      hitLocation = other.hitLocation;

      boundingVolumeHierarchy = std::make_unique<Locus::SphereTree_t>(*other.boundingVolumeHierarchy);

      visible = other.visible;
      lastCollision = other.lastCollision;
      lastCollisionTime = other.lastCollisionTime;
   }

   return *this;
}

Locus::Texture* Asteroid::GetTexture()
{
   return texture;
}

int Asteroid::getHitsLeft()
{
   return hitsLeft;
}

const Locus::SphereTree_t& Asteroid::GetBoundingVolumeHierarchy() const
{
   return *boundingVolumeHierarchy;
}

void Asteroid::SetTexture(Locus::Texture* texture)
{
   this->texture = texture;
}

void Asteroid::GrabMesh(const Mesh& mesh)
{
   Mesh::operator=(mesh);
}

void Asteroid::GrabMeshAndCollidable(const Asteroid& other)
{
   GrabMesh(other);
   Collidable::operator=(other);

   boundingVolumeHierarchy = std::make_unique<Locus::SphereTree_t>(*other.boundingVolumeHierarchy);
}

//////////////////////////////////////Asteroid logic//////////////////////////////////////////

void Asteroid::decreaseHitsLeft()
{
   hitsLeft--;
}

void Asteroid::negateDirection()
{
   //invert the current direction of the asteroid

   negateXDirection();
   negateYDirection();
   negateZDirection();
}

void Asteroid::negateXDirection()
{
   motionProperties.direction.x = -motionProperties.direction.x;
}

void Asteroid::negateYDirection()
{
   motionProperties.direction.y = -motionProperties.direction.y;
}

void Asteroid::negateZDirection()
{
   motionProperties.direction.z = -motionProperties.direction.z;
}

void Asteroid::UpdateBroadCollisionExtent()
{
   Collidable::UpdateBroadCollisionExtent(centroid, maxDistanceToCenter);
}

void Asteroid::CreateBoundingVolumeHierarchy()
{
   boundingVolumeHierarchy = std::make_unique<Locus::SphereTree_t>(*this, 6);
}

bool Asteroid::GetAsteroidIntersection(Asteroid& other,  Locus::Triangle3D_t& intersectingTriangle1, Locus::Triangle3D_t& intersectingTriangle2)
{
   std::unordered_set<std::size_t> thisIntersectionSet;
   std::unordered_set<std::size_t> otherIntersectionSet;

   boundingVolumeHierarchy->GetIntersection(*this, *other.boundingVolumeHierarchy, other, thisIntersectionSet, otherIntersectionSet);

   if ((thisIntersectionSet.size() > 0) && (otherIntersectionSet.size() > 0))
   {
      return GetResolvedCollision(other, CurrentModelTransformation(), other.CurrentModelTransformation(), thisIntersectionSet, otherIntersectionSet, intersectingTriangle1, intersectingTriangle2);
   }
   else
   {
      return false;
   }
}

void Asteroid::ResolveCollision(Collidable& collidable)
{
   if (collidable.GetCollidableType() == CollidableType_Asteroid)
   {
      dynamic_cast<Asteroid&>(collidable).ResolveCollision(*this);
   }
   else if (collidable.GetCollidableType() == CollidableType_Shot)
   {
      dynamic_cast<Shot&>(collidable).ResolveCollision(*this);
   }
   else if (collidable.GetCollidableType() == CollidableType_Player)
   {
      dynamic_cast<Player&>(collidable).ResolveCollision(*this);
   }
}

void Asteroid::ResolveCollision(Asteroid& otherAsteroid)
{
   std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

   if (lastCollision == &otherAsteroid)
   {
      std::chrono::high_resolution_clock::duration diff = now - lastCollisionTime;
      if (std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() < 500)
      {
         return;
      }
   }

   if (otherAsteroid.lastCollision == this)
   {
      std::chrono::high_resolution_clock::duration diff = now - otherAsteroid.lastCollisionTime;
      if (std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() < 500)
      {
         return;
      }
   }

   Locus::Triangle3D_t intersectingTriangle1, intersectingTriangle2;

   if ( GetAsteroidIntersection(otherAsteroid, intersectingTriangle1, intersectingTriangle2) )
   {
      Locus::Vector3 collisionPoint = Locus::Triangle3D_t::ComputeCentroid(intersectingTriangle1, intersectingTriangle2);

      Locus::Vector3 impulseDirection = intersectingTriangle1.Normal().normVector();

      Locus::ResolveCollision(1.0f, BoundingSphere(), otherAsteroid.BoundingSphere(), collisionPoint, impulseDirection,
                               motionProperties, otherAsteroid.motionProperties);

      lastCollision = &otherAsteroid;
      otherAsteroid.lastCollision = this;

      now = std::chrono::high_resolution_clock::now();

      lastCollisionTime = now;
      otherAsteroid.lastCollisionTime = now;

      if (Asteroid::asteroidAsteroidCollsionSoundEffect != nullptr)
      {
         Asteroid::asteroidAsteroidCollsionSoundEffect->Play();
      }
   }
}

bool Asteroid::WasHit() const
{
   return hit;
}

void Asteroid::RegisterHit(const Locus::Vector3& hitLocation)
{
   this->hitLocation = hitLocation;
   hit = true;
}

const Locus::Vector3& Asteroid::GetHitLocation() const
{
   return hitLocation;
}

void Asteroid::tick(double DT)
{
   Translate((motionProperties.speed * motionProperties.direction) * static_cast<float>(DT));
   Rotate(motionProperties.angularSpeed * static_cast<float>(DT) * motionProperties.rotation);
}

}