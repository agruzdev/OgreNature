/**
* @file EternalForest.cpp
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/


#include "EternalForest.h"

#include <boost/multi_array.hpp>

#include <OgreSceneManager.h>
#include <OgreEntity.h>
#include <OgreSceneNode.h>

#include "Ground.h"
#include "World.h"

const float EternalForest::FIELD_BLOCK_SIZE  = 1.0f;
const float EternalForest::FIELD_UPDATE_TICK = 1.0f;
//-------------------------------------------------------
EternalForest::EternalForest(Ogre::SceneManager* sceneManager, const World* world, const Ground* ground, const Ogre::AxisAlignedBox & forestBorders):
    mBorders(forestBorders), mSceneManager(sceneManager), mGround(ground), mWorld(world), mUpdateTickController(FIELD_UPDATE_TICK)
{
    
}
//-------------------------------------------------------
EternalForest::~EternalForest()
{

}
//-------------------------------------------------------
void EternalForest::InitField(size_t startAmount)
{
    Ogre::Vector3 minBorder = mBorders.getMinimum();
    Ogre::Vector3 maxBorder = mBorders.getMaximum();

    uint32_t fieldSizeX = static_cast<uint32_t>((maxBorder[0] - minBorder[0]) / FIELD_BLOCK_SIZE);
    uint32_t fieldSizeZ = static_cast<uint32_t>((maxBorder[2] - minBorder[2]) / FIELD_BLOCK_SIZE);

    mFieldOffset[0] = 0.5f * std::fmod(maxBorder[0] - minBorder[0], FIELD_BLOCK_SIZE) + minBorder[0];
    mFieldOffset[1] = 0.5f * std::fmod(maxBorder[0] - minBorder[0], FIELD_BLOCK_SIZE) + minBorder[2];

    mLifeField = std::make_unique<LifeField>(boost::extents[fieldSizeZ][fieldSizeX]);
    mLifeFieldNext = std::make_unique<LifeField>(boost::extents[fieldSizeZ][fieldSizeX]);
    
    LifeField& field = *mLifeField;
    for (uint32_t z = 0; z < fieldSizeZ; ++z)
    {
        for (uint32_t x = 0; x < fieldSizeX; ++x)
        {
            if (z == 0 || z == fieldSizeZ - 1 || x == 0 || x == fieldSizeX - 1)
            {
                field[z][x].flags = BlockInfo::BLOCKED;
                continue;
            }
            float s = mFieldOffset[0] + (x + 0.5f) * FIELD_BLOCK_SIZE;
            float t = mFieldOffset[1] + (z + 0.5f) * FIELD_BLOCK_SIZE;
            float h = mWorld->GetGroundHeightAt(s, t);
            field[z][x].flags = (h >= minBorder[1] && h <= maxBorder[1]) ? BlockInfo::EMPTY : BlockInfo::BLOCKED;
            field[z][x]._height = h;
        }
    }

    //generate random start positions
    size_t amount = std::min(startAmount, fieldSizeX * fieldSizeZ);
    while (amount > 0)
    {
        uint8_t attempts = 0;
        while (attempts < 10)
        {
            uint32_t x = static_cast<uint32_t>(Ogre::Math::UnitRandom() * fieldSizeX);
            uint32_t z = static_cast<uint32_t>(Ogre::Math::UnitRandom() * fieldSizeZ);
            if ((*mLifeField)[z][x].flags & BlockInfo::EMPTY)
            {
                (*mLifeField)[z][x].flags = BlockInfo::TREE;
                auto tree = mSceneManager->createEntity("tree_1.mesh");
                auto node = mSceneManager->getRootSceneNode()->createChildSceneNode();
                node->setScale(0.0005f, 0.0005f, 0.0005f);
                node->setPosition(Ogre::Vector3(mFieldOffset[0] + (x + 0.5f) * FIELD_BLOCK_SIZE, (*mLifeField)[z][x]._height, mFieldOffset[1] + (z + 0.5f) * FIELD_BLOCK_SIZE));
                node->attachObject(tree);
                (*mLifeField)[z][x].treeNode = node;
                break;
            }
            ++attempts;
        }
        --amount;
    }
}
//-------------------------------------------------------
void EternalForest::UpdateField(float time, size_t quota)
{
    LifeField& field = *mLifeField;
    uint32_t width =  mLifeField->shape()[0];
    uint32_t height = mLifeField->shape()[1];

    for (uint32_t z = 1; z < height - 1; ++z)
    {
        for (uint32_t x = 1; x < width - 1; ++x)
        {
            BlockInfo& current = field[z][x];
            BlockInfo& next = (*mLifeFieldNext)[z][x];
            assert(current.flags == BlockInfo::BLOCKED || current.flags == BlockInfo::TREE || current.flags == BlockInfo::EMPTY);

            next = current;
            if (0 == (current.flags & BlockInfo::BLOCKED))
            {
                auto neighbors = {
                    field[z - 1][x - 1].flags,
                    field[z - 1][x].flags,
                    field[z - 1][x + 1].flags,
                    field[z][x - 1].flags,
                    field[z][x + 1].flags,
                    field[z + 1][x - 1].flags,
                    field[z + 1][x].flags,
                    field[z + 1][x + 1].flags
                };
                uint8_t sum = 0;
                std::for_each(std::cbegin(neighbors), std::cend(neighbors), [&sum](const uint8_t& flags)->void { if(flags & BlockInfo::TREE) ++sum; });
                if (current.flags & BlockInfo::TREE)
                {
                    if (sum < 3 || sum > 4)
                    {
                        current.treeNode->detachAllObjects();
                        current.treeNode->removeAndDestroyAllChildren();
                        current.treeNode = nullptr;

                        next.flags = BlockInfo::EMPTY;
                        next.treeNode = nullptr;
                    }
                }
                else if (current.flags & BlockInfo::EMPTY)
                {
                    if (sum >= 3 && sum <= 4)
                    {
                        auto tree = mSceneManager->createEntity("tree_1.mesh");
                        auto node = mSceneManager->getRootSceneNode()->createChildSceneNode();
                        node->setScale(0.0005f, 0.0005f, 0.0005f);
                        node->setPosition(Ogre::Vector3(mFieldOffset[0] + (x + 0.5f) * FIELD_BLOCK_SIZE, (*mLifeField)[z][x]._height, mFieldOffset[1] + (z + 0.5f) * FIELD_BLOCK_SIZE));
                        node->attachObject(tree);

                        next.flags = BlockInfo::TREE;
                        next.treeNode = node;
                    }
                }
            }
        }
    }
    mLifeField.swap(mLifeFieldNext);
}
//-------------------------------------------------------
void EternalForest::Update(float time)
{
    if (nullptr == mLifeField.get())
    {
        InitField(6 * mTreesQuota);
    }
    else
    {
        if (mUpdateTickController.Tick(time))
        {
            UpdateField(time, mTreesQuota);
        }
    }
}