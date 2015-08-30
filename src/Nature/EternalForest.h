/**
* @file EternalForest.h
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/


#ifndef _ETERNAL_FOREST_H_
#define _ETERNAL_FOREST_H_

#include <memory>
#include <cstdint>

#include <OgrePrerequisites.h>
#include <OgreCommon.h>
#include <OgreAxisAlignedBox.h>
#include <OgreVector2.h>

namespace boost
{
    template<typename T, std::size_t NumDims, typename Allocator>
    class multi_array;
}

namespace Ogre
{
    class SceneManager;
    class Entity;
}


class Ground;
class World;


class EternalForest
{
    struct BlockInfo
    {
        enum : uint8_t
        {
            EMPTY = 1,
            BLOCKED = 2,
            TREE = 4
        };

        Ogre::Entity* tree = nullptr;
        uint8_t flags = EMPTY;
        float _height = 0.0f;
    };
    using LifeField = boost::multi_array<BlockInfo, 2, std::allocator<BlockInfo> >;

    static const float FIELD_BLOCK_SIZE;

    Ogre::SceneManager* mSceneManager;
    const World* mWorld;
    const Ground* mGround;

    size_t mTreesQuota = 1000;
    Ogre::AxisAlignedBox mBorders;

    std::unique_ptr<LifeField> mLifeField;
    Ogre::Vector2 mFieldOffset = Ogre::Vector2::ZERO;

protected:
    void InitField(size_t startAmount);
    void UpdateField(float time, size_t quota);

public:
    /**
     * Create eternal forest
     * @param forestBorders - box of borders: XZ borders of the forest and min and max Y value of the ground where trees can appear
     */
    EternalForest(Ogre::SceneManager* sceneManager, const World* world, const Ground* ground, const Ogre::AxisAlignedBox & forestBorders);
    /**
     *	Destructor
     */
    ~EternalForest();

    /**
     *	Update
     */
    void Update(float time);
};


#endif