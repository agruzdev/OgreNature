/**
* @file Ground.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/


#include "Ground.h"

#include <OgreEntity.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreManualObject.h>
#include <OgreSceneManager.h>
#include <OgreMesh.h>
#include <OgreSubMesh.h>
#include <OgreVertexIndexData.h>
#include <OgreHardwarePixelBuffer.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreSceneNode.h>
#include <OgreImage.h>

namespace
{
    static const char Shader_GL_Simple_V[] = ""
        "#version 120                                                              \n"
        "                                                                          \n"
        "void main()                                                               \n"
        "{                                                                         \n"
        "    gl_TexCoord[0] = gl_MultiTexCoord0;                                   \n"
        "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;               \n"
        "}                                                                         \n"
        "";

    static const char Shader_GL_Simple_F[] = ""
        "#version 120                                                   \n"
        "                                                               \n"
        "uniform sampler2D texture;                                     \n"
        "                                                               \n"
        "void main()                                                    \n"
        "{                                                              \n"
        "    gl_FragColor = texture2D(texture, gl_TexCoord[0].st);      \n"
        "}                                                              \n"
        "";
}

const std::string Ground::CLASS_NAME = "Ground";

const size_t Ground::GROUND_SIZE = 512;
const size_t Ground::REGION_SIZE = 64;
const size_t Ground::REGIONS_NUMBER = 10;


//-------------------------------------------------------
Ogre::Material* Ground::CreateGroundMaterialTextured(const std::string & name, const Ogre::Image* texture)
{
    Ogre::TexturePtr heightMapTexture = Ogre::TextureManager::getSingleton().loadImage("Texture/Terrain",
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, *texture);

    Ogre::MaterialPtr material = Ogre::MaterialManager::getSingleton().create(name, Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    {
        Ogre::Technique* techniqueGL = material->getTechnique(0);
        Ogre::Pass* pass = techniqueGL->getPass(0);
        {
            auto vprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/" + CLASS_NAME + "/GL/Textured/V",
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_VERTEX_PROGRAM);
            vprogram->setSource(Shader_GL_Simple_V);
            //auto vparams = vprogram->createParameters();
            //vparams->setNamedAutoConstant("modelviewproj", Ogre::GpuProgramParameters::ACT_WORLDVIEWPROJ_MATRIX);

            pass->setVertexProgram(vprogram->getName());
        }
        {
            auto fprogram = Ogre::HighLevelGpuProgramManager::getSingleton().createProgram("Shader/" + CLASS_NAME + "/GL/Textured/F",
                Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
            fprogram->setSource(Shader_GL_Simple_F);

            auto unit0 = pass->createTextureUnitState(heightMapTexture->getName());
            unit0->setTextureAddressingMode(Ogre::TextureUnitState::TAM_CLAMP);
            unit0->setTextureFiltering(Ogre::TFO_NONE);

            pass->setFragmentProgram(fprogram->getName());
        }
    }
#if !NDEBUG
    material->load();
#endif
    return material.get();
}
//-------------------------------------------------------
// http://www.ogre3d.org/tikiwiki/Raycasting+to+the+polygon+level
std::pair<bool, float> Ground::GetVertexIntersection(const Ogre::Ray & ray, const Ogre::SubMesh* subMesh)
{
    OgreAssert(false == subMesh->useSharedVertices, "Mesh with shared data is not supported");

    const Ogre::VertexElement* posElem = subMesh->vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);
    
    Ogre::HardwareVertexBufferSharedPtr vbuffer = subMesh->vertexData->vertexBufferBinding->getBuffer(posElem->getSource());

    unsigned char* vertexes = reinterpret_cast<unsigned char*>(vbuffer->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

    size_t count = subMesh->vertexData->vertexCount;

    float intersection = -1.0f;

    OgreAssert(count == REGION_SIZE * REGION_SIZE * 4, "Wrong buffer size");
    float* pReal;
    for (size_t i = 0; i < REGION_SIZE * REGION_SIZE; ++i)
    {
        Ogre::Vector3 v0, v1, v2, v3;
        for (auto vp : { &v0, &v1, &v2, &v3 })
        {
            posElem->baseVertexPointerToElement(vertexes, &pReal);
            *vp = Ogre::Vector3(pReal[0], pReal[1], pReal[2]);
            vertexes += vbuffer->getVertexSize();
        }

        auto hit1 = Ogre::Math::intersects(ray, v1, v2, v0, true, false);
        if (hit1.first && (intersection < 0.0f || hit1.second < intersection))
        {
            intersection = hit1.second;
        }
        auto hit2 = Ogre::Math::intersects(ray, v3, v2, v1, true, false);
        if (hit2.first && (intersection < 0.0f || hit2.second < intersection))
        {
            intersection = hit2.second;
        }
    }
    vbuffer->unlock();
    if (intersection >= 0.0f)
    {
        return std::make_pair(true, intersection);
    }
    return std::make_pair(false, -1.0f);
}
//-------------------------------------------------------
Ground::Ground(const std::string & name, Ogre::SceneManager* sceneManager):
    mName(name), mSceneManager(sceneManager)
{
    
}
//-------------------------------------------------------
Ground::~Ground()
{
    mImage.reset();
}
//-------------------------------------------------------
Ogre::MeshPtr Ground::CreateRegion(size_t id, const std::string & material, const Ogre::Box & roi, const Ogre::Vector3 & offset, const Ogre::Vector3 & steps, const Ogre::Vector2 & texOffset)
{
    assert(mSceneManager != nullptr);

    Ogre::ManualObject* object = mSceneManager->createManualObject();
    object->begin(material, Ogre::RenderOperation::OT_TRIANGLE_LIST);
    {
        size_t lastIdx = 0;
        for (size_t y = 0; y < REGION_SIZE; ++y)
        {
            size_t texY = static_cast<size_t>(static_cast<float>(y) / REGION_SIZE * (roi.getHeight() - 1));
            size_t texYn = static_cast<size_t>(static_cast<float>(y + 1) / REGION_SIZE * (roi.getHeight() - 1));

            //Flip texture vertically
            texY  = roi.getHeight() - 1 - texY;
            texYn = roi.getHeight() - 1 - texYn;

            float texCrdT = texOffset[1] + static_cast<float>(texY) / (mImage->getHeight() - 1);
            float texCrdTn = texOffset[1] + static_cast<float>(texYn) / (mImage->getHeight() - 1);

            for (size_t x = 0; x < REGION_SIZE; ++x)
            {
                size_t texX = static_cast<size_t>(static_cast<float>(x) / REGION_SIZE * (roi.getWidth() - 1));
                size_t texXn = static_cast<size_t>(static_cast<float>(x + 1) / REGION_SIZE * (roi.getWidth() - 1));

                float texCrdS = texOffset[0] + static_cast<float>(texX) / (mImage->getWidth() - 1);
                float texCrdSn = texOffset[0] + static_cast<float>(texXn) / (mImage->getWidth() - 1);

                float h00 = mImage->getColourAt(roi.left + texX,  roi.top + texY, 0)[0];
                float h10 = mImage->getColourAt(roi.left + texXn, roi.top + texY, 0)[0];
                float h01 = mImage->getColourAt(roi.left + texX,  roi.top + texYn, 0)[0];
                float h11 = mImage->getColourAt(roi.left + texXn, roi.top + texYn, 0)[0];

                object->position(x * steps[0] + offset[0], y * steps[1] + offset[1], h00 * steps[2]);
                object->textureCoord(texCrdS, texCrdT);
                object->colour(h00, h00, h00);
                
                object->position((x + 1) * steps[0] + offset[0], y * steps[1] + offset[1], h10 * steps[2]);
                object->textureCoord(texCrdSn, texCrdT);
                object->colour(h10, h10, h10);

                object->position(x * steps[0] + offset[0], (y + 1) * steps[1] + offset[1], h01 * steps[2]);
                object->textureCoord(texCrdS, texCrdTn);
                object->colour(h01, h01, h01);

                object->position((x + 1) * steps[0] + offset[0], (y + 1) * steps[1] + offset[1], h11 * steps[2]);
                object->textureCoord(texCrdSn, texCrdTn);
                object->colour(h11, h11, h11);

                object->triangle(lastIdx + 1, lastIdx + 2, lastIdx);
                object->triangle(lastIdx + 3, lastIdx + 2, lastIdx + 1);
                lastIdx += 4;
            }
        }
    }
    object->end();
    return object->convertToMesh("Mesh/" + CLASS_NAME  + "/" + mName + "/" + std::to_string(id));
}
//-------------------------------------------------------
void Ground::LoadFromHeightMap(std::shared_ptr<Ogre::Image> hmap, Ogre::SceneNode* parentNode)
{
    mImage = hmap;
    Ogre::Box box = Ogre::Box(0, 0, mImage->getWidth(), mImage->getHeight());

    mRootNode = parentNode->createChildSceneNode();
    
    mGlobalBoundingBox.setNull();

    Ogre::Material* groundMaterial = CreateGroundMaterialTextured("Material/" + CLASS_NAME + "/Textured", mImage.get());
    
    static const float VERTEX_STEP = 1.0f;
    static const float HEIGHT_STEP = 8.0f;

    float offsetX = GROUND_SIZE * VERTEX_STEP / 2.0f;
    float offsetY = GROUND_SIZE * VERTEX_STEP / 2.0f;

    size_t width  = mImage->getWidth();
    size_t height = mImage->getHeight();

    float texStep = 1.0f / REGIONS_NUMBER;

    size_t texRegionWidth  = static_cast<size_t>(std::ceil(static_cast<float>(width) / REGIONS_NUMBER));
    size_t texRegionHeight = static_cast<size_t>(std::ceil(static_cast<float>(height) / REGIONS_NUMBER));
    for (size_t y = 0; y < REGIONS_NUMBER; ++y)
    {
        size_t top = y * texRegionHeight;
        for (size_t x = 0; x < REGIONS_NUMBER; ++x)
        {
            size_t left = x * texRegionWidth;
            Ogre::Box roi = Ogre::Box(left, height - std::min(top + texRegionHeight + 1, height), std::min(left + texRegionWidth + 1, width), height - top);
                
            Ogre::MeshPtr mesh = CreateRegion(y * REGIONS_NUMBER + x, groundMaterial->getName(), roi,
                Ogre::Vector3(x * VERTEX_STEP * REGION_SIZE - offsetX, y * VERTEX_STEP * REGION_SIZE - offsetY, 0.0f),
                Ogre::Vector3(VERTEX_STEP, VERTEX_STEP, HEIGHT_STEP),
                Ogre::Vector2(x * texStep, 1.0f - (y + 1) * texStep));

            Ogre::Entity* entity = mSceneManager->createEntity(mesh);
            
            auto node = mRootNode->createChildSceneNode();
            node->attachObject(entity);
            node->showBoundingBox(true);

            mGlobalBoundingBox.merge(entity->getBoundingBox());

            mEntities.push_back(entity);
        }
    }
}
//-------------------------------------------------------
float Ground::GetHeightAt(float s, float t)
{
    OgreAssert(0.0f <= s && s <= 1.0f && 0.0f <= t && t <= 1.0f, "S and T should be from [0, 1]");
    OgreAssert(nullptr != mImage.get(), "Ground[GetHeightAt]: Not initialized");

    size_t x = static_cast<size_t>(s * (mImage->getWidth() - 1));
    size_t y = static_cast<size_t>(t * (mImage->getHeight() - 1));

    return mImage->getColourAt(x, y, 0)[0];
}
//-------------------------------------------------------
std::pair<bool, Ogre::Vector3> Ground::GetIntersectionLocalSpace(const Ogre::Ray & ray) const
{
    if (ray.intersects(mGlobalBoundingBox).first)
    {
        float intersection = -1.0f;
        for (const auto & region : mEntities)
        {
            auto hit = ray.intersects(region->getBoundingBox());
            if (hit.first)
            {
                auto meshHit = GetVertexIntersection(ray, region->getMesh()->getSubMesh(0));
                if (meshHit.first && (intersection < 0.0f || meshHit.second < intersection))
                {
                    intersection = meshHit.second;
                }
            }
        }
        if (intersection >= 0.0f)
        {
            return std::make_pair(true, ray.getPoint(intersection));
        }
    }
    return std::make_pair(false, Ogre::Vector3::ZERO);
}

//-------------------------------------------------------