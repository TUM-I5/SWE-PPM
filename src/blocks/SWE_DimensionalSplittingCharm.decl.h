#ifndef _DECL_SWE_DimensionalSplittingCharm_H_
#define _DECL_SWE_DimensionalSplittingCharm_H_
#include "charm++.h"
#include "envelope.h"
#include <memory>
#include "sdag.h"
#include "types/BlockConnectInterface.hh"

#include "types/Boundary.hh"

#include "scenarios/SWE_Scenario.hh"

#include "tools/Float2DNative.hh"

#ifndef GROUPDEPNUM_DECLARED
# define GROUPDEPNUM_DECLARED
struct GroupDepNum
{
  int groupDepNum;
  explicit GroupDepNum(int g = 0) : groupDepNum{g} { }
  operator int() const { return groupDepNum; }
};
#endif
/* DECLS: message copyLayer{
Boundary boundary;
bool containsBathymetry;
float b[];
float h[];
float hu[];
float hv[];
}
;
 */
class copyLayer;
class CMessage_copyLayer:public CkMessage{
  public:
    static int __idx;
    void* operator new(size_t, void*p) { return p; }
    void* operator new(size_t);
    void* operator new(size_t, int*, const int);
    void* operator new(size_t, int*, const int, const GroupDepNum);
    void* operator new(size_t, int*);
#if CMK_MULTIPLE_DELETE
    void operator delete(void*p, void*){dealloc(p);}
    void operator delete(void*p){dealloc(p);}
    void operator delete(void*p, int*, const int){dealloc(p);}
    void operator delete(void*p, int*, const int, const GroupDepNum){dealloc(p);}
    void operator delete(void*p, int*){dealloc(p);}
#endif
    void operator delete(void*p, size_t){dealloc(p);}
    static void* alloc(int,size_t, int*, int, GroupDepNum);
    static void dealloc(void *p);
    CMessage_copyLayer();
    static void *pack(copyLayer *p);
    static copyLayer* unpack(void* p);
    void *operator new(size_t, int, int, int, int);
    void *operator new(size_t, int, int, int, int, const int);
    void *operator new(size_t, int, int, int, int, const int, const GroupDepNum);
#if CMK_MULTIPLE_DELETE
    void operator delete(void *p, int, int, int, int){dealloc(p);}
    void operator delete(void *p, int, int, int, int, const int){dealloc(p);}
    void operator delete(void *p, int, int, int, int, const int, const GroupDepNum){dealloc(p);}
#endif
    static void __register(const char *s, size_t size, CkPackFnPtr pack, CkUnpackFnPtr unpack) {
      __idx = CkRegisterMsg(s, pack, unpack, dealloc, size);
    }
};

/* DECLS: array SWE_DimensionalSplittingCharm: ArrayElement{
SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
void compute();
void receiveGhostLeft(copyLayer* impl_msg);
void receiveGhostRight(copyLayer* impl_msg);
void receiveGhostBottom(copyLayer* impl_msg);
void receiveGhostTop(copyLayer* impl_msg);
void reductionTrigger();
void reduceWaveSpeed(float maxWaveSpeed);
SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
};
 */
 class SWE_DimensionalSplittingCharm;
 class CkIndex_SWE_DimensionalSplittingCharm;
 class CProxy_SWE_DimensionalSplittingCharm;
 class CProxyElement_SWE_DimensionalSplittingCharm;
 class CProxySection_SWE_DimensionalSplittingCharm;
/* --------------- index object ------------------ */
class CkIndex_SWE_DimensionalSplittingCharm:public CkIndex_ArrayElement{
  public:
    typedef SWE_DimensionalSplittingCharm local_t;
    typedef CkIndex_SWE_DimensionalSplittingCharm index_t;
    typedef CProxy_SWE_DimensionalSplittingCharm proxy_t;
    typedef CProxyElement_SWE_DimensionalSplittingCharm element_t;
    typedef CProxySection_SWE_DimensionalSplittingCharm section_t;

    static int __idx;
    static void __register(const char *s, size_t size);
    /* DECLS: SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
     */
    // Entry point registration at startup
    
    static int reg_SWE_DimensionalSplittingCharm_marshall1();
    // Entry point index lookup
    
