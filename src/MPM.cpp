 /*********************************************************************************************************\
 *                                                                                                        *
 *   This file is part of Minor Planet Mayhem                                                             *
 *                                                                                                        *
 *   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
 *                                                                                                        *
 *   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
 *                                                                                                        *
 \*********************************************************************************************************/

#include "Locus/Preprocessor/CompilerDefinitions.h"

#ifdef LOCUS_WINDOWS
   #define NOMINMAX
   #include <windows.h>
#endif

#include "Locus/FileSystem/FileSystem.h"
#include "Locus/FileSystem/FileSystemUtil.h"

#include "Locus/Simulation/SceneManager.h"
#include "Locus/Simulation/Window.h"
#include "Locus/Simulation/WindowContext.h"

#include "Locus/Common/Exception.h"

#include <Locus/Rendering/Locus_glew.h>

#include "Config.h"
#include "DemoScene.h"

#include <string>

#include <stdlib.h>

static const char* Window_Name = "Minor Planet Mayhem";

void ShowFatalError(const std::string& error)
{
#ifdef LOCUS_WINDOWS
   MessageBox(GetActiveWindow(), error.c_str(), "Fatal Error", MB_ICONEXCLAMATION | MB_OK);
#else
   std::cout << "Fatal Error: " << error.c_str() << std::endl;
#endif
}

#if defined(LOCUS_WINDOWS)
  int CALLBACK WinMain(
  _In_  HINSTANCE /*hInstance*/,
  _In_  HINSTANCE /*hPrevInstance*/,
  _In_  LPSTR /*lpCmdLine*/,
  _In_  int /*nCmdShow*/
  )
#else
   int main(int argc, char** argv)
#endif
{
   try
   {
#ifdef LOCUS_WINDOWS
      std::string argv0 = Locus::GetExePath();
#else
      std::string argv0 = argv[0];
#endif

      Locus::FileSystem fileSystem(argv0.c_str());

#ifdef MPM_USE_ARCHIVE
      Locus::MountDirectoryOrArchive(Locus::GetExePath() + "resources.zip");
#else
      Locus::MountDirectoryOrArchive(Locus::GetExePath() + "resources/");
#endif

      Locus::WindowContext windowContext;

      int monitorWidth = 0;
      int monitorHeight = 0;
      windowContext.GetPrimaryMonitorWindowSize(monitorWidth, monitorHeight);

      bool fullScreen = true;

#ifdef _DEBUG
      fullScreen = false;
      monitorHeight = monitorHeight - 200;
#endif

      int targetRefreshRate = 60;

      Locus::Window window(windowContext, monitorWidth, monitorHeight, Window_Name, fullScreen, &targetRefreshRate);

      window.MakeContextCurrent();

      if (!GLEW_VERSION_2_0)
      {
         ShowFatalError("OpenGL 2.0 is required to run this game");
         return EXIT_FAILURE;
      }

      MPM::Config::Set();

      Locus::SceneManager sceneManager(window);

      sceneManager.RunSimulation( std::unique_ptr<Locus::Scene>(new MPM::DemoScene(sceneManager, monitorWidth, monitorHeight)) );
   }
   catch (Locus::Exception& locusException)
   {
      ShowFatalError(locusException.Message());
      return EXIT_FAILURE;
   }
   catch (std::exception& stdException)
   {
      ShowFatalError(stdException.what());
      return EXIT_FAILURE;
   }
   catch (...)
   {
      ShowFatalError("Unknown Error");
      return EXIT_FAILURE;
   }

   return EXIT_SUCCESS;
}