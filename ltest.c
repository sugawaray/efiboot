#include "efiboot.h"
#include "stub.h"
#include "loader.h"
#include <stdio.h>
#include <string.h>

void
loadh_test()
{
	int r;
	EFI_FILE_HANDLE h;
	Elf32_Ehdr *hd;

	fprintf(stderr, "#loadh_test\n");
	h = NULL;
	r = loadh(h, &hd);
	if (r != 0)
		fprintf(stderr, "retval(%d)\n", r);
	if (hd->e_ident[EI_MAG0] != 0x7f)
		fprintf(stderr, "e_ident[EI_MAG0](%hhu)\n", hd->e_ident[EI_MAG0]);
}

static
void
setmagic(unsigned char *b)
{
	b[EI_MAG0] = 0x7f;
	b[EI_MAG1] = 'E';
	b[EI_MAG2] = 'L';
	b[EI_MAG3] = 'F';
}

void
setident(unsigned char *b)
{
	setmagic(b);
	b[EI_CLASS] = ELFCLASS32;
	b[EI_DATA] = ELFDATA2LSB;
	b[EI_VERSION] = EV_CURRENT;
}

static
void
sethdr(Elf32_Ehdr *p)
{
	memset(p, 0, sizeof *p);
	setident((unsigned char *)p);
	p->e_type = ET_EXEC;
	p->e_machine = EM_386;
	p->e_version = EV_CURRENT;
}

void
setphdr(Elf32_Phdr *p)
{
	memset(p, 0, sizeof *p);
	p->p_type = PT_LOAD;
	p->p_align = 0x1000;
}

void
iself_test()
{
	unsigned char good[EI_NIDENT];
	unsigned char ident[EI_NIDENT];

	fprintf(stderr, "#iself_test\n");
	memset(good, 0, sizeof good);
	setmagic(good);

	memcpy(ident, good, sizeof ident);
	if (!iself(ident))
		fprintf(stderr, "0\n");
	ident[EI_MAG0] = 0x7e;
	if (iself(ident))
		fprintf(stderr, "1\n");
	memcpy(ident, good, sizeof ident);
	ident[EI_MAG1] = 'F';
	if (iself(ident))
		fprintf(stderr, "2\n");
	memcpy(ident, good, sizeof ident);
	ident[EI_MAG2] = 'M';
	if (iself(ident))
		fprintf(stderr, "3\n");
	memcpy(ident, good, sizeof ident);
	ident[EI_MAG3] = 'G';
	if (iself(ident))
		fprintf(stderr, "4\n");
}

void
verify_test()
{
	int r;
	Elf32_Ehdr hdr;

	fprintf(stderr, "#verify_test\n");
	memset(&hdr, 0, sizeof hdr);
	sethdr(&hdr);

	r = verify(&hdr);
	if (r != 0)
		fprintf(stderr, "retval(%d)\n", r);
	memset(&hdr, 0, sizeof hdr);
	sethdr(&hdr);
	hdr.e_ident[EI_MAG0] = 0x7e;

	r = verify(&hdr);
	if (r == 0)
		fprintf(stderr, "1retval(%d)\n", r);

	memset(&hdr, 0, sizeof hdr);
	sethdr(&hdr);
	hdr.e_ident[EI_CLASS] = ELFCLASS64;
	r = verify(&hdr);
	if (r == 0)
		fprintf(stderr, "2retval(%d)\n", r);

	memset(&hdr, 0, sizeof hdr);
	sethdr(&hdr);
	hdr.e_ident[EI_DATA] = ELFDATA2MSB;
	r = verify(&hdr);
	if (r == 0)
		fprintf(stderr, "3retval(%d)\n", r);

	memset(&hdr, 0, sizeof hdr);
	sethdr(&hdr);
	hdr.e_ident[EI_VERSION] = EV_NONE;
	r = verify(&hdr);
	if (r == 0)
		fprintf(stderr, "4retval(%d)\n", r);

	memset(&hdr, 0, sizeof hdr);
	sethdr(&hdr);
	hdr.e_type = ET_REL;
	r = verify(&hdr);
	if (r == 0)
		fprintf(stderr, "5retval(%d)\n", r);

	memset(&hdr, 0, sizeof hdr);
	sethdr(&hdr);
	hdr.e_machine = EM_SPARC;
	r = verify(&hdr);
	if (r == 0)
		fprintf(stderr, "6retval(%d)\n", r);
}

