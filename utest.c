#include <efi.h>
#include <efilib.h>
#include "efiboot.h"
#include <stdio.h>

void
todec_tests()
{
	int r;
	CHAR16 b[64];

	fprintf(stderr, "#todec_tests\n");
	r = todec(0, b, ARRSZ(b));
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		return;
	}
	if (b[0] != L'0' || b[1] != 0) {
		fprintf(stderr, "string(%hd, %hd)\n", b[0], b[1]);
		return;
	}
}

void
buff_not_enough_test()
{
	int r;
	CHAR16 b[2];

	fprintf(stderr, "#buff_not_enough_test\n");
	r = todec(10, b, ARRSZ(b));
	if (r != RBUFFER) {
		fprintf(stderr, "retval(%d)\n", r);
		return;
	}
}

void
one_digit_test()
{
	int r;
	CHAR16 b[64];

	fprintf(stderr, "#one_digit_test\n");
	r = todec(9, b, ARRSZ(b));
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		return;
	}
	if (b[0] != L'9' || b[1] != 0) {
		fprintf(stderr, "string(%hd, %hd)\n", b[0], b[1]);
		return;
	}
}

void
multi_digits_test()
{
	int r;
	CHAR16 b[64];

	fprintf(stderr, "#multi_digits_test\n");
	r = todec(987, b, ARRSZ(b));
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		return;
	}
	if (b[0] != L'9' || b[1] != L'8' || b[2] != L'7' || b[3] != 0) {
		fprintf(stderr, "string(%hd, %hd, %hd, %hd)\n", b[0], b[1], b[2], b[3]);
		return;
	}
}

void
minus_test()
{
	int r;
	CHAR16 b[64];

	fprintf(stderr, "#minus_test\n");
	r = todec(-10, b, ARRSZ(b));
	if (r != 0) {
		fprintf(stderr, "retval(%d)\n", r);
		return;
	}
	if (b[0] != L'-' || b[1] != L'1' || b[2] != L'0' || b[3] != 0) {
		fprintf(stderr, "string(%hd, %hd, %hd, %hd)\n", b[0], b[1], b[2], b[3]);
		return;
	}
}

void
nofpages_test()
{
	UINTN v1;

	fprintf(stderr, "#nofpages_test\n");
	v1 = 8193;
	v1 = ((unsigned)(v1 + ~(~0 << 12))) >> 12;
	if (v1 != 3) {
		fprintf(stderr, "v1(%u)\n", (unsigned)v1);
	}
}

void
align_test()
{
	int v;

	fprintf(stderr, "#align_test\n");
	v = 4097;
	v = v & (~0 << 12);
	if (v != 4096)
		fprintf(stderr, "v1(%d)\n", v);
	v = 4096;
	v = v & (~0 << 12);
	if (v != 4096)
		fprintf(stderr, "v2(%d)\n", v);
}

int
main()
{
	todec_tests();
	buff_not_enough_test();
	one_digit_test();
	multi_digits_test();
	minus_test();
	nofpages_test();
	align_test();
}
