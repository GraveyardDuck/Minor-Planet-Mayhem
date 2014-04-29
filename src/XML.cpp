 /*********************************************************************************************************\
 *                                                                                                        *
 *   This file is part of Minor Planet Mayhem                                                             *
 *                                                                                                        *
 *   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
 *                                                                                                        *
 *   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
 *                                                                                                        *
 \*********************************************************************************************************/

#include "XML.h"

#include "Locus/FileSystem/FileOnDisk.h"
#include "Locus/FileSystem/File.h"

#include "RapidXML/rapidxml.hpp"

namespace MPM
{

bool ParseXMLFile(const std::string& filePath, rapidxml::xml_document<char>& xmlDocument, std::vector<char>& xmlContents)
{
   Locus::FileOnDisk fileOnDisk(filePath, Locus::DataStream::OpenOperation::Read);

   return ParseXMLFile(fileOnDisk, xmlDocument, xmlContents);
}

bool ParseXMLFile(const Locus::MountedFilePath& mountedFilePath, rapidxml::xml_document<char>& xmlDocument, std::vector<char>& xmlContents)
{
   Locus::File file(mountedFilePath, Locus::DataStream::OpenOperation::Read);

   return ParseXMLFile(file, xmlDocument, xmlContents);
}

bool ParseXMLFile(Locus::DataStream& xmlDataStream, rapidxml::xml_document<char>& xmlDocument, std::vector<char>& xmlContents)
{
   if (!xmlDataStream.Seek(0, Locus::DataStream::SeekType::Beginning))
   {
      return false;
   }

   std::size_t fileSizeInBytes = xmlDataStream.SizeInBytes();

   xmlContents.clear();
   xmlContents.resize(fileSizeInBytes + 1);

   if (xmlDataStream.Read(xmlContents, fileSizeInBytes, 0) != fileSizeInBytes)
   {
      return false;
   }

   try
   {
      xmlDocument.parse<0>(xmlContents.data());
   }
   catch(rapidxml::parse_error&)
   {
      return false;
   }

   return true;
}

}