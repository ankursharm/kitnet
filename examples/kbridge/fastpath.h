#ifndef _KBRIDGE_FASTPATH_

#include "common.h"

typedef struct krbridge_lcore_args {
    struct lcore_queue_conf *qconf;
} kbridge_lcore_args;

int kbridge_lcore_loop(kbridge_lcore_args *lcore_args);
#endif
