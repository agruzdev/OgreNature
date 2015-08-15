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


const size_t Ground::GROUND_SIZE = 512;
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
void Ground::LoadFromHeightMap(const Ogre::Texture* hmap)
{
    mTexture = Ogre::TextureManager::getSingleton().createManual(
        "Texture/Ground/" + mName, 
        Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
        Ogre::TEX_TYPE_2D, hmap->getWidth(), hmap->getHeight(),
        0, Ogre::PF_L8);
    const_cast<Ogre::Texture*>(hmap)->copyToTexture(mTexture);

    Ogre::Box box = Ogre::Box(0, 0, mTexture->getWidth(), mTexture->getHeight());

    auto buffer = mTexture->getBuffer();
    Ogre::PixelBox data = buffer->lock(box, Ogre::HardwareBuffer::HBL_READ_ONLY);
    

    Ogre::ManualObject *man = mSceneManager->createManualObject();
    
    
    auto raw_data = reinterpret_cast<unsigned char*>(data.data);

    static const float VERTEX_STEP = 1.0f;
    static const float HEIGHT_STEP = 8.0f;

    float offsetX = GROUND_SIZE * VERTEX_STEP / 2.0f;
    float offsetY = GROUND_SIZE * VERTEX_STEP / 2.0f;

    man->begin("BaseWhiteNoLighting", Ogre::RenderOperation::OT_TRIANGLE_LIST);
    {
        size_t lastIdx = 0;
        for (size_t y = 0; y < GROUND_SIZE - 1; ++y)
        {
            size_t texY  = static_cast<size_t>(static_cast<float>(y)     / (GROUND_SIZE - 1) * mTexture->getHeight());
            size_t texYn = static_cast<size_t>(static_cast<float>(y + 1) / (GROUND_SIZE - 1) * mTexture->getHeight());
            if (texYn > mTexture->getHeight() - 1)
                texYn = mTexture->getHeight() - 1;
            for (size_t x = 0; x < GROUND_SIZE - 1; ++x)
            {
                size_t texX  = static_cast<size_t>(static_cast<float>(x)     / (GROUND_SIZE - 1) * mTexture->getWidth());
                size_t texXn = static_cast<size_t>(static_cast<float>(x + 1) / (GROUND_SIZE - 1) * mTexture->getWidth());
                if (texXn > mTexture->getWidth() - 1)
                    texXn = mTexture->getWidth() - 1;
                
                float h00 = data.getColourAt(texX,  texY,  0)[0];
                float h10 = data.getColourAt(texXn, texY,  0)[0];
                float h01 = data.getColourAt(texX,  texYn, 0)[0];
                float h11 = data.getColourAt(texXn, texYn, 0)[0];

                man->position(x * VERTEX_STEP - offsetX, y * VERTEX_STEP - offsetY, h00 * HEIGHT_STEP);
                man->colour(h00, h00, h00);
                man->position((x + 1) * VERTEX_STEP - offsetX, y * VERTEX_STEP - offsetY, h10 * HEIGHT_STEP);
                man->colour(h10, h10, h10);
                man->position(x * VERTEX_STEP - offsetX, (y + 1) * VERTEX_STEP - offsetY, h01 * HEIGHT_STEP);
                man->colour(h01, h01, h01);
                man->position((x + 1) * VERTEX_STEP - offsetX, (y + 1) * VERTEX_STEP - offsetY, h11 * HEIGHT_STEP);
                man->colour(h11, h11, h11);
                

                man->triangle(lastIdx + 1, lastIdx + 2, lastIdx);
                man->triangle(lastIdx + 3, lastIdx + 2, lastIdx + 1);
                lastIdx += 4;
            }
        }
        /*
        man->position(0, 1, 0);
        man->colour(1, 0, 0);
        man->position(-1, -1, 0);
        man->colour(0, 1, 0);
        man->position(1, -1, 0);
        man->colour(0, 0, 1);
        man->triangle(0, 1, 2);
        */
    }
    man->end();

    Ogre::MeshPtr mesh = man->convertToMesh("Mesh/Ground/" + mName);
    mEntity = mSceneManager->createEntity(mesh);
}
//-------------------------------------------------------