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
#ifndef _PCA_MGR_H_
#define _PCA_MGR_H_
namespace NSIspTuning
{

/*******************************************************************************
* PCA Manager Interface
*******************************************************************************/
class IPcaMgr
{
protected:  ////    Ctor/Dtor.
    IPcaMgr() {}
    virtual ~IPcaMgr() {}

private:
    IPcaMgr(const IPcaMgr&);
    IPcaMgr& operator=(const IPcaMgr&);


public:
    static IPcaMgr* getInstance(ECamMode_T a_eCamMode, ISP_NVRAM_PCA_STRUCT& rIspPca);

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Change Count.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MBOOL isChanged() const = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MUINT32 getIdx() const = 0;
    virtual MBOOL setIdx(MUINT32 const u4Idx) = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Address
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MVOID* getPhyTBA() const = 0;
    virtual MVOID savePhyTBA(MVOID*const pTBA) = 0;
    virtual MVOID* getVirTBA() const = 0;
    virtual MVOID saveVirTBA(MVOID*const pTBA) = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LUT
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MVOID* getLut() const = 0;
    virtual MUINT32 getLutSize() const = 0;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    virtual MBOOL loadLut(MBOOL const fgForceToLoad = MFALSE) = 0;
    virtual MVOID loadLutToSysram() = 0;
    virtual MBOOL loadSysramLutToISP() = 0;
    virtual MVOID loadConfig() = 0;
    virtual MBOOL preset() = 0;
    virtual MBOOL isEnable() const = 0;
    virtual MBOOL isLoading() const = 0;
};


/*******************************************************************************
* PCA Manager
*******************************************************************************/
class PcaMgr : public IPcaMgr
{
public:
    static IPcaMgr* getInstance(ISP_NVRAM_PCA_STRUCT& rIspPca)
    {
        static PcaMgr singleton(rIspPca);
        return &singleton;
    }


private:
    enum
    {
        LUT_LOW = 0,
        LUT_MIDDLE,
        LUT_HIGH,
        NUM_OF_LUTS
    };

    enum
    {
        LUT_SIZE = sizeof(ISP_NVRAM_PCA_BIN_T)*PCA_BIN_NUM
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Change Count.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    virtual MBOOL
    isChanged() const
    {
        return  ( 0 < m_u4ChangeCount );
    }

private:    ////
    inline
    MVOID
    markChange()
    {
        m_u4ChangeCount++;
    }

    template <typename T>
    inline
    MVOID
    setIfChange(T& dst, T const src)
    {
        if  ( src != dst )
        {
            dst = src;
            markChange();
        }
    }

private:    ////    Data Members.
    //  Nonzero indicates any member has changed; otherwise zero.
    MUINT32                 m_u4ChangeCount;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    virtual MUINT32
    getIdx() const
    {
        return m_u4Idx;
    }

    inline
    virtual MBOOL
    setIdx(MUINT32 const u4Idx)
    {
        if  ( NUM_OF_LUTS <= u4Idx )
            return  MFALSE;
        setIfChange(m_u4Idx, u4Idx);
        return  MTRUE;
    }

private:    ////    Common.

    //  PCA index
    MUINT32         m_u4Idx;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Address
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.

    inline
    virtual MVOID*
    getPhyTBA() const
    {
        return  m_pvPhyTBA;
    }

    inline
    virtual MVOID
    savePhyTBA(MVOID*const pTBA)
    {
        setIfChange(m_pvPhyTBA, pTBA);
        markChange();
    }

    inline
    virtual MVOID*
    getVirTBA() const
    {
        return  m_pvVirTBA;
    }

    inline
    virtual MVOID
    saveVirTBA(MVOID*const pTBA)
    {
        setIfChange(m_pvVirTBA, pTBA);
        markChange();
    }

private:    ////    Address.
    //  Physical Base address of PCA table.
    MVOID*                  m_pvPhyTBA;
    //  Virtual Base address of PCA table.
    MVOID*                  m_pvVirTBA;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LUT
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    get

