#ifndef MT6516_MEMORY_H
#define MT6516_MEMORY_H

/**************************************************************************
*  DEBUG CONTROL
**************************************************************************/
//<20121009-14999-Eric Lin, [secu] Remove MEM_TEST functions in PL.
#define MEM_TEST                (0)
//>20121009-14999-Eric Lin

// do not change the test size !!!!
#define MEM_TEST_SIZE           (0x2000)

/**************************************************************************
*  DRAM SIZE 
**************************************************************************/
#define E1_DRAM_SIZE            (0x10000000)
#define E2_DRAM_SIZE            (0x08000000)

/**************************************************************************
*  EXPOSED API
**************************************************************************/
extern u32 mt6516_get_hardware_ver (void);
extern void mt6516_mem_init (void);

#if MEM_TEST
extern int complex_mem_test (unsigned int start, unsigned int len);
#endif

#endif
