/*
 * Author: Pierre Pfister <pierre pfister@darou.fr>
 *
 * Copyright (c) 2014 Cisco Systems, Inc.
 *
 * Prefix delegation support in prefix assignment.
 * This file provides an API for reserving prefixes
 * for sub-delegation.
 *
 */


#ifndef PA_PD_H_
#define PA_PD_H_

#include "pa_data.h"
#include "pa_timer.h"

//#define PA_PD_RIGOUROUS_LEASES

struct pa_pd_conf {
	uint8_t pd_min_len;       /* pd will not provide bigger prefixes : default = 62 */
	uint8_t pd_min_ratio_exp; /* pd will not provide more than 1/(2^x) of the dp size : default 3 (1/8) */
};

/* General structure for pa_pd management.
 * Put in pa structure. */
struct pa_pd {
	struct list_head leases;       /* List of known leases */
	struct pa_data_user data_user; /* Used to receive data updates */
	struct pa_pd_conf conf;        /* The current pa_pd conf */
	struct pa_timer timer;         /* Update leases with new dps or retry for other dps */
};

/* This structure keeps track of a given delegation lease. */
struct pa_pd_lease {
	/* Called whenever some modification is made, after a short delay elapsed (about half a second)
	 * so that if multiple modifications are made in a row, it is only called once. */
	void (*update_cb)(struct pa_pd_lease *lease);

	/* Contains a list of pa_cpds (see pa_data.h).
	 * A cpd may be delegated only if  cpd->cp.dp != NULL.
	 * If cpd->cp.applied == false, the prefix MAY be delegated, but only with a significantly short delay.
	 * cpd->cp.applied is false initially and becomes true after some time.
	 * cpd->cp.dp is the associated delegated prefix. If null, it means the element is not valid
	 * and will be removed.
	 * Lifetimes may be found there: cpd->cp.dp->valid_lifetime (or preferred).
	 * Dhcp data may be found here: cpd->cp.dp->dhcp_data/len.
	 * Whenever one of these values are changed, the update_cb is called (after a delay). */
	struct btrie cpds;

	/****** Private *****/
	char *lease_id;
	struct list_head le;            /* Linked in pa_pd structure */
	struct btrie dp_reqs;           /* Linked with dp lease_links tree (n:n relation)*/
	struct pa_timer timer;
	bool just_created;          /* If true, missing prefixes will be computed for all dps */
	struct pa_pd *pd;
	uint8_t preferred_len;
	uint8_t max_len;

#define PA_PDL_ID_L "'%s'"
#define PA_PDL_ID_LA(lease_id) (lease_id)?(lease_id):"NO-ID"
#define PA_PDL_L "pa_pd_lease("PA_PDL_ID_L")"
#define PA_PDL_LA(lease) PA_PDL_ID_LA((lease)->lease_id)
};

#define pa_pd_for_each_lease(lease, pa_pd) list_for_each_entry(lease, &(pa_pd)->leases, le)
#define pa_pd_for_each_cpd(pa_cpd, lease) btrie_for_each_down_entry(pa_cpd, &(lease)->cpds, NULL, 0, lease_be)
#define pa_pd_for_each_cpd_safe(pa_cpd, cpd2, lease) btrie_for_each_down_entry_safe(pa_cpd, cpd2, &(lease)->cpds, NULL, 0, lease_be)

/* Adds a new lease request.
 * When the function returns, the prefix list is empty. It is updated later and the.
 * The client_id is used as a seed for random prefix selection (and printing logs).
 * If client_id is NULL is set, the prefix is chosen randomly.
 * Returns 0 on success. -1 otherwise. */
int pa_pd_lease_init(struct pa_pd *, struct pa_pd_lease *, const char *lease_id,
		uint8_t preferred_len, uint8_t max_len);

/* Terminates an existing lease. */
void pa_pd_lease_term(struct pa_pd *, struct pa_pd_lease *);

void pa_pd_conf_defaults(struct pa_pd_conf *);

void pa_pd_init(struct pa_pd *, const struct pa_pd_conf *);
void pa_pd_start(struct pa_pd *);
void pa_pd_stop(struct pa_pd *);
void pa_pd_term(struct pa_pd *);

#endif /* PA_PD_H_ */
