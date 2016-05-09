#include <efi.h>
#include "elf.h"

#define ARRSZ(a)	(sizeof(a) / sizeof(a)[0])

enum {
	RBUFFER	= -1,
	RMEMORY = -2,
	RFREAD	= -3,
	RCONTEXT	= -4,
	REFSETPOS	= -5
};

int todec(int v, CHAR16 *b, int bsz);

struct Context {
	EFI_HANDLE h;
	EFI_FILE_IO_INTERFACE *fs;
	EFI_FILE_HANDLE fsroot;
	EFI_FILE_HANDLE file;
	Elf32_Ehdr *eh;
	Elf32_Phdr *eph;
	unsigned char *img;
	UINTN mmkey;
	UINT32 minaddr;
};

int ctx_init(struct Context *o, EFI_HANDLE h);
int ctx_openfs(struct Context *o, EFI_HANDLE dh);
int ctx_openfile(struct Context *o);
int ctx_loadelf(struct Context *o);
int ctx_getmmap(struct Context *o);
int ctx_exec(struct Context *o, EFI_HANDLE img);
void printerr(CONST CHAR16 *m, EFI_STATUS st);

EFI_STATUS eallocpg(UINTN sz, UINT32 addr, VOID **b);
EFI_STATUS eallocp(UINTN sz, VOID **b);
EFI_STATUS efreep(VOID *b);
EFI_STATUS efread(EFI_FILE_HANDLE h, UINTN *bsz, VOID *b);
EFI_STATUS efsetpos(EFI_FILE_HANDLE h, UINT64 pos);
void esetmem(VOID *p, UINTN sz, UINT8 v);
void eprint(CHAR16 *fmt, ...);

