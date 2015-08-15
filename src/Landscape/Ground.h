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

#include <string>
#include <OgreCommon.h>
#include <OgreTexture.h>

namespace Ogre
{
    class SceneManager;
    class Entity;
    class Texture;
    class ManualObject;
}

class Ground
{
    static const size_t GROUND_SIZE;
    //-------------------------------------------------------

    std::string mName;
    Ogre::SceneManager* mSceneManager;
    Ogre::TexturePtr mTexture;
    Ogre::Entity* mEntity;
    //-------------------------------------------------------

    void CreateRegion(Ogre::ManualObject* obj, const Ogre::Texture* hmap, Ogre::Rect roi);

    Ground(const Ground&) = delete;
    Ground& operator=(const Ground&) = delete;
    //-------------------------------------------------------
public:
    Ground(const std::string & name, Ogre::SceneManager* sceneManager);
    ~Ground();

    void LoadFromHeightMap(const Ogre::Texture* hmap);

    Ogre::Entity* GetEntity()
    {
        return mEntity;
    }
};


#endif