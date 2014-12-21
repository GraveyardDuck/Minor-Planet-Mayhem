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

namespace Locus
{

class Mesh;
struct MountedFilePath;

}

#include <string>
#include <vector>
#include <memory>

namespace MPM
{

void ParseSAPFile(const Locus::MountedFilePath& mountedFilePath, std::vector<std::unique_ptr<Locus::Mesh>>& meshes);

}