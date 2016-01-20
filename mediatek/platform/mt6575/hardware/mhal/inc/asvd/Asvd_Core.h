#ifndef _ASVD_CORE_H
#define _ASVD_CORE_H

//20120921
#define ASVD_MAX_INSTANCE 16

//-------- Default Tuning Para.------------
#define DEFAULT_NROW 4
#define DEFAULT_QUAN 8
#define DEFAULT_THR_MID 0.9
#define DEFAULT_THR_PRB 0.5
#define DEFAULT_THR_SIM_HIGH 0.9
#define DEFAULT_THR_SIM_LOW 0.3

//---------------------------------------------------------------------------
void AsvdCoreMain(MINT32 instance_index);
void AsvdCoreInit( ASVD_SET_ENV_INFO_STRUCT *pInitInfo, MINT32 instance_index);
void AsvdCoreSetLogBuffer(MUINT32 LogBufAddr, MINT32 instance_count);
void AsvdCoreSetProcInfo(P_ASVD_SET_PROC_INFO_STRUCT pSetProcInfo,MINT32 instance_index);
void AsvdCoreGetResultInfo(P_ASVD_RESULT_STRUCT pAsvdResultInfo, MINT32 instance_index);
//---------------------------------------------------------------------------
MUINT32 AsvdCoreSetWorkingBufInfo(P_ASVD_SET_WORK_BUF_INFO_STRUCT pWorkBufInfo, MINT32);
MUINT32 AsvdQueryWorkingBufSize(MINT32);
//---------------------------------------------------------------------------
void SaveAsvdProcLog(MINT32 instance_index);

#endif