#include <efi.h>
#include <efilib.h>
#include "efiboot.h"
#include "loader.h"

int
ctx_init(struct Context *o, EFI_HANDLE h)
{
	ZeroMem(o, sizeof *o);
	o->h = h;
	return 0;
}

int
ctx_openfs(struct Context *o, EFI_HANDLE dh)
{
	EFI_STATUS st;
	EFI_OPEN_PROTOCOL op;
	EFI_FILE_IO_INTERFACE *iface;
	EFI_VOLUME_OPEN opv;
	EFI_FILE_HANDLE root;
	UINT32 attr;

	Print(L"ctx_openfs ...\n");
	op = BS->OpenProtocol;
	attr = EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL;
	st = uefi_call_wrapper(op, 6, dh, &FileSystemProtocol, (VOID **)&iface, o->h, NULL, attr);
	if (st != EFI_SUCCESS) {
		printerr(L"OpenProtocol failed.", st);
		return -1;
	}
	opv = iface->OpenVolume;
	st = uefi_call_wrapper(opv, 2, iface, &root);
	if (st != EFI_SUCCESS) {
		printerr(L"OpenVolume failed.", st);
		return -1;
	}
	o->fs = iface;
	o->fsroot = root;
	return 0;
}

int
ctx_openfile(struct Context *o)
{
	EFI_STATUS st;
	EFI_FILE_OPEN opop;

	opop = o->fsroot->Open;
	st = uefi_call_wrapper(opop, 5, o->fsroot, &o->file, L"\\test.elf", EFI_FILE_MODE_READ, 0);
	if (st != EFI_SUCCESS) {
		printerr(L"Open failed.", st);
		return -1;
	}
	return 0;
}

int
ctx_loadelf(struct Context *o)
{
	int r;
	UINT32 mmin;

	r = loadh(o->file, &o->eh);
	if (r != 0) {
		Print(L"loadh failed(%d)\n", r);
		return -1;
	}
	r = verify(o->eh);
	if (r != 0) {
		Print(L"verify failed(%d)\n", r);
		return -1;
	}
	r = loadph(o->file, o->eh, &o->eph);
	if (r != 0) {
		Print(L"loadph failed(%d)\n", r);
		return -1;
	}
	r = memrange(o->eph, o->eh->e_phnum, &mmin, NULL);
	if (r != 0) {
		Print(L"memrange failed(%d)\n", r);
		return -1;
	}
	o->minaddr = mmin;
	r = loadscts(o->file, o->eh, o->eph, (VOID **)&o->img);
	if (r != 0) {
		Print(L"loadscts failed(%d)\n", r);
		return -1;
	}
	return 0;
}

int
ctx_getmmap(struct Context *o)
{
	EFI_STATUS st;
	int ok;
	int err;
	EFI_GET_MEMORY_MAP op;
	UINTN mapsz;
	EFI_MEMORY_DESCRIPTOR *map;
	UINTN dscsz;
	UINT32 dscver;

#define GMCALL(x)	uefi_call_wrapper(op, 5, &mapsz, map, &o->mmkey, &dscsz, &dscver)

	ok = err = 0;
	op = BS->GetMemoryMap;
	mapsz = 1 << 12;
	for (mapsz = 1 << 12; !ok && !err; mapsz += 1 << 12) {
		if ((st = eallocp(mapsz, (VOID **)&map)) != EFI_SUCCESS) {
			Print(L"eallocp failed(%r)\n", st);
			err = 1;
		} else if ((st = GMCALL()) == EFI_SUCCESS) {
			ok = 1;
		} else if (st == EFI_BUFFER_TOO_SMALL) {
			efreep(map);
		} else {
			Print(L"GetMemoryMap failed(%r)\n", st);
			efreep(map);
			err = 1;
		}
	}
	if (ok)
		return 0;
	else
		return -1;
}

int
ctx_exec(struct Context *o, EFI_HANDLE img)
{
	EFI_STATUS st;
	EFI_EXIT_BOOT_SERVICES op;

	op = BS->ExitBootServices;
	st = uefi_call_wrapper(op, 2, img, o->mmkey);
	if (st != EFI_SUCCESS) {
		Print(L"EFI_BOOT_SERVICES.ExitBootServices failed(%r).\n", st);
		return -1;
	}
	goto *o->eh->e_entry;
	return 0;
}
