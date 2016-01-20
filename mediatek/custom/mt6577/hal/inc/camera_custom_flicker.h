#ifndef __CAMERA_CUSTOM_FLICKER_H__
#define __CAMERA_CUSTOM_FLICKER_H__


MINT32* cust_getFlickerTable1(CAMERA_DUAL_CAMERA_SENSOR_ENUM eDeviceId, MINT32 bZsdOn, int* tabDim1); //flicker_table1[9][500] --> tabDim1=500
MINT32* cust_getFlickerTable2(CAMERA_DUAL_CAMERA_SENSOR_ENUM eDeviceId, MINT32 bZsdOn, int* tabDim1); //flicker_table1[9][500] --> tabDim1=500



#endif