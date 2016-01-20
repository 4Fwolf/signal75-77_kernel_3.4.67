/********************************************************************************************
 *     LEGAL DISCLAIMER
 *
 *     (Header of MediaTek Software/Firmware Release or Documentation)
 *
 *     BY OPENING OR USING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 *     THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE") RECEIVED
 *     FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON AN "AS-IS" BASIS
 *     ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES, EXPRESS OR IMPLIED,
 *     INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS FOR
 *     A PARTICULAR PURPOSE OR NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY
 *     WHATSOEVER WITH RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 *     INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK
 *     ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
 *     NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S SPECIFICATION
 *     OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
 *
 *     BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE LIABILITY WITH
 *     RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION,
TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE
 *     FEES OR SERVICE CHARGE PAID BY BUYER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 *     THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE WITH THE LAWS
 *     OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF LAWS PRINCIPLES.
 ************************************************************************************************/
//
#define LOG_TAG "scenario/mHal3A"
#include <utils/threads.h>
//
#include <mhal/inc/camera.h>
#include <scenario_types.h>
//
#include <mcam_log.h>
#include <mcam_profile.h>

#include "mhal_cam.h"
#include "kd_camera_feature.h"
#include "camera_custom_if.h"


#ifdef  MHAL_CAM_FLICKER_SERVICE_SUPPORT
  #include <frameservice/IFlickerService.h>
#endif

