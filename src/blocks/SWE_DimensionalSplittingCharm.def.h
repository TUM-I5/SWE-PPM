




/* ---------------- method closures -------------- */
#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_SWE_DimensionalSplittingCharm::compute_2_closure : public SDAG::Closure {
      

      compute_2_closure() {
        init();
      }
      compute_2_closure(CkMigrateMessage*) {
        init();
      }
            void pup(PUP::er& __p) {
        packClosure(__p);
      }
      virtual ~compute_2_closure() {
      }
      PUPable_decl(SINGLE_ARG(compute_2_closure));
    };
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_SWE_DimensionalSplittingCharm::reductionTrigger_7_closure : public SDAG::Closure {
      

      reductionTrigger_7_closure() {
        init();
      }
      reductionTrigger_7_closure(CkMigrateMessage*) {
        init();
      }
            void pup(PUP::er& __p) {
        packClosure(__p);
      }
      virtual ~reductionTrigger_7_closure() {
      }
      PUPable_decl(SINGLE_ARG(reductionTrigger_7_closure));
    };
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_SWE_DimensionalSplittingCharm::printFlops_8_closure : public SDAG::Closure {
            double flop;


      printFlops_8_closure() {
        init();
      }
      printFlops_8_closure(CkMigrateMessage*) {
        init();
      }
            double & getP0() { return flop;}
      void pup(PUP::er& __p) {
        __p | flop;
        packClosure(__p);
      }
      virtual ~printFlops_8_closure() {
      }
      PUPable_decl(SINGLE_ARG(printFlops_8_closure));
    };
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_SWE_DimensionalSplittingCharm::reduceWaveSpeed_9_closure : public SDAG::Closure {
            float maxWaveSpeed;


      reduceWaveSpeed_9_closure() {
        init();
      }
      reduceWaveSpeed_9_closure(CkMigrateMessage*) {
        init();
      }
            float & getP0() { return maxWaveSpeed;}
      void pup(PUP::er& __p) {
        __p | maxWaveSpeed;
        packClosure(__p);
      }
      virtual ~reduceWaveSpeed_9_closure() {
      }
      PUPable_decl(SINGLE_ARG(reduceWaveSpeed_9_closure));
    };
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */






/* DEFS: message copyLayer{
Boundary boundary;
bool containsBathymetry;
float b[];
float h[];
float hu[];
float hv[];
}
;
 */
#ifndef CK_TEMPLATES_ONLY
void *CMessage_copyLayer::operator new(size_t s){
  return copyLayer::alloc(__idx, s, 0, 0, GroupDepNum{});
}
void *CMessage_copyLayer::operator new(size_t s, int* sz){
  return copyLayer::alloc(__idx, s, sz, 0, GroupDepNum{});
}
void *CMessage_copyLayer::operator new(size_t s, int* sz,const int pb){
  return copyLayer::alloc(__idx, s, sz, pb, GroupDepNum{});
}
void *CMessage_copyLayer::operator new(size_t s, int* sz,const int pb, const GroupDepNum groupDepNum){
  return copyLayer::alloc(__idx, s, sz, pb, groupDepNum);
}
void *CMessage_copyLayer::operator new(size_t s, int sz0, int sz1, int sz2, int sz3) {
  int sizes[4];
  sizes[0] = sz0;
  sizes[1] = sz1;
  sizes[2] = sz2;
  sizes[3] = sz3;
  return copyLayer::alloc(__idx, s, sizes, 0, GroupDepNum{});
}
void *CMessage_copyLayer::operator new(size_t s, int sz0, int sz1, int sz2, int sz3, const int p) {
  int sizes[4];
  sizes[0] = sz0;
  sizes[1] = sz1;
  sizes[2] = sz2;
  sizes[3] = sz3;
  return copyLayer::alloc(__idx, s, sizes, p, GroupDepNum{});
}
void *CMessage_copyLayer::operator new(size_t s, int sz0, int sz1, int sz2, int sz3, const int p, const GroupDepNum groupDepNum) {
  int sizes[4];
  sizes[0] = sz0;
  sizes[1] = sz1;
  sizes[2] = sz2;
  sizes[3] = sz3;
  return copyLayer::alloc(__idx, s, sizes, p, groupDepNum);
}
void* CMessage_copyLayer::alloc(int msgnum, size_t sz, int *sizes, int pb, GroupDepNum groupDepNum) {
  CkpvAccess(_offsets)[0] = ALIGN_DEFAULT(sz);
  if(sizes==0)
    CkpvAccess(_offsets)[1] = CkpvAccess(_offsets)[0];
  else
    CkpvAccess(_offsets)[1] = CkpvAccess(_offsets)[0] + ALIGN_DEFAULT(sizeof(float)*sizes[0]);
  if(sizes==0)
    CkpvAccess(_offsets)[2] = CkpvAccess(_offsets)[0];
  else
    CkpvAccess(_offsets)[2] = CkpvAccess(_offsets)[1] + ALIGN_DEFAULT(sizeof(float)*sizes[1]);
  if(sizes==0)
    CkpvAccess(_offsets)[3] = CkpvAccess(_offsets)[0];
  else
    CkpvAccess(_offsets)[3] = CkpvAccess(_offsets)[2] + ALIGN_DEFAULT(sizeof(float)*sizes[2]);
  if(sizes==0)
    CkpvAccess(_offsets)[4] = CkpvAccess(_offsets)[0];
  else
    CkpvAccess(_offsets)[4] = CkpvAccess(_offsets)[3] + ALIGN_DEFAULT(sizeof(float)*sizes[3]);
  return CkAllocMsg(msgnum, CkpvAccess(_offsets)[4], pb, groupDepNum);
}
CMessage_copyLayer::CMessage_copyLayer() {
copyLayer *newmsg = (copyLayer *)this;
  newmsg->b = (float *) ((char *)newmsg + CkpvAccess(_offsets)[0]);
  newmsg->h = (float *) ((char *)newmsg + CkpvAccess(_offsets)[1]);
  newmsg->hu = (float *) ((char *)newmsg + CkpvAccess(_offsets)[2]);
  newmsg->hv = (float *) ((char *)newmsg + CkpvAccess(_offsets)[3]);
}
void CMessage_copyLayer::dealloc(void *p) {
  CkFreeMsg(p);
}
void* CMessage_copyLayer::pack(copyLayer *msg) {
  msg->b = (float *) ((char *)msg->b - (char *)msg);
  msg->h = (float *) ((char *)msg->h - (char *)msg);
  msg->hu = (float *) ((char *)msg->hu - (char *)msg);
  msg->hv = (float *) ((char *)msg->hv - (char *)msg);
  return (void *) msg;
}
copyLayer* CMessage_copyLayer::unpack(void* buf) {
  copyLayer *msg = (copyLayer *) buf;
  msg->b = (float *) ((size_t)msg->b + (char *)msg);
  msg->h = (float *) ((size_t)msg->h + (char *)msg);
  msg->hu = (float *) ((size_t)msg->hu + (char *)msg);
  msg->hv = (float *) ((size_t)msg->hv + (char *)msg);
  return msg;
}
int CMessage_copyLayer::__idx=0;
#endif /* CK_TEMPLATES_ONLY */

/* DEFS: array SWE_DimensionalSplittingCharm: ArrayElement{
SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
void compute();
void receiveGhostLeft(copyLayer* impl_msg);
void receiveGhostRight(copyLayer* impl_msg);
void receiveGhostBottom(copyLayer* impl_msg);
void receiveGhostTop(copyLayer* impl_msg);
void reductionTrigger();
void printFlops(double flop);
void reduceWaveSpeed(float maxWaveSpeed);
SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
};
 */
