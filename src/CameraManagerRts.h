/**
* @file CameraManagerRts.h
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/

#ifndef _CAMERA_MANAGER_RTS_H_
#define _CAMERA_MANAGER_RTS_H_

#include "OgreCamera.h"
#include "OgreSceneNode.h"
#include "OgreFrameListener.h"

//Rewritten SdkCameraMan
class CameraManagerRts
{
protected:
    Ogre::Camera* mCamera = nullptr;
    
    Ogre::Real mTopSpeed = 150.0f;
    Ogre::Vector3 mVelocity = Ogre::Vector3::ZERO;

    bool mGoingLeft = false;
    bool mGoingRight = false;
    bool mGoingUp = false;
    bool mGoingDown = false;

    bool mZoomingMore = false;
    bool mZoomingLess = false;

    bool mTiltForward = false;
    bool mTiltBackward = false;

    bool mFastMove = false;
    //-------------------------------------------------------

public:
    CameraManagerRts(Ogre::Camera* camera)
    {
        setCamera(camera);
    }

    virtual ~CameraManagerRts() {}

    /**
     * Swaps the camera on our camera man for another camera.
     */
    virtual void setCamera(Ogre::Camera* cam)
    {
        mCamera = cam;
    }

    virtual Ogre::Camera* getCamera()
    {
        return mCamera;
    }

    /**
     * Sets the camera's top speed. Only applies for free-look style.
     */
    virtual void setTopSpeed(Ogre::Real topSpeed)
    {
        mTopSpeed = topSpeed;
    }

    virtual Ogre::Real getTopSpeed()
    {
        return mTopSpeed;
    }

    /**
     * Manually stops the camera when in free-look mode.
     */
    virtual void manualStop()
    {
        mGoingLeft = false;
        mGoingRight = false;
        mGoingUp = false;
        mGoingDown = false;
        mZoomingMore = false;
        mZoomingLess = false;
        mVelocity = Ogre::Vector3::ZERO;
    }

    /**
     *	Update camera position
     */
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt)
    {
        if (mTiltForward)
        {
            mCamera->pitch(Ogre::Radian(Ogre::Degree(-0.1f)));
        }
        if (mTiltBackward)
        {
            mCamera->pitch(Ogre::Radian(Ogre::Degree(0.1f)));
        }
        

        // build our acceleration vector based on keyboard input composite
        Ogre::Vector3 accel = Ogre::Vector3::ZERO;
        
        //ToDo: temporal solution
        Ogre::Vector3 up = mCamera->getDirection();
        up[1] = 0.0f;
        up.normalise();

        if (mGoingRight) accel += mCamera->getRight();
        if (mGoingLeft) accel -= mCamera->getRight();
        if (mGoingUp) accel += up; //mCamera->getUp();
        if (mGoingDown) accel -= up; //mCamera->getUp();
        if (mZoomingMore) accel += mCamera->getDirection();
        if (mZoomingLess) accel -= mCamera->getDirection();

        // if accelerating, try to reach top speed in a certain time
        Ogre::Real topSpeed = mFastMove ? mTopSpeed * 20 : mTopSpeed;
        if (accel.squaredLength() != 0)
        {
            accel.normalise();
            mVelocity += accel * topSpeed * evt.timeSinceLastFrame * 10;
        }
        // if not accelerating, try to stop in a certain time
        else mVelocity -= mVelocity * evt.timeSinceLastFrame * 10;

        Ogre::Real tooSmall = std::numeric_limits<Ogre::Real>::epsilon();

        // keep camera velocity below top speed and above epsilon
        if (mVelocity.squaredLength() > topSpeed * topSpeed)
        {
            mVelocity.normalise();
            mVelocity *= topSpeed;
        }
        else if (mVelocity.squaredLength() < tooSmall * tooSmall)
            mVelocity = Ogre::Vector3::ZERO;

        if (mVelocity != Ogre::Vector3::ZERO) mCamera->move(mVelocity * evt.timeSinceLastFrame);
        

        return true;
    }

    /**
     * Processes key presses for free-look style movement.
     */
    virtual void injectKeyDown(const OIS::KeyEvent& evt)
    {
        switch (evt.key)
        {
        case OIS::KC_W:
        case OIS::KC_UP:
            mGoingUp = true;
            break;
        case OIS::KC_S:
        case OIS::KC_DOWN:
            mGoingDown = true;
            break;
        case OIS::KC_A:
        case OIS::KC_LEFT:
            mGoingLeft = true;
            break;
        case OIS::KC_D:
        case OIS::KC_RIGHT:
            mGoingRight = true;
            break;
        case OIS::KC_E:
            mZoomingMore = true;
            break;
        case OIS::KC_Q:
            mZoomingLess = true;
            break;
        case OIS::KC_LSHIFT:
            mFastMove = true;
            break;
        case OIS::KC_PGUP:
            mTiltBackward = true;
            break;
        case OIS::KC_PGDOWN:
            mTiltForward = true;
            break;
        default:
            break;
        }
    }

    /**
     * Processes key releases for free-look style movement.
     */
    virtual void injectKeyUp(const OIS::KeyEvent& evt)
    {
        switch (evt.key)
        {
        case OIS::KC_W:
        case OIS::KC_UP:
            mGoingUp = false;
            break;
        case OIS::KC_S:
        case OIS::KC_DOWN:
            mGoingDown = false;
            break;
        case OIS::KC_A:
        case OIS::KC_LEFT:
            mGoingLeft = false;
            break;
        case OIS::KC_D:
        case OIS::KC_RIGHT:
            mGoingRight = false;
            break;
        case OIS::KC_E:
            mZoomingMore = false;
            break;
        case OIS::KC_Q:
            mZoomingLess = false;
            break;
        case OIS::KC_LSHIFT:
            mFastMove = false;
            break;
        case OIS::KC_PGUP:
            mTiltBackward = false;
            break;
        case OIS::KC_PGDOWN:
            mTiltForward = false;
            break;
        default:
            break;
        }
    }

    /**
     * Processes mouse movement differently for each style.
     */
    virtual void injectPointerMove(const OIS::PointerEvent& evt)
    {
        (void)evt;
    }

    /**
     * Processes mouse presses. Only applies for orbit style.
     * Left button is for orbiting, and right button is for zooming.
     */
    virtual void injectPointerDown(const OIS::PointerEvent& evt, OIS::MouseButtonID id)
    {
        (void)evt;
        (void)id;
    }

    /**
     * Processes mouse releases. Only applies for orbit style.
     * Left button is for orbiting, and right button is for zooming.
     */
    virtual void injectPointerUp(const OIS::PointerEvent& evt, OIS::MouseButtonID id)
    {
        (void)evt;
        (void)id;
    }

};

#endif