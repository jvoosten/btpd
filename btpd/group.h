/* 
 * File:   group.h
 * Author: jvoosten
 *
 * Created on February 14, 2011, 2:03 AM
 */

#ifndef GROUP_H
#define	GROUP_H


#include "btpd.h"

#ifdef	__cplusplus
extern "C"
{
#endif

/* Information per group of torrents. Includes requested percentage,
 * allocated bandwidth and name.
 */
struct group
{
  /* Descriptive name (optional) */
  const char *name;

  /* Requested part of total bandwidth */
  unsigned int ratio;

  /* Calculated bandwidth up/download */
  unsigned long bw_limit_out;
  unsigned long bw_limit_in;

  /* Current bytes in queue */
  unsigned long bw_bytes_out;
  unsigned long bw_bytes_in; 

  BTPDQ_ENTRY(group) entry;
};

BTPDQ_HEAD(group_lst, group);


/* Initialize groups list; also create initial (root) group */
void group_init ();
/* Return number of groups; there's always one */
unsigned group_count ();
/* Return list of groups */
const struct group_lst *group_get_all(void);
/* Parse name:ratio strings and add groups */
void group_parse_ratios (const char *ratios);

/* Find group based on name; returns NULL if no match */
struct group *group_by_name (const char *name);
/* Find group based on name, but returns root group is there is no match */
struct group *group_by_ref (const char *name);

/* Return name of the group */
const char *group_name (struct group *gr);

/* Create new group */
struct group *group_add (const char *name, unsigned ratio);

/* Redistribute/calculate requested bandwidth across groups */
void group_recalculate ();

/* Reset bandwidth count for all groups */
void group_reset_bw ();

#ifdef	__cplusplus
}
#endif

#endif	/* GROUP_H */

