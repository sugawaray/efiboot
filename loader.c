#include "loader.h"
#include "efiboot.h"

#ifdef PHYADDR
#define MADDR	p_paddr
#else
#define MADDR	p_vaddr
#endif

int
loadh(EFI_FILE_HANDLE h, Elf32_Ehdr **hd)
{
	EFI_STATUS st;
	UINTN bsz;

	bsz = sizeof **hd;
	st = eallocp(bsz, (VOID **)hd);
	if (st != EFI_SUCCESS) {
		eprint(L"memory allocation failed(%r)\n", st);
		return RMEMORY;
	}
	st = efread(h, &bsz, *hd);
	if (st != EFI_SUCCESS) {
		eprint(L"reading the file failed(%r)\n", st);
		efreep(*hd);
		return RFREAD;
	}
	return 0;
}

int
iself(const unsigned char *magic)
{
	if (magic[EI_MAG0] != 0x7f)
		return 0;
	else if (magic[EI_MAG1] != 'E')
		return 0;
	else if (magic[EI_MAG2] != 'L')
		return 0;
	else if (magic[EI_MAG3] != 'F')
		return 0;
	else
		return 1;
}

int
verify(const Elf32_Ehdr *hdr)
{
	const unsigned char *ident;

	ident = hdr->e_ident;
	if (!iself(ident))
		return LRNOTELF;
	if (ident[EI_CLASS] != ELFCLASS32)
		return LRUNSUPBIT;
	if (ident[EI_DATA] != ELFDATA2LSB)
		return LRUNSUPDATA;
	if (ident[EI_VERSION] != EV_CURRENT)
		return LRUNSUPVER;
	if (hdr->e_type != ET_EXEC)
		return LRNOTEXEC;
	if (hdr->e_machine != EM_386)
		return LRUNSUPMACH;
	return 0;
}

int
loadph(EFI_FILE_HANDLE h, const Elf32_Ehdr *hdr, Elf32_Phdr **phdr)
{
	EFI_STATUS st;
	UINTN sz;
	UINTN osz;

	sz = osz = hdr->e_phnum * hdr->e_phentsize;
	st = eallocp(sz, (VOID*)phdr);
	if (st != EFI_SUCCESS) {
		eprint(L"memory allocation failed(%r)\n", st);
		return RMEMORY;
	}
	st = efsetpos(h, hdr->e_phoff);
	if (st != EFI_SUCCESS) {
		eprint(L"setting the file position failed(%r)\n", st);
		return REFSETPOS;
	}
	st = efread(h, &osz, *phdr);
	if (st != EFI_SUCCESS || sz != osz) {
		eprint(L"reading the program header failed(%r)\n", st);
		return RFREAD;
	}
	return 0;
}

int
memrange(const Elf32_Phdr *ph, int cnt, UINT32 *min, UINT32 *max)
{
	int i;
	UINT32 mi = ~0;
	UINT32 ma = 0;
	const Elf32_Phdr *p;

	for (i = 0; i < cnt; ++i) {
		p = &ph[i];
		if (p->p_type != PT_LOAD)
			continue;
		if (p->MADDR < mi)
			mi = p->MADDR;
		if (p->MADDR + p->p_memsz > ma)
			ma = p->MADDR + p->p_memsz;
	}
	if (mi > ma)
		return LRARG;
	if (min != NULL)
		*min = mi;
	if (max != NULL)
		*max = ma;
	return 0;
}

int
loadscts(EFI_FILE_HANDLE h, const Elf32_Ehdr *eh, const Elf32_Phdr *phs, VOID **out)
{
	UINT32 mmin;
	UINT32 mmax;
	int i;
	int ir;
	int r;
	char *ra;
	EFI_STATUS st;
	UINTN sz;

	ir = memrange(phs, eh->e_phnum, &mmin, &mmax);
	if (ir != 0)
		return LRMEMRANGE;
	st = eallocpg(mmax - mmin, mmin, out);
	if (st != EFI_SUCCESS)
		return LRNOMEM;
	for (i = 0; i < eh->e_phnum; ++i) {
		if (phs[i].p_type != PT_LOAD)
			continue;
		sz = phs[i].p_filesz;
		ra = (char *)*out + phs[i].MADDR - mmin;
		if ((st = efsetpos(h, phs[i].p_offset)) != EFI_SUCCESS)
			r = LRSEEK;
		else if ((st = efread(h, &sz, ra)) != EFI_SUCCESS)
			r = LRREAD;
		else {
			esetmem(ra + sz, phs[i].p_memsz - sz, 0);
			r = 0;
		}
		if (r != 0)
			break;
	}
	if (r != 0) {
		efreep(*out);
		return r;
	} else
		return 0;
}
