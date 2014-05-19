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

#include "Locus/Simulation/Scene.h"

namespace Locus
{
   class SceneManager;
}

namespace MPM
{

class PauseScene : public Locus::Scene
{
public:
   PauseScene(Locus::SceneManager& sceneManager, Scene& sceneCurrentlyPaused, Locus::Key_t pauseKey);

   virtual void Activate() override;

   virtual bool Update(double DT) override;
   virtual void Draw() override;

   virtual void InitializeRenderingState() override;

   virtual void KeyPressed(Locus::Key_t key) override;
   virtual void KeyReleased(Locus::Key_t key) override;

   virtual void MousePressed(Locus::MouseButton_t button, int x, int y) override;
   virtual void MouseReleased(Locus::MouseButton_t button, int x, int y) override;
   virtual void MouseMoved(int x, int y) override;

   virtual void Resized(int width, int height) override;

private:
   Scene& sceneCurrentlyPaused;
   Locus::Key_t pauseKey;
   bool dieOnNextFrame;
};

}