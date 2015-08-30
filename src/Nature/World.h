/**
* @file World.h
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/


#ifndef _WORLD_H_
#define _WORLD_H_

#include <memory>
#include <OgrePrerequisites.h>
#include <OgreVector3.h>
#include <OgreRay.h>

namespace Ogre
{
    class SceneManager;
    class Entity;
    class SceneNode;
}

class Ground;
class EternalForest;

class World
{
    std::string mName;

    Ogre::SceneManager* mSceneManager;

    std::unique_ptr<Ground> mGround;
    Ogre::SceneNode* mGroundNode;

    std::unique_ptr<EternalForest> mForest;

    //-------------------------------------------------------

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    //-------------------------------------------------------

public:
    World(const std::string & name, Ogre::SceneManager* sceneManager);
    ~World();
    /**
     *	Update world's state
     */
    void Update(float time);
    /**
     *	Find intersection with a ray
     *  @param ray - a ray in world space
     *  @return intersection status, intersection position, intersected object 
     */
    std::tuple<bool, Ogre::Vector3, Ogre::Entity*> GetIntersection(const Ogre::Ray & ray) const;

    /**
     *	Ground is supposed to be parallel to the XZ plane
     */
    float GetGroundHeightAt(float x, float z) const;
};


#endif