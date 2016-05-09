#include "efiboot.h"
#include "stub.h"
#include "loader.h"
#include <stdio.h>
#include <string.h>


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
init()
{
	int r;

	r = stub_efopen("./testsects", "r");
	if (r != 0) {
		fprintf(stderr, "file(%s) open failed.\n", "./testsects");
		return;
	}
	stub_setmode(STUBMODEFILE);
}

void
cleanup()
{
	stub_efclose();
}

void
single_test()
{
	int r;
	EFI_FILE_HANDLE h = NULL;
	Elf32_Phdr hd[1];
	unsigned char *p;
	Elf32_Ehdr ehd;

	fprintf(stderr, "#single_test\n");
	init();
	sethdr(&ehd);
	ehd.e_phnum = 1;

	setphdr(hd);
	hd[0].p_offset = 0x1000;
	hd[0].p_vaddr = 0x2000;
	hd[0].p_memsz = 0x1000;
	hd[0].p_filesz = 0x11;

	r = loadscts(h, &ehd, hd, (VOID **)&p);
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		cleanup();
		return;
	}
	if (p[0] != 0x01)
		fprintf(stderr, "p[0](%hhu)\n", p[0]);
	if (p[0x10] != 0x01)
		fprintf(stderr, "p[0x10](%hhu)\n", p[0x10]);
	cleanup();
}

/* sets program header information for testsects binary. */
void
setphdr1st(Elf32_Phdr *h)
{
	setphdr(h);
	h->p_offset = 0x1000;
	h->p_vaddr = 0x2000;
	h->p_memsz = 0x1000;
	h->p_filesz = 0x11;
}

void
setphdr2nd(Elf32_Phdr *h)
{
	setphdr(h);
	h->p_offset = 0x2000;
	h->p_vaddr = 0x3000;
	h->p_memsz = 0x1000;
	h->p_filesz = 0x11;
}

void
multiple_test()
{
	int r;
	EFI_FILE_HANDLE h = NULL;
	Elf32_Phdr hd[2];
	unsigned char *p;
	Elf32_Ehdr ehd;

	fprintf(stderr, "#multiple_test\n");
	init();
	sethdr(&ehd);
	ehd.e_phnum = 2;

	setphdr1st(hd);
	setphdr2nd(&hd[1]);

	r = loadscts(h, &ehd, hd, (VOID **)&p);
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		cleanup();
		return;
	}
	if (p[0] != 0x01)
		fprintf(stderr, "p[0](%hhu)\n", p[0]);
	if (p[0x10] != 0x01)
		fprintf(stderr, "p[0x10](%hhu)\n", p[0x10]);
	if (p[0x1000] != 0x02)
		fprintf(stderr, "p[0x1000](%hhu)\n", p[0x1000]);
	if (p[0x1010] != 0x02)
		fprintf(stderr, "p[0x1010](%hhu)\n", p[0x1010]);
	cleanup();
}

void
bssinit_test()
{
	int r;
	EFI_FILE_HANDLE h = NULL;
	Elf32_Phdr hd[1];
	unsigned char *p;
	Elf32_Ehdr ehd;

	fprintf(stderr, "#bssinit_test\n");
	init();
	sethdr(&ehd);
	ehd.e_phnum = 1;

	setphdr(hd);
	hd[0].p_offset = 0x2000;
	hd[0].p_filesz = 0x11;
	hd[0].p_vaddr = 0x1000;
	hd[0].p_memsz = 0x1000;

	r = loadscts(h, &ehd, hd, (VOID **)&p);
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		cleanup();
		return;
	}
	if (p[0x11] != 0) {
		fprintf(stderr, "p[0x11](%hhu)\n", p[0x11]);
	}
	if (p[0x999] != 0) {
		fprintf(stderr, "p[0x999](%hhu)\n", p[0x999]);
	}
	cleanup();
}

void
ptype_test()
{
	unsigned char *p;
	EFI_FILE_HANDLE fh = NULL;
	Elf32_Ehdr h;
	Elf32_Phdr ph[2];
	int r;

	fprintf(stderr, "#ptype_test\n");
	init();
	sethdr(&h);
	h.e_phnum = 2;

	setphdr1st(ph);
	ph->p_type = PT_NULL;
	setphdr2nd(ph + 1);
	ph->p_type = PT_LOAD;

	r = loadscts(fh, &h, ph, (VOID **)&p);
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		cleanup();
		return;
	}
	if (p[0x1000] != 0x02)
		fprintf(stderr, "p[0](%hhu)\n", p[0]);
	if (p[0x1010] != 0x02)
		fprintf(stderr, "p[0x10](%hhu)\n", p[0x11]);
}

int
main()
{
	single_test();
	multiple_test();
	bssinit_test();
	ptype_test();
}