#ifndef CK_TEMPLATES_ONLY
 int CkIndex_SWE_DimensionalSplittingCharm::__idx=0;
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void CProxySection_SWE_DimensionalSplittingCharm::contribute(CkSectionInfo &sid, int userData, int fragSize)
{
   CkArray *ckarr = CProxy_CkArray(sid.get_aid()).ckLocalBranch();
   CkMulticastMgr *mCastGrp = CProxy_CkMulticastMgr(ckarr->getmCastMgr()).ckLocalBranch();
   mCastGrp->contribute(sid, userData, fragSize);
}

void CProxySection_SWE_DimensionalSplittingCharm::contribute(int dataSize,void *data,CkReduction::reducerType type, CkSectionInfo &sid, int userData, int fragSize)
{
   CkArray *ckarr = CProxy_CkArray(sid.get_aid()).ckLocalBranch();
   CkMulticastMgr *mCastGrp = CProxy_CkMulticastMgr(ckarr->getmCastMgr()).ckLocalBranch();
   mCastGrp->contribute(dataSize, data, type, sid, userData, fragSize);
}

template <typename T>
void CProxySection_SWE_DimensionalSplittingCharm::contribute(std::vector<T> &data, CkReduction::reducerType type, CkSectionInfo &sid, int userData, int fragSize)
{
   CkArray *ckarr = CProxy_CkArray(sid.get_aid()).ckLocalBranch();
   CkMulticastMgr *mCastGrp = CProxy_CkMulticastMgr(ckarr->getmCastMgr()).ckLocalBranch();
   mCastGrp->contribute(data, type, sid, userData, fragSize);
}

void CProxySection_SWE_DimensionalSplittingCharm::contribute(CkSectionInfo &sid, const CkCallback &cb, int userData, int fragSize)
{
   CkArray *ckarr = CProxy_CkArray(sid.get_aid()).ckLocalBranch();
   CkMulticastMgr *mCastGrp = CProxy_CkMulticastMgr(ckarr->getmCastMgr()).ckLocalBranch();
   mCastGrp->contribute(sid, cb, userData, fragSize);
}

void CProxySection_SWE_DimensionalSplittingCharm::contribute(int dataSize,void *data,CkReduction::reducerType type, CkSectionInfo &sid, const CkCallback &cb, int userData, int fragSize)
{
   CkArray *ckarr = CProxy_CkArray(sid.get_aid()).ckLocalBranch();
   CkMulticastMgr *mCastGrp = CProxy_CkMulticastMgr(ckarr->getmCastMgr()).ckLocalBranch();
   mCastGrp->contribute(dataSize, data, type, sid, cb, userData, fragSize);
}

template <typename T>
void CProxySection_SWE_DimensionalSplittingCharm::contribute(std::vector<T> &data, CkReduction::reducerType type, CkSectionInfo &sid, const CkCallback &cb, int userData, int fragSize)
{
   CkArray *ckarr = CProxy_CkArray(sid.get_aid()).ckLocalBranch();
   CkMulticastMgr *mCastGrp = CProxy_CkMulticastMgr(ckarr->getmCastMgr()).ckLocalBranch();
   mCastGrp->contribute(data, type, sid, cb, userData, fragSize);
}

#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
/* DEFS: SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
 */
void CProxyElement_SWE_DimensionalSplittingCharm::insert(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, int onPE, const CkEntryOptions *impl_e_opts)
{ 
   //Marshall: int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile
  int impl_off=0;
  int impl_arrstart=0;
  int impl_off_boundaries, impl_cnt_boundaries;
  impl_off_boundaries=impl_off=CK_ALIGN(impl_off,sizeof(BoundaryType));
  impl_off+=(impl_cnt_boundaries=sizeof(BoundaryType)*(4));
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
    impl_arrstart=CK_ALIGN(implP.size(),16);
    impl_off+=impl_arrstart;
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
  }
  char *impl_buf=impl_msg->msgBuf+impl_arrstart;
  memcpy(impl_buf+impl_off_boundaries,boundaries,impl_cnt_boundaries);
   UsrToEnv(impl_msg)->setMsgtype(ArrayEltInitMsg);
   ckInsert((CkArrayMessage *)impl_msg,CkIndex_SWE_DimensionalSplittingCharm::idx_SWE_DimensionalSplittingCharm_marshall1(),onPE);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void compute();
 */
