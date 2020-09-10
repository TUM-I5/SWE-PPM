






/* ---------------- method closures -------------- */
#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_swe_charm::done_2_closure : public SDAG::Closure {
            int index;
            double ctr_flop;
            double ctr_exchange;
            double ctr_barrier;
            double ctr_reduce;
            double ctr_wall;


      done_2_closure() {
        init();
      }
      done_2_closure(CkMigrateMessage*) {
        init();
      }
            int & getP0() { return index;}
            double & getP1() { return ctr_flop;}
            double & getP2() { return ctr_exchange;}
            double & getP3() { return ctr_barrier;}
            double & getP4() { return ctr_reduce;}
            double & getP5() { return ctr_wall;}
      void pup(PUP::er& __p) {
        __p | index;
        __p | ctr_flop;
        __p | ctr_exchange;
        __p | ctr_barrier;
        __p | ctr_reduce;
        __p | ctr_wall;
        packClosure(__p);
      }
      virtual ~done_2_closure() {
      }
      PUPable_decl(SINGLE_ARG(done_2_closure));
    };
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_swe_charm::allEnd_3_closure : public SDAG::Closure {
      

      allEnd_3_closure() {
        init();
      }
      allEnd_3_closure(CkMigrateMessage*) {
        init();
      }
            void pup(PUP::er& __p) {
        packClosure(__p);
      }
      virtual ~allEnd_3_closure() {
      }
      PUPable_decl(SINGLE_ARG(allEnd_3_closure));
    };
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_swe_charm::exit_4_closure : public SDAG::Closure {
      

      exit_4_closure() {
        init();
      }
      exit_4_closure(CkMigrateMessage*) {
        init();
      }
            void pup(PUP::er& __p) {
        packClosure(__p);
      }
      virtual ~exit_4_closure() {
      }
      PUPable_decl(SINGLE_ARG(exit_4_closure));
    };
#endif /* CK_TEMPLATES_ONLY */


/* DEFS: readonly CProxy_swe_charm mainProxy;
 */
extern CProxy_swe_charm mainProxy;
#ifndef CK_TEMPLATES_ONLY
extern "C" void __xlater_roPup_mainProxy(void *_impl_pup_er) {
  PUP::er &_impl_p=*(PUP::er *)_impl_pup_er;
  _impl_p|mainProxy;
}
#endif /* CK_TEMPLATES_ONLY */

/* DEFS: readonly int blockCountX;
 */
extern int blockCountX;
#ifndef CK_TEMPLATES_ONLY
extern "C" void __xlater_roPup_blockCountX(void *_impl_pup_er) {
  PUP::er &_impl_p=*(PUP::er *)_impl_pup_er;
  _impl_p|blockCountX;
}
#endif /* CK_TEMPLATES_ONLY */

/* DEFS: readonly int blockCountY;
 */
extern int blockCountY;
#ifndef CK_TEMPLATES_ONLY
extern "C" void __xlater_roPup_blockCountY(void *_impl_pup_er) {
  PUP::er &_impl_p=*(PUP::er *)_impl_pup_er;
  _impl_p|blockCountY;
}
#endif /* CK_TEMPLATES_ONLY */

/* DEFS: readonly float simulationDuration;
 */
extern float simulationDuration;
#ifndef CK_TEMPLATES_ONLY
extern "C" void __xlater_roPup_simulationDuration(void *_impl_pup_er) {
  PUP::er &_impl_p=*(PUP::er *)_impl_pup_er;
  _impl_p|simulationDuration;
}
#endif /* CK_TEMPLATES_ONLY */

/* DEFS: readonly int checkpointCount;
 */
extern int checkpointCount;
#ifndef CK_TEMPLATES_ONLY
extern "C" void __xlater_roPup_checkpointCount(void *_impl_pup_er) {
  PUP::er &_impl_p=*(PUP::er *)_impl_pup_er;
  _impl_p|checkpointCount;
}
#endif /* CK_TEMPLATES_ONLY */


/* DEFS: message collectorMsg{
double deserialized[];
}
;
 */
