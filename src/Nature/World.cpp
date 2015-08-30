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
#include "EternalForest.h"

#include <OgreSubEntity.h>

static Ogre::AxisAlignedBox TransformBox(const Ogre::AxisAlignedBox & box, const Ogre::Vector3 & translate, const Ogre::Vector3 & scale, const Ogre::Quaternion & rotation)
{
    Ogre::Matrix4 transformMatrix;
    transformMatrix.makeTransform(translate, scale, rotation);

    Ogre::Vector3 min = transformMatrix.transformAffine(box.getMinimum());
    Ogre::Vector3 max = transformMatrix.transformAffine(box.getMaximum());

    Ogre::Real mx, my, mz, Mx, My, Mz;
    std::tie(mx, Mx) = std::minmax(min[0], max[0]);
    std::tie(my, My) = std::minmax(min[1], max[1]);
    std::tie(mz, Mz) = std::minmax(min[2], max[2]);
    return Ogre::AxisAlignedBox(mx, my, mz, Mx, My, Mz);
}


World::World(const std::string & name, Ogre::SceneManager* sceneManager):
    mName(name), mSceneManager(sceneManager)
{

    std::shared_ptr<Ogre::Image> heightMapImage = std::make_shared<Ogre::Image>();
    heightMapImage->load("terrain.jpg", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);

    mGround = std::make_unique<Ground>("Ground", mSceneManager);
    mGround->LoadFromHeightMap(heightMapImage, mSceneManager->getRootSceneNode());

    Ogre::Vector3 groundScale = Ogre::Vector3(0.27f, 0.27f, 1.0f);
    Ogre::Quaternion groundOrientation;
    groundOrientation.FromAngleAxis(Ogre::Radian(Ogre::Degree(-90)), Ogre::Vector3::UNIT_X);

    mGroundNode = mGround->GetNode();
    mGroundNode->setScale(groundScale);
    mGroundNode->setOrientation(groundOrientation);

    Ogre::AxisAlignedBox bounds = mGround->GetLocalSpaceBounds();
    bounds.setMinimumZ(1.0f);
    bounds.setMaximumZ(bounds.getMaximum()[1] * 0.7f);
    mForest = std::make_unique<EternalForest>(mSceneManager, this, mGround.get(), TransformBox(bounds, Ogre::Vector3::ZERO, groundScale, groundOrientation));
}
//-------------------------------------------------------
World::~World()
{
    //empty
}
//-------------------------------------------------------
std::tuple<bool, Ogre::Vector3, Ogre::Entity*> World::GetIntersection(const Ogre::Ray & ray) const
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
//-------------------------------------------------------
void World::Update(float time)
{
    if (nullptr != mForest.get())
    {
        mForest->Update(time);
    }
}
//-------------------------------------------------------
float World::GetGroundHeightAt(float x, float z) const
{
    Ogre::Ray ray;
    ray.setOrigin(Ogre::Vector3(x, 1000.0f, z));
    ray.setDirection(Ogre::Vector3(0.0f, -1.0f, 0.0f));
    auto hit = GetIntersection(ray);
    return std::get<0>(hit) ? std::get<1>(hit)[1] : std::numeric_limits<float>::infinity();
}