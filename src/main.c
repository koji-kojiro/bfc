#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <libgen.h>
#include <string.h>
#include <libgccjit.h>
#include "bfc.h"

static void
show_help (char *prog)
{
  printf ("usage: %s [options] file\n\n", prog);
  printf ("options:\n");
  printf ("  -h         \tshow this screen and exit.\n");
  printf ("  -o <file>  \tplace the output into <file>.\n");
  printf ("  -r         \texecute file directly.\n");
  printf ("  -v         \tshow version info and exit.\n\n");
  exit(1);
}

static void
show_version (char *prog)
{
  printf ("%s v%s\n", prog, VERSION);
  exit(1);
}

static void
show_hint(char *prog, const char* msg)
{
  printf ("%s: try `%s -h`.\n", msg, prog);
  exit (1);
}

int
main (int argc, char *argv[])
{
  int c;
  char *src;
  char *dst = "a.out";
  bool aot = true;
  char *prog = basename (argv[0]);
  
  opterr = false;

  while ((c = getopt (argc, argv, "rhvo:")) != -1)
    switch (c) {
      case 'o':
        dst = malloc (strlen (optarg) + 1);
        strcpy (dst, optarg);
        break;
      case 'r':
        aot = false;
        break;
      case 'h':
        show_help (prog);
        break;
      case 'v':
        show_version (prog);
        break;
      default:
        show_hint (prog, "invalid argument.");
        break;
    }
  
  switch (argc - optind) {
    case 0:
      show_hint (prog, "missing filename.");
      break;
    case 1:
      src = malloc (strlen (argv[optind]) + 1);
      strcpy (src, argv[optind]);
      break;
    default:
      show_hint (prog, "too many arguments."); 
      break; 
  }

  bfc_t *bfc = bfc_new (30000, 200);


  if (aot)
    bfc_compile_file (bfc, src, dst);
  else
    bfc_exec_file (bfc, src);
  bfc_release (bfc);
  return 0;
}
