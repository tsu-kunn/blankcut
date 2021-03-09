#include "mto_common.h"


/*-------------------------------------------------------------------
y@\zoCit@CŞ
-------------------------------------------------------------------*/
int main(int argc, char *argv[])
{
	FILE *fp, *ofp;
	int i, loop, out;
	uint8 *mem, *stp;
	uint32 fsize, csize, frac;
	char opath[_MAX_PATH], drive[_MAX_DRIVE], dir[_MAX_DIR], name[_MAX_FNAME], ext[_MAX_EXT];

	// argument check
	if (argc != 3 && argc != 4) {
		printf("|||||||||||||||          BINARY CUT          |||||||||||||||\n");
		printf("bcut.exe Version 1.00                 (c)T.Araki-NOV 4 2004-\n");
		printf("\n");
		printf("bcut [cut size] [filename] [output path]\n");
		printf("cut size    : ŞTCY\n");
		printf("output path : oÍćĚpX(ČŞľ˝ęüÍĆŻśpX)\n");
		return 0;
	}

	// output path?
	out = (argc == 4 ? 1 : 0);

	// files open
	if ((fp = fopen(argv[2], "rb")) == NULL) {
		printf("%s not found!\n", argv[2]);
		return 0;
	}

	// get file size
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// get cut size
	csize = strtol(argv[1], &stp, 0);

	// read file to memory
	if ((mem = malloc(csize)) == NULL) {
		printf("can't alloc memory!!\n");
		goto prog_end;
	}

	// get file path info
	_splitpath(argv[2], drive, dir, name, ext);

	// get number of a loop
	loop = fsize / csize;
	frac = fsize % csize;

	// file put&out
	for (i = 0; i < (loop + 1); i++) {
		// read binary data
		if (i >= loop && frac) {
			fread(mem, 1, frac, fp);
		} else {
			fread(mem, 1, csize, fp);
		}

		// open output file & write binary data 
		sprintf(ext, "%03d", i);
		_makepath(opath, drive, (out ? argv[3] : dir), name, ext);
		if ((ofp = fopen(opath, "wb")) == NULL) {
			printf("%s not found!\n", opath);
			goto prog_end;
		}

		if (i >= loop && frac) {
			fwrite(mem, 1, frac, ofp);
		} else {
			fwrite(mem, 1, csize, ofp);
		}
		fclose(ofp);
	}

	printf("binary cut end : %s(%d)\n", argv[2], i);

prog_end:
	fclose(fp);
	FREE(mem);

	return 0;
}