#define DYNAMIC_FRAME_COUNT (10)

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCam3AProc(
)
{
    MINT32 ret = MHAL_NO_ERROR;
    mHalCamState_e nowState;
    static AAA_STATE_T aaaState = AAA_STATE_AUTO_FRAMERATE_PREVIEW;
    CamTimer camTmr("mHalCam3AProc", MHAL_CAM_PRV_PROFILE_OPT);
    //MCAM_DBG("[mHalCam3AProc] Start \n");
    nowState = mHalCamGetState();
    switch (nowState) {
    case MHAL_CAM_PREVIEW:
        aaaState = ((mmHalCamParam.u4CamMode != MHAL_CAM_MODE_MVIDEO) && (mmHalCamParam.u4CamMode != MHAL_CAM_MODE_VT))
            ?   AAA_STATE_AUTO_FRAMERATE_PREVIEW    //  camera preview
            :   AAA_STATE_MANUAL_FRAMERATE_PREVIEW; //  video preview
        break;
    case MHAL_CAM_FOCUS:
        aaaState = AAA_STATE_AF;
        break;
    case MHAL_CAM_PRE_CAPTURE:
        aaaState = AAA_STATE_PRE_CAPTURE;
        break;
    case MHAL_CAM_CAPTURE:
        aaaState = AAA_STATE_CAPTURE;
        break;
    case MHAL_CAM_STOP:
        return ret;
        break;
    default :
        // Remain previous state
        break;
    }
    if (mAaaState != nowState) {
        if ( (nowState == MHAL_CAM_PREVIEW) ||
             (nowState == MHAL_CAM_FOCUS) ||
             (nowState == MHAL_CAM_PRE_CAPTURE) ) {
            MCAM_DBG("[mHalCam3AProc] State, Pre: 0x%x, Now: 0x%x \n", mAaaState, nowState);
            mpHal3AObj->setState(aaaState);
        }
        mAaaState = nowState;
    }
    //
    AfCallBackData AfData;
    memset(&AfData, 0, sizeof(AfData));

#if 0 // move after do3A() for speed up AF
    if (nowState == MHAL_CAM_FOCUS) {
        MCAM_DBG("[mHalCam3AProc] AF Done = %d\n", mpHal3AObj->getAFDone()); 
        if (mpHal3AObj->getAFDone() && 0 == mIsFocusCBDone) {
            AfData.isFocused = (MUINT32)mpHal3AObj->isFocused();
            AfData.afSize = sizeof(AF_WIN_RESULT_T);
            AfData.winResult.i4Count = mpHal3AObj->getAFWinResult((MINT32*)AfData.winResult.sRect, 
                &AfData.winResult.i4Width, &AfData.winResult.i4Height);
            // reset
            mPreAfStatus = AF_MARK_NONE;
            mCurAfStatus = AF_MARK_NONE;

            mHalCamCBHandle(MHAL_CAM_CB_AF, &AfData);
            mIsFocusCBDone = 1;
            if (mmHalCamParam.cam3AParam.afEngMode == 0) {
                mAFBestPos = mpHal3AObj->getAFBestPos();
            }
            // delay the smile indicator flag here due to the 
            // SMILE PREVIEW command may send before Autofocus done

                /* // ICS FD: no indicator anymore
                   // FIXME: needed?
                if (mIsSmileCapture == 1) {
                    mIsSmileIndicator = 1; 
                }
                */
        }
    }
#endif

    //
    if (mAaaWaitVDNum > 1) {
        mAaaWaitVDNum--;
        MCAM_DBG("  aaaWaitVDNum: %d \n", mAaaWaitVDNum);
    }
    else {
        if (mmHalCamParam.u4CamMode == MHAL_CAM_MODE_MVIDEO) {
            using namespace NSCamCustom;
            VdoDynamicFrameRate_T vdoDynamicFps;
            vdoDynamicFps = getParamVdoDynamicFrameRate();
        
            if(vdoDynamicFps.isEnableDFps){
                using namespace NSFeature;
                MINT32 fpsMode = 0, EVvlue = 0, EVTheshold = 0;
                mpHal3AObj->get3AParam(HAL_3A_AE_FRAMERATE_MODE, &fpsMode);
                mpHal3AObj->getLV(&EVvlue);

                if (mmHalCamParam.cam3AParam.sceneMode == SCENE_MODE_OFF) {
                    // normal mode
                    EVTheshold = vdoDynamicFps.EVThresNormal;
                }
                else if (mmHalCamParam.cam3AParam.sceneMode == SCENE_MODE_NIGHTSCENE) {
                    // night mode
                    EVTheshold = vdoDynamicFps.EVThresNight;
                }
                else {
                    MCAM_ERR("[mHalCam3AProc] scene mode error.");
                    return -1;
                }

                if(EVvlue < EVTheshold) {
                    // the environment brightness is less than EVTheshold and the frame rate is auto (dynamic)
                    aaaState = AAA_STATE_AUTO_FRAMERATE_PREVIEW;
                    if((fpsMode != 0) && (mIsDynamicFpsStable)) {
                        mpHal3AObj->setState(aaaState);
                        mIsDynamicFpsStable = MFALSE;
                        mDynamicFrameCnt = 0;
                    }
                    else {
                        mDynamicFrameCnt++;
                    }
                } else {
                    // the environment brightness is bigger or equal  than EVTheshold and the frame rate is manual (fix)
                    aaaState = AAA_STATE_MANUAL_FRAMERATE_PREVIEW;
                    if((fpsMode == 0) && (mIsDynamicFpsStable)) {
                        mpHal3AObj->set3AParam(HAL_3A_AE_FRAMERATE_MODE, (MINT32)(mmHalCamParam.cam3AParam.prvFps), NULL, NULL);
                        mpHal3AObj->setState(aaaState);
                        mIsDynamicFpsStable = MFALSE;
                        mDynamicFrameCnt = 0;
                    }
                    else {
                        mDynamicFrameCnt++;
                    }
                }
                MCAM_DBG("[mHalCam3AProc] (EV, EVThres, mDynamicFrameCnt): (%d, %d, %d) \n", EVvlue, EVTheshold, mDynamicFrameCnt);

                mIsDynamicFpsStable = (mDynamicFrameCnt > DYNAMIC_FRAME_COUNT) ? MTRUE : MFALSE;
                
            }
            // Update the metering mode status for 3A
            mpHal3AObj->setHal3AMeteringModeStatus((MINT32) mIsVideoRecording);
        }else if (mmHalCamParam.u4CamMode == MHAL_CAM_MODE_MPREVIEW) {
            // Update the metering mode status for 3A
            mpHal3AObj->setHal3AMeteringModeStatus(2);        
        }


        //
        ret = mpHal3AObj->do3A(aaaState, &mAaaWaitVDNum);
        // -1: error
        //  0: no error
        //  1: update ISO
        //  2: update CCT
        //  4: update ASD info
        if (ret < 0) {
            MCAM_DBG("  mHal3ADo err \n\n");
        }
        else {
            #if (MHAL_CAM_ISP_TUNING)
            if (ret & 0x01) {
                MINT32 iso;
                mpHal3AObj->getISO(0, &iso);
                MINT32 ret2 = mpIspHalObj->sendCommand(ISP_CMD_SET_ISO, iso);
                if (ret2 < 0) {
                    MCAM_ERR("  ISP_CMD_SET_ISO err \n\n");
                    return ret2;
                }
            }
            if (ret & 0x02) {
                AWB_CCT_T awbcct;
                mpHal3AObj->getCCT(0, &awbcct);
                MINT32 ret2 = mpIspHalObj->sendCommand(ISP_CMD_SET_FLUORESCENT_CCT, (MINT32)&awbcct);
                if (ret2 < 0) {
                    MCAM_ERR("  ISP_CMD_SET_FLUORESCENT_CCT err \n\n");
                    return ret2;
                }
            }

            /* sample code for ASD info
            if (ret & 0x04) {
                AAA_ASD_INFO_T rASDInfo;
                mpHal3AObj->getASDInfo(rASDInfo);
            }
            */

            //
            MINT32 lv;
            mpHal3AObj->getLV(&lv);
            ret = mpIspHalObj->sendCommand(ISP_CMD_SET_SCENE_LIGHT_VALUE, lv);
            if (ret < 0) {
                MCAM_ERR("  ISP_CMD_SET_SCENE_LIGHT_VALUE err \n\n");
                return ret;
            }
            //
            mpIspHalObj->sendCommand(ISP_CMD_LOCK_REG, 1);
            ret = mpIspHalObj->sendCommand(ISP_CMD_VALIDATE_FRAME, false);
            mpIspHalObj->sendCommand(ISP_CMD_LOCK_REG, 0);
            if (ret < 0) {
                MCAM_ERR("  ISP_CMD_VALIDATE_FRAME err \n\n");
                return ret;
            }
            #endif 
        }
    }


    if (nowState == MHAL_CAM_FOCUS) {
        MCAM_DBG("[mHalCam3AProc] AF Done = %d\n", mpHal3AObj->getAFDone()); 
        if (mpHal3AObj->getAFDone() && 0 == mIsFocusCBDone) {
            AfData.isFocused = (MUINT32)mpHal3AObj->isFocused();
            AfData.afSize = sizeof(AF_WIN_RESULT_T);
            AfData.winResult.i4Count = mpHal3AObj->getAFWinResult((MINT32*)AfData.winResult.sRect, 
                &AfData.winResult.i4Width, &AfData.winResult.i4Height);
            // reset
            mPreAfStatus = AF_MARK_NONE;
            mCurAfStatus = AF_MARK_NONE;

            mHalCamCBHandle(MHAL_CAM_CB_AF, &AfData);
            mIsFocusCBDone = 1;
            if (mmHalCamParam.cam3AParam.afEngMode == 0) {
                mAFBestPos = mpHal3AObj->getAFBestPos();
            }
            // delay the smile indicator flag here due to the 
            // SMILE PREVIEW command may send before Autofocus done

                /* // ICS FD: no indicator anymore
                   // FIXME: needed?
                if (mIsSmileCapture == 1) {
                    mIsSmileIndicator = 1; 
                }
                */
        }
    }
    
    if (nowState == MHAL_CAM_PREVIEW) {
        MINT32 afMode = 0;            
        MUINT32 isFocusMoving = 0; 
        mpHal3AObj->get3AParam(HAL_3A_AF_MODE, &afMode);
        MCAM_DBG("(afMode, AFC, VideoAFC1): (%d, %d, %d)", afMode, AF_MODE_AFC, AF_MODE_AFC_VIDEO);
        if (((afMode == AF_MODE_AFC) || (afMode == AF_MODE_AFC_VIDEO))){

            AfData.isFocused = (MUINT32)mpHal3AObj->isFocused();
            AfData.afSize = sizeof(AF_WIN_RESULT_T);
            AfData.winResult.i4Count = mpHal3AObj->getAFWinResult((MINT32*)AfData.winResult.sRect, 
                &AfData.winResult.i4Width, &AfData.winResult.i4Height);

            // getAFWinResult() stored eMarkStatus value in i4Bottom.
            mCurAfStatus = AfData.winResult.sRect[0].i4Bottom;

            if (mCurAfStatus != mPreAfStatus) {
                MCAM_DBG("(mCurAfStatus, mPreAfStatus2): (%d, %d)", mCurAfStatus, mPreAfStatus);
                mHalCamCBHandle(MHAL_CAM_CB_AF, &AfData);                                
                isFocusMoving = (mCurAfStatus == AF_MARK_NORMAL)  ? (1) : (0); 
                mHalCamCBHandle(MHAL_CAM_CB_AF_MOVE, &isFocusMoving); 
            }
            mPreAfStatus = mCurAfStatus;
        }
        else {
            mCurAfStatus = AF_MARK_NONE;

            if (mCurAfStatus != mPreAfStatus) {
                MCAM_DBG("(mCurAfStatus, mPreAfStatus3): (%d, %d)", mCurAfStatus, mPreAfStatus);
                mHalCamCBHandle(MHAL_CAM_CB_AF, &AfData);                                
                isFocusMoving = 0; 
                mHalCamCBHandle(MHAL_CAM_CB_AF_MOVE, &isFocusMoving); 
            }
            mPreAfStatus = mCurAfStatus;
        }
    }
    camTmr.printTime(); 
    return ret;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 mHalCam::mHalCam3ACtrl(
    MINT32 isAEEnable,
    MINT32 isAWBEnable,
    MINT32 isAFEnable
)
{
    MCAM_DBG("[mHalCam3ACtrl] %d, %d, %d \n", isAEEnable, isAWBEnable, isAFEnable);
    //
    if (isAEEnable) {
        mpHal3AObj->enableAE();
    }
    else {
        mpHal3AObj->disableAE();
    }
    //
    if (isAWBEnable) {
        mpHal3AObj->enableAWB();
    }
    else {
        mpHal3AObj->disableAWB();
    }
    //
    if (isAFEnable) {
        mpHal3AObj->enableAF();
    }
    else {
        mpHal3AObj->disableAF();
    }

    return 0;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamSet3AParams(
    mhalCam3AParam_t const *p3AParamNew,
    mhalCam3AParam_t const *p3AParamOld
)
{
    MINT32 err = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamSet3AParams] \n");

     // AF lamp mode
     if (p3AParamNew->afLampMode != p3AParamOld->afLampMode) {
         MCAM_DBG("  afLampMode: %d \n", p3AParamNew->afLampMode);
         if (p3AParamNew->afLampMode == 2)  {            
             mpHal3AObj->autoAFLamp();
         }
         else if (p3AParamNew->afLampMode == 1)  {
             mpHal3AObj->enableAFLamp();
         }
         else  {
             mpHal3AObj->disableAFLamp();
         }            
     }

    //
    if (p3AParamNew->strobeMode != p3AParamOld->strobeMode) {
        MCAM_DBG("  HAL_3A_AE_STROBE_MODE: %d \n", p3AParamNew->strobeMode);
        #if 0
        //set 3A strobe mode by flashlight mode.
        strobeMode = p3AParamNew->strobeMode;
        err = mHalCamFlashLightModeCtrl((MINT32)((AE_STROBE_MODE_FORCE_ON == p3AParamNew->strobeMode)?1:0),&strobeMode);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
        #endif
        err = mpHal3AObj->set3AParam(HAL_3A_AE_STROBE_MODE, p3AParamNew->strobeMode, NULL, NULL);
        if (err != MHAL_NO_ERROR)
        {
            MCAM_ERR("[mHalCamSet3AParams] wrong strobe mode = %d\n", p3AParamNew->strobeMode);
            return err;
        }

        // Use strobe menu if afLampMode == FLASH
        if (p3AParamNew->afLampMode == 3)  {

            if (p3AParamNew->strobeMode == FLASHLIGHT_AUTO)  {            
                mpHal3AObj->autoAFLamp();
            }        
            else if (p3AParamNew->strobeMode == FLASHLIGHT_FORCE_ON)  {            
                mpHal3AObj->enableAFLamp();
            }
            else {
                mpHal3AObj->disableAFLamp();
            }       
        }
    }
    //
    if (p3AParamNew->afMode != p3AParamOld->afMode) {
        MCAM_DBG("  HAL_3A_AF_MODE: %d \n", p3AParamNew->afMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AF_MODE, p3AParamNew->afMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->afMeterMode != p3AParamOld->afMeterMode) {
        MCAM_DBG("  HAL_3A_AF_METERING_MODE: %d \n", p3AParamNew->afMeterMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AF_METERING_MODE, p3AParamNew->afMeterMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
#if 1
    if (p3AParamNew->afMeterMode == AF_METER_MOVESPOT) {
        MCAM_DBG("  AF_METER_MOVESPOT: %d, %d \n", 
            p3AParamNew->afX, p3AParamNew->afY);
        mhalCamMoveSpotInfo_t msInfo = {
            p3AParamNew->afX, p3AParamNew->afY, 0, 0, 0, 0, 0
        }; 
        err = mpHal3AObj->set3AParam(HAL_3A_AF_METERING_POS, (INT32) &msInfo, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
#endif     
    //
    if (p3AParamNew->afEngPos != p3AParamOld->afEngPos) {
        MCAM_DBG("  HAL_3A_AF_Full_Step: %d \n", p3AParamNew->afEngPos);
        err = mpHal3AObj->setAFFullStep(p3AParamNew->afEngPos);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }  

    //    
#ifdef MHAL_CAM_FLICKER_SERVICE_SUPPORT
    if  ( NSCamera::IFrameService* pService = mvpFrameService[NSCamera::eFSID_Flicker] )
    {
        if  ( ! NSCamera::IFlickerService::FSCmd_SetFlickerMode(pService, p3AParamNew->antiBandingMode, mIsVideoRecording).execute() )
        {
            MCAM_ERR("[mHalCamSet3AParams] IFlickerService::FSCmd_SetFlickerMode(%d)", p3AParamNew->antiBandingMode);
            return  -1;
        }
    }
#else
    if (p3AParamNew->antiBandingMode != p3AParamOld->antiBandingMode) {
        MCAM_DBG("  HAL_3A_AE_FLICKER_MODE: %d \n", p3AParamNew->antiBandingMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AE_FLICKER_MODE, p3AParamNew->antiBandingMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
#endif  //  MHAL_CAM_FLICKER_SERVICE_SUPPORT
    //
    if (p3AParamNew->aeMeterMode != p3AParamOld->aeMeterMode) {
        MCAM_DBG("  HAL_3A_AE_METERING_MODE: %d \n", p3AParamNew->afMeterMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AE_METERING_MODE, p3AParamNew->aeMeterMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->aeExpMode != p3AParamOld->aeExpMode) {
        MCAM_DBG("  HAL_3A_AE_EVCOMP: %d \n", p3AParamNew->aeExpMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AE_EVCOMP, p3AParamNew->aeExpMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->awbMode != p3AParamOld->awbMode) {
        MCAM_DBG("  HAL_3A_AWB_MODE: %d \n", p3AParamNew->awbMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AWB_MODE, p3AParamNew->awbMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
        if (p3AParamNew->isoSpeedMode != p3AParamOld->isoSpeedMode) {
        MCAM_DBG("  HAL_3A_AE_ISOSPEED new : %d \n", p3AParamNew->isoSpeedMode);
        MCAM_DBG("  HAL_3A_AE_ISOSPEED old : %d \n", p3AParamOld->isoSpeedMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AE_ISOSPEED, p3AParamNew->isoSpeedMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    } else {
    	MCAM_DBG("  HAL_3A_AE_ISOSPEED new and old : %d \n", p3AParamNew->isoSpeedMode);
    }
    //
    if (p3AParamOld->EngMode == 0 && p3AParamNew->isoSpeedMode != p3AParamOld->isoSpeedMode) {
    	MCAM_DBG("  EngMode is 0.\n");
        MCAM_DBG("  HAL_3A_AE_ISOSPEED: %d \n", p3AParamNew->isoSpeedMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AE_ISOSPEED, p3AParamNew->isoSpeedMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
   if (p3AParamNew->isoSpeedModeEng != p3AParamOld->isoSpeedModeEng) {
    	   MCAM_DBG(" EngMode old : %d: \n" , p3AParamOld->EngMode);
    	   MCAM_DBG(" EngMode new : %d: \n" , p3AParamNew->EngMode);
           MCAM_DBG("  HAL_3A_AE_ISOSPEED_ENG new : %d \n", p3AParamNew->isoSpeedModeEng);
           MCAM_DBG("  HAL_3A_AE_ISOSPEED_ENG old : %d \n", p3AParamOld->isoSpeedModeEng);
//           err = mpHal3AObj->set3AParam(HAL_3A_AE_ISOSPEED, p3AParamNew->isoSpeedMode, NULL, NULL);
//           if (err != MHAL_NO_ERROR) {
//               return err;
//           }
       } else {
       	MCAM_DBG("  HAL_3A_AE_ISOSPEED_ENG new and old : %d \n", p3AParamNew->isoSpeedModeEng);
       }
    //
    if (p3AParamNew->EngMode > 0 && p3AParamNew->isoSpeedModeEng!= p3AParamOld->isoSpeedModeEng) {
        MCAM_DBG(" EngMode: %d, HAL_3A_AE_ISOSPEED_ENG new: %d \n", p3AParamOld->EngMode, p3AParamNew->isoSpeedModeEng);
        MCAM_DBG(" EngMode new : %d: \n" , p3AParamNew->EngMode);
        MCAM_DBG("  HAL_3A_AE_ISOSPEED_ENG old : %d \n", p3AParamOld->isoSpeedModeEng);
        err = mpHal3AObj->set3AParam(HAL_3A_AE_ISOSPEED, p3AParamNew->isoSpeedModeEng, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }

    //
    if (p3AParamNew->aeMode!= p3AParamOld->aeMode) {
        MCAM_DBG("  HAL_3A_AE_MODE: %d \n", p3AParamNew->aeMode);
        err = mpHal3AObj->set3AParam(HAL_3A_AE_MODE, p3AParamNew->aeMode, NULL, NULL);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }

    //
    if (p3AParamNew->brightnessMode != p3AParamOld->brightnessMode) {
        MCAM_DBG("  HAL_ISP_USER_BRIGHTNESS: %d \n", p3AParamNew->brightnessMode);
        err = mpIspHalObj->sendCommand(ISP_CMD_SET_BRIGHTNESS, p3AParamNew->brightnessMode);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->hueMode != p3AParamOld->hueMode) {
        MCAM_DBG("  HAL_ISP_USER_HUE: %d \n", p3AParamNew->hueMode);
        err = mpIspHalObj->sendCommand(ISP_CMD_SET_HUE, p3AParamNew->hueMode);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->saturationMode != p3AParamOld->saturationMode) {
        MCAM_DBG("  HAL_ISP_USER_SAT: %d \n", p3AParamNew->saturationMode);
        err = mpIspHalObj->sendCommand(ISP_CMD_SET_SATURATION, p3AParamNew->saturationMode);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->edgeMode != p3AParamOld->edgeMode) {
        MCAM_DBG("  HAL_ISP_USER_EDGE: %d \n", p3AParamNew->edgeMode);
        err = mpIspHalObj->sendCommand(ISP_CMD_SET_EDGE, p3AParamNew->edgeMode);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->contrastMode != p3AParamOld->contrastMode) {
        MCAM_DBG("  HAL_ISP_USER_CONTRAST: %d \n", p3AParamNew->contrastMode);
        err = mpIspHalObj->sendCommand(ISP_CMD_SET_CONTRAST, p3AParamNew->contrastMode);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->sceneMode != p3AParamOld->sceneMode) {
        MCAM_DBG("  ISP_CMD_SET_SCENE_MODE: %d \n", p3AParamNew->sceneMode);
        mpIspHalObj->sendCommand(ISP_CMD_SET_SCENE_MODE, p3AParamNew->sceneMode);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
    if (p3AParamNew->effectMode!= p3AParamOld->effectMode) {
        MCAM_DBG("  ISP_CMD_SET_EFFECT: %d \n", p3AParamNew->effectMode);
        err = mpIspHalObj->sendCommand(ISP_CMD_SET_EFFECT, p3AParamNew->effectMode);
        if (err != MHAL_NO_ERROR) {
            return err;
        }
    }
    //
#if (MHAL_CAM_EIS_OPTION)
    if (p3AParamNew->eisMode != p3AParamOld->eisMode) {
        if (mmHalCamParam.u4CamMode == MHAL_CAM_MODE_MVIDEO)
        {
            if (p3AParamNew->eisMode == MHAL_CAM_FEATURE_ON) {
                mpCameraIOObj->setZoomCropSrc(0, 0, p3AParamNew->eisW, p3AParamNew->eisH); 
            }
            else {
                mpCameraIOObj->setZoomCropSrc(0, 0, mEISSrcW, mEISSrcH); 
            }
            mmHalCamParam.cam3AParam.eisMode = p3AParamNew->eisMode;  //mHalCamEISProc() using 
            MCAM_DBG(" eisMode: %d \n", mmHalCamParam.cam3AParam.eisMode);
        }
        else 
        {
            mpCameraIOObj->setZoomCropSrc(0, 0, mEISSrcW, mEISSrcH); 
            mmHalCamParam.cam3AParam.eisMode = p3AParamNew->eisMode;  //mHalCamEISProc() using 
            MCAM_DBG(" eisMode: %d \n", mmHalCamParam.cam3AParam.eisMode);
        }
    }
#endif

    //
    if ((mmHalCamParam.u4CamMode == MHAL_CAM_MODE_MVIDEO)||(mmHalCamParam.u4CamMode == MHAL_CAM_MODE_VT)) {
        // Video frame rate is decided by manual in video mode.
        if (p3AParamNew->prvFps != p3AParamOld->prvFps) {
            MCAM_DBG("  prvFps: %d, u4CamMode: %d \n", p3AParamNew->prvFps,mmHalCamParam.u4CamMode);
            err = mpHal3AObj->set3AParam(HAL_3A_AE_FRAMERATE_MODE, p3AParamNew->prvFps, NULL, NULL);
            if (err < 0) {
                MCAM_ERR("  HAL_3A_AE_FRAMERATE_MODE fail \n");
                return err;
            }
        }
    }
    // AE lock               //android 4.0 
    MCAM_DBG(" AE Lock:%d, current:%d\n", p3AParamNew->isAELock, (int)mpHal3AObj->getAutoExposureLock()); 
    if (p3AParamNew->isAELock != (int)mpHal3AObj->getAutoExposureLock()) {
        mpHal3AObj->setAutoExposureLock(p3AParamNew->isAELock); 
    }
    // AWB lock            //android 4.0 
    MCAM_DBG(" AWB Lock:%d, current:%d\n", p3AParamNew->isAELock,  (int)mpHal3AObj->getAutoWhiteBalanceLock()); 
    if (p3AParamNew->isAWBLock != (int)mpHal3AObj->getAutoWhiteBalanceLock()) {
        mpHal3AObj->setAutoWhiteBalanceLock(p3AParamNew->isAWBLock); 
    }
    // AF Area 
    if (p3AParamNew->focusAreas.count > 0) {
        AREA_T focusArea[MAX_FOCUS_AREAS]; 
        for (MUINT32 i = 0; i < p3AParamNew->focusAreas.count; i++) {
            focusArea[i].i4Left = p3AParamNew->focusAreas.areas[i].left; 
            focusArea[i].i4Top = p3AParamNew->focusAreas.areas[i].top; 
            focusArea[i].i4Right = p3AParamNew->focusAreas.areas[i].right; 
            focusArea[i].i4Bottom = p3AParamNew->focusAreas.areas[i].bottom; 
            focusArea[i].u4Weight = p3AParamNew->focusAreas.areas[i].weight; 
        }
        mpHal3AObj->setFocusAreas(p3AParamNew->focusAreas.count, focusArea);
    }
    // AE Area 
    if (p3AParamNew->meteringAreas.count > 0) {
        AREA_T meteringArea[MAX_METERING_AREAS]; 
        for (MUINT32 i = 0; i < p3AParamNew->meteringAreas.count; i++) {
            meteringArea[i].i4Left = p3AParamNew->meteringAreas.areas[i].left; 
            meteringArea[i].i4Top = p3AParamNew->meteringAreas.areas[i].top; 
            meteringArea[i].i4Right = p3AParamNew->meteringAreas.areas[i].right; 
            meteringArea[i].i4Bottom = p3AParamNew->meteringAreas.areas[i].bottom; 
            meteringArea[i].u4Weight = p3AParamNew->meteringAreas.areas[i].weight; 
        }
        mpHal3AObj->setMeteringAreas(p3AParamNew->meteringAreas.count, meteringArea); 
    }
    
    if (p3AParamNew->isBurstShotMode != p3AParamOld->isBurstShotMode) {
        MCAM_DBG("shot mode change : %d (old), %d \n", p3AParamOld->isBurstShotMode, p3AParamNew->isBurstShotMode);        
        mpHal3AObj->setIsBurtShootMode(p3AParamNew->isBurstShotMode);
    }

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamSet3AParameter(
    VOID *a_pInBuffer,
    MUINT32 a_u4InBufSize
)
{
    MINT32 err = MHAL_NO_ERROR;
    mhalCam3AParam_t *pmhalCam3AParam = (mhalCam3AParam_t *) a_pInBuffer;
    //
    MCAM_DBG("[mHalCamSet3AParameter] \n");
    //

    MINT32 sensorW = 0, sensorH = 0;  // check whether use capture frame for video 1080p or not
    err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (MINT32)&sensorW, (MINT32)&sensorH);
    
    if( (1920 == mmHalCamParam.frmYuv.w && 1088 == mmHalCamParam.frmYuv.h)
        && (sensorW < mmHalCamParam.frmYuv.w || sensorH < mmHalCamParam.frmYuv.h))
    {
        if(pmhalCam3AParam->prvFps > 15)
        {
            MCAM_DBG("pmhalCam3AParam->prvfps = %d", pmhalCam3AParam->prvFps);
            pmhalCam3AParam->prvFps = 15;
            MCAM_DBG("For video 1080p, reset prvfps to %d\n", pmhalCam3AParam->prvFps);
        }
        
        if(pmhalCam3AParam->eisMode != 0)
        {
            MCAM_DBG("pmhalCam3AParam->eisMode = %d", pmhalCam3AParam->eisMode);
            pmhalCam3AParam->eisMode = 0;
            MCAM_DBG("For video 1080p, reset eisMode to %d\n", pmhalCam3AParam->eisMode);
        }
    }
    
    mHalCamSet3AParams(pmhalCam3AParam, &mmHalCamParam.cam3AParam);
    //
    memcpy(&mmHalCamParam.cam3AParam, pmhalCam3AParam, sizeof(mhalCam3AParam_t));

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamDoFocus(
)
{
    MINT32 err = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamDoFocus] \n");

    if (mHalCamGetState() == MHAL_CAM_FOCUS) {
        // The upper layer does not call cancelFoucs before
        //mHalCamSetState(MHAL_CAM_PREVIEW);
        mHalCamCancelFocus();
        mAaaState = MHAL_CAM_PREVIEW;
    }
    mhalCam3AParam_t const *pmHalCam3AParam = &mmHalCamParam.cam3AParam;
    // ICS 4.0, 
    // In Continuous mode, The focus position is locked after autoFocus() call
    if (AF_MODE_AFC == pmHalCam3AParam->afMode) {
        mpHal3AObj->pauseFocus(); 
    }
    //
    mIsFocusCBDone = 0;
    mHalCamSetState(MHAL_CAM_FOCUS);
    mpHal3AObj->resetHalfPushState();

    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32
mHalCam::mHalCamCancelFocus(
)
{
    MINT32 err = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamCancelFocus] \n");

    // T.B.D, has to modify for video mode
    mHalCamSetState(MHAL_CAM_PREVIEW);
    // reset the focus position to default    //Android 4.0 focus behavior  
    mpHal3AObj->resetFocus(); 
    return err;
}

/*******************************************************************************
*
********************************************************************************/
MINT32 mHalCam::mHalCam3AInit(
    MUINT32 const u4CamMode, 
    mhalCam3AParam_t const &prCam3AParam, 
    MUINT32 u4IsZSDMode
)
{
    MINT32 err = MHAL_NO_ERROR;
    mhalCamParam_t *pmhalCamParam = &mmHalCamParam;
    MINT32 ispInW, ispInH;
    
    mhalCam3AParam_t rCam3AParam;

    CamTimer camTmr("mHalCam3AInit", MHAL_CAM_GENERIC_PROFILE_OPT);
    // 
    MCAM_DBG("[mHalCam3AInit] \n");
    //
    
    //
//    if (( 1 == u4IsZSDMode)&&(MHAL_CAM_MODE_MPREVIEW == u4CamMode)) {
    if ( 1 == u4IsZSDMode) {
        MCAM_DBG("[ZSD]Set AE fps \n");
        err = mpHal3AObj->set3AParam(HAL_3A_AE_FPS_MIN_MAX, 4000, 15000, NULL);
        if (err < 0) {
            MCAM_ERR("[ZSD] HAL_3A_AE_FPS_MIN_MAX fail \n");
            return err;
        }
        MCAM_DBG("[ZSD]Set AE Pline \n");
        err = mpHal3AObj->set3AParam(HAL_3A_AE_PREVIEW_MODE, 1, NULL, NULL);
        if (err < 0) {
            MCAM_ERR("[ZSD] HAL_3A_AE_PREVIEW_MODE \n");
            return err;
        }
    } else {
        //MCAM_DBG("Set AE fps \n");
        err = mpHal3AObj->set3AParam(HAL_3A_AE_FPS_MIN_MAX, 4000, 30000, NULL);
        if (err < 0) {
            MCAM_ERR("HAL_3A_AE_FPS_MIN_MAX fail \n");
            return err;
        }
        //MCAM_DBG("Set AE Pline \n");
        err = mpHal3AObj->set3AParam(HAL_3A_AE_PREVIEW_MODE, 0, NULL, NULL);
        if (err < 0) {
            MCAM_ERR("HAL_3A_AE_PREVIEW_MODE \n");
            return err;
        }
    }
    
    mpHal3AObj->setZSDMode((MBOOL)u4IsZSDMode);
    
    mAaaWaitVDNum = 0;
    //
    if (MHAL_CAM_MODE_MVIDEO != u4CamMode) {
        if (MHAL_CAM_MODE_VT == u4CamMode){
            mpHal3AObj->setState(AAA_STATE_MANUAL_FRAMERATE_PREVIEW);
            MCAM_DBG("  VT prvFps: %d \n", pmhalCamParam->cam3AParam.prvFps);
            err = mpHal3AObj->set3AParam(HAL_3A_AE_FRAMERATE_MODE, prCam3AParam.prvFps, NULL, NULL);
            if (err < 0) {
               MCAM_ERR("  HAL_3A_AE_FRAMERATE_MODE fail \n");
               return err;
            }
        } 
        else {
            mpHal3AObj->setState(AAA_STATE_AUTO_FRAMERATE_PREVIEW);
           // Preivew frame rate is decided by 3A
        }
    }
    else {
        mpHal3AObj->setState(AAA_STATE_MANUAL_FRAMERATE_PREVIEW);
        MCAM_DBG("  prvFps: %d \n", pmhalCamParam->cam3AParam.prvFps);
        err = mpHal3AObj->set3AParam(HAL_3A_AE_FRAMERATE_MODE, prCam3AParam.prvFps, NULL, NULL);
        if (err < 0) {
            MCAM_ERR("  HAL_3A_AE_FRAMERATE_MODE fail \n");
            return err;
        }
    }
    //
    ::memset(&rCam3AParam, -1, sizeof(mhalCam3AParam_t));
    // @sean workaround , don't set the eis for zoom setting 
    rCam3AParam.eisMode = prCam3AParam.eisMode; 
    err = mHalCamSet3AParams(&prCam3AParam, &rCam3AParam);
    if (err < 0) {
        MCAM_ERR("  mHalCamSet3AParams fail \n");
        return -1;
    }
    #if (MHAL_CAM_ISP_TUNING)
    //  Force to validate isp frame.
    err = mpIspHalObj->sendCommand(ISP_CMD_VALIDATE_FRAME, true);
    if (err < 0) {
        MCAM_ERR("  ISP_CMD_VALIDATE_FRAME err \n\n");
        return err;
    }
    #endif 
    //
    if (MHAL_CAM_MODE_DEFAULT == u4CamMode) {
        // Default, there is no lock ae/awb behavior while focused done
        mpHal3AObj->lockHalfPushAEAWB(0);
    }
    else {
        // If mtk's preview/video, we will lock ae/awb while focused done
        mpHal3AObj->lockHalfPushAEAWB(1);
    }
    
    mpHal3AObj->setPreviewParam();
    
    //
    mAaaState = MHAL_CAM_IDLE;

    camTmr.printTime(); 
    return err;
}


/*******************************************************************************
*
********************************************************************************/
MINT32 mHalCam::mHalCamSetFlashlightParameter(MINT32 *pParam)
{
    INT32 ret = MHAL_NO_ERROR;

    MCAM_DBG("[mHalCamSetFlashlightParameter] \n");
    //
    if (mpFlashlightObj) {
        ret = mpFlashlightObj->mHalFlashlightSetParam((MINT32)pParam,NULL,NULL,NULL);
    }
    return ret;
}


/******************************************************************************
*
*******************************************************************************/
MINT32          
mHalCam::
setEISConfig(
    MUINT32 const ispOutW, 
    MUINT32 const ispOutH, 
    MUINT32 const targetW, 
    MUINT32 const targetH
 )
{
    MINT32 err = 0; 
    #if (MHAL_CAM_EIS_OPTION)
    MUINT32 ispInW = 0, ispInH = 0; 
    eis_config_t eisCfg;
    eis_factor_t eisFactor;

    ::memset (&eisCfg, 0, sizeof(eis_config_t));
    ::memset (&eisFactor, 0, sizeof(eis_factor_t));
    //
    err = mpIspHalObj->sendCommand(ISP_CMD_GET_SENSOR_PRV_RANGE, (int) &ispInW, (int) &ispInH);
    //err = mpCameraIOObj->getIspDestResolution(&ispInW, &ispInH); 
    if (err < 0) {
        MCAM_ERR("[setEISConfig] ISP_CMD_GET_SENSOR_PRV_RANGE fail \n"); 
        return err; 
    }
    //
    eisCfg.i4IspSize_X = ispInW;
    eisCfg.i4IspSize_Y = ispInH;
    eisCfg.i4CrzSize_X = ispOutW;
    eisCfg.i4CrzSize_Y = ispOutH;
    eisCfg.i4TarSize_X = targetW;
    eisCfg.i4TarSize_Y = targetH;
    eisCfg.bForce_ISP_Path = FALSE;
    mpEISHalObj->enableEIS(eisCfg, eisFactor);
    //
    MCAM_DBG("[setEISConfig] EIS ISP Size (W, H) = (%d, %d) \n", 
                                            eisCfg.i4IspSize_X, eisCfg.i4IspSize_Y); 
    MCAM_DBG("[setEISConfig] EIS CRZ size (W, H) = (%d, %d)\n", 
                                            eisCfg.i4CrzSize_X, eisCfg.i4CrzSize_Y); 
    MCAM_DBG("[setEISConfig] EIS Target size (W, H) = (%d, %d)\n", 
                                            eisCfg.i4TarSize_X, eisCfg.i4TarSize_Y); 

    #endif 
    return err; 
    
} 

void mHalCam::mHalCamFocusThread(MINT32 enable)
{
    mpHal3AObj->FocusThread(enable);
}

