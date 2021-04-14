#include "mto_common.h"

/*=======================================================================
【機能】バイナリ分割
【引数】outPath：出力先
        inPath ：保存先のバッファサイズ
        cutSize：ファイルのパス
 =======================================================================*/
int binary_cut(const char *outPath, const char *inPath, const sint32 cutSize)
{
	FILE *fp, *ofp;
	int i, loop, out;
	uint8 *mem, *stp;
	uint32 fsize, frac;

	// get output file path info
	char opath[_MAX_PATH], dir[_MAX_DIR], name[_MAX_FNAME];

	MtoGetFilePath(dir, sizeof(dir), outPath);
	MtoGetFileName(name, sizeof(name), outPath, 0);

	// files open
	if ((fp = fopen(inPath, "rb")) == NULL) {
		printf("%s not found!\n", inPath);
		return 0;
	}

	// get file size
	fseek(fp, 0, SEEK_END);
	fsize = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// read file to memory
	if ((mem = malloc(cutSize)) == NULL) {
		printf("can't alloc memory!! : %dByte\n", cutSize);
		goto prog_end;
	}

	// get number of a loop
	loop = fsize / cutSize;
	frac = fsize % cutSize;

	printf("In file  : %s\n", inPath);
	printf("File size: %d\n", fsize);
	printf("Out path : %s\n", dir);
	printf("split    : %d\n", loop);
	printf("frac size: %d\n", frac);

#if 0
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
#endif

prog_end:
	fclose(fp);
	SAFE_FREE(mem);

	return 0;
}
