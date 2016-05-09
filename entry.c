#include <efi.h>
#include <efilib.h>
#include "efiboot.h"

void *
alloc(EFI_BOOT_SERVICES *bsrv, UINTN size)
{
	EFI_STATUS st;
	EFI_ALLOCATE_PAGES ap;
	EFI_PHYSICAL_ADDRESS addr;

	if (size >= 4 * 1024) {
		Print(L"alloc: not supported operation.\n");
		return NULL;
	}
	ap = bsrv->AllocatePages;
	addr = 0;
	st = uefi_call_wrapper(ap, 4, AllocateAnyPages, EfiLoaderData, 1, &addr);
	switch (st) {
	case EFI_SUCCESS:
		return (void *)addr;
	case EFI_OUT_OF_RESOURCES:
		Print(L"AllocatePages: EFI_OUT_OF_RESOURCES.\n");
		return NULL;
	case EFI_INVALID_PARAMETER:
		Print(L"AllocatePages: EFI_INVALID_PARAMETER.\n");
		return NULL;
	case EFI_NOT_FOUND:
		Print(L"AllocatePages: EFI_NOT_FOUND.\n");
		return NULL;
	default:
		Print(L"AllocatePages: unexpected.\n");
		return NULL;
	}
}

int
findfs(EFI_BOOT_SERVICES *bsrv, EFI_HANDLE **oh, int *oc)
{
	EFI_STATUS st;
	UINTN bsz;
	EFI_LOCATE_HANDLE lh;
	EFI_HANDLE *hlist;

	lh = bsrv->LocateHandle;
	bsz = 0;
	st = uefi_call_wrapper(lh, 5, ByProtocol, &FileSystemProtocol, NULL, &bsz, NULL);
	switch (st) {
	case EFI_SUCCESS:
		Print(L"locate handle EFI_SUCCESS.\n");
		return -1;
	case EFI_NOT_FOUND:
		Print(L"locate handle EFI_NOT_FOUND.\n");
		return -1;
	case EFI_INVALID_PARAMETER:
		Print(L"locate handle EFI_INVALID_PARAMETER.\n");
		return -1;
	case EFI_BUFFER_TOO_SMALL:
		break;
	default:
		Print(L"locate handle ?.\n");
		return -1;
	}
	hlist = alloc(bsrv, bsz);
	st = uefi_call_wrapper(lh, 5, ByProtocol, &FileSystemProtocol, NULL, &bsz, hlist);
	switch (st) {
	case EFI_SUCCESS:
		Print(L"locate handle EFI_SUCCESS.\n");
		if (oh)
			*oh = hlist;
		if (oc)
			*oc = bsz / sizeof(EFI_HANDLE);
		return 0;
	default:
		Print(L"locate handle some kind of error.\n");
		return -1;
	}
}

EFI_STATUS
getloadedimg(EFI_HANDLE h, EFI_BOOT_SERVICES *bs, EFI_LOADED_IMAGE **out)
{
	EFI_STATUS st;
	EFI_OPEN_PROTOCOL f;
	void **op;
	int attr;
	
	f = bs->OpenProtocol;
	op = (void **)out;
	attr = EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL;
	st = uefi_call_wrapper(f, 6, h, &LoadedImageProtocol, op, h, NULL, attr);
	return st;
}

EFI_STATUS
freeloadedimg(EFI_HANDLE h, EFI_BOOT_SERVICES *bs, EFI_LOADED_IMAGE *proto)
{
	EFI_STATUS st;
	EFI_CLOSE_PROTOCOL f;

	f = bs->CloseProtocol;
	st = uefi_call_wrapper(f, 3, proto, &LoadedImageProtocol, h, NULL);
	return st;
}

EFI_STATUS
getloadfile(EFI_HANDLE ah, EFI_HANDLE h, EFI_BOOT_SERVICES *bs, EFI_LOAD_FILE **out)
{
	EFI_STATUS st;
	EFI_OPEN_PROTOCOL f;
	void **op;
	int attr;

	f = bs->OpenProtocol;
	op = (void **)out;
	attr = EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL;
	st = uefi_call_wrapper(f, 6, h, &LoadFileProtocol, op, ah, NULL, attr);
	return st;
}

