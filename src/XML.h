 /*********************************************************************************************************\
 *                                                                                                        *
 *   This file is part of Minor Planet Mayhem                                                             *
 *                                                                                                        *
 *   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
 *                                                                                                        *
 *   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
 *                                                                                                        *
 \*********************************************************************************************************/

#pragma once

#include "Locus/FileSystem/MountedFilePath.h"

#include <string>
#include <vector>

namespace Locus
{

class DataStream;

}

namespace rapidxml
{

template <class Ch>
class xml_document;

}

namespace MPM
{

bool ParseXMLFile(const std::string& filePath, rapidxml::xml_document<char>& xmlDocument, std::vector<char>& xmlContents);
bool ParseXMLFile(const Locus::MountedFilePath& mountedFilePath, rapidxml::xml_document<char>& xmlDocument, std::vector<char>& xmlContents);
bool ParseXMLFile(Locus::DataStream& xmlDataStream, rapidxml::xml_document<char>& xmlDocument, std::vector<char>& xmlContents);

}