#ifndef CK_TEMPLATES_ONLY
void *CMessage_collectorMsg::operator new(size_t s){
  return collectorMsg::alloc(__idx, s, 0, 0, GroupDepNum{});
}
void *CMessage_collectorMsg::operator new(size_t s, int* sz){
  return collectorMsg::alloc(__idx, s, sz, 0, GroupDepNum{});
}
void *CMessage_collectorMsg::operator new(size_t s, int* sz,const int pb){
  return collectorMsg::alloc(__idx, s, sz, pb, GroupDepNum{});
}
void *CMessage_collectorMsg::operator new(size_t s, int* sz,const int pb, const GroupDepNum groupDepNum){
  return collectorMsg::alloc(__idx, s, sz, pb, groupDepNum);
}
void *CMessage_collectorMsg::operator new(size_t s, int sz0) {
  int sizes[1];
  sizes[0] = sz0;
  return collectorMsg::alloc(__idx, s, sizes, 0, GroupDepNum{});
}
void *CMessage_collectorMsg::operator new(size_t s, int sz0, const int p) {
  int sizes[1];
  sizes[0] = sz0;
  return collectorMsg::alloc(__idx, s, sizes, p, GroupDepNum{});
}
void *CMessage_collectorMsg::operator new(size_t s, int sz0, const int p, const GroupDepNum groupDepNum) {
  int sizes[1];
  sizes[0] = sz0;
  return collectorMsg::alloc(__idx, s, sizes, p, groupDepNum);
}
void* CMessage_collectorMsg::alloc(int msgnum, size_t sz, int *sizes, int pb, GroupDepNum groupDepNum) {
  CkpvAccess(_offsets)[0] = ALIGN_DEFAULT(sz);
  if(sizes==0)
    CkpvAccess(_offsets)[1] = CkpvAccess(_offsets)[0];
  else
    CkpvAccess(_offsets)[1] = CkpvAccess(_offsets)[0] + ALIGN_DEFAULT(sizeof(double)*sizes[0]);
  return CkAllocMsg(msgnum, CkpvAccess(_offsets)[1], pb, groupDepNum);
}
CMessage_collectorMsg::CMessage_collectorMsg() {
collectorMsg *newmsg = (collectorMsg *)this;
  newmsg->deserialized = (double *) ((char *)newmsg + CkpvAccess(_offsets)[0]);
}
void CMessage_collectorMsg::dealloc(void *p) {
  CkFreeMsg(p);
}
void* CMessage_collectorMsg::pack(collectorMsg *msg) {
  msg->deserialized = (double *) ((char *)msg->deserialized - (char *)msg);
  return (void *) msg;
}
collectorMsg* CMessage_collectorMsg::unpack(void* buf) {
  collectorMsg *msg = (collectorMsg *) buf;
  msg->deserialized = (double *) ((size_t)msg->deserialized + (char *)msg);
  return msg;
}
int CMessage_collectorMsg::__idx=0;
#endif /* CK_TEMPLATES_ONLY */

/* DEFS: mainchare swe_charm: Chare{
swe_charm(CkArgMsg* impl_msg);
void done(int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall);
void allEnd();
void exit();
};
 */
#ifndef CK_TEMPLATES_ONLY
 int CkIndex_swe_charm::__idx=0;
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
/* DEFS: swe_charm(CkArgMsg* impl_msg);
 */
CkChareID CProxy_swe_charm::ckNew(CkArgMsg* impl_msg, int impl_onPE)
{
  CkChareID impl_ret;
  CkCreateChare(CkIndex_swe_charm::__idx, CkIndex_swe_charm::idx_swe_charm_CkArgMsg(), impl_msg, &impl_ret, impl_onPE);
  return impl_ret;
}
void CProxy_swe_charm::ckNew(CkArgMsg* impl_msg, CkChareID* pcid, int impl_onPE)
{
  CkCreateChare(CkIndex_swe_charm::__idx, CkIndex_swe_charm::idx_swe_charm_CkArgMsg(), impl_msg, pcid, impl_onPE);
}
  CProxy_swe_charm::CProxy_swe_charm(CkArgMsg* impl_msg, int impl_onPE)
{
  CkChareID impl_ret;
  CkCreateChare(CkIndex_swe_charm::__idx, CkIndex_swe_charm::idx_swe_charm_CkArgMsg(), impl_msg, &impl_ret, impl_onPE);
  ckSetChareID(impl_ret);
}