void
printerr(CONST CHAR16 *m, EFI_STATUS st)
{
	switch (st) {
	case EFI_UNSUPPORTED:
		Print(L"EFI_UNSUPPORTED. %s\n", m);
		break;
	case EFI_NO_MEDIA:
		Print(L"EFI_NO_MEDIA. %s\n", m);
		break;
	case EFI_DEVICE_ERROR:
		Print(L"EFI_DEVICE_ERROR. %s\n", m);
		break;
	case EFI_VOLUME_CORRUPTED:
		Print(L"EFI_VOLUME_CORRUPTED. %s\n", m);
		break;
	case EFI_ACCESS_DENIED:
		Print(L"EFI_ACCESS_DENIED. %s\n", m);
		break;
	case EFI_OUT_OF_RESOURCES:
		Print(L"EFI_OUT_OF_RESOURCES. %s\n", m);
		break;
	case EFI_MEDIA_CHANGED:
		Print(L"EFI_MEDIA_CHANGED. %s\n", m);
		break;
	default:
		Print(L"printerr default. %s, %r\n", m, st);
		break;
	}
}

void
printvolinfo(EFI_BOOT_SERVICES *bsv, EFI_FILE_IO_INTERFACE *fio)
{
	EFI_STATUS st;
	EFI_VOLUME_OPEN op;
	EFI_FILE_HANDLE root;
	EFI_FILE_GET_INFO opgi;
	UINTN bsz;
	EFI_FILE_SYSTEM_INFO *info;
	EFI_FILE_INFO *info1;

	Print(L"doing printvolinfo ... \n");

	Print(L"doing OpenVolume ... \n");
	op = fio->OpenVolume;
	st = uefi_call_wrapper(op, 2, fio, &root);
	if (st != EFI_SUCCESS) {
		printerr(L"OpenVolume failed.", st);
		return;
	}
	Print(L"getting a buffer size for GetInfo ... \n");
	opgi = root->GetInfo;
	bsz = 0;
	st = uefi_call_wrapper(opgi, 4, root, &FileSystemInfo, &bsz, NULL);
	if (st != EFI_SUCCESS && st != EFI_BUFFER_TOO_SMALL) {
		printerr(L"GetInfo failed.", st);
		return;
	} else if (st == EFI_SUCCESS) {
		return;
	}
	info = alloc(bsv, bsz);
	if (info == NULL) {
		Print(L"alloc failed.\n");
		return;
	}
	info->Size = 0;
	info->ReadOnly = 0;
	info->VolumeSize = 0;
	info->FreeSpace = 0;
	info->BlockSize = 0;
	info->VolumeLabel[0] = 0;
	Print(L"doing GetInfo ... \n");
	st = uefi_call_wrapper(opgi, 4, root, &FileSystemInfo, &bsz, info);
	if (st != EFI_SUCCESS) {
		printerr(L"GetInfo with buffer failed.", st);
		return;
	}
	Print(L"VolumeSize(%ld),FreeSpace(%ld),BlockSize(%d)\n", info->VolumeSize, info->FreeSpace, info->BlockSize);
	Print(L"VolumeLabel(%s).\n", info->VolumeLabel);
	bsz = 0;
	st = uefi_call_wrapper(opgi, 4, root, &GenericFileInfo, &bsz, NULL);
	if (st != EFI_BUFFER_TOO_SMALL) {
		printerr(L"GetInfo FileInfo", st);
		return;
	}
	info1 = (void*)info;
	st = uefi_call_wrapper(opgi, 4, root, &GenericFileInfo, &bsz, info1);
	if (st != EFI_SUCCESS) {
		printerr(L"GetInfo FileInfo failed", st);
		return;
	}

	Print(L"Size(%ld), FileSize(%ld), PhysicalSize(%ld), FileName(%s)\n", info1->Size, info1->FileSize, info1->PhysicalSize, info1->FileName);
}

