#include <rte_common.h>
#include <rte_log.h>
#include <rte_malloc.h>
#include <rte_memory.h>
#include <rte_memcpy.h> 
#include <rte_eal.h>
#include <rte_launch.h>
#include <rte_cycles.h>
#include <rte_prefetch.h>
#include <rte_lcore.h>
#include <rte_per_lcore.h>
#include <rte_branch_prediction.h>
#include <rte_interrupts.h>
#include <rte_random.h>
#include <rte_debug.h>
#include <rte_ether.h>
#include <rte_ethdev.h>
#include <rte_mempool.h>
#include <rte_mbuf.h>
#include <rte_string_fns.h>


#include "fastpath.h"

extern bool force_quit;

#define FP_MAX_PKT_BURST 32

int
kbridge_lcore_loop(kbridge_lcore_args *lcore_args)
{
    unsigned lcore_id = rte_lcore_id();
    struct lcore_queue_conf *qconf;
    unsigned i, portid, nb_rx;
    struct rte_mbuf *pkts_burst[FP_MAX_PKT_BURST];

    qconf = &((lcore_args->qconf)[lcore_id]);

    printf("[%s], lcore ID: %u, num_ports: %u \n", __FUNCTION__,
           lcore_id, qconf->n_rx_port);

    while (!force_quit) {
        /* RX packet */
        for (i = 0; i < qconf->n_rx_port; i++) {

            portid = qconf->rx_port_list[i];
            nb_rx = rte_eth_rx_burst(portid, 0, pkts_burst, FP_MAX_PKT_BURST);

            if (unlikely(nb_rx == 0))
                continue;
            printf("[%s], received number of packets: %u\n", __FUNCTION__, nb_rx);
#if 0
            port_statistics[portid].rx += nb_rx;

            for (j = 0; j < nb_rx; j++) {
                m = pkts_burst[j];
                rte_prefetch0(rte_pktmbuf_mtod(m, void *));
                l2fwd_simple_forward(m, portid);
            }
#endif
        }
    }

    printf("[%s], received forced quit for lcore_id: %u\n", __FUNCTION__, lcore_id);

    return 0;
}
