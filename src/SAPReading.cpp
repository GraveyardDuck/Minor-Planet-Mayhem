/********************************************************************************************************\
*                                                                                                        *
*   This file is part of Minor Planet Mayhem                                                             *
*                                                                                                        *
*   Copyright (c) 2014 Shachar Avni. All rights reserved.                                                *
*                                                                                                        *
*   Use of this file is governed by a BSD-style license. See the accompanying LICENSE.txt for details    *
*                                                                                                        *
\********************************************************************************************************/

#include "SAPReading.h"

#include "Locus/Common/Parsing.h"

#include "Locus/FileSystem/File.h"

#include "Locus/XML/XMLParsing.h"
#include "Locus/XML/XMLTag.h"

#include "Locus/Geometry/Vector3Geometry.h"

#include "Locus/Rendering/TextureCoordinate.h"
#include "Locus/Rendering/Mesh.h"

#include <cstdlib>

namespace MPM
{

static const std::string XML_SAP = "SAP";
static const std::string XML_Model = "Model";
static const std::string XML_Positions = "Positions";
static const std::string XML_Position = "Position";
static const std::string XML_TextureCoordinates = "TextureCoordinates";
static const std::string XML_TextureCoordinate = "TextureCoordinate";
static const std::string XML_X = "x";
static const std::string XML_Y = "y";
static const std::string XML_Z = "z";
static const std::string XML_TX = "tx";
static const std::string XML_TY = "ty";
static const std::string XML_Faces = "Faces";
static const std::string XML_Face = "Face";
static const std::string XML_Vertex = "Vertex";
static const std::string XML_PositionIndex = "PositionIndex";
static const std::string XML_TextureCoordinateIndex = "TextureCoordinateIndex";

void ParseSAPFile(const Locus::MountedFilePath& mountedFilePath, std::vector<std::unique_ptr<Locus::Mesh>>& meshes)
{
   Locus::XMLTag rootTag;

   Locus::ParseXMLFile(mountedFilePath, rootTag);

   #define CHECK_TAG(tag) if (tag == nullptr) throw std::runtime_error("Failed to find SAP XML tag")
   #define CHECK_TAG_NAME(tagName, expectedName) if (tagName != expectedName) std::runtime_error(std::string("Unexpected tag found in SAP file: ") + tagName)

   Locus::FVector3 positionFromFile;
   Locus::Mesh::face_t faceFromFile;
   Locus::TextureCoordinate textureCoordFromFile;

   for (Locus::XMLTag& modelTag : rootTag.subTags)
   {
      CHECK_TAG_NAME(modelTag.name, XML_Model);

      std::unique_ptr<Locus::Mesh> mesh = std::make_unique<Locus::Mesh>();

      Locus::XMLTag* positionsTag = modelTag.FindSubTag(XML_Positions, 0);
      CHECK_TAG(positionsTag);

      for (Locus::XMLTag& singlePositionTag : positionsTag->subTags)
      {
         CHECK_TAG_NAME(singlePositionTag.name, XML_Position);

         //read X
         Locus::XMLTag* coordTag = singlePositionTag.FindSubTag(XML_X, 0);
         CHECK_TAG(coordTag);
         positionFromFile.x = std::stof(coordTag->value);

         //read Y
         coordTag = singlePositionTag.FindSubTag(XML_Y, 0);
         CHECK_TAG(coordTag);
         positionFromFile.y = std::stof(coordTag->value);

         //read Z
         coordTag = singlePositionTag.FindSubTag(XML_Z, 0);
         CHECK_TAG(coordTag);
         positionFromFile.z = std::stof(coordTag->value);

         mesh->AddPosition(positionFromFile);
      }

      Locus::XMLTag* textureCoordsTag = modelTag.FindSubTag(XML_TextureCoordinates, 0);
      CHECK_TAG(textureCoordsTag);

      for (Locus::XMLTag& singleTextureCoordTag : textureCoordsTag->subTags)
      {
         CHECK_TAG_NAME(singleTextureCoordTag.name, XML_TextureCoordinate);

         //read tx
         Locus::XMLTag* coordTag = singleTextureCoordTag.FindSubTag(XML_TX, 0);
         CHECK_TAG(coordTag);
         textureCoordFromFile.x = std::stof(coordTag->value);

         //read tY
         coordTag = singleTextureCoordTag.FindSubTag(XML_TY, 0);
         CHECK_TAG(coordTag);
         textureCoordFromFile.y = std::stof(coordTag->value);

         mesh->AddTextureCoordinate(textureCoordFromFile);
      }

      Locus::XMLTag* facesTag = modelTag.FindSubTag(XML_Faces, 0);
      CHECK_TAG(facesTag);

      for (Locus::XMLTag& singleFaceTag : facesTag->subTags)
      {
         CHECK_TAG_NAME(singleFaceTag.name, XML_Face);

         faceFromFile.clear();

         for (Locus::XMLTag& vertexTag : singleFaceTag.subTags)
         {
            CHECK_TAG_NAME(vertexTag.name, XML_Vertex);

            Locus::XMLTag* positionIndexTag = vertexTag.FindSubTag(XML_PositionIndex, 0);
            CHECK_TAG(positionIndexTag);
            std::size_t positionIndex = static_cast<std::size_t>(std::stoul(positionIndexTag->value));

            Locus::XMLTag* textureCoordIndexTag = vertexTag.FindSubTag(XML_TextureCoordinateIndex, 0);
            CHECK_TAG(textureCoordIndexTag);
            std::size_t textureCoordIndex = static_cast<std::size_t>(std::stoul(textureCoordIndexTag->value));

            faceFromFile.push_back( Locus::MeshVertexIndexer(positionIndex, textureCoordIndex, 0, 0) );
         }

         mesh->AddFace(faceFromFile);
      }

      mesh->ComputeCentroid();
      mesh->ToModel();
      mesh->centroid = Locus::Vec3D::ZeroVector();
      mesh->Triangulate();
      mesh->UpdateEdgeAdjacency();
      mesh->AssignNormals();

      meshes.push_back( std::move(mesh) );
   }

   #undef CHECK_TAG
   #undef CHECK_TAG_NAME
}

}