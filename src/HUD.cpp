/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "HUD.h"

#include "Locus/Geometry/Geometry.h"
#include "Locus/Geometry/Vector3Geometry.h"

#include "Locus/Rendering/RenderingState.h"
#include "Locus/Rendering/ShaderController.h"
#include "Locus/Rendering/ShaderVariables.h"
#include "Locus/Rendering/Texture.h"

#include <Locus/Rendering/Locus_glew.h>

#include "TextureManager.h"

#include <cmath>

//TODO: Draw HUD independently of screen resolution

//HUD Constants
#define HUD_NUM_SCORE_DIGITS 11
#define HUD_NUM_LEVEL_DIGITS 2
#define HUD_NUM_FPS_DIGITS 4

namespace MPM
{

const Locus::Color HUD::QuadTextureColor(255, 255, 255, 204);
const float HUD::ammoPadding = 2.0f;

HUD::HUD()
   :  resolutionX(0),
      resolutionY(0),
      notTexturedNotLitProgramID(Locus::BAD_ID),
      texturedNotLitProgramID(Locus::BAD_ID),
      score(0),
      level(0),
      lives(0),
      currentShots(0),
      crosshairsX(0),
      crosshairsY(0),
      maxShots(0),
      topStripY(0.0f),
      bottomStripY(0.0f),
      scoreX(0.0f),
      levelX(0.0f),
      ammoBoxX(0.0f),
      digitWidth(0.0f),
      ammoWidth(0.0f)
{
}

void HUD::Initialize(MPM::TextureManager* textureManager, std::size_t maxShots, unsigned int resolutionX, unsigned int resolutionY)
{
   this->textureManager = textureManager;
   this->maxShots = maxShots;
   this->resolutionX = resolutionX;
   this->resolutionY = resolutionY;

   //make crosshair circle
   const int crosshairsWidth = static_cast<int>( (15.0f/800) * resolutionX );

   Locus::LineSegmentCollection::ColoredLineSegment lineSegment;

   lineSegment.color = Locus::Color::White();

   bool makeLine = false;

   double increment = 0.2;

   const std::size_t NUM_CROSSHAIR_LINE_SEGMENTS = ((360 * 5) + 4);

   crosshairs.lineSegments.clear();
   crosshairs.lineSegments.reserve(NUM_CROSSHAIR_LINE_SEGMENTS);

   for (int i = 0; i < 360; ++i)
   {
      for (int j = 1; j <= 5; ++j)
      {
         float angle = static_cast<float>( (i + j * increment) * Locus::TO_RADIANS );
         lineSegment.segment.P2 = Locus::FVector3(std::sin(angle) * crosshairsWidth, std::cos(angle) * crosshairsWidth, 0.0);

         if (makeLine)
         {
            crosshairs.lineSegments.push_back(lineSegment);
         }

         lineSegment.segment.P1 = lineSegment.segment.P2;
         makeLine = true;
      }
   }

   float firstAngle = static_cast<float>( increment * Locus::TO_RADIANS );

   lineSegment.segment.P1 = lineSegment.segment.P2;
   lineSegment.segment.P2.Set(std::sin(firstAngle) * crosshairsWidth, std::cos(firstAngle) * crosshairsWidth, 0.0f);
   crosshairs.lineSegments.push_back(lineSegment);

   //make crosshair lines
   int halfWidth = crosshairsWidth/2;

   lineSegment.segment.P1.Set(0.0f, static_cast<float>(-halfWidth), 0.0f);
   lineSegment.segment.P2.Set(0.0f, static_cast<float>(-3*halfWidth), 0.0f);
   crosshairs.lineSegments.push_back(lineSegment);

   lineSegment.segment.P1.Set(0.0f, static_cast<float>(halfWidth), 0.0f);
   lineSegment.segment.P2.Set(0.0f, static_cast<float>(3*halfWidth), 0.0f);
   crosshairs.lineSegments.push_back(lineSegment);

   lineSegment.segment.P1.Set(static_cast<float>(-halfWidth), 0.0f, 0.0f);
   lineSegment.segment.P2.Set(static_cast<float>(-3*halfWidth), 0.0f, 0.0f);
   crosshairs.lineSegments.push_back(lineSegment);

   lineSegment.segment.P1.Set(static_cast<float>(halfWidth), 0.0f, 0.0f);
   lineSegment.segment.P2.Set(static_cast<float>(3*halfWidth), 0.0f, 0.0f);
   crosshairs.lineSegments.push_back(lineSegment);

   Locus::FVector3 right(1.0, 0.0, 0.0);
   Locus::FVector3 up(0.0, -1.0, 0.0);

   topStripY = (80.0f/600) * resolutionY;
   bottomStripY = (550.0f/600) * resolutionY;

   digitWidth = (20.0f/800) * resolutionX;

   fpsX = (600.0f/800) * resolutionX;

   const float topStripHeight = (40.0f/600) * resolutionY;
   const float spaceBetweenLivesAndScore = (65.0f/800) * resolutionX;
   const float spaceBetweenScoreAndLevel = (50.0f/800) * resolutionX;

   const float ammoBoxHeight = topStripHeight / 2;
   const float ammoBoxWidth = (100.0f/800) * resolutionX;

   const float livesIconX = (50.0f/800) * resolutionX;
   const float livesIconWidth = 2 * digitWidth;

   const float livesTimesX = (90.0f/800) * resolutionX;

   const float numLivesX = livesIconX + livesIconWidth + digitWidth;

   const float scoreLabelX = numLivesX + digitWidth + spaceBetweenLivesAndScore;
   const float scoreLabelWidth = (120.0f/800) * resolutionX;

   scoreX = scoreLabelX + scoreLabelWidth;

   const float levelLabelX = scoreX + HUD_NUM_SCORE_DIGITS * digitWidth + spaceBetweenScoreAndLevel;
   const float levelLabelWidth = scoreLabelWidth;

   levelX = levelLabelX + levelLabelWidth;

   ammoBoxX = (70.0f/800) * resolutionX;

   livesIcon.Set( Locus::FVector3(livesIconX, topStripY, 0.0), right, up, livesIconWidth, topStripHeight, QuadTextureColor );
   livesTimes.Set( Locus::FVector3(livesTimesX, topStripY, 0.0), right, up, digitWidth, topStripHeight, QuadTextureColor );
   livesQuad.Set( Locus::FVector3(numLivesX, topStripY, 0.0), right, up, digitWidth, topStripHeight, QuadTextureColor );
   scoreLabel.Set( Locus::FVector3(scoreLabelX, topStripY, 0.0), right, up, scoreLabelWidth, topStripHeight, QuadTextureColor );
   levelLabel.Set( Locus::FVector3(levelLabelX, topStripY, 0.0), right, up, levelLabelWidth, topStripHeight, QuadTextureColor );
   digit.Set( Locus::Vec3D::ZeroVector(), right, up, digitWidth, topStripHeight, QuadTextureColor );

   ammoWidth = static_cast<float>(ammoBoxWidth - 2 * ammoPadding)/(this->maxShots);
   ammo.Set( Locus::Vec3D::ZeroVector(), right, up, ammoWidth, ammoBoxHeight - 2 * ammoPadding, QuadTextureColor );
}

void HUD::Update(int score, int level, int lives, std::size_t currentShots, int crosshairsX, int crosshairsY, int fps)
{
   this->score = score;
   this->level = level;
   this->lives = lives;
   this->currentShots = currentShots;
   this->crosshairsX = crosshairsX;
   this->crosshairsY = crosshairsY;
   this->fps = fps;
}

void HUD::SetProgramIDs(Locus::ID_t notTexturedNotLitProgramID, Locus::ID_t texturedNotLitProgramID)
{
   this->notTexturedNotLitProgramID = notTexturedNotLitProgramID;
   this->texturedNotLitProgramID = texturedNotLitProgramID;
}

void HUD::DrawDigits(Locus::RenderingState& renderingState, int value, int numDigits, float x, float y) const
{
   std::vector<int> digits(numDigits);

   for (int d = numDigits, denominator = 1; d >= 1; d--, denominator *= 10)
   {
      digits[d-1] = ( value/denominator ) % 10;
   }

   renderingState.transformationStack.Push();

   renderingState.transformationStack.LoadIdentity();
   renderingState.transformationStack.Translate( Locus::FVector3(x, y, 0.0) );

   for (int i = 0; i < numDigits; ++i)
   {
      textureManager->GetTexture(MPM::TextureManager::MakeDigitTextureName(digits[i]))->Bind();
      renderingState.shaderController.SetTextureUniform(Locus::ShaderSource::Map_Diffuse, 0);

      renderingState.transformationStack.UploadTransformations(renderingState.shaderController);
      digit.Draw(renderingState);

      renderingState.transformationStack.Translate( Locus::FVector3(digitWidth, 0.0, 0.0) );
   }

   renderingState.transformationStack.Pop();
}

void HUD::DrawAmmo(Locus::RenderingState& renderingState) const
{
   textureManager->GetTexture(MPM::TextureManager::Ammo_TextureName)->Bind();
   renderingState.shaderController.SetTextureUniform(Locus::ShaderSource::Map_Diffuse, 0);

   renderingState.transformationStack.Push();

   renderingState.transformationStack.LoadIdentity();
   renderingState.transformationStack.Translate( Locus::FVector3(ammoBoxX + ammoPadding, bottomStripY - ammoPadding, 0.0) );

   for (std::size_t i = 0; i < maxShots - currentShots; ++i)
   {
      renderingState.transformationStack.UploadTransformations(renderingState.shaderController);
      ammo.Draw(renderingState);

      renderingState.transformationStack.Translate( Locus::FVector3(ammoWidth, 0.0, 0.0) );
   }

   renderingState.transformationStack.Pop();
}

void HUD::CreateGPUVertexData()
{
   crosshairs.CreateGPUVertexData();
   livesIcon.CreateGPUVertexData();
   livesTimes.CreateGPUVertexData();
   livesQuad.CreateGPUVertexData();
   scoreLabel.CreateGPUVertexData();
   levelLabel.CreateGPUVertexData();
   digit.CreateGPUVertexData();
   ammo.CreateGPUVertexData();
}

void HUD::DeleteGPUVertexData()
{
   crosshairs.DeleteGPUVertexData();
   livesIcon.DeleteGPUVertexData();
   livesTimes.DeleteGPUVertexData();
   livesQuad.DeleteGPUVertexData();
   scoreLabel.DeleteGPUVertexData();
   levelLabel.DeleteGPUVertexData();
   digit.DeleteGPUVertexData();
   ammo.DeleteGPUVertexData();
}

void HUD::UpdateGPUVertexData()
{
   crosshairs.UpdateGPUVertexData();
   livesIcon.UpdateGPUVertexData();
   livesTimes.UpdateGPUVertexData();
   livesQuad.UpdateGPUVertexData();
   scoreLabel.UpdateGPUVertexData();
   levelLabel.UpdateGPUVertexData();
   digit.UpdateGPUVertexData();
   ammo.UpdateGPUVertexData();
}

void HUD::Draw(Locus::RenderingState& renderingState) const
{
   //draw crosshairs
   renderingState.shaderController.UseProgram(notTexturedNotLitProgramID);

   glLineWidth(3.0f);

   renderingState.transformationStack.Push();
   renderingState.transformationStack.Translate(Locus::FVector3(static_cast<float>(crosshairsX), static_cast<float>(crosshairsY), 0.0f));
   renderingState.UploadTransformations();

   crosshairs.Draw(renderingState);

   renderingState.transformationStack.Pop();

   glLineWidth(1.0f);

   renderingState.shaderController.UseProgram(texturedNotLitProgramID);

   glEnable (GL_BLEND);

   //draw lives icon
   textureManager->GetTexture(MPM::TextureManager::Lives_Icon_TextureName)->Bind();
   renderingState.shaderController.SetTextureUniform(Locus::ShaderSource::Map_Diffuse, 0);

   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, livesIcon.CurrentModelTransformation());
   livesIcon.Draw(renderingState);