// Entry point registration function
int CkIndex_swe_charm::reg_swe_charm_CkArgMsg() {
  int epidx = CkRegisterEp("swe_charm(CkArgMsg* impl_msg)",
      reinterpret_cast<CkCallFnPtr>(_call_swe_charm_CkArgMsg), CMessage_CkArgMsg::__idx, __idx, 0);
  CkRegisterMessagePupFn(epidx, (CkMessagePupFn)CkArgMsg::ckDebugPup);
  return epidx;
}

void CkIndex_swe_charm::_call_swe_charm_CkArgMsg(void* impl_msg, void* impl_obj_void)
{
  swe_charm* impl_obj = static_cast<swe_charm*>(impl_obj_void);
  new (impl_obj_void) swe_charm((CkArgMsg*)impl_msg);
}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void done(int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall);
 */
void CProxy_swe_charm::done(int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall, const CkEntryOptions *impl_e_opts)
{
  ckCheck();
  //Marshall: int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall
  int impl_off=0;
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|index;
    implP|ctr_flop;
    implP|ctr_exchange;
    implP|ctr_barrier;
    implP|ctr_reduce;
    implP|ctr_wall;
    impl_off+=implP.size();
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|index;
    implP|ctr_flop;
    implP|ctr_exchange;
    implP|ctr_barrier;
    implP|ctr_reduce;
    implP|ctr_wall;
  }
  if (ckIsDelegated()) {
    int destPE=CkChareMsgPrep(CkIndex_swe_charm::idx_done_marshall2(), impl_msg, &ckGetChareID());
    if (destPE!=-1) ckDelegatedTo()->ChareSend(ckDelegatedPtr(),CkIndex_swe_charm::idx_done_marshall2(), impl_msg, &ckGetChareID(),destPE);
  } else {
    CkSendMsg(CkIndex_swe_charm::idx_done_marshall2(), impl_msg, &ckGetChareID(),0);
  }
}

// Entry point registration function
int CkIndex_swe_charm::reg_done_marshall2() {
  int epidx = CkRegisterEp("done(int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall)",
      reinterpret_cast<CkCallFnPtr>(_call_done_marshall2), CkMarshallMsg::__idx, __idx, 0+CK_EP_NOKEEP);
  CkRegisterMarshallUnpackFn(epidx, _callmarshall_done_marshall2);
  CkRegisterMessagePupFn(epidx, _marshallmessagepup_done_marshall2);

  return epidx;
}