void CProxyElement_SWE_DimensionalSplittingCharm::compute(const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_compute_void(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostLeft(copyLayer* impl_msg);
 */
void CProxyElement_SWE_DimensionalSplittingCharm::receiveGhostLeft(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostLeft_copyLayer(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostRight(copyLayer* impl_msg);
 */
void CProxyElement_SWE_DimensionalSplittingCharm::receiveGhostRight(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostRight_copyLayer(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostBottom(copyLayer* impl_msg);
 */
void CProxyElement_SWE_DimensionalSplittingCharm::receiveGhostBottom(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostBottom_copyLayer(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostTop(copyLayer* impl_msg);
 */
void CProxyElement_SWE_DimensionalSplittingCharm::receiveGhostTop(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostTop_copyLayer(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void reductionTrigger();
 */
void CProxyElement_SWE_DimensionalSplittingCharm::reductionTrigger(const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_reductionTrigger_void(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void printFlops(double flop);
 */
void CProxyElement_SWE_DimensionalSplittingCharm::printFlops(double flop, const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  //Marshall: double flop
  int impl_off=0;
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|flop;
    impl_off+=implP.size();
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|flop;
  }
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_printFlops_marshall8(),0);
}
void CkIndex_SWE_DimensionalSplittingCharm::_call_redn_wrapper_printFlops_marshall8(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*> (impl_obj_void);
  char* impl_buf = (char*)((CkReductionMsg*)impl_msg)->getData();
  /*Unmarshall pup'd fields: double flop*/
  PUP::fromMem implP(impl_buf);
  /* non two-param case */
  PUP::detail::TemporaryObjectHolder<double> flop;
  implP|flop;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->printFlops(std::move(flop.t));
  delete (CkReductionMsg*)impl_msg;
}

#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void reduceWaveSpeed(float maxWaveSpeed);
 */
void CProxyElement_SWE_DimensionalSplittingCharm::reduceWaveSpeed(float maxWaveSpeed, const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  //Marshall: float maxWaveSpeed
  int impl_off=0;
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|maxWaveSpeed;
    impl_off+=implP.size();
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|maxWaveSpeed;
  }
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_reduceWaveSpeed_marshall9(),0);
}
void CkIndex_SWE_DimensionalSplittingCharm::_call_redn_wrapper_reduceWaveSpeed_marshall9(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*> (impl_obj_void);
  char* impl_buf = (char*)((CkReductionMsg*)impl_msg)->getData();
  /*Unmarshall pup'd fields: float maxWaveSpeed*/
  PUP::fromMem implP(impl_buf);
  /* non two-param case */
  PUP::detail::TemporaryObjectHolder<float> maxWaveSpeed;
  implP|maxWaveSpeed;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->reduceWaveSpeed(std::move(maxWaveSpeed.t));
  delete (CkReductionMsg*)impl_msg;
}

#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
 */
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
 */
CkArrayID CProxy_SWE_DimensionalSplittingCharm::ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, const CkArrayOptions &opts, const CkEntryOptions *impl_e_opts)
{
  //Marshall: int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile
  int impl_off=0;
  int impl_arrstart=0;
  int impl_off_boundaries, impl_cnt_boundaries;
  impl_off_boundaries=impl_off=CK_ALIGN(impl_off,sizeof(BoundaryType));
  impl_off+=(impl_cnt_boundaries=sizeof(BoundaryType)*(4));
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
    impl_arrstart=CK_ALIGN(implP.size(),16);
    impl_off+=impl_arrstart;
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
  }
  char *impl_buf=impl_msg->msgBuf+impl_arrstart;
  memcpy(impl_buf+impl_off_boundaries,boundaries,impl_cnt_boundaries);
  UsrToEnv(impl_msg)->setMsgtype(ArrayEltInitMsg);
  CkArrayID gId = ckCreateArray((CkArrayMessage *)impl_msg, CkIndex_SWE_DimensionalSplittingCharm::idx_SWE_DimensionalSplittingCharm_marshall1(), opts);
  return gId;
}
void CProxy_SWE_DimensionalSplittingCharm::ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, const CkArrayOptions &opts, CkCallback _ck_array_creation_cb, const CkEntryOptions *impl_e_opts)
{
  //Marshall: int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile
  int impl_off=0;
  int impl_arrstart=0;
  int impl_off_boundaries, impl_cnt_boundaries;
  impl_off_boundaries=impl_off=CK_ALIGN(impl_off,sizeof(BoundaryType));
  impl_off+=(impl_cnt_boundaries=sizeof(BoundaryType)*(4));
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
    impl_arrstart=CK_ALIGN(implP.size(),16);
    impl_off+=impl_arrstart;
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
  }
  char *impl_buf=impl_msg->msgBuf+impl_arrstart;
  memcpy(impl_buf+impl_off_boundaries,boundaries,impl_cnt_boundaries);
  UsrToEnv(impl_msg)->setMsgtype(ArrayEltInitMsg);
  CkSendAsyncCreateArray(CkIndex_SWE_DimensionalSplittingCharm::idx_SWE_DimensionalSplittingCharm_marshall1(), _ck_array_creation_cb, opts, impl_msg);
}
CkArrayID CProxy_SWE_DimensionalSplittingCharm::ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, const int s1, const CkEntryOptions *impl_e_opts)
{
  //Marshall: int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile
  int impl_off=0;
  int impl_arrstart=0;
  int impl_off_boundaries, impl_cnt_boundaries;
  impl_off_boundaries=impl_off=CK_ALIGN(impl_off,sizeof(BoundaryType));
  impl_off+=(impl_cnt_boundaries=sizeof(BoundaryType)*(4));
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
    impl_arrstart=CK_ALIGN(implP.size(),16);
    impl_off+=impl_arrstart;
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
  }
  char *impl_buf=impl_msg->msgBuf+impl_arrstart;
  memcpy(impl_buf+impl_off_boundaries,boundaries,impl_cnt_boundaries);
  CkArrayOptions opts(s1);
  UsrToEnv(impl_msg)->setMsgtype(ArrayEltInitMsg);
  CkArrayID gId = ckCreateArray((CkArrayMessage *)impl_msg, CkIndex_SWE_DimensionalSplittingCharm::idx_SWE_DimensionalSplittingCharm_marshall1(), opts);
  return gId;
}
void CProxy_SWE_DimensionalSplittingCharm::ckNew(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile, const int s1, CkCallback _ck_array_creation_cb, const CkEntryOptions *impl_e_opts)
{
  //Marshall: int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile
  int impl_off=0;
  int impl_arrstart=0;
  int impl_off_boundaries, impl_cnt_boundaries;
  impl_off_boundaries=impl_off=CK_ALIGN(impl_off,sizeof(BoundaryType));
  impl_off+=(impl_cnt_boundaries=sizeof(BoundaryType)*(4));
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
    impl_arrstart=CK_ALIGN(implP.size(),16);
    impl_off+=impl_arrstart;
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|nx;
    implP|ny;
    implP|dy;
    implP|dx;
    implP|originX;
    implP|originY;
    implP|posX;
    implP|posY;
    implP|impl_off_boundaries;
    implP|impl_cnt_boundaries;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)outputFilename;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)bathymetryFile;
    //Have to cast away const-ness to get pup routine
    implP|(typename std::remove_cv<typename std::remove_reference<std::string>::type>::type &)displacementFile;
  }
  char *impl_buf=impl_msg->msgBuf+impl_arrstart;
  memcpy(impl_buf+impl_off_boundaries,boundaries,impl_cnt_boundaries);
  CkArrayOptions opts(s1);
  UsrToEnv(impl_msg)->setMsgtype(ArrayEltInitMsg);
  CkSendAsyncCreateArray(CkIndex_SWE_DimensionalSplittingCharm::idx_SWE_DimensionalSplittingCharm_marshall1(), _ck_array_creation_cb, opts, impl_msg);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_SWE_DimensionalSplittingCharm_marshall1() {
  int epidx = CkRegisterEp("SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile)",
      _call_SWE_DimensionalSplittingCharm_marshall1, CkMarshallMsg::__idx, __idx, 0+CK_EP_NOKEEP);
  CkRegisterMarshallUnpackFn(epidx, _callmarshall_SWE_DimensionalSplittingCharm_marshall1);
  CkRegisterMessagePupFn(epidx, _marshallmessagepup_SWE_DimensionalSplittingCharm_marshall1);

  return epidx;
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_SWE_DimensionalSplittingCharm_marshall1(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  /*Unmarshall pup'd fields: int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> nx;
  implP|nx;
  PUP::detail::TemporaryObjectHolder<int> ny;
  implP|ny;
  PUP::detail::TemporaryObjectHolder<float> dy;
  implP|dy;
  PUP::detail::TemporaryObjectHolder<float> dx;
  implP|dx;
  PUP::detail::TemporaryObjectHolder<float> originX;
  implP|originX;
  PUP::detail::TemporaryObjectHolder<float> originY;
  implP|originY;
  PUP::detail::TemporaryObjectHolder<int> posX;
  implP|posX;
  PUP::detail::TemporaryObjectHolder<int> posY;
  implP|posY;
  int impl_off_boundaries, impl_cnt_boundaries;
  implP|impl_off_boundaries;
  implP|impl_cnt_boundaries;
  PUP::detail::TemporaryObjectHolder<std::string> outputFilename;
  implP|outputFilename;
  PUP::detail::TemporaryObjectHolder<std::string> bathymetryFile;
  implP|bathymetryFile;
  PUP::detail::TemporaryObjectHolder<std::string> displacementFile;
  implP|displacementFile;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  BoundaryType *boundaries=(BoundaryType *)(impl_buf+impl_off_boundaries);
  new (impl_obj_void) SWE_DimensionalSplittingCharm(std::move(nx.t), std::move(ny.t), std::move(dy.t), std::move(dx.t), std::move(originX.t), std::move(originY.t), std::move(posX.t), std::move(posY.t), boundaries, std::move(outputFilename.t), std::move(bathymetryFile.t), std::move(displacementFile.t));
}
int CkIndex_SWE_DimensionalSplittingCharm::_callmarshall_SWE_DimensionalSplittingCharm_marshall1(char* impl_buf, void* impl_obj_void) {
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  /*Unmarshall pup'd fields: int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> nx;
  implP|nx;
  PUP::detail::TemporaryObjectHolder<int> ny;
  implP|ny;
  PUP::detail::TemporaryObjectHolder<float> dy;
  implP|dy;
  PUP::detail::TemporaryObjectHolder<float> dx;
  implP|dx;
  PUP::detail::TemporaryObjectHolder<float> originX;
  implP|originX;
  PUP::detail::TemporaryObjectHolder<float> originY;
  implP|originY;
  PUP::detail::TemporaryObjectHolder<int> posX;
  implP|posX;
  PUP::detail::TemporaryObjectHolder<int> posY;
  implP|posY;
  int impl_off_boundaries, impl_cnt_boundaries;
  implP|impl_off_boundaries;
  implP|impl_cnt_boundaries;
  PUP::detail::TemporaryObjectHolder<std::string> outputFilename;
  implP|outputFilename;
  PUP::detail::TemporaryObjectHolder<std::string> bathymetryFile;
  implP|bathymetryFile;
  PUP::detail::TemporaryObjectHolder<std::string> displacementFile;
  implP|displacementFile;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  BoundaryType *boundaries=(BoundaryType *)(impl_buf+impl_off_boundaries);
  new (impl_obj_void) SWE_DimensionalSplittingCharm(std::move(nx.t), std::move(ny.t), std::move(dy.t), std::move(dx.t), std::move(originX.t), std::move(originY.t), std::move(posX.t), std::move(posY.t), boundaries, std::move(outputFilename.t), std::move(bathymetryFile.t), std::move(displacementFile.t));
  return implP.size();
}
void CkIndex_SWE_DimensionalSplittingCharm::_marshallmessagepup_SWE_DimensionalSplittingCharm_marshall1(PUP::er &implDestP,void *impl_msg) {
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  /*Unmarshall pup'd fields: int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> nx;
  implP|nx;
  PUP::detail::TemporaryObjectHolder<int> ny;
  implP|ny;
  PUP::detail::TemporaryObjectHolder<float> dy;
  implP|dy;
  PUP::detail::TemporaryObjectHolder<float> dx;
  implP|dx;
  PUP::detail::TemporaryObjectHolder<float> originX;
  implP|originX;
  PUP::detail::TemporaryObjectHolder<float> originY;
  implP|originY;
  PUP::detail::TemporaryObjectHolder<int> posX;
  implP|posX;
  PUP::detail::TemporaryObjectHolder<int> posY;
  implP|posY;
  int impl_off_boundaries, impl_cnt_boundaries;
  implP|impl_off_boundaries;
  implP|impl_cnt_boundaries;
  PUP::detail::TemporaryObjectHolder<std::string> outputFilename;
  implP|outputFilename;
  PUP::detail::TemporaryObjectHolder<std::string> bathymetryFile;
  implP|bathymetryFile;
  PUP::detail::TemporaryObjectHolder<std::string> displacementFile;
  implP|displacementFile;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  BoundaryType *boundaries=(BoundaryType *)(impl_buf+impl_off_boundaries);
  if (implDestP.hasComments()) implDestP.comment("nx");
  implDestP|nx;
  if (implDestP.hasComments()) implDestP.comment("ny");
  implDestP|ny;
  if (implDestP.hasComments()) implDestP.comment("dy");
  implDestP|dy;
  if (implDestP.hasComments()) implDestP.comment("dx");
  implDestP|dx;
  if (implDestP.hasComments()) implDestP.comment("originX");
  implDestP|originX;
  if (implDestP.hasComments()) implDestP.comment("originY");
  implDestP|originY;
  if (implDestP.hasComments()) implDestP.comment("posX");
  implDestP|posX;
  if (implDestP.hasComments()) implDestP.comment("posY");
  implDestP|posY;
  if (implDestP.hasComments()) implDestP.comment("boundaries");
  implDestP.synchronize(PUP::sync_begin_array);
  for (int impl_i=0;impl_i*(sizeof(*boundaries))<impl_cnt_boundaries;impl_i++) {
    implDestP.synchronize(PUP::sync_item);
    implDestP|boundaries[impl_i];
  }
  implDestP.synchronize(PUP::sync_end_array);
  if (implDestP.hasComments()) implDestP.comment("outputFilename");
  implDestP|outputFilename;
  if (implDestP.hasComments()) implDestP.comment("bathymetryFile");
  implDestP|bathymetryFile;
  if (implDestP.hasComments()) implDestP.comment("displacementFile");
  implDestP|displacementFile;
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void compute();
 */
void CProxy_SWE_DimensionalSplittingCharm::compute(const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckBroadcast(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_compute_void(),0);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_compute_void() {
  int epidx = CkRegisterEp("compute()",
      _call_compute_void, 0, __idx, 0);
  return epidx;
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_compute_void(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  impl_obj->_sdag_fnc_compute();
  if(UsrToEnv(impl_msg)->isVarSysMsg() == 0)
    CkFreeSysMsg(impl_msg);
}
PUPable_def(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::compute_2_closure))
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostLeft(copyLayer* impl_msg);
 */
void CProxy_SWE_DimensionalSplittingCharm::receiveGhostLeft(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckBroadcast(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostLeft_copyLayer(),0);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_receiveGhostLeft_copyLayer() {
  int epidx = CkRegisterEp("receiveGhostLeft(copyLayer* impl_msg)",
      _call_receiveGhostLeft_copyLayer, CMessage_copyLayer::__idx, __idx, 0);
  CkRegisterMessagePupFn(epidx, (CkMessagePupFn)copyLayer::ckDebugPup);
  return epidx;
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_receiveGhostLeft_copyLayer(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  impl_obj->receiveGhostLeft((copyLayer*)impl_msg);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostRight(copyLayer* impl_msg);
 */
void CProxy_SWE_DimensionalSplittingCharm::receiveGhostRight(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckBroadcast(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostRight_copyLayer(),0);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_receiveGhostRight_copyLayer() {
  int epidx = CkRegisterEp("receiveGhostRight(copyLayer* impl_msg)",
      _call_receiveGhostRight_copyLayer, CMessage_copyLayer::__idx, __idx, 0);
  CkRegisterMessagePupFn(epidx, (CkMessagePupFn)copyLayer::ckDebugPup);
  return epidx;
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_receiveGhostRight_copyLayer(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  impl_obj->receiveGhostRight((copyLayer*)impl_msg);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostBottom(copyLayer* impl_msg);
 */
void CProxy_SWE_DimensionalSplittingCharm::receiveGhostBottom(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckBroadcast(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostBottom_copyLayer(),0);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_receiveGhostBottom_copyLayer() {
  int epidx = CkRegisterEp("receiveGhostBottom(copyLayer* impl_msg)",
      _call_receiveGhostBottom_copyLayer, CMessage_copyLayer::__idx, __idx, 0);
  CkRegisterMessagePupFn(epidx, (CkMessagePupFn)copyLayer::ckDebugPup);
  return epidx;
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_receiveGhostBottom_copyLayer(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  impl_obj->receiveGhostBottom((copyLayer*)impl_msg);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostTop(copyLayer* impl_msg);
 */
void CProxy_SWE_DimensionalSplittingCharm::receiveGhostTop(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckBroadcast(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostTop_copyLayer(),0);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_receiveGhostTop_copyLayer() {
  int epidx = CkRegisterEp("receiveGhostTop(copyLayer* impl_msg)",
      _call_receiveGhostTop_copyLayer, CMessage_copyLayer::__idx, __idx, 0);
  CkRegisterMessagePupFn(epidx, (CkMessagePupFn)copyLayer::ckDebugPup);
  return epidx;
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_receiveGhostTop_copyLayer(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  impl_obj->receiveGhostTop((copyLayer*)impl_msg);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void reductionTrigger();
 */
void CProxy_SWE_DimensionalSplittingCharm::reductionTrigger(const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckBroadcast(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_reductionTrigger_void(),0);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_reductionTrigger_void() {
  int epidx = CkRegisterEp("reductionTrigger()",
      _call_reductionTrigger_void, 0, __idx, 0);
  return epidx;
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_reductionTrigger_void(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  impl_obj->reductionTrigger();
  if(UsrToEnv(impl_msg)->isVarSysMsg() == 0)
    CkFreeSysMsg(impl_msg);
}
PUPable_def(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::reductionTrigger_7_closure))
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void printFlops(double flop);
 */
void CProxy_SWE_DimensionalSplittingCharm::printFlops(double flop, const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  //Marshall: double flop
  int impl_off=0;
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|flop;
    impl_off+=implP.size();
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|flop;
  }
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckBroadcast(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_printFlops_marshall8(),0);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_printFlops_marshall8() {
  int epidx = CkRegisterEp("printFlops(double flop)",
      _call_printFlops_marshall8, CkMarshallMsg::__idx, __idx, 0+CK_EP_NOKEEP);
  CkRegisterMarshallUnpackFn(epidx, _callmarshall_printFlops_marshall8);
  CkRegisterMessagePupFn(epidx, _marshallmessagepup_printFlops_marshall8);

  return epidx;
}


// Redn wrapper registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_redn_wrapper_printFlops_marshall8() {
  return CkRegisterEp("redn_wrapper_printFlops(CkReductionMsg *impl_msg)",
      _call_redn_wrapper_printFlops_marshall8, CkMarshallMsg::__idx, __idx, 0);
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_printFlops_marshall8(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  /*Unmarshall pup'd fields: double flop*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<double> flop;
  implP|flop;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->printFlops(std::move(flop.t));
}
int CkIndex_SWE_DimensionalSplittingCharm::_callmarshall_printFlops_marshall8(char* impl_buf, void* impl_obj_void) {
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  /*Unmarshall pup'd fields: double flop*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<double> flop;
  implP|flop;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->printFlops(std::move(flop.t));
  return implP.size();
}
void CkIndex_SWE_DimensionalSplittingCharm::_marshallmessagepup_printFlops_marshall8(PUP::er &implDestP,void *impl_msg) {
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  /*Unmarshall pup'd fields: double flop*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<double> flop;
  implP|flop;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  if (implDestP.hasComments()) implDestP.comment("flop");
  implDestP|flop;
}
PUPable_def(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::printFlops_8_closure))
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void reduceWaveSpeed(float maxWaveSpeed);
 */
void CProxy_SWE_DimensionalSplittingCharm::reduceWaveSpeed(float maxWaveSpeed, const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  //Marshall: float maxWaveSpeed
  int impl_off=0;
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|maxWaveSpeed;
    impl_off+=implP.size();
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|maxWaveSpeed;
  }
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckBroadcast(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_reduceWaveSpeed_marshall9(),0);
}

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_reduceWaveSpeed_marshall9() {
  int epidx = CkRegisterEp("reduceWaveSpeed(float maxWaveSpeed)",
      _call_reduceWaveSpeed_marshall9, CkMarshallMsg::__idx, __idx, 0+CK_EP_NOKEEP);
  CkRegisterMarshallUnpackFn(epidx, _callmarshall_reduceWaveSpeed_marshall9);
  CkRegisterMessagePupFn(epidx, _marshallmessagepup_reduceWaveSpeed_marshall9);

  return epidx;
}


// Redn wrapper registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_redn_wrapper_reduceWaveSpeed_marshall9() {
  return CkRegisterEp("redn_wrapper_reduceWaveSpeed(CkReductionMsg *impl_msg)",
      _call_redn_wrapper_reduceWaveSpeed_marshall9, CkMarshallMsg::__idx, __idx, 0);
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_reduceWaveSpeed_marshall9(void* impl_msg, void* impl_obj_void)
{
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  /*Unmarshall pup'd fields: float maxWaveSpeed*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<float> maxWaveSpeed;
  implP|maxWaveSpeed;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->reduceWaveSpeed(std::move(maxWaveSpeed.t));
}
int CkIndex_SWE_DimensionalSplittingCharm::_callmarshall_reduceWaveSpeed_marshall9(char* impl_buf, void* impl_obj_void) {
  SWE_DimensionalSplittingCharm* impl_obj = static_cast<SWE_DimensionalSplittingCharm*>(impl_obj_void);
  /*Unmarshall pup'd fields: float maxWaveSpeed*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<float> maxWaveSpeed;
  implP|maxWaveSpeed;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->reduceWaveSpeed(std::move(maxWaveSpeed.t));
  return implP.size();
}
void CkIndex_SWE_DimensionalSplittingCharm::_marshallmessagepup_reduceWaveSpeed_marshall9(PUP::er &implDestP,void *impl_msg) {
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  /*Unmarshall pup'd fields: float maxWaveSpeed*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<float> maxWaveSpeed;
  implP|maxWaveSpeed;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  if (implDestP.hasComments()) implDestP.comment("maxWaveSpeed");
  implDestP|maxWaveSpeed;
}
PUPable_def(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::reduceWaveSpeed_9_closure))
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
 */

// Entry point registration function
int CkIndex_SWE_DimensionalSplittingCharm::reg_SWE_DimensionalSplittingCharm_CkMigrateMessage() {
  int epidx = CkRegisterEp("SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg)",
      _call_SWE_DimensionalSplittingCharm_CkMigrateMessage, 0, __idx, 0);
  return epidx;
}

void CkIndex_SWE_DimensionalSplittingCharm::_call_SWE_DimensionalSplittingCharm_CkMigrateMessage(void* impl_msg, void* impl_obj_void)
{
  call_migration_constructor<SWE_DimensionalSplittingCharm> c = impl_obj_void;
  c((CkMigrateMessage*)impl_msg);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
 */
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void compute();
 */
void CProxySection_SWE_DimensionalSplittingCharm::compute(const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_compute_void(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostLeft(copyLayer* impl_msg);
 */
void CProxySection_SWE_DimensionalSplittingCharm::receiveGhostLeft(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostLeft_copyLayer(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostRight(copyLayer* impl_msg);
 */
void CProxySection_SWE_DimensionalSplittingCharm::receiveGhostRight(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostRight_copyLayer(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostBottom(copyLayer* impl_msg);
 */
void CProxySection_SWE_DimensionalSplittingCharm::receiveGhostBottom(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostBottom_copyLayer(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void receiveGhostTop(copyLayer* impl_msg);
 */
void CProxySection_SWE_DimensionalSplittingCharm::receiveGhostTop(copyLayer* impl_msg) 
{
  ckCheck();
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_receiveGhostTop_copyLayer(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void reductionTrigger();
 */
void CProxySection_SWE_DimensionalSplittingCharm::reductionTrigger(const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_reductionTrigger_void(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void printFlops(double flop);
 */
void CProxySection_SWE_DimensionalSplittingCharm::printFlops(double flop, const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  //Marshall: double flop
  int impl_off=0;
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|flop;
    impl_off+=implP.size();
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|flop;
  }
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_printFlops_marshall8(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void reduceWaveSpeed(float maxWaveSpeed);
 */
void CProxySection_SWE_DimensionalSplittingCharm::reduceWaveSpeed(float maxWaveSpeed, const CkEntryOptions *impl_e_opts) 
{
  ckCheck();
  //Marshall: float maxWaveSpeed
  int impl_off=0;
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|maxWaveSpeed;
    impl_off+=implP.size();
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|maxWaveSpeed;
  }
  UsrToEnv(impl_msg)->setMsgtype(ForArrayEltMsg);
  CkArrayMessage *impl_amsg=(CkArrayMessage *)impl_msg;
  impl_amsg->array_setIfNotThere(CkArray_IfNotThere_buffer);
  ckSend(impl_amsg, CkIndex_SWE_DimensionalSplittingCharm::idx_reduceWaveSpeed_marshall9(),0);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
 */
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void CkIndex_SWE_DimensionalSplittingCharm::__register(const char *s, size_t size) {
  __idx = CkRegisterChare(s, size, TypeArray);
  CkRegisterArrayDimensions(__idx, 1);
  CkRegisterBase(__idx, CkIndex_ArrayElement::__idx);
  // REG: SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
  idx_SWE_DimensionalSplittingCharm_marshall1();

  // REG: void compute();
  idx_compute_void();

  // REG: void receiveGhostLeft(copyLayer* impl_msg);
  idx_receiveGhostLeft_copyLayer();

  // REG: void receiveGhostRight(copyLayer* impl_msg);
  idx_receiveGhostRight_copyLayer();

  // REG: void receiveGhostBottom(copyLayer* impl_msg);
  idx_receiveGhostBottom_copyLayer();

  // REG: void receiveGhostTop(copyLayer* impl_msg);
  idx_receiveGhostTop_copyLayer();

  // REG: void reductionTrigger();
  idx_reductionTrigger_void();

  // REG: void printFlops(double flop);
  idx_printFlops_marshall8();
  idx_redn_wrapper_printFlops_marshall8();

  // REG: void reduceWaveSpeed(float maxWaveSpeed);
  idx_reduceWaveSpeed_marshall9();
  idx_redn_wrapper_reduceWaveSpeed_marshall9();

  // REG: SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
  idx_SWE_DimensionalSplittingCharm_CkMigrateMessage();
  CkRegisterMigCtor(__idx, idx_SWE_DimensionalSplittingCharm_CkMigrateMessage());

  SWE_DimensionalSplittingCharm::__sdag_register(); // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::compute(){
  CkPrintf("Error> Direct call to SDAG entry method \'%s::%s\'!\n", "SWE_DimensionalSplittingCharm", "compute()"); 
  CkAbort("Direct SDAG call is not allowed for SDAG entry methods having when constructs. Call such SDAG methods using a proxy"); 
}

#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_sdag_fnc_compute() {
  _TRACE_END_EXECUTE(); 
  if (!__dep.get()) _sdag_init();
  _slist_0();
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, _sdagEP, CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::compute_end() {
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_0() {
  _serial_0();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_0_end() {
  compute_end();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_serial_0() {
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, (_sdag_idx_SWE_DimensionalSplittingCharm_serial_0()), CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
  { // begin serial block
#line 18 "SWE_DimensionalSplittingCharm.ci"

    clock_gettime(CLOCK_MONOTONIC, &startTime);
   
#line 1444 "SWE_DimensionalSplittingCharm.def.h"
  } // end serial block
  _TRACE_END_EXECUTE(); 
  _while_0();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_while_0() {
  if (currentCheckpoint < checkpointCount) {
    _slist_1();
  } else {
    _slist_0_end();
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_while_0_end() {
  if (currentCheckpoint < checkpointCount) {
    _slist_1();
  } else {
    _slist_0_end();
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_1() {
  _serial_1();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_1_end() {
  _while_0_end();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_serial_1() {
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, (_sdag_idx_SWE_DimensionalSplittingCharm_serial_1()), CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
  { // begin serial block
#line 22 "SWE_DimensionalSplittingCharm.ci"

     sendCopyLayers(true);
     setGhostLayer();
    
#line 1497 "SWE_DimensionalSplittingCharm.def.h"
  } // end serial block
  _TRACE_END_EXECUTE(); 
  _overlap_0();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_overlap_0() {
  _olist_0();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_overlap_0_end() {
  _serial_6();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_olist_0() {
  SDAG::CCounter *_co0= new SDAG::CCounter(4);
  _when_0(_co0);
  _when_1(_co0);
  _when_2(_co0);
  _when_3(_co0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_olist_0_end(SDAG::CCounter* _co0) {
  _co0->decrement();
  if (_co0->isDone()) {
  _co0->deref();
    _overlap_0_end();
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
SDAG::Continuation* SWE_DimensionalSplittingCharm::_when_0(SDAG::CCounter* _co0) {
  SDAG::Buffer* buf0 = __dep->tryFindMessage(0, false, 0, 0);
  if (buf0) {
    __dep->removeMessage(buf0);
    _if_0(_co0, static_cast<copyLayer*>(static_cast<SDAG::MsgClosure*>(buf0->cl)->msg));
    delete buf0;
    return 0;
  } else {
    SDAG::Continuation* c = new SDAG::Continuation(0);
    c->addClosure(_co0);
    c->anyEntries.push_back(0);
    __dep->reg(c);
    return c;
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_when_0_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  {
    {
      copyLayer*& msg = gen1;
      CmiFree(UsrToEnv(msg));
    }
  }
  _olist_0_end(_co0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_if_0(SDAG::CCounter* _co0, copyLayer* gen1) {
  {
    {
      copyLayer*& msg = gen1;
      if (!msg->isDummy) {
        _slist_2(_co0, gen1);
      } else {
        _if_0_end(_co0, gen1);
      }
    }
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_if_0_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  _when_0_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_2(SDAG::CCounter* _co0, copyLayer* gen1) {
  _serial_2(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_2_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  _if_0_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_serial_2(SDAG::CCounter* _co0, copyLayer* gen1) {
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, (_sdag_idx_SWE_DimensionalSplittingCharm_serial_2()), CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
  {
    {
      copyLayer*& msg = gen1;
      { // begin serial block
#line 29 "SWE_DimensionalSplittingCharm.ci"
 processCopyLayer(msg); 
#line 1619 "SWE_DimensionalSplittingCharm.def.h"
      } // end serial block
    }
  }
  _TRACE_END_EXECUTE(); 
  _slist_2_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
SDAG::Continuation* SWE_DimensionalSplittingCharm::_when_1(SDAG::CCounter* _co0) {
  SDAG::Buffer* buf0 = __dep->tryFindMessage(1, false, 0, 0);
  if (buf0) {
    __dep->removeMessage(buf0);
    _if_1(_co0, static_cast<copyLayer*>(static_cast<SDAG::MsgClosure*>(buf0->cl)->msg));
    delete buf0;
    return 0;
  } else {
    SDAG::Continuation* c = new SDAG::Continuation(1);
    c->addClosure(_co0);
    c->anyEntries.push_back(1);
    __dep->reg(c);
    return c;
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_when_1_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  {
    {
      copyLayer*& msg = gen1;
      CmiFree(UsrToEnv(msg));
    }
  }
  _olist_0_end(_co0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_if_1(SDAG::CCounter* _co0, copyLayer* gen1) {
  {
    {
      copyLayer*& msg = gen1;
      if (!msg->isDummy) {
        _slist_3(_co0, gen1);
      } else {
        _if_1_end(_co0, gen1);
      }
    }
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_if_1_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  _when_1_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_3(SDAG::CCounter* _co0, copyLayer* gen1) {
  _serial_3(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_3_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  _if_1_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_serial_3(SDAG::CCounter* _co0, copyLayer* gen1) {
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, (_sdag_idx_SWE_DimensionalSplittingCharm_serial_3()), CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
  {
    {
      copyLayer*& msg = gen1;
      { // begin serial block
#line 33 "SWE_DimensionalSplittingCharm.ci"
 processCopyLayer(msg); 
#line 1707 "SWE_DimensionalSplittingCharm.def.h"
      } // end serial block
    }
  }
  _TRACE_END_EXECUTE(); 
  _slist_3_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
SDAG::Continuation* SWE_DimensionalSplittingCharm::_when_2(SDAG::CCounter* _co0) {
  SDAG::Buffer* buf0 = __dep->tryFindMessage(2, false, 0, 0);
  if (buf0) {
    __dep->removeMessage(buf0);
    _if_2(_co0, static_cast<copyLayer*>(static_cast<SDAG::MsgClosure*>(buf0->cl)->msg));
    delete buf0;
    return 0;
  } else {
    SDAG::Continuation* c = new SDAG::Continuation(2);
    c->addClosure(_co0);
    c->anyEntries.push_back(2);
    __dep->reg(c);
    return c;
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_when_2_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  {
    {
      copyLayer*& msg = gen1;
      CmiFree(UsrToEnv(msg));
    }
  }
  _olist_0_end(_co0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_if_2(SDAG::CCounter* _co0, copyLayer* gen1) {
  {
    {
      copyLayer*& msg = gen1;
      if (!msg->isDummy) {
        _slist_4(_co0, gen1);
      } else {
        _if_2_end(_co0, gen1);
      }
    }
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_if_2_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  _when_2_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_4(SDAG::CCounter* _co0, copyLayer* gen1) {
  _serial_4(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_4_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  _if_2_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_serial_4(SDAG::CCounter* _co0, copyLayer* gen1) {
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, (_sdag_idx_SWE_DimensionalSplittingCharm_serial_4()), CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
  {
    {
      copyLayer*& msg = gen1;
      { // begin serial block
#line 37 "SWE_DimensionalSplittingCharm.ci"
 processCopyLayer(msg); 
#line 1795 "SWE_DimensionalSplittingCharm.def.h"
      } // end serial block
    }
  }
  _TRACE_END_EXECUTE(); 
  _slist_4_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
SDAG::Continuation* SWE_DimensionalSplittingCharm::_when_3(SDAG::CCounter* _co0) {
  SDAG::Buffer* buf0 = __dep->tryFindMessage(3, false, 0, 0);
  if (buf0) {
    __dep->removeMessage(buf0);
    _if_3(_co0, static_cast<copyLayer*>(static_cast<SDAG::MsgClosure*>(buf0->cl)->msg));
    delete buf0;
    return 0;
  } else {
    SDAG::Continuation* c = new SDAG::Continuation(3);
    c->addClosure(_co0);
    c->anyEntries.push_back(3);
    __dep->reg(c);
    return c;
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_when_3_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  {
    {
      copyLayer*& msg = gen1;
      CmiFree(UsrToEnv(msg));
    }
  }
  _olist_0_end(_co0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_if_3(SDAG::CCounter* _co0, copyLayer* gen1) {
  {
    {
      copyLayer*& msg = gen1;
      if (!msg->isDummy) {
        _slist_5(_co0, gen1);
      } else {
        _if_3_end(_co0, gen1);
      }
    }
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_if_3_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  _when_3_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_5(SDAG::CCounter* _co0, copyLayer* gen1) {
  _serial_5(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_slist_5_end(SDAG::CCounter* _co0, copyLayer* gen1) {
  _if_3_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_serial_5(SDAG::CCounter* _co0, copyLayer* gen1) {
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, (_sdag_idx_SWE_DimensionalSplittingCharm_serial_5()), CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
  {
    {
      copyLayer*& msg = gen1;
      { // begin serial block
#line 41 "SWE_DimensionalSplittingCharm.ci"
 processCopyLayer(msg); 
#line 1883 "SWE_DimensionalSplittingCharm.def.h"
      } // end serial block
    }
  }
  _TRACE_END_EXECUTE(); 
  _slist_5_end(_co0, gen1);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_serial_6() {
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, (_sdag_idx_SWE_DimensionalSplittingCharm_serial_6()), CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
  { // begin serial block
#line 44 "SWE_DimensionalSplittingCharm.ci"

     xSweep();
    
#line 1901 "SWE_DimensionalSplittingCharm.def.h"
  } // end serial block
  _TRACE_END_EXECUTE(); 
  _when_4();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
SDAG::Continuation* SWE_DimensionalSplittingCharm::_when_4() {
  SDAG::Buffer* buf0 = __dep->tryFindMessage(4, false, 0, 0);
  if (buf0) {
    __dep->removeMessage(buf0);
    _serial_7();
    delete buf0;
    return 0;
  } else {
    SDAG::Continuation* c = new SDAG::Continuation(4);
    c->anyEntries.push_back(4);
    __dep->reg(c);
    return c;
  }
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_when_4_end() {
  _slist_1_end();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_serial_7() {
  _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, (_sdag_idx_SWE_DimensionalSplittingCharm_serial_7()), CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
  { // begin serial block
#line 48 "SWE_DimensionalSplittingCharm.ci"

     ySweep();
     updateUnknowns(maxTimestep);
     currentSimulationTime += maxTimestep;
     if (currentSimulationTime >= checkpointInstantOfTime[currentCheckpoint]) {
      clock_gettime(CLOCK_MONOTONIC, &endTime);
      wallTime += (endTime.tv_sec - startTime.tv_sec);
      wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
      if(thisIndex == 0) {
       CkPrintf("Write timestep (%fs)\n", currentSimulationTime);
      }
      writeTimestep();
      currentCheckpoint++;
      clock_gettime(CLOCK_MONOTONIC, &startTime);
     }
     if (currentSimulationTime < simulationDuration) {
      clock_gettime(CLOCK_MONOTONIC, &commTime);
      sendCopyLayers(false);
      setGhostLayer();
      clock_gettime(CLOCK_MONOTONIC, &endTime);
                            communicationTime += (endTime.tv_sec - startTime.tv_sec);
                            communicationTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
     } else {
      clock_gettime(CLOCK_MONOTONIC, &endTime);
      wallTime += (endTime.tv_sec - startTime.tv_sec);
      wallTime += (float) (endTime.tv_nsec - startTime.tv_nsec) / 1E9;
      CkPrintf("Rank %i : Compute Time (CPU): %fs - (WALL): %fs | Total Time (Wall): %fs\n", thisIndex, computeTime, computeTimeWall, wallTime);
      mainProxy.done(thisIndex,flopCounter,communicationTime,wallTime);
     }
    
#line 1969 "SWE_DimensionalSplittingCharm.def.h"
  } // end serial block
  _TRACE_END_EXECUTE(); 
  _when_4_end();
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::receiveGhostLeft(copyLayer* msg_msg){
  if (!__dep.get()) _sdag_init();
  CmiReference(UsrToEnv(msg_msg));
  __dep->pushBuffer(0, new SDAG::MsgClosure(msg_msg));
  SDAG::Continuation* c = __dep->tryFindContinuation(0);
  if (c) {
    _TRACE_END_EXECUTE(); 
    _when_0(
      static_cast<SDAG::CCounter*>(c->closure[0])
    );
    _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, _sdagEP, CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
    delete c;
  }
}

#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::receiveGhostRight(copyLayer* msg_msg){
  if (!__dep.get()) _sdag_init();
  CmiReference(UsrToEnv(msg_msg));
  __dep->pushBuffer(1, new SDAG::MsgClosure(msg_msg));
  SDAG::Continuation* c = __dep->tryFindContinuation(1);
  if (c) {
    _TRACE_END_EXECUTE(); 
    _when_1(
      static_cast<SDAG::CCounter*>(c->closure[0])
    );
    _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, _sdagEP, CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
    delete c;
  }
}

#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::receiveGhostBottom(copyLayer* msg_msg){
  if (!__dep.get()) _sdag_init();
  CmiReference(UsrToEnv(msg_msg));
  __dep->pushBuffer(2, new SDAG::MsgClosure(msg_msg));
  SDAG::Continuation* c = __dep->tryFindContinuation(2);
  if (c) {
    _TRACE_END_EXECUTE(); 
    _when_2(
      static_cast<SDAG::CCounter*>(c->closure[0])
    );
    _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, _sdagEP, CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
    delete c;
  }
}

#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::receiveGhostTop(copyLayer* msg_msg){
  if (!__dep.get()) _sdag_init();
  CmiReference(UsrToEnv(msg_msg));
  __dep->pushBuffer(3, new SDAG::MsgClosure(msg_msg));
  SDAG::Continuation* c = __dep->tryFindContinuation(3);
  if (c) {
    _TRACE_END_EXECUTE(); 
    _when_3(
      static_cast<SDAG::CCounter*>(c->closure[0])
    );
    _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, _sdagEP, CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
    delete c;
  }
}

#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::reductionTrigger(){
  Closure_SWE_DimensionalSplittingCharm::reductionTrigger_7_closure* genClosure = new Closure_SWE_DimensionalSplittingCharm::reductionTrigger_7_closure();
  reductionTrigger(genClosure);
  genClosure->deref();
}

#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::reductionTrigger(Closure_SWE_DimensionalSplittingCharm::reductionTrigger_7_closure* genClosure){
  if (!__dep.get()) _sdag_init();
  __dep->pushBuffer(4, genClosure);
  SDAG::Continuation* c = __dep->tryFindContinuation(4);
  if (c) {
    _TRACE_END_EXECUTE(); 
    _when_4(
    );
    _TRACE_BEGIN_EXECUTE_DETAILED(-1, -1, _sdagEP, CkMyPe(), 0, ckGetArrayIndex().getProjectionID(), this); 
    delete c;
  }
}

#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_sdag_init() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  __dep.reset(new SDAG::Dependency(5,5));
  __dep->addDepends(0,0);
  __dep->addDepends(1,1);
  __dep->addDepends(2,2);
  __dep->addDepends(3,3);
  __dep->addDepends(4,4);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::__sdag_init() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::_sdag_pup(PUP::er &p) {  // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  p|__dep;
}
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void SWE_DimensionalSplittingCharm::__sdag_register() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  (void)_sdag_idx_SWE_DimensionalSplittingCharm_serial_0();
  (void)_sdag_idx_SWE_DimensionalSplittingCharm_serial_1();
  (void)_sdag_idx_SWE_DimensionalSplittingCharm_serial_2();
  (void)_sdag_idx_SWE_DimensionalSplittingCharm_serial_3();
  (void)_sdag_idx_SWE_DimensionalSplittingCharm_serial_4();
  (void)_sdag_idx_SWE_DimensionalSplittingCharm_serial_5();
  (void)_sdag_idx_SWE_DimensionalSplittingCharm_serial_6();
  (void)_sdag_idx_SWE_DimensionalSplittingCharm_serial_7();
  PUPable_reg(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::compute_2_closure));
  PUPable_reg(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::reductionTrigger_7_closure));
  PUPable_reg(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::printFlops_8_closure));
  PUPable_reg(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::reduceWaveSpeed_9_closure));
  PUPable_reg(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::compute_2_closure));
  PUPable_reg(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::reductionTrigger_7_closure));
  PUPable_reg(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::printFlops_8_closure));
  PUPable_reg(SINGLE_ARG(Closure_SWE_DimensionalSplittingCharm::reduceWaveSpeed_9_closure));
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_idx_SWE_DimensionalSplittingCharm_serial_0() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  static int epidx = _sdag_reg_SWE_DimensionalSplittingCharm_serial_0();
  return epidx;
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_reg_SWE_DimensionalSplittingCharm_serial_0() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  return CkRegisterEp("SWE_DimensionalSplittingCharm_serial_0", NULL, 0, CkIndex_SWE_DimensionalSplittingCharm::__idx, 0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_idx_SWE_DimensionalSplittingCharm_serial_1() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  static int epidx = _sdag_reg_SWE_DimensionalSplittingCharm_serial_1();
  return epidx;
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_reg_SWE_DimensionalSplittingCharm_serial_1() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  return CkRegisterEp("SWE_DimensionalSplittingCharm_serial_1", NULL, 0, CkIndex_SWE_DimensionalSplittingCharm::__idx, 0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_idx_SWE_DimensionalSplittingCharm_serial_2() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  static int epidx = _sdag_reg_SWE_DimensionalSplittingCharm_serial_2();
  return epidx;
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_reg_SWE_DimensionalSplittingCharm_serial_2() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  return CkRegisterEp("SWE_DimensionalSplittingCharm_serial_2", NULL, 0, CkIndex_SWE_DimensionalSplittingCharm::__idx, 0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_idx_SWE_DimensionalSplittingCharm_serial_3() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  static int epidx = _sdag_reg_SWE_DimensionalSplittingCharm_serial_3();
  return epidx;
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_reg_SWE_DimensionalSplittingCharm_serial_3() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  return CkRegisterEp("SWE_DimensionalSplittingCharm_serial_3", NULL, 0, CkIndex_SWE_DimensionalSplittingCharm::__idx, 0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_idx_SWE_DimensionalSplittingCharm_serial_4() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  static int epidx = _sdag_reg_SWE_DimensionalSplittingCharm_serial_4();
  return epidx;
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_reg_SWE_DimensionalSplittingCharm_serial_4() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  return CkRegisterEp("SWE_DimensionalSplittingCharm_serial_4", NULL, 0, CkIndex_SWE_DimensionalSplittingCharm::__idx, 0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_idx_SWE_DimensionalSplittingCharm_serial_5() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  static int epidx = _sdag_reg_SWE_DimensionalSplittingCharm_serial_5();
  return epidx;
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_reg_SWE_DimensionalSplittingCharm_serial_5() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  return CkRegisterEp("SWE_DimensionalSplittingCharm_serial_5", NULL, 0, CkIndex_SWE_DimensionalSplittingCharm::__idx, 0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_idx_SWE_DimensionalSplittingCharm_serial_6() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  static int epidx = _sdag_reg_SWE_DimensionalSplittingCharm_serial_6();
  return epidx;
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_reg_SWE_DimensionalSplittingCharm_serial_6() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  return CkRegisterEp("SWE_DimensionalSplittingCharm_serial_6", NULL, 0, CkIndex_SWE_DimensionalSplittingCharm::__idx, 0);
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_idx_SWE_DimensionalSplittingCharm_serial_7() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  static int epidx = _sdag_reg_SWE_DimensionalSplittingCharm_serial_7();
  return epidx;
}
#endif /* CK_TEMPLATES_ONLY */


#ifndef CK_TEMPLATES_ONLY
int SWE_DimensionalSplittingCharm::_sdag_reg_SWE_DimensionalSplittingCharm_serial_7() { // Potentially missing SWE_DimensionalSplittingCharm_SDAG_CODE in your class definition?
  return CkRegisterEp("SWE_DimensionalSplittingCharm_serial_7", NULL, 0, CkIndex_SWE_DimensionalSplittingCharm::__idx, 0);
}
#endif /* CK_TEMPLATES_ONLY */



#ifndef CK_TEMPLATES_ONLY
void _registerSWE_DimensionalSplittingCharm(void)
{
  static int _done = 0; if(_done) return; _done = 1;




/* REG: message copyLayer{
Boundary boundary;
bool containsBathymetry;
float b[];
float h[];
float hu[];
float hv[];
}
;
*/
CMessage_copyLayer::__register("copyLayer", sizeof(copyLayer),(CkPackFnPtr) copyLayer::pack,(CkUnpackFnPtr) copyLayer::unpack);

/* REG: array SWE_DimensionalSplittingCharm: ArrayElement{
SWE_DimensionalSplittingCharm(int nx, int ny, float dy, float dx, float originX, float originY, int posX, int posY, const BoundaryType *boundaries, const std::string &outputFilename, const std::string &bathymetryFile, const std::string &displacementFile);
void compute();
void receiveGhostLeft(copyLayer* impl_msg);
void receiveGhostRight(copyLayer* impl_msg);
void receiveGhostBottom(copyLayer* impl_msg);
void receiveGhostTop(copyLayer* impl_msg);
void reductionTrigger();
void printFlops(double flop);
void reduceWaveSpeed(float maxWaveSpeed);
SWE_DimensionalSplittingCharm(CkMigrateMessage* impl_msg);
};
*/
  CkIndex_SWE_DimensionalSplittingCharm::__register("SWE_DimensionalSplittingCharm", sizeof(SWE_DimensionalSplittingCharm));

}
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
template <>
void CBase_SWE_DimensionalSplittingCharm::virtual_pup(PUP::er &p) {
    recursive_pup<SWE_DimensionalSplittingCharm>(dynamic_cast<SWE_DimensionalSplittingCharm*>(this), p);
}
#endif /* CK_TEMPLATES_ONLY */
