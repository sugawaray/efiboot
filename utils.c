#include <efibind.h>
#include <efidef.h>
#include <efi.h>
#include <efilib.h>
#include "efiboot.h"

int
todec(int v, CHAR16 *b, int bsz)
{
	int i;
	int j;
	int ov;
	CHAR16 c;

	ov = v;
	if (ov < 0) {
		++b;
		--bsz;
		v = -v;
	}
	for (i = 0; i < bsz - 1; ++i) {
		b[i] = L'0' + v % 10;
		v /= 10;
		if (v == 0)
			break;
	}
	if (i == bsz - 1)
		return RBUFFER;
	j = i + 1;
	for (i = 0; i < j / 2; ++i) {
		c = b[i];
		b[i] = b[j - 1 - i];
		b[j - 1 - i] = c;
	}
	b[j] = 0;
	if (ov < 0)
		b[-1] = L'-';
	return 0;
}
