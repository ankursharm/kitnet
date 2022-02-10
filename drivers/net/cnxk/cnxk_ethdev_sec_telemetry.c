/* SPDX-License-Identifier: BSD-3-Clause
 * Copyright(C) 2022 Marvell.
 */

#include <rte_telemetry.h>

#include <roc_api.h>

#include "cnxk_ethdev.h"

#define STR_MAXLEN 20
#define W0_MAXLEN  21

static int
copy_outb_sa_9k(struct rte_tel_data *d, uint32_t i, void *sa)
{
	struct roc_onf_ipsec_outb_sa *out_sa;
	union {
		struct roc_ie_onf_sa_ctl ctl;
		uint64_t u64;
	} w0;
	char strw0[W0_MAXLEN];
	char str[STR_MAXLEN];

	out_sa = (struct roc_onf_ipsec_outb_sa *)sa;
	w0.ctl = out_sa->ctl;

	snprintf(str, sizeof(str), "outsa_w0_%u", i);
	snprintf(strw0, sizeof(strw0), "%" PRIu64, w0.u64);
	rte_tel_data_add_dict_string(d, str, strw0);

	snprintf(str, sizeof(str), "outsa_src_%u", i);
	rte_tel_data_add_dict_u64(d, str, out_sa->udp_src);

	snprintf(str, sizeof(str), "outsa_dst_%u", i);
	rte_tel_data_add_dict_u64(d, str, out_sa->udp_dst);

	snprintf(str, sizeof(str), "outsa_isrc_%u", i);
	rte_tel_data_add_dict_u64(d, str, out_sa->ip_src);

	snprintf(str, sizeof(str), "outsa_idst_%u", i);
	rte_tel_data_add_dict_u64(d, str, out_sa->ip_dst);

	return 0;
}

static int
copy_inb_sa_9k(struct rte_tel_data *d, uint32_t i, void *sa)
{
	struct roc_onf_ipsec_inb_sa *in_sa;
	union {
		struct roc_ie_onf_sa_ctl ctl;
		uint64_t u64;
	} w0;
	char strw0[W0_MAXLEN];
	char str[STR_MAXLEN];

	in_sa = (struct roc_onf_ipsec_inb_sa *)sa;
	w0.ctl = in_sa->ctl;

	snprintf(str, sizeof(str), "insa_w0_%u", i);
	snprintf(strw0, sizeof(strw0), "%" PRIu64, w0.u64);
	rte_tel_data_add_dict_string(d, str, strw0);

	snprintf(str, sizeof(str), "insa_esnh_%u", i);
	rte_tel_data_add_dict_u64(d, str, in_sa->esn_hi);

	snprintf(str, sizeof(str), "insa_esnl_%u", i);
	rte_tel_data_add_dict_u64(d, str, in_sa->esn_low);

	return 0;
}

static int
ethdev_sec_tel_handle_info(const char *cmd __rte_unused, const char *params,
			   struct rte_tel_data *d)
{
	struct cnxk_eth_sec_sess *eth_sec, *tvar;
	struct rte_eth_dev *eth_dev;
	struct cnxk_eth_dev *dev;
	uint16_t port_id;
	char *end_p;
	uint32_t i;
	int ret;

	port_id = strtoul(params, &end_p, 0);
	if (errno != 0)
		return -EINVAL;

	if (*end_p != '\0')
		plt_err("Extra parameters passed to telemetry, ignoring it");

	if (!rte_eth_dev_is_valid_port(port_id)) {
		plt_err("Invalid port id %u", port_id);
		return -EINVAL;
	}

	eth_dev = &rte_eth_devices[port_id];
	if (!eth_dev) {
		plt_err("Ethdev not available");
		return -EINVAL;
	}

	dev = cnxk_eth_pmd_priv(eth_dev);

	rte_tel_data_start_dict(d);

	rte_tel_data_add_dict_int(d, "nb_outb_sa", dev->outb.nb_sess);

	i = 0;
	if (dev->tx_offloads & RTE_ETH_TX_OFFLOAD_SECURITY) {
		tvar = NULL;
		RTE_TAILQ_FOREACH_SAFE(eth_sec, &dev->outb.list, entry, tvar) {
			ret = copy_outb_sa_9k(d, i++, eth_sec->sa);
			if (ret < 0)
				return ret;
		}
	}

	rte_tel_data_add_dict_int(d, "nb_inb_sa", dev->inb.nb_sess);

	i = 0;
	if (dev->rx_offloads & RTE_ETH_RX_OFFLOAD_SECURITY) {
		tvar = NULL;
		RTE_TAILQ_FOREACH_SAFE(eth_sec, &dev->inb.list, entry, tvar) {
			ret = copy_inb_sa_9k(d, i++, eth_sec->sa);
			if (ret < 0)
				return ret;
		}
	}

	return 0;
}

RTE_INIT(cnxk_ipsec_init_telemetry)
{
	rte_telemetry_register_cmd("/cnxk/ipsec/info",
				   ethdev_sec_tel_handle_info,
				   "Returns ipsec info. Parameters: port id");
}