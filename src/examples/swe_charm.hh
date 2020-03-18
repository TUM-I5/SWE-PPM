#ifndef __SWE_CHARM_HH
#define __SWE_CHARM_HH

#include "swe_charm.decl.h"
#include "tools/CollectorCharm.hpp"
class swe_charm : public CBase_swe_charm {
	public:
		swe_charm(CkArgMsg *msg);
		swe_charm(CkMigrateMessage *msg);

		void done(int index,float flop, float commTime, float wallTime,float reductionTime);
		void done1(collectorMsg *collector);
		void exit();
		int chareCount;
    float sumFlops;
    float sumCommTime;
    float sumReductionTime;
    CollectorCharm mainCollector;
	private:

};

#endif // __SWE_CHARM_HH