void
printvolsinfo(EFI_HANDLE *ih, EFI_BOOT_SERVICES *bsv, EFI_HANDLE *hs, int hc)
{
	EFI_STATUS st;
	EFI_OPEN_PROTOCOL op;
	EFI_FILE_IO_INTERFACE *iface;
	UINT32 attr;
	int i;

	Print(L"doing printvolsinfo ... \n");
	op = bsv->OpenProtocol;
	attr = EFI_OPEN_PROTOCOL_BY_HANDLE_PROTOCOL;
	for (i = 0; i < hc; ++i) {
		int succeeded = 0;
		st = uefi_call_wrapper(op, 6, hs[i], &FileSystemProtocol, (VOID **)&iface, ih, NULL, attr);
		switch (st) {
		case EFI_SUCCESS:
			succeeded = 1;
			break;
		case EFI_INVALID_PARAMETER:
			Print(L"OpenProtocol failed. Invalid parameter.\n");
			break;
		case EFI_UNSUPPORTED:
			Print(L"OpenProtocol failed. Unsupported.\n");
			break;
		}
		if (!succeeded)
			continue;
		printvolinfo(bsv, iface);
	}
}

EFI_STATUS
efi_main(EFI_HANDLE h, EFI_SYSTEM_TABLE *tab)
{
	EFI_STATUS st;
	EFI_BOOT_SERVICES *bs;
	EFI_LOADED_IMAGE *loadedimg;
	EFI_HANDLE *fsh;
	int fshc;
	int r;
	struct Context ctx;
#if 0
	int wait = 0;
#endif


	InitializeLib(h, tab);
	Print(L"InitializeLib ...\n");

	Print(L"ctx_init ...\n");
	r = ctx_init(&ctx, h);
	if (r != 0) {
		Print(L"ctx_init failed(%d).\n", r);
		return EFI_ABORTED;
	}

	bs = tab->BootServices;

	st = getloadedimg(h, bs, &loadedimg);
	if (st != EFI_SUCCESS) {
		Print(L"getloadedimg failed.\n");
		return EFI_ABORTED;
	}
	Print(L"acquired the loaded image.(0x%x)\n", loadedimg->ImageBase);

	r = findfs(bs, &fsh, &fshc);
	if (r != 0) {
		Print(L"findfs failed.\n");
		return EFI_ABORTED;
	}
	Print(L"findfs succeeded.\n");
	if (fshc != 1) {
		Print(L"filesystem count is %d(not 1). Not supported.\n", fshc);
		return EFI_ABORTED;
	}
	r = ctx_openfs(&ctx, fsh[0]);
	if (r != 0) {
		Print(L"ctx_openfs failed(%d)\n", r);
		return EFI_ABORTED;
	}
	Print(L"ctx_openfs done\n");
	r = ctx_openfile(&ctx);
	if (r != 0) {
		Print(L"ctx_openfile failed(%d).\n", r);
		return EFI_ABORTED;
	}
	Print(L"ctx_openfile done\n");
#if 0
	wait = 1;
	while (wait) {
		__asm__ __volatile__("pause");
	}
#endif
	r = ctx_loadelf(&ctx);
	if (r != 0) {
		Print(L"ctx_loadelf failed(%d).\n", r);
		return EFI_ABORTED;
	}
	Print(L"ctx_loadelf done\n");
#if 0
	wait = 1;
	while (wait) {
		__asm__ __volatile__("pause");
	}
#endif
	Print(L"image address(0x%x).\n", ctx.img);
	Print(L"entry(0x%x).\n", ctx.eh->e_entry);
	r = ctx_getmmap(&ctx);
	if (r != 0) {
		Print(L"ctx_getmmap failed(%d).\n", r);
		return EFI_ABORTED;
	}
	r = ctx_exec(&ctx, loadedimg);
	if (r != 0) {
		Print(L"ctx_exec failed(%d).\n", r);
		return EFI_ABORTED;
	}
	return EFI_SUCCESS;
}
