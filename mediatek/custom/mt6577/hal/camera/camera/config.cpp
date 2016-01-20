#include <stdlib.h>
#include <stdio.h>
#include "camera_custom_if.h"

namespace NSCamCustom
{
/*******************************************************************************
* 
*******************************************************************************/
#include "cfg_tuning.h"
#include "cfg_facebeauty_tuning.h"
#include "flicker_tuning.h"
//
#include "cfg_setting_imgsensor.h"
#include "cfg_tuning_imgsensor.h"

/*******************************************************************************
* custom exif
*******************************************************************************/
//<2012/10/11,daisy zang,[8317][CQ][BU2SC00136449][BU2SC00136677][CAMERA]<The "Maker" and "Model" display have fault. "Maker" and "Model"  are empty>
#define EN_CUSTOM_EXIF_INFO
char buf[PROPERTY_VALUE_MAX];
bool ReadFinish=false;
//>2012/10/11,daisy zang
MINT32 custom_SetExif(void **ppCustomExifTag)
{
#ifdef EN_CUSTOM_EXIF_INFO
#define CUSTOM_EXIF_STRING_MAKE  "WIKO"
#define CUSTOM_EXIF_STRING_MODEL "CINK PEAX"
#define CUSTOM_EXIF_STRING_SOFTWARE "Android"
static customExifInfo_t exifTag = {CUSTOM_EXIF_STRING_MAKE,CUSTOM_EXIF_STRING_MODEL,CUSTOM_EXIF_STRING_SOFTWARE};

//<2012/11/30,daisy zang,[8315][8317][CQ][BU2SC00140403][BU2SC00140312][Camera]Change the model name read from Systerm property
       if (ReadFinish==false)
      {
	 	memset(buf, 0x00, PROPERTY_VALUE_MAX);
	        property_get("ro.build.product", buf, "0");
		//LOGD("Daisy test,get value form system property,%s",buf);
		ReadFinish=true;
      	}
		strcpy( (char *)exifTag.strModel,(char const *)buf);
//>2012/11/30,daisy zang

    if (0 != ppCustomExifTag) {
        *ppCustomExifTag = (void*)&exifTag;
    }
    return 0;
#else
    return -1;
#endif
}
//
customExif_t const&
getCustomExif()
{
    static customExif_t inst = {
//<2013/02/06-ClarkLin-ExpProgram change to program normal
        bEnCustom       :   true,  // default value: false.
        u4ExpProgram    :   2,      // default value: 0.    '0' means not defined, '1' manual control, '2' program normal
//>2013/02/06-ClarkLin
    };
    return inst;
}
//
/*******************************************************************************
* LCM physical orienation 
*   Return:
*       0   : no inverse
*       1   : inverse
*       -1  : error
*******************************************************************************/
MUINT32
getLCMPhysicalOrientation()
{
    return ::atoi(MTK_LCM_PHYSICAL_ROTATION); 
}

/*******************************************************************************
* Author : cotta
* Functionality : custom flashlight gain between preview/capture flash
*******************************************************************************/
#define FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X 10
MUINT32 custom_GetFlashlightGain10X(void)
{
    // x10 , 1 mean 0.1x gain
    //10 means no difference. use torch mode for preflash and cpaflash
    //> 10 means capture flashlight is lighter than preflash light. < 10 is opposite condition.

    return (MUINT32)FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X;
}

MUINT32 custom_BurstFlashlightGain10X(void)
{
	return FLASHLIGHT_CALI_LED_GAIN_PRV_TO_CAP_10X;
}

/*******************************************************************************
* Author : Jiale
* Functionality : custom yuv flashlight threshold
*******************************************************************************/

#define FLASHLIGHT_YUV_THRESHOlD 3.0
double custom_GetYuvFlashlightThreshold(void)
{
    return (double)FLASHLIGHT_YUV_THRESHOlD;
}
/*******************************************************************************
* Author : Jiale
* Functionality : custom yuv sensor convergence frame count
*******************************************************************************/

#define FLASHLIGHT_YUV_CONVERGENCE_FRAME 7
int custom_GetYuvFlashlightFrameCnt(void)
{
    return (int)FLASHLIGHT_YUV_CONVERGENCE_FRAME;
}

/*******************************************************************************
* 
*******************************************************************************/
};  //NSCamCustom

