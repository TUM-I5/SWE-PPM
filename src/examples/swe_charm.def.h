





/* ---------------- method closures -------------- */
#ifndef CK_TEMPLATES_ONLY
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_swe_charm::done_2_closure : public SDAG::Closure {
            int index;
            float flop;
            float commTime;
            float wallTime;
            float reductionTime;


      done_2_closure() {
        init();
      }
      done_2_closure(CkMigrateMessage*) {
        init();
      }
            int & getP0() { return index;}
            float & getP1() { return flop;}
            float & getP2() { return commTime;}
            float & getP3() { return wallTime;}
            float & getP4() { return reductionTime;}
      void pup(PUP::er& __p) {
        __p | index;
        __p | flop;
        __p | commTime;
        __p | wallTime;
        __p | reductionTime;
        packClosure(__p);
      }
      virtual ~done_2_closure() {
      }
      PUPable_decl(SINGLE_ARG(done_2_closure));
    };
#endif /* CK_TEMPLATES_ONLY */

#ifndef CK_TEMPLATES_ONLY

    struct Closure_swe_charm::exit_3_closure : public SDAG::Closure {
      

      exit_3_closure() {
        init();
      }
      exit_3_closure(CkMigrateMessage*) {
        init();
      }
            void pup(PUP::er& __p) {
        packClosure(__p);
      }
      virtual ~exit_3_closure() {
      }
      PUPable_decl(SINGLE_ARG(exit_3_closure));
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


/* DEFS: mainchare swe_charm: Chare{
swe_charm(CkArgMsg* impl_msg);
void done(int index, float flop, float commTime, float wallTime, float reductionTime);
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
      _call_swe_charm_CkArgMsg, CMessage_CkArgMsg::__idx, __idx, 0);
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
/* DEFS: void done(int index, float flop, float commTime, float wallTime, float reductionTime);
 */
void CProxy_swe_charm::done(int index, float flop, float commTime, float wallTime, float reductionTime, const CkEntryOptions *impl_e_opts)
{
  ckCheck();
  //Marshall: int index, float flop, float commTime, float wallTime, float reductionTime
  int impl_off=0;
  { //Find the size of the PUP'd data
    PUP::sizer implP;
    implP|index;
    implP|flop;
    implP|commTime;
    implP|wallTime;
    implP|reductionTime;
    impl_off+=implP.size();
  }
  CkMarshallMsg *impl_msg=CkAllocateMarshallMsg(impl_off,impl_e_opts);
  { //Copy over the PUP'd data
    PUP::toMem implP((void *)impl_msg->msgBuf);
    implP|index;
    implP|flop;
    implP|commTime;
    implP|wallTime;
    implP|reductionTime;
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
  int epidx = CkRegisterEp("done(int index, float flop, float commTime, float wallTime, float reductionTime)",
      _call_done_marshall2, CkMarshallMsg::__idx, __idx, 0+CK_EP_NOKEEP);
  CkRegisterMarshallUnpackFn(epidx, _callmarshall_done_marshall2);
  CkRegisterMessagePupFn(epidx, _marshallmessagepup_done_marshall2);

  return epidx;
}

void CkIndex_swe_charm::_call_done_marshall2(void* impl_msg, void* impl_obj_void)
{
  swe_charm* impl_obj = static_cast<swe_charm*>(impl_obj_void);
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  /*Unmarshall pup'd fields: int index, float flop, float commTime, float wallTime, float reductionTime*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> index;
  implP|index;
  PUP::detail::TemporaryObjectHolder<float> flop;
  implP|flop;
  PUP::detail::TemporaryObjectHolder<float> commTime;
  implP|commTime;
  PUP::detail::TemporaryObjectHolder<float> wallTime;
  implP|wallTime;
  PUP::detail::TemporaryObjectHolder<float> reductionTime;
  implP|reductionTime;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->done(std::move(index.t), std::move(flop.t), std::move(commTime.t), std::move(wallTime.t), std::move(reductionTime.t));
}
int CkIndex_swe_charm::_callmarshall_done_marshall2(char* impl_buf, void* impl_obj_void) {
  swe_charm* impl_obj = static_cast<swe_charm*>(impl_obj_void);
  /*Unmarshall pup'd fields: int index, float flop, float commTime, float wallTime, float reductionTime*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> index;
  implP|index;
  PUP::detail::TemporaryObjectHolder<float> flop;
  implP|flop;
  PUP::detail::TemporaryObjectHolder<float> commTime;
  implP|commTime;
  PUP::detail::TemporaryObjectHolder<float> wallTime;
  implP|wallTime;
  PUP::detail::TemporaryObjectHolder<float> reductionTime;
  implP|reductionTime;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  impl_obj->done(std::move(index.t), std::move(flop.t), std::move(commTime.t), std::move(wallTime.t), std::move(reductionTime.t));
  return implP.size();
}
void CkIndex_swe_charm::_marshallmessagepup_done_marshall2(PUP::er &implDestP,void *impl_msg) {
  CkMarshallMsg *impl_msg_typed=(CkMarshallMsg *)impl_msg;
  char *impl_buf=impl_msg_typed->msgBuf;
  /*Unmarshall pup'd fields: int index, float flop, float commTime, float wallTime, float reductionTime*/
  PUP::fromMem implP(impl_buf);
  PUP::detail::TemporaryObjectHolder<int> index;
  implP|index;
  PUP::detail::TemporaryObjectHolder<float> flop;
  implP|flop;
  PUP::detail::TemporaryObjectHolder<float> commTime;
  implP|commTime;
  PUP::detail::TemporaryObjectHolder<float> wallTime;
  implP|wallTime;
  PUP::detail::TemporaryObjectHolder<float> reductionTime;
  implP|reductionTime;
  impl_buf+=CK_ALIGN(implP.size(),16);
  /*Unmarshall arrays:*/
  if (implDestP.hasComments()) implDestP.comment("index");
  implDestP|index;
  if (implDestP.hasComments()) implDestP.comment("flop");
  implDestP|flop;
  if (implDestP.hasComments()) implDestP.comment("commTime");
  implDestP|commTime;
  if (implDestP.hasComments()) implDestP.comment("wallTime");
  implDestP|wallTime;
  if (implDestP.hasComments()) implDestP.comment("reductionTime");
  implDestP|reductionTime;
}
PUPable_def(SINGLE_ARG(Closure_swe_charm::done_2_closure))
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
      _call_exit_void, 0, __idx, 0);
  return epidx;
}

void CkIndex_swe_charm::_call_exit_void(void* impl_msg, void* impl_obj_void)
{
  swe_charm* impl_obj = static_cast<swe_charm*>(impl_obj_void);
  impl_obj->exit();
  if(UsrToEnv(impl_msg)->isVarSysMsg() == 0)
    CkFreeSysMsg(impl_msg);
}
PUPable_def(SINGLE_ARG(Closure_swe_charm::exit_3_closure))
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

  // REG: void done(int index, float flop, float commTime, float wallTime, float reductionTime);
  idx_done_marshall2();

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

/* REG: mainchare swe_charm: Chare{
swe_charm(CkArgMsg* impl_msg);
void done(int index, float flop, float commTime, float wallTime, float reductionTime);
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
