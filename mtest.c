#include <efi.h>
#include <efilib.h>

EFI_STATUS
efi_main(EFI_HANDLE h, EFI_SYSTEM_TABLE *tab)
{
	CHAR16 b[64];
	UINT64 v;
	UINTN r;

	v = 1;
	InitializeLib(h, tab);
	DivU64x32(1, 10, &r);
	Print(L"DivU64x32 returned.\n");
	ValueToString(b, FALSE, v);
	Print(L"ValueToString(b, FALSE, 1) -> %s\n", b);
	return EFI_SUCCESS;
}
