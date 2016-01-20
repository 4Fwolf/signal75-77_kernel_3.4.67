#include "camera_custom_types.h"
#include "kd_camera_feature.h"
#include "FlickerDetectionTable_MainPreview.h"
#include "FlickerDetectionTable_MainZsd.h"

MINT32* cust_getFlickerTable1(CAMERA_DUAL_CAMERA_SENSOR_ENUM eDeviceId, MINT32 bZsdOn, int* tabDim1)
{
	if(bZsdOn==0)
	{
		*tabDim1 = 500;
		return (MINT32*)(&flicker_table1_main_zsd[0][0]);
	}
	else
	{
		*tabDim1 = 500;
		return (MINT32*)(&flicker_table1_main_pv[0][0]);
	}
}
MINT32* cust_getFlickerTable2(CAMERA_DUAL_CAMERA_SENSOR_ENUM eDeviceId, MINT32 bZsdOn, int* tabDim1) //flicker_table1[9][500] --> tabDim1=500
{
	if(bZsdOn==0)
	{
		*tabDim1 = 500;
		return (MINT32*)(&flicker_table2_main_zsd[0][0]);
	}
	else
	{
		*tabDim1 = 500;
		return (MINT32*)(&flicker_table2_main_pv[0][0]);
	}
}