void CkIndex_swe_charm::_call_done_marshall2(void* impl_msg, void* impl_obj_void)
{
  swe_charm* impl_obj = static_cast<swe_charm*>(impl_obj_void);
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  envelope *env = UsrToEnv(impl_msg_typed);
  /*Unmarshall pup'd fields: int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> index;
  implP|index;
  PUP::detail::TemporaryObjectHolder<double> ctr_flop;
  implP|ctr_flop;
  PUP::detail::TemporaryObjectHolder<double> ctr_exchange;
  implP|ctr_exchange;
  PUP::detail::TemporaryObjectHolder<double> ctr_barrier;
  implP|ctr_barrier;
  PUP::detail::TemporaryObjectHolder<double> ctr_reduce;
  implP|ctr_reduce;
  PUP::detail::TemporaryObjectHolder<double> ctr_wall;
  implP|ctr_wall;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->done(std::move(index.t), std::move(ctr_flop.t), std::move(ctr_exchange.t), std::move(ctr_barrier.t), std::move(ctr_reduce.t), std::move(ctr_wall.t));
}
int CkIndex_swe_charm::_callmarshall_done_marshall2(char* impl_buf, void* impl_obj_void) {
  swe_charm* impl_obj = static_cast<swe_charm*>(impl_obj_void);
  envelope *env = UsrToEnv(impl_buf);
  /*Unmarshall pup'd fields: int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> index;
  implP|index;
  PUP::detail::TemporaryObjectHolder<double> ctr_flop;
  implP|ctr_flop;
  PUP::detail::TemporaryObjectHolder<double> ctr_exchange;
  implP|ctr_exchange;
  PUP::detail::TemporaryObjectHolder<double> ctr_barrier;
  implP|ctr_barrier;
  PUP::detail::TemporaryObjectHolder<double> ctr_reduce;
  implP|ctr_reduce;
  PUP::detail::TemporaryObjectHolder<double> ctr_wall;
  implP|ctr_wall;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->done(std::move(index.t), std::move(ctr_flop.t), std::move(ctr_exchange.t), std::move(ctr_barrier.t), std::move(ctr_reduce.t), std::move(ctr_wall.t));
  return implP.size();
}
void CkIndex_swe_charm::_marshallmessagepup_done_marshall2(PUP::er &implDestP,void *impl_msg) {
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  envelope *env = UsrToEnv(impl_msg_typed);
  /*Unmarshall pup'd fields: int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> index;
  implP|index;
  PUP::detail::TemporaryObjectHolder<double> ctr_flop;
  implP|ctr_flop;
  PUP::detail::TemporaryObjectHolder<double> ctr_exchange;
  implP|ctr_exchange;
  PUP::detail::TemporaryObjectHolder<double> ctr_barrier;
  implP|ctr_barrier;
  PUP::detail::TemporaryObjectHolder<double> ctr_reduce;
  implP|ctr_reduce;
  PUP::detail::TemporaryObjectHolder<double> ctr_wall;
  implP|ctr_wall;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  if (implDestP.hasComments()) implDestP.comment("index");
  implDestP|index;
  if (implDestP.hasComments()) implDestP.comment("ctr_flop");
  implDestP|ctr_flop;
  if (implDestP.hasComments()) implDestP.comment("ctr_exchange");
  implDestP|ctr_exchange;
  if (implDestP.hasComments()) implDestP.comment("ctr_barrier");
  implDestP|ctr_barrier;
  if (implDestP.hasComments()) implDestP.comment("ctr_reduce");
  implDestP|ctr_reduce;
  if (implDestP.hasComments()) implDestP.comment("ctr_wall");
  implDestP|ctr_wall;
}
PUPable_def(SINGLE_ARG(Closure_swe_charm::done_2_closure))
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void allEnd();
 */
void CProxy_swe_charm::allEnd(const CkEntryOptions *impl_e_opts)
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  if (ckIsDelegated()) {
    int destPE=CkChareMsgPrep(CkIndex_swe_charm::idx_allEnd_void(), impl_msg, &ckGetChareID());
    if (destPE!=-1) ckDelegatedTo()->ChareSend(ckDelegatedPtr(),CkIndex_swe_charm::idx_allEnd_void(), impl_msg, &ckGetChareID(),destPE);
  } else {
    CkSendMsg(CkIndex_swe_charm::idx_allEnd_void(), impl_msg, &ckGetChareID(),0);
  }
}
void CkIndex_swe_charm::_call_redn_wrapper_allEnd_void(void* impl_msg, void* impl_obj_void)
{
  swe_charm* impl_obj = static_cast<swe_charm*> (impl_obj_void);
  char* impl_buf = (char*)((CkReductionMsg*)impl_msg)->getData();
  impl_obj->allEnd();
  delete (CkReductionMsg*)impl_msg;
}


// Entry point registration function
int CkIndex_swe_charm::reg_allEnd_void() {
  int epidx = CkRegisterEp("allEnd()",
      reinterpret_cast<CkCallFnPtr>(_call_allEnd_void), 0, __idx, 0);
  return epidx;
}


// Redn wrapper registration function
int CkIndex_swe_charm::reg_redn_wrapper_allEnd_void() {
  return CkRegisterEp("redn_wrapper_allEnd(CkReductionMsg *impl_msg)",
      reinterpret_cast<CkCallFnPtr>(_call_redn_wrapper_allEnd_void), CMessage_CkReductionMsg::__idx, __idx, 0);
}

void CkIndex_swe_charm::_call_allEnd_void(void* impl_msg, void* impl_obj_void)
{
  swe_charm* impl_obj = static_cast<swe_charm*>(impl_obj_void);
  impl_obj->allEnd();
  if(UsrToEnv(impl_msg)->isVarSysMsg() == 0)
    CkFreeSysMsg(impl_msg);
}
PUPable_def(SINGLE_ARG(Closure_swe_charm::allEnd_3_closure))
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
/* DEFS: void exit();
 */