   //draw lives times
   textureManager->GetTexture(MPM::TextureManager::Lives_Times_TextureName)->Bind();
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, livesTimes.CurrentModelTransformation());
   livesTimes.Draw(renderingState);

   //draw num extra lives
   textureManager->GetTexture(MPM::TextureManager::MakeDigitTextureName(lives - 1))->Bind();
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, livesQuad.CurrentModelTransformation());
   livesQuad.Draw(renderingState);

   //draw score label
   textureManager->GetTexture(MPM::TextureManager::Score_Label_TextureName)->Bind();
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, scoreLabel.CurrentModelTransformation());
   scoreLabel.Draw(renderingState);

   //draw level label
   textureManager->GetTexture(MPM::TextureManager::Level_Label_TextureName)->Bind();
   renderingState.transformationStack.UploadTransformations(renderingState.shaderController, levelLabel.CurrentModelTransformation());
   levelLabel.Draw(renderingState);

   //draw score
   DrawDigits(renderingState, score, HUD_NUM_SCORE_DIGITS, scoreX, topStripY);

   //draw level
   DrawDigits(renderingState, level, HUD_NUM_LEVEL_DIGITS, levelX, topStripY);

   DrawAmmo(renderingState);

   //draw FPS
   DrawDigits(renderingState, fps, HUD_NUM_FPS_DIGITS, fpsX, bottomStripY);

   glDisable(GL_BLEND);
}

}