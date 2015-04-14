/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "Player.h"
#include "Asteroid.h"
#include "CollidableTypes.h"
#include "Config.h"

#include "Locus/Geometry/Geometry.h"
#include "Locus/Geometry/ModelUtility.h"
#include "Locus/Geometry/Vector3Geometry.h"

#include "Locus/FileSystem/MountedFilePath.h"

#include <cmath>

#define RESTRICT_CAMERA true
//#define CAMERA_BOUNDARY ASTEROIDS_BOUNDARY

namespace MPM
{

Player::Player()
   : translateAhead(false), translateBack(false), translateRight(false),
     translateLeft(false), translateUp(false), translateDown(false)
{
   collidableType = CollidableType_Player;
}

void Player::SetModel(float radius)
{
   model = Locus::ModelUtility::MakeCube(2 * radius);

   boundingVolumeHierarchy = std::make_unique<Locus::SphereTree_t>(model, 6);

   model.UpdateMaxDistanceToCenter();

   model.Reset(viewpoint.GetPosition(), viewpoint.GetRotation(), Locus::Transformation::IdentityScale());
}

void Player::Rotate(const Locus::FVector3& rotation)
{
   viewpoint.RotateBy(rotation);
   model.Rotate(rotation);
}

void Player::LoadCollisionSoundEffect(const std::string& pathToSoundEffect)
{
   collisionSoundEffect = std::make_unique<Locus::SoundEffect>();

   collisionSoundEffect->Load(Locus::MountedFilePath(pathToSoundEffect));
}

void Player::UpdateBroadCollisionExtent()
{
   Collidable::UpdateBroadCollisionExtent(viewpoint.GetPosition(), model.GetMaxDistanceToCenter());
}

bool Player::CollidesWith(Collidable& collidable) const
{
   return (collidable.GetCollidableType() == CollidableType_Asteroid);
}

void Player::ResolveCollision(Collidable& collidable)
{
   ResolveCollision( dynamic_cast<Asteroid&>(collidable) );
}

void Player::ResolveCollision(Asteroid& asteroid)
{
   std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();

   if (asteroid.lastCollision == this)
   {
      std::chrono::high_resolution_clock::duration diff = now - asteroid.lastCollisionTime;
      if (std::chrono::duration_cast<std::chrono::milliseconds>(diff).count() < 500)
      {
         return;
      }
   }

   std::unordered_set<std::size_t> thisIntersectionSet;
   std::unordered_set<std::size_t> asteroidIntersectionSet;

   boundingVolumeHierarchy->GetIntersection(model, asteroid.GetBoundingVolumeHierarchy(), asteroid, thisIntersectionSet, asteroidIntersectionSet);

   if ((thisIntersectionSet.size() > 0) && (asteroidIntersectionSet.size() > 0))
   {
      Locus::Triangle3D_t thisIntersectingTriangle;
      Locus::Triangle3D_t asteroidTriangle;

      if (model.GetResolvedCollision(asteroid, model.CurrentModelTransformation(), asteroid.CurrentModelTransformation(), thisIntersectionSet, asteroidIntersectionSet, thisIntersectingTriangle, asteroidTriangle))
      {
         Locus::FVector3 collisionPoint = (viewpoint.GetPosition() + asteroid.centroid) / 2.0f;
         Locus::FVector3 impulseDirection = NormVector(asteroid.centroid - viewpoint.GetPosition());

         Locus::ResolveCollision(1.0f, model.BoundingSphere(), asteroid.BoundingSphere(), collisionPoint, impulseDirection,
                                 motionProperties, asteroid.motionProperties);

         asteroid.lastCollision = this;
         asteroid.lastCollisionTime = std::chrono::high_resolution_clock::now();

         if (collisionSoundEffect != nullptr)
         {
            collisionSoundEffect->Play();
         }
      }
   }
}

void Player::tick(double DT)
{
   //translate the player based on the user's control key input
   //rotation is done directly when the game responds to the
   //user's mouse movements

   Locus::FVector3 translation
   ( 
      (translateLeft  ? -Config::GetPlayerTranslationSpeed() : 0.0f) + (translateRight ?  Config::GetPlayerTranslationSpeed() : 0.0f),
      (translateDown  ? -Config::GetPlayerTranslationSpeed() : 0.0f) + (translateUp    ?  Config::GetPlayerTranslationSpeed() : 0.0f),
      (translateAhead ? -Config::GetPlayerTranslationSpeed() : 0.0f) + (translateBack  ?  Config::GetPlayerTranslationSpeed() : 0.0f)
   );

   bool move = false;

   Locus::FVector3 originalPosition = viewpoint.GetPosition();
   Locus::FVector3 newPosition;

   if (!ApproximatelyEqual(translation, Locus::Vec3D::ZeroVector()))
   {
      translation *= static_cast<float>(DT);

      newPosition = originalPosition;

      newPosition += translation.x * viewpoint.GetRight();
      newPosition += translation.y  * viewpoint.GetUp();
      newPosition += -translation.z  * viewpoint.GetForward();

      //camera is bound within a cube with length 2*CAMERA_BOUNDARY centered at the origin
      float boundary = Config::GetAsteroidsBoundary();

      bool inBoundary = (std::abs((int)newPosition.x) <= boundary) && (std::abs((int)newPosition.y) <= boundary) && (std::abs((int)newPosition.z) <= boundary);

      move = inBoundary || !(RESTRICT_CAMERA);
   }

   if (move)
   {
      viewpoint.TranslateBy(translation); //meters = meters per second * second

      motionProperties.direction = (newPosition - originalPosition);
      motionProperties.speed = Config::GetPlayerTranslationSpeed();

      model.Translate(motionProperties.direction);

      Normalize(motionProperties.direction);

      UpdateBroadCollisionExtent();
   }
   else
   {
      motionProperties.direction = Locus::Vec3D::ZeroVector();
      motionProperties.speed = 0.0f;
   }
}

}