void CProxy_swe_charm::exit(const CkEntryOptions *impl_e_opts)
{
  ckCheck();
  void *impl_msg = CkAllocSysMsg(impl_e_opts);
  if (ckIsDelegated()) {
    int destPE=CkChareMsgPrep(CkIndex_swe_charm::idx_exit_void(), impl_msg, &ckGetChareID());
    if (destPE!=-1) ckDelegatedTo()->ChareSend(ckDelegatedPtr(),CkIndex_swe_charm::idx_exit_void(), impl_msg, &ckGetChareID(),destPE);
  } else {
    CkSendMsg(CkIndex_swe_charm::idx_exit_void(), impl_msg, &ckGetChareID(),0);
  }
}

// Entry point registration function
int CkIndex_swe_charm::reg_exit_void() {
  int epidx = CkRegisterEp("exit()",
      reinterpret_cast<CkCallFnPtr>(_call_exit_void), 0, __idx, 0);
  return epidx;
}

void CkIndex_swe_charm::_call_exit_void(void* impl_msg, void* impl_obj_void)
{
  swe_charm* impl_obj = static_cast<swe_charm*>(impl_obj_void);
  impl_obj->exit();
  if(UsrToEnv(impl_msg)->isVarSysMsg() == 0)
    CkFreeSysMsg(impl_msg);
}
PUPable_def(SINGLE_ARG(Closure_swe_charm::exit_4_closure))
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
void CkIndex_swe_charm::__register(const char *s, size_t size) {
  __idx = CkRegisterChare(s, size, TypeMainChare);
  CkRegisterBase(__idx, CkIndex_Chare::__idx);
  // REG: swe_charm(CkArgMsg* impl_msg);
  idx_swe_charm_CkArgMsg();
  CkRegisterMainChare(__idx, idx_swe_charm_CkArgMsg());

  // REG: void done(int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall);
  idx_done_marshall2();

  // REG: void allEnd();
  idx_allEnd_void();
  idx_redn_wrapper_allEnd_void();

  // REG: void exit();
  idx_exit_void();

}
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY
void _registerswe_charm(void)
{
  static int _done = 0; if(_done) return; _done = 1;
  CkRegisterReadonly("mainProxy","CProxy_swe_charm",sizeof(mainProxy),(void *) &mainProxy,__xlater_roPup_mainProxy);

  CkRegisterReadonly("blockCountX","int",sizeof(blockCountX),(void *) &blockCountX,__xlater_roPup_blockCountX);

  CkRegisterReadonly("blockCountY","int",sizeof(blockCountY),(void *) &blockCountY,__xlater_roPup_blockCountY);

  CkRegisterReadonly("simulationDuration","float",sizeof(simulationDuration),(void *) &simulationDuration,__xlater_roPup_simulationDuration);

  CkRegisterReadonly("checkpointCount","int",sizeof(checkpointCount),(void *) &checkpointCount,__xlater_roPup_checkpointCount);

  _registerSWE_DimensionalSplittingCharm();

/* REG: message collectorMsg{
double deserialized[];
}
;
*/
CMessage_collectorMsg::__register("collectorMsg", sizeof(collectorMsg),(CkPackFnPtr) collectorMsg::pack,(CkUnpackFnPtr) collectorMsg::unpack);

/* REG: mainchare swe_charm: Chare{
swe_charm(CkArgMsg* impl_msg);
void done(int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall);
void allEnd();
void exit();
};
*/
  CkIndex_swe_charm::__register("swe_charm", sizeof(swe_charm));

}
extern "C" void CkRegisterMainModule(void) {
  _registerswe_charm();
}
#endif /* CK_TEMPLATES_ONLY */
#ifndef CK_TEMPLATES_ONLY
template <>
void CBase_swe_charm::virtual_pup(PUP::er &p) {
    recursive_pup<swe_charm>(dynamic_cast<swe_charm*>(this), p);
}
#endif /* CK_TEMPLATES_ONLY */
