/*
-----------------------------------------------------------------------------
Filename:    MinimalOgre.h
-----------------------------------------------------------------------------
 
This source file is part of the
   ___                 __    __ _ _    _ 
  /___\__ _ _ __ ___  / / /\ \ (_) | _(_)
 //  // _` | '__/ _ \ \ \/  \/ / | |/ / |
/ \_// (_| | | |  __/  \  /\  /| |   <| |
\___/ \__, |_|  \___|   \/  \/ |_|_|\_\_|
      |___/                              
      Tutorial Framework
      http://www.ogre3d.org/tikiwiki/
-----------------------------------------------------------------------------
*/
#ifndef __MinimalOgre_h_
#define __MinimalOgre_h_
 
#include <OgreCamera.h>
#include <OgreEntity.h>
#include <OgreLogManager.h>
#include <OgreRoot.h>
#include <OgreViewport.h>
#include <OgreSceneManager.h>
#include <OgreRenderWindow.h>
#include <OgreConfigFile.h>
#include <OgreWindowEventUtilities.h>
#include <OgreTextureManager.h>
#include <OgreMaterialManager.h>
#include <OgreOverlay.h>

#include <OISEvents.h>
#include <OISInputManager.h>
#include <OISKeyboard.h>
#include <OISMouse.h>
 
#include <SdkTrays.h>
#include <SdkCameraMan.h>

#include <OgreRenderTarget.h>
#include <OgreRenderTargetListener.h>
#include <OgreCompositorInstance.h>

#include "CameraManagerRts.h"

#if OGRE_VERSION_MINOR == 9 && OGRE_VERSION_PATCH < 1
//In OGRE SDK 1.9.0 is used name HashMap
#define OGRE_HashMap HashMap
#endif

class World;

class MinimalOgre : public Ogre::FrameListener, 
	public Ogre::WindowEventListener, public OIS::KeyListener, 
	public OIS::MouseListener, OgreBites::SdkTrayListener,
	public Ogre::RenderTargetListener,
	public Ogre::CompositorInstance::Listener
{
public:
    MinimalOgre(void);
    virtual ~MinimalOgre(void);
    bool go(void);
protected:

    static const Ogre::Real ROTATION_VELOCITY;
    static const Ogre::Real ZOOM_VELOCITY;
    static const Ogre::Real HEAD_SCALE_MIN;
    static const Ogre::Real HEAD_SCALE_MAX;

    Ogre::Timer mTimer;

    Ogre::Root *mRoot;
    Ogre::Camera* mCamera;
    Ogre::SceneManager* mSceneMgr;
    Ogre::RenderWindow* mWindow;
    Ogre::String mResourcesCfg;
    Ogre::String mPluginsCfg;
 
	Ogre::OverlaySystem *mOverlaySystem;
 
    // OgreBites
	OgreBites::InputContext mInputContext;
    OgreBites::SdkTrayManager* mTrayMgr;
    //OgreBites::SdkCameraMan* mCameraMan;      // basic camera controller
    CameraManagerRts* mCameraMan;      // rts camera controller
    //OgreBites::ParamsPanel* mDetailsPanel;    // sample details panel
    //bool mCursorWasVisible;                   // was cursor visible before dialog appeared
    bool mShutDown;
 
    // OIS Input devices
    OIS::InputManager* mInputManager;
    OIS::Mouse*    mMouse;
    OIS::Keyboard* mKeyboard;
 
    // Ogre::FrameListener
    virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);
 
    // OIS::KeyListener
    virtual bool keyPressed( const OIS::KeyEvent &arg );
    virtual bool keyReleased( const OIS::KeyEvent &arg );
    // OIS::MouseListener
    virtual bool mouseMoved( const OIS::MouseEvent &arg );
    virtual bool mousePressed( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
    virtual bool mouseReleased( const OIS::MouseEvent &arg, OIS::MouseButtonID id );
 
    // Ogre::WindowEventListener
    virtual void windowResized(Ogre::RenderWindow* rw);
    virtual void windowClosed(Ogre::RenderWindow* rw);

    // Ogre::TrayListener
    virtual void checkBoxToggled(OgreBites::CheckBox* box);

	// Ogre::RenderTargetListener
	virtual void preRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);
	virtual void postRenderTargetUpdate(const Ogre::RenderTargetEvent& evt);

private:

	static Ogre::RenderTarget* CreateRenderTarget(const Ogre::String & name, Ogre::Camera * camera, size_t width, size_t height);

	Ogre::RenderTarget* mRenderTarget;
	Ogre::Entity* mOgreHead;
	Ogre::Entity* mBgTexturePlane;

    void SetupEffectsGui();
	void CreateMaterials();
	void SetupScene();
    void SetupPostEffects();

    std::unique_ptr<World> mWorld;
};
 
#endif // #ifndef __MinimalOgre_h_