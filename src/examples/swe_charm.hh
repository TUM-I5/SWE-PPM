#ifndef __SWE_CHARM_HH
#define __SWE_CHARM_HH

#include "swe_charm.decl.h"

class swe_charm : public CBase_swe_charm {
	public:
		swe_charm(CkArgMsg *msg);
		swe_charm(CkMigrateMessage *msg);

		void done(int index,float flop, float commTime, float wallTime);
		void exit();
            int chareCount;
    float sumFlops;
    float sumCommTime;
	private:

};
#endif // __SWE_CHARM_HH
