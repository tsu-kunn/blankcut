#include "mto_common.h"


/*=======================================================================
【機能】バイナリ分割
【引数】outPath：出力先
        inPath ：保存先のバッファサイズ
        cutSize：ファイルのパス
		optq   ： 出力の抑制
 =======================================================================*/
int binary_cut(const char *outPath, const char *inPath, const sint32 cutSize, const uint8 optq)
{
	FILE *fp, *ofp;
	int i, loop;
	uint8 *mem;
	uint32 fsize, frac;

	// get output file path info
	char opath[_MAX_PATH] = {0};
	char dir[_MAX_DIR] = {0};
	char name[_MAX_FNAME] = {0};
	char ext[8] ={0};

	MtoGetFilePath(dir, sizeof(dir), outPath);
	MtoGetFileName(name, sizeof(name), inPath, 0);

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

	if (!optq) {
		printf("In file  : %s\n", inPath);
		printf("File size: %d\n", fsize);
		printf("Out path : %s\n", dir);
		printf("split    : %d\n", loop);
		printf("frac size: %d\n", frac);
		printf("\n");
	}

	// file put&out
	for (i = 0; i < (loop + 1); i++) {
		if (!optq) {
			int j, e, n;
			printf("\r%s : |", name);
			e = (i * 10) / (loop + 1);
			n = 10 - e;
			for (j = 0; j < e; j++) printf("*");
			for (j = 0; j < n; j++) printf("-");
			printf("| %3.2f%%", ((float)i * 100.0f / (float)(loop + 1)));
		}

		// read binary data
		if (i >= loop && frac) {
			fread(mem, 1, frac, fp);
		} else {
			fread(mem, 1, cutSize, fp);
		}

		// open output file & write binary data 
		sprintf(ext, "%03d", i);
		MtoMakePath(opath, sizeof(opath), dir, name, ext, DIR_MODE);
		if ((ofp = fopen(opath, "wb")) == NULL) {
			printf("%s not found!\n", opath);
			goto prog_end;
		}

		if (i >= loop && frac) {
			fwrite(mem, 1, frac, ofp);
		} else {
			fwrite(mem, 1, cutSize, ofp);
		}
		fclose(ofp);
	}

	if (!optq) {
		printf("\r%s : |**********| %3.2f%%\n", name, ((float)i * 100.0f / (float)(loop + 1)));
	}

prog_end:
	fclose(fp);
	SAFE_FREE(mem);

	return 0;
}
