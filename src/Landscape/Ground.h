/**
* @file Ground.h
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/


#ifndef _GROUND_H_
#define _GROUND_H_

#include <OgrePrerequisites.h>
#include <OgreCommon.h>
#include <OgreTexture.h>
#include <OgreMesh.h>

namespace Ogre
{
    class SceneManager;
    class Entity;
    class Texture;
    class ManualObject;
    class SceneNode;
}

class Ground
{
    static const std::string CLASS_NAME;

    static const size_t GROUND_SIZE;
    static const size_t REGION_SIZE;
    static const size_t REGIONS_NUMBER;
    //-------------------------------------------------------

    static Ogre::Material* CreateGroundMaterialTextured(const std::string & name, const Ogre::Texture* texture);
    //-------------------------------------------------------

    std::string mName;
    Ogre::SceneManager* mSceneManager;
    //Ogre::ManualObject* mObject;
    Ogre::TexturePtr mTexture;
    std::vector<Ogre::Entity*> mEntities;
    Ogre::SceneNode* mRootNode;
    //-------------------------------------------------------



    /**
     *	Create ground submesh for the given area of the height map
     *  @param pixels - are of the height map
     *  @param offset - top left vertex position
     *  @param steps - horizontal and vertical steps between vertex
     */
    Ogre::MeshPtr CreateRegion(size_t id, const std::string & material, Ogre::PixelBox& pixels, const Ogre::Vector3 & offset, const Ogre::Vector3 & steps, const Ogre::Vector2 & texOffset);

    Ground(const Ground&) = delete;
    Ground& operator=(const Ground&) = delete;
    //-------------------------------------------------------
public:
    Ground(const std::string & name, Ogre::SceneManager* sceneManager);
    ~Ground();

    void LoadFromHeightMap(const Ogre::Texture* hmap, Ogre::SceneNode* parentNode);

    //Ogre::Entity* GetEntity()
    //{
    //    return mEntity;
    //}

    Ogre::SceneNode* GetNode()
    {
        return mRootNode;
    }

};


#endif