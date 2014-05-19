/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "PauseScene.h"

#include "Locus/Simulation/SceneManager.h"

namespace MPM
{

PauseScene::PauseScene(Locus::SceneManager& sceneManager, Scene& sceneCurrentlyPaused, Locus::Key_t pauseKey)
   : Scene(sceneManager), sceneCurrentlyPaused(sceneCurrentlyPaused), pauseKey(pauseKey), dieOnNextFrame(false)
{
}

void PauseScene::Activate()
{
   sceneManager.MakeWindowed();
}

bool PauseScene::Update(double /*DT*/)
{
   return !dieOnNextFrame;
}

void PauseScene::Draw()
{
   sceneCurrentlyPaused.Draw();
}

void PauseScene::InitializeRenderingState()
{
   sceneCurrentlyPaused.InitializeRenderingState();
}

void PauseScene::KeyPressed(Locus::Key_t key)
{
   if (key == pauseKey)
   {
      dieOnNextFrame = true;
   }
}

void PauseScene::KeyReleased(Locus::Key_t /*key*/)
{
}

void PauseScene::MousePressed(Locus::MouseButton_t /*button*/, int /*x*/, int /*y*/)
{
}

void PauseScene::MouseReleased(Locus::MouseButton_t /*button*/, int /*x*/, int /*y*/)
{
}

void PauseScene::MouseMoved(int /*x*/, int /*y*/)
{
}

void PauseScene::Resized(int width, int height)
{
   sceneCurrentlyPaused.Resized(width, height);
}

}