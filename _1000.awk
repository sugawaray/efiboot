function tonum(s,	len, i, ss, d) {
	d = 0;
	len = length(s);
	for (i = 1; i <= len; ++i) {
		ss = substr(s, i, 1);
		d = d * 16 + match("123456789abcdef", ss);
	}
	return d;
}
function output(	i) {
	for (i = 0; i < size; ++i) {
		if (i % 16 == 0)
			printf("%06x", base + i);
		printf(" 00");
		if (i % 16 == 15)
			printf("\n");
	}
}
BEGIN {
	if (base != 0)
		base = tonum(base);
	else
		base = 0;
	if (size != 0)
		size = tonum(size);
	else
		size = 0x1000;
	output();
}
