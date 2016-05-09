#ifndef LOADER_H
#define LOADER_H	1

#include "elf.h"

enum {
	LRNOTELF	= -1,
	LRUNSUPBIT	= -2,
	LRUNSUPDATA	= -3,
	LRUNSUPVER	= -4,
	LRNOTEXEC	= -5,
	LRUNSUPMACH	= -6,
	LRMEMRANGE	= -7,
	LRNOMEM	= -8,
	LRSEEK	= -9,
	LRREAD	= -10,
	LRARG	= -11
};

int loadh(EFI_FILE_HANDLE h, Elf32_Ehdr **hd);
int loadph(EFI_FILE_HANDLE h, const Elf32_Ehdr *ehdr, Elf32_Phdr **hd);
int loadscts(EFI_FILE_HANDLE h, const Elf32_Ehdr *ehdr, const Elf32_Phdr *phs, VOID **out);
int iself(const unsigned char *magic);
int verify(const Elf32_Ehdr *hdr);
int memrange(const Elf32_Phdr *ph, int cnt, UINT32 *min, UINT32 *max);

#endif
