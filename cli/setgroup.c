#include "btcli.h"

void usage_setgroup(void)
{
  printf(
    "Place torrent in a different group.\n"
    "\n"
    "Usage: setgroup torrent groupname\n"
    "\n"
    "Arguments:\n"
    "torrent\n"
    "\tTorrent number or file\n"
    "\n"
    "groupname\n"
    "\tThe new group for the torrent (see 'btcli grouplist')\n"
    "\n");
  exit(1);
}

static struct option setgroup_opts [] = {
    { "help", no_argument, NULL, 'H' },
    {NULL, 0, NULL, 0}
};


void
cmd_setgroup(int argc, char **argv)
{
    int ch;
    struct ipc_torrent torrent;

    while ((ch = getopt_long(argc, argv, "", setgroup_opts, NULL)) != -1)
        usage_setgroup();
    argc -= optind;
    argv += optind;

    if (argc < 2)
    {
      usage_setgroup();
    }

    // First argument is torrent number, second is the string.
    if (!torrent_spec(argv[0], &torrent))
    {
      exit(1);
    }

    btpd_connect();
    handle_ipc_res(btpd_setgroup(ipc, &torrent, argv[1]), "setgroup", argv[1]);
}


