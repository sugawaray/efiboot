int stub_efopen(const char *path, const char *mode);
int stub_efclose();
void stub_setmode(int mode);

enum {
	STUBMODENFILE,
	STUBMODEFILE
};
