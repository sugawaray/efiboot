#include <efi.h>
#include <efilib.h>
EFI_STATUS
eallocpg(UINTN sz, UINT32 addr, VOID **b)
{
	EFI_STATUS st;
	EFI_ALLOCATE_PAGES op;
	EFI_PHYSICAL_ADDRESS tmp;
	UINTN npg;

	npg = ((unsigned)(sz + ~(~0 << 12))) >> 12;
	tmp = addr & (~0 << 12);

	op = BS->AllocatePages;
	st = uefi_call_wrapper(op, 4, AllocateAddress, EfiLoaderData, npg, &tmp);
	if (st != EFI_SUCCESS)
		return st;
	*b = (VOID *)tmp;
	return st;
}

EFI_STATUS
eallocp(UINTN sz, VOID **b)
{
	EFI_ALLOCATE_POOL op;

	op = BS->AllocatePool;
	return uefi_call_wrapper(op, 3, EfiLoaderData, sz, b);
}

EFI_STATUS
efreep(VOID *b)
{
	EFI_FREE_POOL op;

	op = BS->FreePool;
	return uefi_call_wrapper(op, 1, b);
}

EFI_STATUS
efread(EFI_FILE_HANDLE h, UINTN *bsz, VOID *b)
{
	EFI_FILE_READ op;

	op = h->Read;
	return uefi_call_wrapper(op, 3, h, bsz, b);
}

EFI_STATUS
efsetpos(EFI_FILE_HANDLE h, UINT64 pos)
{
	EFI_FILE_SET_POSITION op;

	op = h->SetPosition;
	return uefi_call_wrapper(op, 2, h, pos);
}

void
esetmem(VOID *p, UINTN sz, UINT8 v)
{
	SetMem(p, sz, v);
}

void
eprint(CHAR16 *fmt, ...)
{
	va_list va;
	va_start(va, fmt);
	VPrint(fmt, va);
	va_end(va);
}