    inline
    virtual MVOID*
    getLut() const
    {
        switch (m_u4Idx)
        {
        case LUT_LOW:       //color temperature: low
            return  static_cast<MVOID*>(&m_rIspPcaLUTs.lut_lo[0]);
        case LUT_MIDDLE:    //color temperature: middle
            return  static_cast<MVOID*>(&m_rIspPcaLUTs.lut_md[0]);
        case LUT_HIGH:      //color temperature: high
            return  static_cast<MVOID*>(&m_rIspPcaLUTs.lut_hi[0]);
        default:
            break;
        }
        return  NULL;
    }

    inline
    virtual MUINT32
    getLutSize() const
    {
        return LUT_SIZE;
    }

private:    ////    Reference.
    //  Reference to m_rIspParam.ISPPca
    ISP_NVRAM_PCA_STRUCT&   m_rIspPca;
    //  Reference to m_rIspParam.ISPPca.Config
    ISP_NVRAM_PCA_T&        m_rIspPcaCfg;
    //  Reference to m_rIspParam.ISPPca.PCA_LUTs
    ISP_NVRAM_PCA_LUTS_T&   m_rIspPcaLUTs;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    PcaMgr(ISP_NVRAM_PCA_STRUCT& rIspPca)
        : IPcaMgr()
        , m_u4ChangeCount(1)
        , m_u4Idx(0)
        , m_pvPhyTBA(NULL)
        , m_pvVirTBA(NULL)
        , m_rIspPca     (rIspPca)
        , m_rIspPcaCfg  (rIspPca.Config)
        , m_rIspPcaLUTs (rIspPca.PCA_LUTs)
    {
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    load

    inline
    virtual MBOOL
    loadLut(MBOOL const fgForceToLoad = MFALSE)
    {
        if  ( isChanged() || fgForceToLoad )
        {
            m_u4ChangeCount = 0;
            loadLutToSysram();
            return  loadSysramLutToISP();
        }
        return  MTRUE;
    }

    inline
    virtual MVOID
    loadLutToSysram()
    {   //  VA <- LUT
        ::memcpy(m_pvVirTBA, getLut(), LUT_SIZE);
    }

    inline
    virtual MBOOL
    loadSysramLutToISP()
    {   //  ISP <- PA
        return  ISP_MGR_PCA_T::getInstance().directSetTBA(
            reinterpret_cast<MUINT32>(m_pvPhyTBA)
        );
    }

    inline
    virtual MVOID
    loadConfig()
    {
        prepareIspHWBuf( m_rIspPcaCfg );
    }

    inline
    virtual MBOOL
    preset()
    {
        MBOOL fgRet = MFALSE;
        fgRet = ISP_MGR_PCA_T::getInstance().reset();
        if  ( fgRet )
        {
            ISP_NVRAM_PCA_T PcaCfg = m_rIspPcaCfg;
            //  During preparing, disable PCA but config 180/360-bins before loading LUT.
            PcaCfg.ctrl.bits.EN = 0;
            prepareIspHWBuf( PcaCfg );
            fgRet = ISP_MGR_PCA_T::getInstance().apply();
        }
        markChange();
        return  fgRet;
    }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////

    inline
    virtual MBOOL
    isEnable() const
    {
        return  (0 != m_rIspPcaCfg.ctrl.bits.EN);
    }

    inline
    virtual MBOOL
    isLoading() const
    {
        //  TODO: Need to implement
        //  Check the register
        return  MTRUE;
    }

};

/*******************************************************************************
* PCA Manager for face beautifier
*******************************************************************************/
class FBPcaMgr : public IPcaMgr
{
public:
    static IPcaMgr* getInstance(ISP_NVRAM_PCA_STRUCT& rIspPca)
    {
        static FBPcaMgr singleton(rIspPca);
        return &singleton;
    }


private:
    enum
    {
        LUT_SIZE = sizeof(ISP_NVRAM_PCA_BIN_T)*PCA_BIN_NUM
    };

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Change Count.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    virtual MBOOL
    isChanged() const
    {
        return  MTRUE;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Index
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////

    inline
    virtual MUINT32
    getIdx() const
    {
        return 0;
    }

    inline
    virtual MBOOL
    setIdx(MUINT32 const u4Idx)
    {
        return  MTRUE;
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Address
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    Operations.

    inline
    virtual MVOID*
    getPhyTBA() const
    {
        return  m_pvPhyTBA;
    }

    inline
    virtual MVOID
    savePhyTBA(MVOID*const pTBA)
    {
        m_pvPhyTBA = pTBA;
    }

    inline
    virtual MVOID*
    getVirTBA() const
    {
        return  m_pvVirTBA;
    }

    inline
    virtual MVOID
    saveVirTBA(MVOID*const pTBA)
    {
        m_pvVirTBA = pTBA;
    }

private:    ////    Address.
    //  Physical Base address of PCA table.
    MVOID*                  m_pvPhyTBA;
    //  Virtual Base address of PCA table.
    MVOID*                  m_pvVirTBA;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  LUT
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    get

    inline
    virtual MVOID*
    getLut() const
    {
        return  static_cast<MVOID*>(&m_rFBPcaLUT.lut[0]);
    }

    inline
    virtual MUINT32
    getLutSize() const
    {
        return LUT_SIZE;
    }

private:    ////    Reference.
    //  Reference to m_rIspParam.ISPPca.Config
    ISP_NVRAM_PCA_T&        m_rIspPcaCfg;
    //  Reference to rFBPcaLut in custom folder
    NSCamCustom::FB_PCA_LUT_T&   m_rFBPcaLUT;

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Ctor
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////
    FBPcaMgr(ISP_NVRAM_PCA_STRUCT& rIspPca)
        : IPcaMgr()
        , m_pvPhyTBA(NULL)
        , m_pvVirTBA(NULL)
        , m_rIspPcaCfg (rIspPca.Config)
        , m_rFBPcaLUT (NSCamCustom::getFBPCALut())
    {
    }

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//  Operations.
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
public:     ////    load

    inline
    virtual MBOOL
    loadLut(MBOOL const fgForceToLoad = MFALSE)
    {
        loadLutToSysram();
        return  loadSysramLutToISP();
    }

    inline
    virtual MVOID
    loadLutToSysram()
    {   //  VA <- LUT
        ::memcpy(m_pvVirTBA, getLut(), LUT_SIZE);
    }

    inline
    virtual MBOOL
    loadSysramLutToISP()
    {   //  ISP <- PA
        return  ISP_MGR_PCA_T::getInstance().directSetTBA(
            reinterpret_cast<MUINT32>(m_pvPhyTBA)
        );
    }

    inline
    virtual MVOID
    loadConfig()
    {
        ISP_NVRAM_PCA_T IspPcaCfg = m_rIspPcaCfg;
        IspPcaCfg.ctrl.bits.EN = 1; // enable PCA
        prepareIspHWBuf( IspPcaCfg );
    }

    inline
    virtual MBOOL
    preset()
    {
        MBOOL fgRet = MFALSE;
        fgRet = ISP_MGR_PCA_T::getInstance().reset();
        if  ( fgRet )
        {
            ISP_NVRAM_PCA_T PcaCfg = m_rIspPcaCfg;
            //  During preparing, disable PCA but config 180/360-bins before loading LUT.
            PcaCfg.ctrl.bits.EN = 0;
            prepareIspHWBuf( PcaCfg );
            fgRet = ISP_MGR_PCA_T::getInstance().apply();
        }

        return  fgRet;
    }

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
public:     ////

    inline
    virtual MBOOL
    isEnable() const
    {
        return  MTRUE;
    }

    inline
    virtual MBOOL
    isLoading() const
    {
        //  TODO: Need to implement
        //  Check the register
        return  MTRUE;
    }

};

/*******************************************************************************
*
*******************************************************************************/
};  //  NSIspTuning
#endif // _PCA_MGR_H_

