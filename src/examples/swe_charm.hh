#ifndef __SWE_CHARM_HH
#define __SWE_CHARM_HH

#include "swe_charm.decl.h"
#include "tools/CollectorCharm.hpp"

class swe_charm : public CBase_swe_charm {
public:
    swe_charm(CkArgMsg *msg);

    swe_charm(CkMigrateMessage *msg);

    void done(int index, double ctr_flop, double ctr_exchange, double ctr_barrier, double ctr_reduce, double ctr_wall);

    void exit();

    int chareCount;
    float sumFlops;
    float sumCommTime;
    float sumReductionTime;
    CollectorCharm mainCollector;
private:

};

#endif // __SWE_CHARM_HH