    inline static int idx_SWE_DimensionalSplittingCharm_marshall1() {
      static int epidx = reg_SWE_DimensionalSplittingCharm_marshall1();
      return epidx;
    }

    
    static int ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile) { return idx_SWE_DimensionalSplittingCharm_marshall1(); }
    
    static void _call_SWE_DimensionalSplittingCharm_marshall1(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_SWE_DimensionalSplittingCharm_marshall1(void* impl_msg, void* impl_obj);
    
    static int _callmarshall_SWE_DimensionalSplittingCharm_marshall1(char* impl_buf, void* impl_obj_void);
    
    static void _marshallmessagepup_SWE_DimensionalSplittingCharm_marshall1(PUP::er &p,void *msg);
    /* DECLS: void compute();
     */
    // Entry point registration at startup
    
    static int reg_compute_void();
    // Entry point index lookup
    
    inline static int idx_compute_void() {
      static int epidx = reg_compute_void();
      return epidx;
    }

    
    inline static int idx_compute(void (SWE_DimensionalSplittingCharm::*)() ) {
      return idx_compute_void();
    }


    
    static int compute() { return idx_compute_void(); }
    
    static void _call_compute_void(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_compute_void(void* impl_msg, void* impl_obj);
    /* DECLS: void receiveGhostLeft(copyLayer* impl_msg);
     */
    // Entry point registration at startup
    
    static int reg_receiveGhostLeft_copyLayer();
    // Entry point index lookup
    
    inline static int idx_receiveGhostLeft_copyLayer() {
      static int epidx = reg_receiveGhostLeft_copyLayer();
      return epidx;
    }

    
    inline static int idx_receiveGhostLeft(void (SWE_DimensionalSplittingCharm::*)(copyLayer* impl_msg) ) {
      return idx_receiveGhostLeft_copyLayer();
    }


    
    static int receiveGhostLeft(copyLayer* impl_msg) { return idx_receiveGhostLeft_copyLayer(); }
    
    static void _call_receiveGhostLeft_copyLayer(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_receiveGhostLeft_copyLayer(void* impl_msg, void* impl_obj);
    /* DECLS: void receiveGhostRight(copyLayer* impl_msg);
     */
    // Entry point registration at startup
    
    static int reg_receiveGhostRight_copyLayer();
    // Entry point index lookup
    
    inline static int idx_receiveGhostRight_copyLayer() {
      static int epidx = reg_receiveGhostRight_copyLayer();
      return epidx;
    }

    
    inline static int idx_receiveGhostRight(void (SWE_DimensionalSplittingCharm::*)(copyLayer* impl_msg) ) {
      return idx_receiveGhostRight_copyLayer();
    }


    
    static int receiveGhostRight(copyLayer* impl_msg) { return idx_receiveGhostRight_copyLayer(); }
    
    static void _call_receiveGhostRight_copyLayer(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_receiveGhostRight_copyLayer(void* impl_msg, void* impl_obj);
    /* DECLS: void receiveGhostBottom(copyLayer* impl_msg);
     */
    // Entry point registration at startup
    
    static int reg_receiveGhostBottom_copyLayer();
    // Entry point index lookup
    
    inline static int idx_receiveGhostBottom_copyLayer() {
      static int epidx = reg_receiveGhostBottom_copyLayer();
      return epidx;
    }

    
    inline static int idx_receiveGhostBottom(void (SWE_DimensionalSplittingCharm::*)(copyLayer* impl_msg) ) {
      return idx_receiveGhostBottom_copyLayer();
    }


    
    static int receiveGhostBottom(copyLayer* impl_msg) { return idx_receiveGhostBottom_copyLayer(); }
    
    static void _call_receiveGhostBottom_copyLayer(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_receiveGhostBottom_copyLayer(void* impl_msg, void* impl_obj);
    /* DECLS: void receiveGhostTop(copyLayer* impl_msg);
     */
    // Entry point registration at startup
    
    static int reg_receiveGhostTop_copyLayer();
    // Entry point index lookup
    
    inline static int idx_receiveGhostTop_copyLayer() {
      static int epidx = reg_receiveGhostTop_copyLayer();
      return epidx;
    }

    
    inline static int idx_receiveGhostTop(void (SWE_DimensionalSplittingCharm::*)(copyLayer* impl_msg) ) {
      return idx_receiveGhostTop_copyLayer();
    }


    
    static int receiveGhostTop(copyLayer* impl_msg) { return idx_receiveGhostTop_copyLayer(); }
    
    static void _call_receiveGhostTop_copyLayer(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_receiveGhostTop_copyLayer(void* impl_msg, void* impl_obj);
    /* DECLS: void reductionTrigger();
     */
    // Entry point registration at startup
    
    static int reg_reductionTrigger_void();
    // Entry point index lookup
    
    inline static int idx_reductionTrigger_void() {
      static int epidx = reg_reductionTrigger_void();
      return epidx;
    }

    
    inline static int idx_reductionTrigger(void (SWE_DimensionalSplittingCharm::*)() ) {
      return idx_reductionTrigger_void();
    }


    
    static int reductionTrigger() { return idx_reductionTrigger_void(); }
    
    static void _call_reductionTrigger_void(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_reductionTrigger_void(void* impl_msg, void* impl_obj);
    /* DECLS: void reduceWaveSpeed(float maxWaveSpeed);
     */
    // Entry point registration at startup
    
    static int reg_reduceWaveSpeed_marshall8();
    // Entry point index lookup
    
    inline static int idx_reduceWaveSpeed_marshall8() {
      static int epidx = reg_reduceWaveSpeed_marshall8();
      return epidx;
    }

    
    inline static int idx_reduceWaveSpeed(void (SWE_DimensionalSplittingCharm::*)(float maxWaveSpeed) ) {
      return idx_reduceWaveSpeed_marshall8();
    }


    
    static int reduceWaveSpeed(float maxWaveSpeed) { return idx_reduceWaveSpeed_marshall8(); }
    // Entry point registration at startup
    
    static int reg_redn_wrapper_reduceWaveSpeed_marshall8();
    // Entry point index lookup
    
    inline static int idx_redn_wrapper_reduceWaveSpeed_marshall8() {
      static int epidx = reg_redn_wrapper_reduceWaveSpeed_marshall8();
      return epidx;
    }
    
    static int redn_wrapper_reduceWaveSpeed(CkReductionMsg* impl_msg) { return idx_redn_wrapper_reduceWaveSpeed_marshall8(); }
    
    static void _call_redn_wrapper_reduceWaveSpeed_marshall8(void* impl_msg, void* impl_obj_void);
    
    static void _call_reduceWaveSpeed_marshall8(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_reduceWaveSpeed_marshall8(void* impl_msg, void* impl_obj);
    
    static int _callmarshall_reduceWaveSpeed_marshall8(char* impl_buf, void* impl_obj_void);
    
    static void _marshallmessagepup_reduceWaveSpeed_marshall8(PUP::er &p,void *msg);
    /* DECLS: SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
     */
    // Entry point registration at startup
    
    static int reg_SWE_DimensionalSplittingCharm_CkMigrateMessage();
    // Entry point index lookup
    
    inline static int idx_SWE_DimensionalSplittingCharm_CkMigrateMessage() {
      static int epidx = reg_SWE_DimensionalSplittingCharm_CkMigrateMessage();
      return epidx;
    }

    
    static int ckNew(CkMigrateMessage* impl_msg) { return idx_SWE_DimensionalSplittingCharm_CkMigrateMessage(); }
    
    static void _call_SWE_DimensionalSplittingCharm_CkMigrateMessage(void* impl_msg, void* impl_obj);
    
    static void _call_sdag_SWE_DimensionalSplittingCharm_CkMigrateMessage(void* impl_msg, void* impl_obj);
};
/* --------------- element proxy ------------------ */
 class CProxyElement_SWE_DimensionalSplittingCharm : public CProxyElement_ArrayElement{
  public:
    typedef SWE_DimensionalSplittingCharm local_t;
    typedef CkIndex_SWE_DimensionalSplittingCharm index_t;
    typedef CProxy_SWE_DimensionalSplittingCharm proxy_t;
    typedef CProxyElement_SWE_DimensionalSplittingCharm element_t;
    typedef CProxySection_SWE_DimensionalSplittingCharm section_t;

    using array_index_t = CkArrayIndex1D;

    /* TRAM aggregators */

    CProxyElement_SWE_DimensionalSplittingCharm(void) {
    }
    CProxyElement_SWE_DimensionalSplittingCharm(const ArrayElement *e) : CProxyElement_ArrayElement(e){
    }

    void ckDelegate(CkDelegateMgr *dTo,CkDelegateData *dPtr=NULL)
    {       CProxyElement_ArrayElement::ckDelegate(dTo,dPtr); }
    void ckUndelegate(void)
    {       CProxyElement_ArrayElement::ckUndelegate(); }
    void pup(PUP::er &p)
    {       CProxyElement_ArrayElement::pup(p);
    }

    int ckIsDelegated(void) const
    { return CProxyElement_ArrayElement::ckIsDelegated(); }
    inline CkDelegateMgr *ckDelegatedTo(void) const
    { return CProxyElement_ArrayElement::ckDelegatedTo(); }
    inline CkDelegateData *ckDelegatedPtr(void) const
    { return CProxyElement_ArrayElement::ckDelegatedPtr(); }
    CkGroupID ckDelegatedIdx(void) const
    { return CProxyElement_ArrayElement::ckDelegatedIdx(); }

    inline void ckCheck(void) const
    { CProxyElement_ArrayElement::ckCheck(); }
    inline operator CkArrayID () const
    { return ckGetArrayID(); }
    inline CkArrayID ckGetArrayID(void) const
    { return CProxyElement_ArrayElement::ckGetArrayID(); }
    inline CkArray *ckLocalBranch(void) const
    { return CProxyElement_ArrayElement::ckLocalBranch(); }
    inline CkLocMgr *ckLocMgr(void) const
    { return CProxyElement_ArrayElement::ckLocMgr(); }

    inline static CkArrayID ckCreateEmptyArray(CkArrayOptions opts = CkArrayOptions())
    { return CProxyElement_ArrayElement::ckCreateEmptyArray(opts); }
    inline static void ckCreateEmptyArrayAsync(CkCallback cb, CkArrayOptions opts = CkArrayOptions())
    { CProxyElement_ArrayElement::ckCreateEmptyArrayAsync(cb, opts); }
    inline static CkArrayID ckCreateArray(CkArrayMessage *m,int ctor,const CkArrayOptions &opts)
    { return CProxyElement_ArrayElement::ckCreateArray(m,ctor,opts); }
    inline void ckInsertIdx(CkArrayMessage *m,int ctor,int onPe,const CkArrayIndex &idx)
    { CProxyElement_ArrayElement::ckInsertIdx(m,ctor,onPe,idx); }
    inline void doneInserting(void)
    { CProxyElement_ArrayElement::doneInserting(); }

    inline void ckBroadcast(CkArrayMessage *m, int ep, int opts=0) const
    { CProxyElement_ArrayElement::ckBroadcast(m,ep,opts); }
    inline void setReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxyElement_ArrayElement::setReductionClient(fn,param); }
    inline void ckSetReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxyElement_ArrayElement::ckSetReductionClient(fn,param); }
    inline void ckSetReductionClient(CkCallback *cb) const
    { CProxyElement_ArrayElement::ckSetReductionClient(cb); }

    inline void ckInsert(CkArrayMessage *m,int ctor,int onPe)
    { CProxyElement_ArrayElement::ckInsert(m,ctor,onPe); }
    inline void ckSend(CkArrayMessage *m, int ep, int opts = 0) const
    { CProxyElement_ArrayElement::ckSend(m,ep,opts); }
    inline void *ckSendSync(CkArrayMessage *m, int ep) const
    { return CProxyElement_ArrayElement::ckSendSync(m,ep); }
    inline const CkArrayIndex &ckGetIndex() const
    { return CProxyElement_ArrayElement::ckGetIndex(); }

    SWE_DimensionalSplittingCharm *ckLocal(void) const
    { return (SWE_DimensionalSplittingCharm *)CProxyElement_ArrayElement::ckLocal(); }

    CProxyElement_SWE_DimensionalSplittingCharm(const CkArrayID &aid,const CkArrayIndex1D &idx,CK_DELCTOR_PARAM)
        :CProxyElement_ArrayElement(aid,idx,CK_DELCTOR_ARGS)
    {
}
    CProxyElement_SWE_DimensionalSplittingCharm(const CkArrayID &aid,const CkArrayIndex1D &idx)
        :CProxyElement_ArrayElement(aid,idx)
    {
}

    CProxyElement_SWE_DimensionalSplittingCharm(const CkArrayID &aid,const CkArrayIndex &idx,CK_DELCTOR_PARAM)
        :CProxyElement_ArrayElement(aid,idx,CK_DELCTOR_ARGS)
    {
}
    CProxyElement_SWE_DimensionalSplittingCharm(const CkArrayID &aid,const CkArrayIndex &idx)
        :CProxyElement_ArrayElement(aid,idx)
    {
}
/* DECLS: SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
 */
    
    void insert(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, int onPE=-1, const CkEntryOptions *impl_e_opts=NULL);
/* DECLS: void compute();
 */
    
    void compute(const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: void receiveGhostLeft(copyLayer* impl_msg);
 */
    
    void receiveGhostLeft(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostRight(copyLayer* impl_msg);
 */
    
    void receiveGhostRight(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostBottom(copyLayer* impl_msg);
 */
    
    void receiveGhostBottom(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostTop(copyLayer* impl_msg);
 */
    
    void receiveGhostTop(copyLayer* impl_msg) ;

/* DECLS: void reductionTrigger();
 */
    
    void reductionTrigger(const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: void reduceWaveSpeed(float maxWaveSpeed);
 */
    
    void reduceWaveSpeed(float maxWaveSpeed, const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
 */

};
PUPmarshall(CProxyElement_SWE_DimensionalSplittingCharm)
/* ---------------- collective proxy -------------- */
 class CProxy_SWE_DimensionalSplittingCharm : public CProxy_ArrayElement{
  public:
    typedef SWE_DimensionalSplittingCharm local_t;
    typedef CkIndex_SWE_DimensionalSplittingCharm index_t;
    typedef CProxy_SWE_DimensionalSplittingCharm proxy_t;
    typedef CProxyElement_SWE_DimensionalSplittingCharm element_t;
    typedef CProxySection_SWE_DimensionalSplittingCharm section_t;

    using array_index_t = CkArrayIndex1D;
    CProxy_SWE_DimensionalSplittingCharm(void) {
    }
    CProxy_SWE_DimensionalSplittingCharm(const ArrayElement *e) : CProxy_ArrayElement(e){
    }

    void ckDelegate(CkDelegateMgr *dTo,CkDelegateData *dPtr=NULL)
    {       CProxy_ArrayElement::ckDelegate(dTo,dPtr); }
    void ckUndelegate(void)
    {       CProxy_ArrayElement::ckUndelegate(); }
    void pup(PUP::er &p)
    {       CProxy_ArrayElement::pup(p);
    }

    int ckIsDelegated(void) const
    { return CProxy_ArrayElement::ckIsDelegated(); }
    inline CkDelegateMgr *ckDelegatedTo(void) const
    { return CProxy_ArrayElement::ckDelegatedTo(); }
    inline CkDelegateData *ckDelegatedPtr(void) const
    { return CProxy_ArrayElement::ckDelegatedPtr(); }
    CkGroupID ckDelegatedIdx(void) const
    { return CProxy_ArrayElement::ckDelegatedIdx(); }

    inline void ckCheck(void) const
    { CProxy_ArrayElement::ckCheck(); }
    inline operator CkArrayID () const
    { return ckGetArrayID(); }
    inline CkArrayID ckGetArrayID(void) const
    { return CProxy_ArrayElement::ckGetArrayID(); }
    inline CkArray *ckLocalBranch(void) const
    { return CProxy_ArrayElement::ckLocalBranch(); }
    inline CkLocMgr *ckLocMgr(void) const
    { return CProxy_ArrayElement::ckLocMgr(); }

    inline static CkArrayID ckCreateEmptyArray(CkArrayOptions opts = CkArrayOptions())
    { return CProxy_ArrayElement::ckCreateEmptyArray(opts); }
    inline static void ckCreateEmptyArrayAsync(CkCallback cb, CkArrayOptions opts = CkArrayOptions())
    { CProxy_ArrayElement::ckCreateEmptyArrayAsync(cb, opts); }
    inline static CkArrayID ckCreateArray(CkArrayMessage *m,int ctor,const CkArrayOptions &opts)
    { return CProxy_ArrayElement::ckCreateArray(m,ctor,opts); }
    inline void ckInsertIdx(CkArrayMessage *m,int ctor,int onPe,const CkArrayIndex &idx)
    { CProxy_ArrayElement::ckInsertIdx(m,ctor,onPe,idx); }
    inline void doneInserting(void)
    { CProxy_ArrayElement::doneInserting(); }

    inline void ckBroadcast(CkArrayMessage *m, int ep, int opts=0) const
    { CProxy_ArrayElement::ckBroadcast(m,ep,opts); }
    inline void setReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxy_ArrayElement::setReductionClient(fn,param); }
    inline void ckSetReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxy_ArrayElement::ckSetReductionClient(fn,param); }
    inline void ckSetReductionClient(CkCallback *cb) const
    { CProxy_ArrayElement::ckSetReductionClient(cb); }

    // Empty array construction
    static CkArrayID ckNew(CkArrayOptions opts = CkArrayOptions()) { return ckCreateEmptyArray(opts); }
    static void      ckNew(CkCallback cb, CkArrayOptions opts = CkArrayOptions()) { ckCreateEmptyArrayAsync(cb, opts); }

    // Generalized array indexing:
    CProxyElement_SWE_DimensionalSplittingCharm operator [] (const CkArrayIndex1D &idx) const
    { return CProxyElement_SWE_DimensionalSplittingCharm(ckGetArrayID(), idx, CK_DELCTOR_CALL); }
    CProxyElement_SWE_DimensionalSplittingCharm operator() (const CkArrayIndex1D &idx) const
    { return CProxyElement_SWE_DimensionalSplittingCharm(ckGetArrayID(), idx, CK_DELCTOR_CALL); }
    CProxyElement_SWE_DimensionalSplittingCharm operator [] (int idx) const 
        {return CProxyElement_SWE_DimensionalSplittingCharm(ckGetArrayID(), CkArrayIndex1D(idx), CK_DELCTOR_CALL);}
    CProxyElement_SWE_DimensionalSplittingCharm operator () (int idx) const 
        {return CProxyElement_SWE_DimensionalSplittingCharm(ckGetArrayID(), CkArrayIndex1D(idx), CK_DELCTOR_CALL);}
    CProxy_SWE_DimensionalSplittingCharm(const CkArrayID &aid,CK_DELCTOR_PARAM) 
        :CProxy_ArrayElement(aid,CK_DELCTOR_ARGS) {}
    CProxy_SWE_DimensionalSplittingCharm(const CkArrayID &aid) 
        :CProxy_ArrayElement(aid) {}
/* DECLS: SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
 */
    
    static CkArrayID ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, const CkArrayOptions &opts = CkArrayOptions(), const CkEntryOptions *impl_e_opts=NULL);
    static void      ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, const CkArrayOptions &opts, CkCallback _ck_array_creation_cb, const CkEntryOptions *impl_e_opts=NULL);
    static CkArrayID ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, const int s1, const CkEntryOptions *impl_e_opts=NULL);
    static void ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, const int s1, CkCallback _ck_array_creation_cb, const CkEntryOptions *impl_e_opts=NULL);

/* DECLS: void compute();
 */
    
    void compute(const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: void receiveGhostLeft(copyLayer* impl_msg);
 */
    
    void receiveGhostLeft(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostRight(copyLayer* impl_msg);
 */
    
    void receiveGhostRight(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostBottom(copyLayer* impl_msg);
 */
    
    void receiveGhostBottom(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostTop(copyLayer* impl_msg);
 */
    
    void receiveGhostTop(copyLayer* impl_msg) ;

/* DECLS: void reductionTrigger();
 */
    
    void reductionTrigger(const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: void reduceWaveSpeed(float maxWaveSpeed);
 */
    
    void reduceWaveSpeed(float maxWaveSpeed, const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
 */

};
PUPmarshall(CProxy_SWE_DimensionalSplittingCharm)
/* ---------------- section proxy -------------- */
 class CProxySection_SWE_DimensionalSplittingCharm : public CProxySection_ArrayElement{
  public:
    typedef SWE_DimensionalSplittingCharm local_t;
    typedef CkIndex_SWE_DimensionalSplittingCharm index_t;
    typedef CProxy_SWE_DimensionalSplittingCharm proxy_t;
    typedef CProxyElement_SWE_DimensionalSplittingCharm element_t;
    typedef CProxySection_SWE_DimensionalSplittingCharm section_t;

    using array_index_t = CkArrayIndex1D;
    CProxySection_SWE_DimensionalSplittingCharm(void) {
    }

    void ckDelegate(CkDelegateMgr *dTo,CkDelegateData *dPtr=NULL)
    {       CProxySection_ArrayElement::ckDelegate(dTo,dPtr); }
    void ckUndelegate(void)
    {       CProxySection_ArrayElement::ckUndelegate(); }
    void pup(PUP::er &p)
    {       CProxySection_ArrayElement::pup(p);
    }

    int ckIsDelegated(void) const
    { return CProxySection_ArrayElement::ckIsDelegated(); }
    inline CkDelegateMgr *ckDelegatedTo(void) const
    { return CProxySection_ArrayElement::ckDelegatedTo(); }
    inline CkDelegateData *ckDelegatedPtr(void) const
    { return CProxySection_ArrayElement::ckDelegatedPtr(); }
    CkGroupID ckDelegatedIdx(void) const
    { return CProxySection_ArrayElement::ckDelegatedIdx(); }

    inline void ckCheck(void) const
    { CProxySection_ArrayElement::ckCheck(); }
    inline operator CkArrayID () const
    { return ckGetArrayID(); }
    inline CkArrayID ckGetArrayID(void) const
    { return CProxySection_ArrayElement::ckGetArrayID(); }
    inline CkArray *ckLocalBranch(void) const
    { return CProxySection_ArrayElement::ckLocalBranch(); }
    inline CkLocMgr *ckLocMgr(void) const
    { return CProxySection_ArrayElement::ckLocMgr(); }

    inline static CkArrayID ckCreateEmptyArray(CkArrayOptions opts = CkArrayOptions())
    { return CProxySection_ArrayElement::ckCreateEmptyArray(opts); }
    inline static void ckCreateEmptyArrayAsync(CkCallback cb, CkArrayOptions opts = CkArrayOptions())
    { CProxySection_ArrayElement::ckCreateEmptyArrayAsync(cb, opts); }
    inline static CkArrayID ckCreateArray(CkArrayMessage *m,int ctor,const CkArrayOptions &opts)
    { return CProxySection_ArrayElement::ckCreateArray(m,ctor,opts); }
    inline void ckInsertIdx(CkArrayMessage *m,int ctor,int onPe,const CkArrayIndex &idx)
    { CProxySection_ArrayElement::ckInsertIdx(m,ctor,onPe,idx); }
    inline void doneInserting(void)
    { CProxySection_ArrayElement::doneInserting(); }

    inline void ckBroadcast(CkArrayMessage *m, int ep, int opts=0) const
    { CProxySection_ArrayElement::ckBroadcast(m,ep,opts); }
    inline void setReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxySection_ArrayElement::setReductionClient(fn,param); }
    inline void ckSetReductionClient(CkReductionClientFn fn,void *param=NULL) const
    { CProxySection_ArrayElement::ckSetReductionClient(fn,param); }
    inline void ckSetReductionClient(CkCallback *cb) const
    { CProxySection_ArrayElement::ckSetReductionClient(cb); }

    inline void ckSend(CkArrayMessage *m, int ep, int opts = 0)
    { CProxySection_ArrayElement::ckSend(m,ep,opts); }
    inline CkSectionInfo &ckGetSectionInfo()
    { return CProxySection_ArrayElement::ckGetSectionInfo(); }
    inline CkSectionID *ckGetSectionIDs()
    { return CProxySection_ArrayElement::ckGetSectionIDs(); }
    inline CkSectionID &ckGetSectionID()
    { return CProxySection_ArrayElement::ckGetSectionID(); }
    inline CkSectionID &ckGetSectionID(int i)
    { return CProxySection_ArrayElement::ckGetSectionID(i); }
    inline CkArrayID ckGetArrayIDn(int i) const
    { return CProxySection_ArrayElement::ckGetArrayIDn(i); } 
    inline CkArrayIndex *ckGetArrayElements() const
    { return CProxySection_ArrayElement::ckGetArrayElements(); }
    inline CkArrayIndex *ckGetArrayElements(int i) const
    { return CProxySection_ArrayElement::ckGetArrayElements(i); }
    inline int ckGetNumElements() const
    { return CProxySection_ArrayElement::ckGetNumElements(); } 
    inline int ckGetNumElements(int i) const
    { return CProxySection_ArrayElement::ckGetNumElements(i); }    // Generalized array indexing:
    CProxyElement_SWE_DimensionalSplittingCharm operator [] (const CkArrayIndex1D &idx) const
        {return CProxyElement_SWE_DimensionalSplittingCharm(ckGetArrayID(), idx, CK_DELCTOR_CALL);}
    CProxyElement_SWE_DimensionalSplittingCharm operator() (const CkArrayIndex1D &idx) const
        {return CProxyElement_SWE_DimensionalSplittingCharm(ckGetArrayID(), idx, CK_DELCTOR_CALL);}
    CProxyElement_SWE_DimensionalSplittingCharm operator [] (int idx) const 
        {return CProxyElement_SWE_DimensionalSplittingCharm(ckGetArrayID(), *(CkArrayIndex1D*)&ckGetArrayElements()[idx], CK_DELCTOR_CALL);}
    CProxyElement_SWE_DimensionalSplittingCharm operator () (int idx) const 
        {return CProxyElement_SWE_DimensionalSplittingCharm(ckGetArrayID(), *(CkArrayIndex1D*)&ckGetArrayElements()[idx], CK_DELCTOR_CALL);}
    static CkSectionID ckNew(const CkArrayID &aid, CkArrayIndex1D *elems, int nElems, int factor=USE_DEFAULT_BRANCH_FACTOR) {
      return CkSectionID(aid, elems, nElems, factor);
    } 
    static CkSectionID ckNew(const CkArrayID &aid, const std::vector<CkArrayIndex1D> &elems, int factor=USE_DEFAULT_BRANCH_FACTOR) {
      return CkSectionID(aid, elems, factor);
    } 
    static CkSectionID ckNew(const CkArrayID &aid, int l, int u, int s, int factor=USE_DEFAULT_BRANCH_FACTOR) {
      std::vector<CkArrayIndex1D> al;
      for (int i=l; i<=u; i+=s) al.emplace_back(i);
      return CkSectionID(aid, al, factor);
    } 
    CProxySection_SWE_DimensionalSplittingCharm(const CkArrayID &aid, CkArrayIndex *elems, int nElems, CK_DELCTOR_PARAM) 
        :CProxySection_ArrayElement(aid,elems,nElems,CK_DELCTOR_ARGS) {}
    CProxySection_SWE_DimensionalSplittingCharm(const CkArrayID &aid, const std::vector<CkArrayIndex> &elems, CK_DELCTOR_PARAM) 
        :CProxySection_ArrayElement(aid,elems,CK_DELCTOR_ARGS) {}
    CProxySection_SWE_DimensionalSplittingCharm(const CkArrayID &aid, CkArrayIndex *elems, int nElems, int factor=USE_DEFAULT_BRANCH_FACTOR) 
        :CProxySection_ArrayElement(aid,elems,nElems, factor) {}
    CProxySection_SWE_DimensionalSplittingCharm(const CkArrayID &aid, const std::vector<CkArrayIndex> &elems, int factor=USE_DEFAULT_BRANCH_FACTOR) 
        :CProxySection_ArrayElement(aid,elems, factor) { ckAutoDelegate(); }
    CProxySection_SWE_DimensionalSplittingCharm(const CkSectionID &sid)  
        :CProxySection_ArrayElement(sid) { ckAutoDelegate(); }
    CProxySection_SWE_DimensionalSplittingCharm(int n, const CkArrayID *aid, CkArrayIndex const * const *elems, const int *nElems, CK_DELCTOR_PARAM) 
        :CProxySection_ArrayElement(n,aid,elems,nElems,CK_DELCTOR_ARGS) {}
    CProxySection_SWE_DimensionalSplittingCharm(const std::vector<CkArrayID> &aid, const std::vector<std::vector<CkArrayIndex> > &elems, CK_DELCTOR_PARAM) 
        :CProxySection_ArrayElement(aid,elems,CK_DELCTOR_ARGS) {}
    CProxySection_SWE_DimensionalSplittingCharm(int n, const CkArrayID *aid, CkArrayIndex const * const *elems, const int *nElems) 
        :CProxySection_ArrayElement(n,aid,elems,nElems) { ckAutoDelegate(); }
    CProxySection_SWE_DimensionalSplittingCharm(const std::vector<CkArrayID> &aid, const std::vector<std::vector<CkArrayIndex> > &elems) 
        :CProxySection_ArrayElement(aid,elems) { ckAutoDelegate(); }
    CProxySection_SWE_DimensionalSplittingCharm(int n, const CkArrayID *aid, CkArrayIndex const * const *elems, const int *nElems, int factor) 
        :CProxySection_ArrayElement(n,aid,elems,nElems, factor) { ckAutoDelegate(); }
    CProxySection_SWE_DimensionalSplittingCharm(const std::vector<CkArrayID> &aid, const std::vector<std::vector<CkArrayIndex> > &elems, int factor) 
        :CProxySection_ArrayElement(aid,elems, factor) { ckAutoDelegate(); }
    static CkSectionID ckNew(const CkArrayID &aid, CkArrayIndex *elems, int nElems) {
      return CkSectionID(aid, elems, nElems);
    } 
    static CkSectionID ckNew(const CkArrayID &aid, const std::vector<CkArrayIndex> &elems) {
       return CkSectionID(aid, elems);
    } 
    static CkSectionID ckNew(const CkArrayID &aid, CkArrayIndex *elems, int nElems, int factor) {
      return CkSectionID(aid, elems, nElems, factor);
    } 
    static CkSectionID ckNew(const CkArrayID &aid, const std::vector<CkArrayIndex> &elems, int factor) {
      return CkSectionID(aid, elems, factor);
    } 
    void ckAutoDelegate(int opts=1) {
      if(ckIsDelegated()) return;
      CProxySection_ArrayElement::ckAutoDelegate(opts);
    } 
    void setReductionClient(CkCallback *cb) {
      CProxySection_ArrayElement::setReductionClient(cb);
    } 
    void resetSection() {
      CProxySection_ArrayElement::resetSection();
    } 
    static void contribute(CkSectionInfo &sid, int userData=-1, int fragSize=-1);
    static void contribute(int dataSize,void *data,CkReduction::reducerType type, CkSectionInfo &sid, int userData=-1, int fragSize=-1);
    template <typename T>
    static void contribute(std::vector<T> &data, CkReduction::reducerType type, CkSectionInfo &sid, int userData=-1, int fragSize=-1);
    static void contribute(CkSectionInfo &sid, const CkCallback &cb, int userData=-1, int fragSize=-1);
    static void contribute(int dataSize,void *data,CkReduction::reducerType type, CkSectionInfo &sid, const CkCallback &cb, int userData=-1, int fragSize=-1);
    template <typename T>
    static void contribute(std::vector<T> &data, CkReduction::reducerType type, CkSectionInfo &sid, const CkCallback &cb, int userData=-1, int fragSize=-1);
/* DECLS: SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
 */
    

/* DECLS: void compute();
 */
    
    void compute(const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: void receiveGhostLeft(copyLayer* impl_msg);
 */
    
    void receiveGhostLeft(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostRight(copyLayer* impl_msg);
 */
    
    void receiveGhostRight(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostBottom(copyLayer* impl_msg);
 */
    
    void receiveGhostBottom(copyLayer* impl_msg) ;

/* DECLS: void receiveGhostTop(copyLayer* impl_msg);
 */
    
    void receiveGhostTop(copyLayer* impl_msg) ;

/* DECLS: void reductionTrigger();
 */
    
    void reductionTrigger(const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: void reduceWaveSpeed(float maxWaveSpeed);
 */
    
    void reduceWaveSpeed(float maxWaveSpeed, const CkEntryOptions *impl_e_opts=NULL) ;

/* DECLS: SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
 */

};
PUPmarshall(CProxySection_SWE_DimensionalSplittingCharm)
#define SWE_DimensionalSplittingCharm_SDAG_CODE                                \
public:                                                                        \
  void compute();                                                              \
  void _sdag_fnc_compute();                                                    \
private:                                                                       \
  void compute_end();                                                          \
  void _slist_0();                                                             \
  void _slist_0_end();                                                         \
  void _serial_0();                                                            \
  void _while_0();                                                             \
  void _while_0_end();                                                         \
  void _slist_1();                                                             \
  void _slist_1_end();                                                         \
  void _serial_1();                                                            \
  void _overlap_0();                                                           \
  void _overlap_0_end();                                                       \
  void _olist_0();                                                             \
  void _olist_0_end(SDAG::CCounter* _co0);                                     \
  SDAG::Continuation* _when_0(SDAG::CCounter* _co0);                           \
  void _when_0_end(SDAG::CCounter* _co0, copyLayer* gen1);                     \
  void _if_0(SDAG::CCounter* _co0, copyLayer* gen1);                           \
  void _if_0_end(SDAG::CCounter* _co0, copyLayer* gen1);                       \
  void _slist_2(SDAG::CCounter* _co0, copyLayer* gen1);                        \
  void _slist_2_end(SDAG::CCounter* _co0, copyLayer* gen1);                    \
  void _serial_2(SDAG::CCounter* _co0, copyLayer* gen1);                       \
  SDAG::Continuation* _when_1(SDAG::CCounter* _co0);                           \
  void _when_1_end(SDAG::CCounter* _co0, copyLayer* gen1);                     \
  void _if_1(SDAG::CCounter* _co0, copyLayer* gen1);                           \
  void _if_1_end(SDAG::CCounter* _co0, copyLayer* gen1);                       \
  void _slist_3(SDAG::CCounter* _co0, copyLayer* gen1);                        \
  void _slist_3_end(SDAG::CCounter* _co0, copyLayer* gen1);                    \
  void _serial_3(SDAG::CCounter* _co0, copyLayer* gen1);                       \
  SDAG::Continuation* _when_2(SDAG::CCounter* _co0);                           \
  void _when_2_end(SDAG::CCounter* _co0, copyLayer* gen1);                     \
  void _if_2(SDAG::CCounter* _co0, copyLayer* gen1);                           \
  void _if_2_end(SDAG::CCounter* _co0, copyLayer* gen1);                       \
  void _slist_4(SDAG::CCounter* _co0, copyLayer* gen1);                        \
  void _slist_4_end(SDAG::CCounter* _co0, copyLayer* gen1);                    \
  void _serial_4(SDAG::CCounter* _co0, copyLayer* gen1);                       \
  SDAG::Continuation* _when_3(SDAG::CCounter* _co0);                           \
  void _when_3_end(SDAG::CCounter* _co0, copyLayer* gen1);                     \
  void _if_3(SDAG::CCounter* _co0, copyLayer* gen1);                           \
  void _if_3_end(SDAG::CCounter* _co0, copyLayer* gen1);                       \
  void _slist_5(SDAG::CCounter* _co0, copyLayer* gen1);                        \
  void _slist_5_end(SDAG::CCounter* _co0, copyLayer* gen1);                    \
  void _serial_5(SDAG::CCounter* _co0, copyLayer* gen1);                       \
  void _serial_6();                                                            \
  SDAG::Continuation* _when_4();                                               \
  void _when_4_end();                                                          \
  void _serial_7();                                                            \
public:                                                                        \
  void receiveGhostLeft(copyLayer* msg_msg);                                   \
  void receiveGhostRight(copyLayer* msg_msg);                                  \
  void receiveGhostBottom(copyLayer* msg_msg);                                 \
  void receiveGhostTop(copyLayer* msg_msg);                                    \
  void reductionTrigger(Closure_SWE_DimensionalSplittingCharm::reductionTrigger_7_closure* genClosure);\
  void reductionTrigger();                                                     \
public:                                                                        \
  SDAG::dep_ptr __dep;                                                         \
  void _sdag_init();                                                           \
  void __sdag_init();                                                          \
public:                                                                        \
  void _sdag_pup(PUP::er &p);                                                  \
  void __sdag_pup(PUP::er &p) { }                                              \
  static void __sdag_register();                                               \
  static int _sdag_idx_SWE_DimensionalSplittingCharm_serial_0();               \
  static int _sdag_reg_SWE_DimensionalSplittingCharm_serial_0();               \
  static int _sdag_idx_SWE_DimensionalSplittingCharm_serial_1();               \
  static int _sdag_reg_SWE_DimensionalSplittingCharm_serial_1();               \
  static int _sdag_idx_SWE_DimensionalSplittingCharm_serial_2();               \
  static int _sdag_reg_SWE_DimensionalSplittingCharm_serial_2();               \
  static int _sdag_idx_SWE_DimensionalSplittingCharm_serial_3();               \
  static int _sdag_reg_SWE_DimensionalSplittingCharm_serial_3();               \
  static int _sdag_idx_SWE_DimensionalSplittingCharm_serial_4();               \
  static int _sdag_reg_SWE_DimensionalSplittingCharm_serial_4();               \
  static int _sdag_idx_SWE_DimensionalSplittingCharm_serial_5();               \
  static int _sdag_reg_SWE_DimensionalSplittingCharm_serial_5();               \
  static int _sdag_idx_SWE_DimensionalSplittingCharm_serial_6();               \
  static int _sdag_reg_SWE_DimensionalSplittingCharm_serial_6();               \
  static int _sdag_idx_SWE_DimensionalSplittingCharm_serial_7();               \
  static int _sdag_reg_SWE_DimensionalSplittingCharm_serial_7();               \

typedef CBaseT1<ArrayElementT<CkIndex1D>, CProxy_SWE_DimensionalSplittingCharm>CBase_SWE_DimensionalSplittingCharm;






/* ---------------- method closures -------------- */
class Closure_SWE_DimensionalSplittingCharm {
  public:


    struct compute_2_closure;






    struct reductionTrigger_7_closure;


    struct reduceWaveSpeed_8_closure;


};

extern void _registerSWE_DimensionalSplittingCharm(void);
#endif
