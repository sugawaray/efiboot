#include "stub.h"
#include "efiboot.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int mode = STUBMODENFILE;
static FILE *stubfile = NULL;

EFI_STATUS
eallocp(UINTN sz, VOID **b)
{
	*b = malloc(sz);
	memset(*b, 0xFF, sz);
	return EFI_SUCCESS;
}

EFI_STATUS
eallocpg(UINTN sz, UINT32 addr, VOID **b)
{
	*b = malloc(sz);
	memset(*b, 0xFF, sz);
	return EFI_SUCCESS;
}

EFI_STATUS
efreep(VOID *b)
{
	free(b);
	return EFI_SUCCESS;
}

void
eprint(CHAR16 *fmt, ...)
{
}

EFI_STATUS
efread(EFI_FILE_HANDLE h, UINTN *bsz, VOID *b)
{
#if 0
	unsigned char *p;

	p = b;
	p[0] = 0x7f;
	return EFI_SUCCESS;
#else
	unsigned char *p;

	if (mode == STUBMODENFILE) {
		p = b;
		p[0] = 0x7f;
		return EFI_SUCCESS;
	}
	if (stubfile == NULL)
		return EFI_NOT_READY;
	*bsz = fread(b, 1, *bsz, stubfile);
	return EFI_SUCCESS;
#endif
}

int
stub_efopen(const char *path, const char *mode)
{
	if (stubfile != NULL)
		return RCONTEXT;
	stubfile = fopen(path, mode);
	if (stubfile != NULL)
		return 0;
	else
		return -1;
}

int
stub_efclose()
{
	if (stubfile == NULL)
		return -1;
	if (fclose(stubfile) == 0) {
		stubfile = NULL;
		return 0;
	} else
		return -1;
}

void
stub_setmode(int newval)
{
	mode = newval;
}

EFI_STATUS
efsetpos(EFI_FILE_HANDLE h, UINT64 pos)
{
	int r;
	if (stubfile == NULL)
		return RCONTEXT;
	r = fseek(stubfile, pos, SEEK_SET);
	if (r != 0)
		return REFSETPOS;
	else
		return EFI_SUCCESS;
}

VOID
esetmem(VOID *p, UINTN sz, UINT8 v)
{
	memset(p, v, sz);
}
