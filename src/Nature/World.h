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

class World
{
    std::string mName;

    Ogre::SceneManager* mSceneManager;

    std::unique_ptr<Ground> mGround;
    Ogre::SceneNode* mGroundNode;

    //-------------------------------------------------------

    World(const World&) = delete;
    World& operator=(const World&) = delete;
    //-------------------------------------------------------

public:
    World(const std::string & name, Ogre::SceneManager* sceneManager);
    ~World();


    std::tuple<bool, Ogre::Vector3, Ogre::Entity*> GetIntersection(const Ogre::Ray & ray);

};


#endif