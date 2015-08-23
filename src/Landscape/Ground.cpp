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
#include <OgreHardwarePixelBuffer.h>
#include <OgreHighLevelGpuProgram.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreMaterialManager.h>
#include <OgreTechnique.h>
#include <OgrePass.h>
#include <OgreSceneNode.h>

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
Ogre::Material* Ground::CreateGroundMaterialTextured(const std::string & name, const Ogre::Texture* texture)
{
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

            auto unit0 = pass->createTextureUnitState(texture->getName());
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
Ground::Ground(const std::string & name, Ogre::SceneManager* sceneManager):
    mName(name), mSceneManager(sceneManager)
{
    
}
//-------------------------------------------------------
Ground::~Ground()
{

}
//-------------------------------------------------------
Ogre::MeshPtr Ground::CreateRegion(size_t id, const std::string & material, Ogre::PixelBox& pixels, const Ogre::Vector3 & offset, const Ogre::Vector3 & steps, const Ogre::Vector2 & texOffset)
{
    assert(mSceneManager != nullptr);

    Ogre::ManualObject* object = mSceneManager->createManualObject();
    object->begin(material, Ogre::RenderOperation::OT_TRIANGLE_LIST);
    {
        size_t lastIdx = 0;
        for (size_t y = 0; y < REGION_SIZE; ++y)
        {
            size_t texY = static_cast<size_t>(static_cast<float>(y) / REGION_SIZE * (pixels.getHeight() - 1));
            size_t texYn = static_cast<size_t>(static_cast<float>(y + 1) / REGION_SIZE * (pixels.getHeight() - 1));

            //Flip texture vertically
            texY  = pixels.getHeight() - 1 - texY;
            texYn = pixels.getHeight() - 1 - texYn;

            float texCrdT = texOffset[1] + static_cast<float>(texY) / (mTexture->getHeight() - 1);
            float texCrdTn = texOffset[1] + static_cast<float>(texYn) / (mTexture->getHeight() - 1);

            for (size_t x = 0; x < REGION_SIZE; ++x)
            {
                size_t texX = static_cast<size_t>(static_cast<float>(x) / REGION_SIZE * (pixels.getWidth() - 1));
                size_t texXn = static_cast<size_t>(static_cast<float>(x + 1) / REGION_SIZE * (pixels.getWidth() - 1));

                float texCrdS = texOffset[0] + static_cast<float>(texX) / (mTexture->getWidth() - 1);
                float texCrdSn = texOffset[0] + static_cast<float>(texXn) / (mTexture->getWidth() - 1);

                float h00 = pixels.getColourAt(texX, texY, 0)[0];
                float h10 = pixels.getColourAt(texXn, texY, 0)[0];
                float h01 = pixels.getColourAt(texX, texYn, 0)[0];
                float h11 = pixels.getColourAt(texXn, texYn, 0)[0];

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
void Ground::LoadFromHeightMap(const Ogre::Texture* hmap, Ogre::SceneNode* parentNode)
{
    mTexture = Ogre::TextureManager::getSingleton().createManual(
        "Texture/" + CLASS_NAME + "/" + mName,
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D, hmap->getWidth(), hmap->getHeight(),
        0, Ogre::PF_L8);
    const_cast<Ogre::Texture*>(hmap)->copyToTexture(mTexture);
    Ogre::Box box = Ogre::Box(0, 0, mTexture->getWidth(), mTexture->getHeight());

    auto buffer = mTexture->getBuffer();
    //Ogre::PixelBox data = buffer->lock(box, Ogre::HardwareBuffer::HBL_READ_ONLY);
    
    mRootNode = parentNode->createChildSceneNode();
    

    //Ogre::ManualObject *man = mSceneManager->createManualObject();
    //mObject = mSceneManager->createManualObject();

    Ogre::Material* groundMaterial = CreateGroundMaterialTextured("Material/" + CLASS_NAME + "/Textured", mTexture.get());
    
    static const float VERTEX_STEP = 1.0f;
    static const float HEIGHT_STEP = 8.0f;

    float offsetX = GROUND_SIZE * VERTEX_STEP / 2.0f;
    float offsetY = GROUND_SIZE * VERTEX_STEP / 2.0f;

    //CreateRegion(data, Ogre::Vector3(-offsetX, -offsetY, 0.0f), Ogre::Vector3(VERTEX_STEP, VERTEX_STEP, HEIGHT_STEP));

    size_t width  = mTexture->getWidth();
    size_t height = mTexture->getHeight();

    float texStep = 1.0f / REGIONS_NUMBER;

    size_t texRegionWidth  = static_cast<size_t>(std::ceil(static_cast<float>(width) / REGIONS_NUMBER));
    size_t texRegionHeight = static_cast<size_t>(std::ceil(static_cast<float>(height) / REGIONS_NUMBER));
    for (size_t y = 0; y < REGIONS_NUMBER; ++y)
    {
        size_t top = y * texRegionHeight;
        for (size_t x = 0; x < REGIONS_NUMBER; ++x)
        {
            size_t left = x * texRegionWidth;
            //Ogre::Box roi = Ogre::Box(left, top, std::min(left + texRegionWidth, width), std::min(top + texRegionHeight, height));
            Ogre::Box roi = Ogre::Box(left, height - std::min(top + texRegionHeight, height), std::min(left + texRegionWidth, width), height - top);
                
            auto pixels = buffer->lock(roi, Ogre::HardwareBuffer::HBL_READ_ONLY);
            Ogre::MeshPtr mesh = CreateRegion(y * REGIONS_NUMBER + x, groundMaterial->getName(), pixels,
                Ogre::Vector3(x * VERTEX_STEP * REGION_SIZE - offsetX, y * VERTEX_STEP * REGION_SIZE - offsetY, 0.0f),
                Ogre::Vector3(VERTEX_STEP, VERTEX_STEP, HEIGHT_STEP),
                Ogre::Vector2(x * texStep, 1.0f - (y + 1) * texStep));
            buffer->unlock();

            Ogre::Entity* entity = mSceneManager->createEntity(mesh);
            
            auto node = mRootNode->createChildSceneNode();
            node->attachObject(entity);

            mEntities.push_back(entity);
        }
    }
   

//     Ogre::MeshPtr mesh = mObject->convertToMesh("Mesh/Ground/" + mName);
//     mEntity = mSceneManager->createEntity(mesh);
}
//-------------------------------------------------------