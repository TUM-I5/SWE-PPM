#ifndef _DECL_swe_charm_H_
#define _DECL_swe_charm_H_
#include "charm++.h"
#include "envelope.h"
#include <memory>
#include "sdag.h"
/* DECLS: readonly CProxy_swe_charm mainProxy;
 */

/* DECLS: readonly int blockCountX;
 */

/* DECLS: readonly int blockCountY;
 */

/* DECLS: readonly float simulationDuration;
 */

/* DECLS: readonly int checkpointCount;
 */

#include "blocks/SWE_DimensionalSplittingCharm.decl.h"

/* DECLS: mainchare swe_charm: Chare{
swe_charm(CkArgMsg* impl_msg);
void done(int index, float flop, float commTime, float wallTime);
void exit();
};
 */
 class swe_charm;
 class CkIndex_swe_charm;
 class CProxy_swe_charm;
/* --------------- index object ------------------ */
class CkIndex_swe_charm:public CkIndex_Chare{
  public:
    typedef swe_charm local_t;
    typedef CkIndex_swe_charm index_t;
    typedef CProxy_swe_charm proxy_t;
    typedef CProxy_swe_charm element_t;

    static int __idx;
    static void __register(const char *s, size_t size);
    /* DECLS: swe_charm(CkArgMsg* impl_msg);
     */
    // Entry point registration at startup
    
    static int reg_swe_charm_CkArgMsg();
    // Entry point index lookup
    
    inline static int idx_swe_charm_CkArgMsg() {
      static int epidx = reg_swe_charm_CkArgMsg();
      return epidx;
    }

    
    static int ckNew(CkArgMsg* impl_msg) { return idx_swe_charm_CkArgMsg(); }
    
    static void _call_swe_charm_CkArgMsg(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_swe_charm_CkArgMsg(void* impl_msg, void* impl_obj);
    /* DECLS: void done(int index, float flop, float commTime, float wallTime);
     */
    // Entry point registration at startup
    
    static int reg_done_marshall2();
    // Entry point index lookup
    
    inline static int idx_done_marshall2() {
      static int epidx = reg_done_marshall2();
      return epidx;
    }

    
    inline static int idx_done(void (swe_charm::*)(int index, float flop, float commTime, float wallTime) ) {
      return idx_done_marshall2();
    }


    
    static int done(int index, float flop, float commTime, float wallTime) { return idx_done_marshall2(); }
    
    static void _call_done_marshall2(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_done_marshall2(void* impl_msg, void* impl_obj);
    
    static int _callmarshall_done_marshall2(char* impl_buf, void* impl_obj_void);
    
    static void _marshallmessagepup_done_marshall2(PUP::er &p,void *msg);
    /* DECLS: void exit();
     */
    // Entry point registration at startup
    
    static int reg_exit_void();
    // Entry point index lookup
    
    inline static int idx_exit_void() {
      static int epidx = reg_exit_void();
      return epidx;
    }

    
    inline static int idx_exit(void (swe_charm::*)() ) {
      return idx_exit_void();
    }


    
    static int exit() { return idx_exit_void(); }
    
    static void _call_exit_void(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_exit_void(void* impl_msg, void* impl_obj);
};
/* --------------- element proxy ------------------ */
class CProxy_swe_charm:public CProxy_Chare{
  public:
    typedef swe_charm local_t;
    typedef CkIndex_swe_charm index_t;
    typedef CProxy_swe_charm proxy_t;
    typedef CProxy_swe_charm element_t;

    CProxy_swe_charm(void) {};
    CProxy_swe_charm(CkChareID __cid) : CProxy_Chare(__cid){  }
    CProxy_swe_charm(const Chare *c) : CProxy_Chare(c){  }

    int ckIsDelegated(void) const
    { return CProxy_Chare::ckIsDelegated(); }
    inline CkDelegateMgr *ckDelegatedTo(void) const
    { return CProxy_Chare::ckDelegatedTo(); }
    inline CkDelegateData *ckDelegatedPtr(void) const
    { return CProxy_Chare::ckDelegatedPtr(); }
    CkGroupID ckDelegatedIdx(void) const
    { return CProxy_Chare::ckDelegatedIdx(); }

    inline void ckCheck(void) const
    { CProxy_Chare::ckCheck(); }
    const CkChareID &ckGetChareID(void) const
    { return CProxy_Chare::ckGetChareID(); }
    operator const CkChareID &(void) const
    { return ckGetChareID(); }

    void ckDelegate(CkDelegateMgr *dTo,CkDelegateData *dPtr=NULL)
    {       CProxy_Chare::ckDelegate(dTo,dPtr); }
    void ckUndelegate(void)
    {       CProxy_Chare::ckUndelegate(); }
    void pup(PUP::er &p)
    {       CProxy_Chare::pup(p);
    }

    void ckSetChareID(const CkChareID &c)
    {      CProxy_Chare::ckSetChareID(c); }
    swe_charm *ckLocal(void) const
    { return (swe_charm *)CkLocalChare(&ckGetChareID()); }
/* DECLS: swe_charm(CkArgMsg* impl_msg);
 */
    static CkChareID ckNew(CkArgMsg* impl_msg, int onPE=CK_PE_ANY);
    static void ckNew(CkArgMsg* impl_msg, CkChareID* pcid, int onPE=CK_PE_ANY);
    CProxy_swe_charm(CkArgMsg* impl_msg, int onPE=CK_PE_ANY);

/* DECLS: void done(int index, float flop, float commTime, float wallTime);
 */
    
    void done(int index, float flop, float commTime, float wallTime, const CkEntryOptions *impl_e_opts=NULL);

/* DECLS: void exit();
 */
    
    void exit(const CkEntryOptions *impl_e_opts=NULL);

};
PUPmarshall(CProxy_swe_charm)
#define swe_charm_SDAG_CODE 
typedef CBaseT1<Chare, CProxy_swe_charm>CBase_swe_charm;







/* ---------------- method closures -------------- */
class Closure_swe_charm {
  public:


    struct done_2_closure;


    struct exit_3_closure;

};

extern void _registerswe_charm(void);
extern "C" void CkRegisterMainModule(void);
#endif
