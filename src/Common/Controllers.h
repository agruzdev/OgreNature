/**
* @file Controllers.h
*
* Copyright (c) 2015 by Gruzdev Alexey
*
* Code covered by the MIT License
* The authors make no representations about the suitability of this software
* for any purpose. It is provided "as is" without express or implied warranty.
*/


#ifndef _CONTROLLERS_H_
#define _CONTROLLERS_H_

#include <type_traits>
#include <OgreException.h>


template <typename DT, typename Enable = void>
class TimeStepController
{ };

template <typename DT>
class TimeStepController<DT, std::enable_if_t<std::is_signed<DT>::value> >
{
    DT mPreviousTime;
    DT mStep;
    
    TimeStepController(const TimeStepController&) = delete;
    TimeStepController& operator=(const TimeStepController&) = delete;
    //-------------------------------------------------------
public:
    /**
     *	Create time step controller with specific step value
     */
    TimeStepController(const DT & step = static_cast<DT>(0)):
        mPreviousTime(static_cast<DT>(-1)), mStep(step)
    {
        if (step < static_cast<DT>(0))
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Time step can't be negative!", "TimeStepController");
        }
    }
    /**
     *	Set new step value
     */
    void SetStep(const DT & step)
    {
        if (step < static_cast<DT>(0))
        {
            OGRE_EXCEPT(Ogre::Exception::ERR_INVALIDPARAMS, "Time step can't be negative!", "TimeStepController");
        }
        mStep = step;
    }
    /**
     *	Update timer with the current time and get tick
     *  @return true if the Step time interval is over
     */
    bool Tick(const DT & time)
    {
        bool tick = false;
        if (time - mPreviousTime > mStep || mPreviousTime < static_cast<DT>(0))
        {
            //ToDo: maybe should be more precise
            mPreviousTime = time;
            tick = true;
        }
        return tick;
    }
};

#endif