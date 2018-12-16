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
  printf ("  -e <string> run <string> directly\n");
  printf ("  -h          show this screen and exit\n");
  printf ("  -m <size>   specify memory size in bytes (default 1024)\n");
  printf ("  -o <file>   place the output into <file> (default a.out)\n");
  printf ("  -O <level>  set optimization level (from 1 to 3, default 0)\n");
  printf ("  -r          execute file directly\n");
  printf ("  -v          show version info and exit\n\n");
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
  printf ("bfc: error: %s; try `%s -h`\n", msg, prog);
  exit (1);
}

int
main (int argc, char *argv[])
{
  int c;
  char *src = NULL;
  char *dst = "a.out";
  size_t mem_size = 1024;
  size_t opt_level = 0;
  bool aot = true;
  char *prog = basename (argv[0]);
  
  opterr = false;

  while ((c = getopt (argc, argv, "erhvo:m:O:")) != -1)
    switch (c) {
      case 'e':
        src = malloc (strlen (argv[optind]) + 1);
        strcpy (src, argv[optind]);
        break;
      case 'o':
        dst = malloc (strlen (optarg) + 1);
        strcpy (dst, optarg);
        break;
      case 'O':
        opt_level = atoi (optarg);
      case 'm':
        mem_size = atoi (optarg);
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
        show_hint (prog, "invalid argument");
        break;
    }
  
  bfc_t *bfc = bfc_new (mem_size, 2000, opt_level);
  
  if (src != NULL)
    return bfc_exec_string (bfc, src);

  switch (argc - optind) {
    case 0:
      show_hint (prog, "missing filename");
      break;
    case 1:
      src = malloc (strlen (argv[optind]) + 1);
      strcpy (src, argv[optind]);
      break;
    default:
      show_hint (prog, "too many arguments"); 
      break; 
  }


  if (aot)
    bfc_compile_file (bfc, src, dst);
  else
    return bfc_exec_file (bfc, src);
  return 0;
}
