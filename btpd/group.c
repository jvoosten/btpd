#include <assert.h>

#include "btpd.h"

/* BTPD groups
 *
 * Torrents can be bundled into groups; these torrents share their bandwidth with eachother
 *
 */

static unsigned m_ngroups = 0;
static struct group_lst m_groups = BTPDQ_HEAD_INITIALIZER(m_groups);


/* Initialize groups list. Creates one 'root' group with a ratio of 10 */
void group_init()
{
  assert (0 == m_ngroups);
  group_add ("/", 10);
}


unsigned group_count ()
{
  return m_ngroups;
}


const struct group_lst *group_get_all(void)
{
  return &m_groups;
}


/* Parses a string of the form "string:number,[string:number....]" to represent ratios for groups.
 * The string is the name of the group, the number the ratio.
 */
void group_parse_ratios (const char *ratios)
{
  if (NULL == ratios || 0 == strcmp ("", ratios))
  {
    return;
  }

  char *work = strdup (ratios);
  if (NULL == work)
  {
    return;
  }

  // Chop 'work' string into pieces. And yes, strtok is evil.
  char *colon = NULL;
  int ratio = 0;
  char *entry = strtok (work, ",");
  while (NULL != entry)
  {
    colon = strchr (entry, ':');
    if (NULL == colon)
    {
      // syntax error
      btpd_log (BTPD_L_ERROR, "Syntax error in groups (missing colon): %s\n", entry);
    }
    else if (0 == strlen (entry))
    {
      btpd_log (BTPD_L_ERROR, "Group name is empty\n");
    }
    else
    {
      ratio = atoi (colon + 1);
      if (ratio <= 0)
      {
        btpd_log (BTPD_L_ERROR, "Invalid numerical value for group: %s\n", entry);
      }
      else
      {
        *colon = '\0';
        
        // search for existing name
        if (NULL != group_by_name (entry))
        {
          btpd_log (BTPD_L_ERROR, "Group %s already exists\n", entry);
        }
        else
        {
          group_add (entry, ratio);
        }
      }
    }

    entry = strtok (NULL, ",");
  }

  free (work);
}

/* Return name of the group */
const char *group_name (struct group *gr)
{
  assert (NULL != gr);
  return gr->name;
}




/* Search existing list of groups with a specific name. If the name was not found,
 * NULL is returned.
 *
 * See also: group_by_ref()
 */
struct group *group_by_name (const char *name)
{
  struct group *gr = NULL;
  BTPDQ_FOREACH(gr, &m_groups, entry)
  {
    if (0 == strcmp (name, gr->name))
    {
      return gr;
    }
  }
  return NULL;
}

/* Searches list of groups for one with the given name. If the name was not found
 * the 'root' group is returned; this differs from group_by_name() which returns NULL.
 * This function always returns a valid group.
 */
struct group *group_by_ref (const char *name)
{
  struct group *gr = NULL;
  BTPDQ_FOREACH(gr, &m_groups, entry)
  {
    if (0 == strcmp (name, gr->name))
    {
      return gr;
    }
  }
  return BTPDQ_FIRST (&m_groups);
}


struct group *group_add (const char *name, unsigned ratio)
{
  assert (NULL != name);
  assert (ratio > 0);
  struct group *gr = NULL;
  gr = btpd_calloc(1, sizeof(struct group));

  btpd_log (BTPD_L_BTPD, "Adding group %s, ratio %u\n", name, ratio);
  gr->ratio = ratio; // initial ratio for root group
  gr->bw_limit_out = 0;
  gr->bw_limit_in = 0;
  gr->bw_bytes_out = 0;
  gr->bw_bytes_in = 0;
  gr->name = strdup(name);
  BTPDQ_INSERT_TAIL (&m_groups, gr, entry);
  m_ngroups++;
  return gr;
}

/*
 * Recalculate the bandwidth across the groups (if applicable). Basically,
 * it adds up all the ratios of the groups, then divides the bandwidth
 * according to ratio / total.
 *
 * The root group has an initial ratio of 10, so if you add one other group
 * with a ratio of 20, bandwidth will be divided by a 1:2 ratio between the
 * groups.
 *
 * This function should be called after the total up/download rate has been changed,
 * a group has been added or a ratio changed.
 *
 * Note that if there is only one group, all bandwidth is allocated to group 0.
 */

void group_recalculate ()
{
  struct group *gr = NULL;
  unsigned total = 0;

  BTPDQ_FOREACH(gr, &m_groups, entry)
  {
    total += gr->ratio;
  }
printf("group_recalculate: total = %u\n", total);
  BTPDQ_FOREACH(gr, &m_groups, entry)
  {
    gr->bw_limit_out = net_bw_limit_out * gr->ratio / total;
    gr->bw_limit_in  = net_bw_limit_in  * gr->ratio / total;
printf ("group %s: bw_out = %lu, bw_in = %lu\n", gr->name, gr->bw_limit_out, gr->bw_limit_in);
  }
}


/* Reset bandwidth counters for the next 'tick'. When writing or reading data from
 * the network we consume peers until our bandwidth is gone.
 *
 */
void group_reset_bw ()
{
  struct group *gr = NULL;
  BTPDQ_FOREACH(gr, &m_groups, entry)
  {
    gr->bw_bytes_out = gr->bw_limit_out;
    gr->bw_bytes_in = gr->bw_limit_in;
  }
}