void
loadph_test()
{
	int r;
	int i;
	EFI_FILE_HANDLE h;
	Elf32_Ehdr *hdr;
	Elf32_Phdr *phdr;

	fprintf(stderr, "#loadph_test\n");
	stub_setmode(STUBMODEFILE);
	r = stub_efopen("./elfsample", "rb");
	if (r != 0) {
		fprintf(stderr, "stub_efopen(%d)\n", r);
		return;
	}
	h = NULL;
	r = loadh(h, &hdr);
	if (r != 0) {
		fprintf(stderr, "loadh failed(%d)\n", r);
		return;
	}
	phdr = NULL;
	r = loadph(h, hdr, &phdr);
	if (r != 0)
		fprintf(stderr, "retval(%d)\n", r);
	if (phdr == NULL)
		fprintf(stderr, "phdr(%p)\n", (void *)phdr);
	for (i = 0; i < hdr->e_phnum; ++i) {
		if (phdr[i].p_type >= PT_NULL && phdr[i].p_type <= PT_PHDR)
			continue;
		if (phdr[i].p_type == PT_LOPROC || phdr[i].p_type == PT_HIPROC)
			continue;
		fprintf(stderr, "p_type(0x%x) wrong at(%d)\n", phdr[i].p_type, i);
	}
	stub_efclose();
}

void
memrange_test()
{
	Elf32_Phdr h[5];
	UINT32 min;
	UINT32 max;
	int r;

	fprintf(stderr, "#memrange_test\n");
	h[0].p_type = PT_LOAD;
	h[0].p_vaddr = 0x1000;
	h[0].p_memsz = 0x999;

	h[1].p_type = PT_LOAD;
	h[1].p_vaddr = 0x2000;
	h[1].p_memsz = 0x999;

	h[2].p_type = PT_LOAD;
	h[2].p_vaddr = 0x3000;
	h[2].p_memsz = 0x999;

	h[3].p_type = PT_NULL;
	h[3].p_vaddr = 0x0;
	h[3].p_memsz = 0x10000;

	r = memrange(h, ARRSZ(h), &min, &max);
	if (r != 0)
		fprintf(stderr, "retval(%d)\n", r);
	if (min != 0x1000)
		fprintf(stderr, "min(0x%x)\n", min);
	if (max != 0x3999)
		fprintf(stderr, "max(0x%x)\n", max);
}

void
memrange_type_test()
{
	Elf32_Phdr hd[1];
	int r;
	UINT32 mi, ma;

	fprintf(stderr, "#memrange_type_test\n");
	setphdr(hd);
	hd[0].p_type = PT_NULL;
	hd[0].p_vaddr = 0x1000;
	hd[0].p_memsz = 0x1000;

	r = memrange(hd, ARRSZ(hd), &mi, &ma);
	if (r == 0)
		fprintf(stderr, "retval(%d)\n", r);
}

void
loadscts_test()
{
	int r;
	EFI_FILE_HANDLE h = NULL;
	Elf32_Phdr hd[1];
	unsigned char *p;
	Elf32_Ehdr ehd;

	fprintf(stderr, "#loadscts_test\n");
	sethdr(&ehd);
	ehd.e_phnum = 1;

	setphdr(hd);
	hd[0].p_offset = 0x1000;
	hd[0].p_vaddr = 0x2000;
	hd[0].p_memsz = 0x1000;
	hd[0].p_filesz = 0x11;

	r = stub_efopen("./testsects", "r");
	if (r != 0) {
		fprintf(stderr, "file(%s) open failed.\n", "./testsects");
		return;
	}
	stub_setmode(STUBMODEFILE);
	r = loadscts(h, &ehd, hd, (VOID **)&p);
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		stub_efclose();
		return;
	}
	if (p[0] != 0x01)
		fprintf(stderr, "p[0](%hhu)\n", p[0]);
	if (p[0x10] != 0x01)
		fprintf(stderr, "p[0x10](%hhu)\n", p[0x10]);
	stub_efclose();
}

int
main()
{
	loadh_test();
	iself_test();
	verify_test();
	loadph_test();
	memrange_test();
	memrange_type_test();
	loadscts_test();
}
