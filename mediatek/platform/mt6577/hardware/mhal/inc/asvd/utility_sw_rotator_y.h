#ifndef _SW_ROTATOR_
#define _SW_ROTATOR_

#include "MTKAsvdType.h"

#define UTL_IUL_I_TO_X(i)				((i) << 16)              		  ///< Convert from integer to S15.16 fixed-point
#define UTL_IUL_X_TO_I(x)         		(((x) + (1 << 15)) >> 16) 	 ///< Convert from S15.16 fixed-point to integer (round)
#define UTL_IUL_X_TO_I_CHOP(x)		((x) >> 16)              		  ///< Convert from S15.16 fixed-point to integer (chop)
#define UTL_IUL_X_TO_I_CARRY(x)		(((x) + 0x0000FFFF) >> 16) ///< Convert from S15.16 fixed-point to integer (carry)
#define UTL_IUL_X_FRACTION(x)		((x) & 0x0000FFFF)

#define LINEAR_INTERPOLATION(val1, val2, weighting2)   \
   UTL_IUL_X_TO_I((val1) * (UTL_IUL_I_TO_X(1) - (weighting2)) + (val2) * (weighting2))

typedef struct
{
    kal_uint8 *srcAddr;
    kal_uint32 srcWidth;
    kal_uint32 srcHeight;
    kal_uint8 *dstAddr;
    kal_uint32 dstWidth;
    kal_uint32 dstHeight;
} UTL_BILINEAR_Y_RESIZER_STRUCT, *P_UTL_BILINEAR_Y_RESIZER_STRUCT; 


typedef enum
{
	ROTATOR_STATUS_OK,
	ROTATOR_STATUS_FAIL,
	ROTATOR_STATUS_NUM
} ROTATOR_STATUS_ENUM;

typedef struct YUV_ROTATOR_WINDOW_STRUCT_T
{
   kal_int32 startX;          /// x of the buffer
   kal_int32 startY;          /// y of the buffer
   kal_uint32 width;          /// the width of the buffer
   kal_uint32 height;        /// the height of the buffer
} YUV_ROTATOR_WINDOW_STRUCT;

typedef struct YUV_ROTATOR_HANDLE_STRUCT_T
{
   void *srcBuffer;
   kal_uint32 srcBufferSize;
   kal_uint32 srcWidth;
   kal_uint32 srcHeight;
   YUV_ROTATOR_WINDOW_STRUCT srcWindow;

   void *dstBuffer;
   kal_uint32 dstBufferSize;
   kal_uint32 dstWidth;
   kal_uint32 dstHeight;
   YUV_ROTATOR_WINDOW_STRUCT dstWindow;

} YUV_ROTATOR_HANDLE_STRUCT;


ROTATOR_STATUS_ENUM swYRotator090(YUV_ROTATOR_HANDLE_STRUCT *handle);
ROTATOR_STATUS_ENUM swYRotator180(YUV_ROTATOR_HANDLE_STRUCT *handle);
ROTATOR_STATUS_ENUM swYRotator270(YUV_ROTATOR_HANDLE_STRUCT *handle);
ROTATOR_STATUS_ENUM swYRotatorMirror000(YUV_ROTATOR_HANDLE_STRUCT *handle);
ROTATOR_STATUS_ENUM swYRotatorMirror090(YUV_ROTATOR_HANDLE_STRUCT *handle);
ROTATOR_STATUS_ENUM swYRotatorMirror180(YUV_ROTATOR_HANDLE_STRUCT *handle);
ROTATOR_STATUS_ENUM swYRotatorMirror270(YUV_ROTATOR_HANDLE_STRUCT *handle);
void swBilinearResizer(P_UTL_BILINEAR_Y_RESIZER_STRUCT pUtlRisizerInfo);

#endif
