#ifndef _CFG_TUNING_IMGSENSOR_H_
#define _CFG_TUNING_IMGSENSOR_H_

/*******************************************************************************
* Dynamic Frame Rate
*******************************************************************************/
VdoDynamicFrameRate_T const&
getParamVdoDynamicFrameRate()
{
    static VdoDynamicFrameRate_T inst = { 
        EVThresNormal  : 50,
        EVThresNight   : 50,
        isEnableDFps    : false,
    };
    return inst;
}

/*******************************************************************************
* Custom EXIF (Imgsensor-related)
*******************************************************************************/
SensorExifInfo_T const&
getParamSensorExif()
{
    static SensorExifInfo_T inst = {
//<2013/02/04-21437-AlbertWu,roll back to original camera setting      
//<2013/02/06-ClarkLin-Focal Length for CTS
        //uFLengthNum     : 35, // Numerator of Focal Length. Default is 35.
        //uFLengthDenom   : 10, // Denominator of Focal Length, it should not be 0.  Default is 10.
		// Hawk3.5 Effective Focal Length is 4.27mm
        uFLengthNum     : 427, // Numerator of Focal Length. Default is 35.
        uFLengthDenom   : 100, // Denominator of Focal Length, it should not be 0.  Default is 10.
//>2013/02/06-ClarkLin
//>2013/02/04-21437-AlbertWu   
    };
    return inst;
}

/*******************************************************************************
* 
*******************************************************************************/
#endif //  _CFG_TUNING_IMGSENSOR_H_

