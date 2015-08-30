/**
* @file World.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#include "World.h"

#include <OgreSceneManager.h>
#include <OgreTexture.h>
#include <OgreTextureManager.h>
#include <OgreSceneNode.h>
#include <OgreEntity.h>
#include <OgreMatrix3.h>
#include <OgreMatrix4.h>

#include "Ground.h"

World::World(const std::string & name, Ogre::SceneManager* sceneManager):
    mName(name), mSceneManager(sceneManager)
{

    std::shared_ptr<Ogre::Image> heightMapImage = std::make_shared<Ogre::Image>();
    heightMapImage->load("terrain.jpg", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    mGround = std::make_unique<Ground>("Ground", mSceneManager);
    mGround->LoadFromHeightMap(heightMapImage, mSceneManager->getRootSceneNode());

    mGroundNode = mGround->GetNode();
    mGroundNode->setScale(Ogre::Vector3(0.27f, 0.27f, 1.0f));

    Ogre::Quaternion rot;
    rot.FromAngleAxis(Ogre::Radian(Ogre::Degree(-90)), Ogre::Vector3::UNIT_X);
    mGroundNode->setOrientation(rot);

}
//-------------------------------------------------------
World::~World()
{
    //empty
}
//-------------------------------------------------------
std::tuple<bool, Ogre::Vector3, Ogre::Entity*> World::GetIntersection(const Ogre::Ray & ray)
{
    //transform world space to local space
    Ogre::Matrix4 groundInvWorldMat;
    groundInvWorldMat.makeInverseTransform(mGroundNode->getPosition(), mGroundNode->getScale(), mGroundNode->getOrientation());
    Ogre::Matrix4 groundInvWorldMatNoTrans;
    groundInvWorldMatNoTrans.makeInverseTransform(Ogre::Vector3::ZERO, mGroundNode->getScale(), mGroundNode->getOrientation());

    Ogre::Ray localSpaceRay;
    localSpaceRay.setOrigin(groundInvWorldMat.transformAffine(ray.getOrigin()));
    localSpaceRay.setDirection(groundInvWorldMatNoTrans.transformAffine(ray.getDirection()).normalisedCopy());

    auto hit = mGround->GetIntersectionLocalSpace(localSpaceRay);
    if (hit.first)
    {
        Ogre::Matrix4 groundWorldMat;
        groundWorldMat.makeTransform(mGroundNode->getPosition(), mGroundNode->getScale(), mGroundNode->getOrientation());

        return std::make_tuple(true, groundWorldMat.transformAffine(hit.second), nullptr);
    }
    return std::make_tuple(false, Ogre::Vector3::ZERO, nullptr);
}