#include "mto_common.h"
#include "tim2.h"
#include "blankcut.h"
#include "blank.h"
#include "bcut.h"

#if defined(_USE_CRTDBG) && !defined(NDEBUG)
#include <crtdbg.h>
#endif



BlankCutManager bcut_mgr;


/*========================================================
【機能】操作説明
=========================================================*/
static void _info_draw(void)
{
	printf("|||||||||||||||          BLANK CUT          ||||||||||||||||\n");
	printf("blankcut.exe Version 1.20            (c)T.Araki-APR 16 2021-\n");
	printf("\n");
	printf("blankcut [option] [in file] [out file]\n");
	printf("    [option]\n");
	printf("       -?       : ヘルプ出力\n");
	printf("       -p [pos] : 基準位置\n");
	printf("                  ０：左上(default)\n");
	printf("                  １：右下\n");
	printf("       -w       : チェックする幅  (省略時は画像幅)\n");
	printf("       -h       : チェックする高さ(省略時は画像高さ)\n");
	printf("       -r       : 幅補正の時、右辺を優先的に変更する※\n");
	printf("       -b       : 高さ補正の時、上辺を優先的に変更する※\n");
	printf("       -t       : テーブル出力\n");
	printf("       -g       : 余白を削ったtim2/bmp出力\n");
	printf("       -c [cut] : バイナリ分割\n");
	printf("                  cut: 分割サイズ\n");
	printf("       -q       : 標準出力への出力制御\n");
	printf("\n");
	printf("[out file]は省略可能。\n");
	printf("その場合は[in file]と同じ位置に出力される。\n");
	printf("\n");
	printf("※省略時は中央揃え。\n");
}

/*========================================================
【機能】オプションの有無を調べる
=========================================================*/
static int _check_option(int argc, char *argv[])
{
	int opt = 1, dat;
	char *stp;

	// 引数なし
	if (argc == 1) {
		_info_draw();
		return 0;
	}

	// オプションがある？
	while (argv[opt][0] == '-') {
		switch (argv[opt][1]) {
		case 'p': case 'P':
			dat = strtol(argv[++opt], &stp, 0);
			if (dat < 2) {
				bcut_mgr.datums = dat;
			} else {
				printf("オプションの値が不正です。\n");
				return 0;
			}
			break;
		case 'w': case 'W':
			if (++opt >= argc) {
				printf("幅の値が指定されていません。\n");
				return 0;
			}
			bcut_mgr.wh.w = strtol(argv[opt], &stp, 0);
			break;
		case 'h': case 'H':
			if (++opt >= argc) {
				printf("高さの値が指定されていません。\n");
				return 0;
			}
			bcut_mgr.wh.h = strtol(argv[opt], &stp, 0);
			break;
		case 'r': case 'R':
			bcut_mgr.optr = true;
			break;
		case 'b': case 'B':
			bcut_mgr.optb = true;
			break;
		case 'q': case 'Q':
			bcut_mgr.optq = true;
			break;
		case 't': case 'T':
			bcut_mgr.optt = true;
			break;
		case 'g': case 'G':
			bcut_mgr.optg = true;
			break;
		case 'c': case 'C':
			if (++opt >= argc) {
				printf("バイナリ分割サイズが指定されていません。\n");
				return 0;
			}
			bcut_mgr.optc = strtol(argv[opt], &stp, 0);

			if (bcut_mgr.optc <= 0) {
				printf("バイナリカットサイズが不正です: %d\n", bcut_mgr.optc);
				return 0;
			}
			break;
		case '?':
			_info_draw();
			return 0;
		default:
			printf("サポートされていない拡張子です。\n");
			printf("『blankcut -?』でヘルプ\n");
			return 0;
		}

		if (++opt > (argc - 1)) { // -1:入力ファイル
			printf("オプションの指定が不正です。\n");
			return 0;
		}
	}

	return opt;
}

/*========================================================
【機能】終了処理
=========================================================*/
static void _release(void)
{
	SAFE_FREE(bcut_mgr.clut);

	if (bcut_mgr.fp != NULL) fclose(bcut_mgr.fp);
}

/*========================================================
【機能】Tim2ヘッダの読み込み＆チェック
=========================================================*/
static bool _read_check_tim2(void)
{
	// get tim2 header
	fread(&bcut_mgr.tm2fHead, sizeof(TIM2_FILEHEADER), 1, bcut_mgr.fp);
	fread(&bcut_mgr.tm2pHead, sizeof(TIM2_PICTUREHEADER), 1, bcut_mgr.fp);

	// check tim2
	if (!(bcut_mgr.tm2fHead.FileId[0] == 'T' && bcut_mgr.tm2fHead.FileId[1] == 'I' &&
		  bcut_mgr.tm2fHead.FileId[2] == 'M' && bcut_mgr.tm2fHead.FileId[3] == '2')) {
		printf("Tim2ではありません\n");
		return false;
	}

	// check picture size
	bcut_mgr.pwh.w = bcut_mgr.tm2pHead.ImageWidth;
	bcut_mgr.pwh.h = bcut_mgr.tm2pHead.ImageHeight;

	if ((bcut_mgr.wh.w > bcut_mgr.pwh.w) || (bcut_mgr.wh.h > bcut_mgr.pwh.h)) {
		printf("指定サイズが画像サイズを超えています\n");
		printf("width :%4d/%4d\n", bcut_mgr.wh.w, bcut_mgr.pwh.w);
		printf("height:%4d/%4d\n", bcut_mgr.wh.h, bcut_mgr.pwh.h);
		return false;
	}

	bcut_mgr.wh.w = bcut_mgr.wh.w ? bcut_mgr.wh.w : bcut_mgr.pwh.w;
	bcut_mgr.wh.h = bcut_mgr.wh.h ? bcut_mgr.wh.h : bcut_mgr.pwh.h;

	// 画像のビット数取得
	switch (bcut_mgr.tm2pHead.ImageType) {
//	case 0x01:
//		bcut_mgr.bit = 16;
//		bcut_mgr.bitcount = 0;
//		break;
//	case 0x02:
//		bcut_mgr.bit = 24;
//		bcut_mgr.bitcount = 0;
//		break;
	case 0x03:
		bcut_mgr.bit = 32;
		bcut_mgr.bitcount = 0;
		break;
//	case 0x04:
//		bcut_mgr.bit = 4;
//		bcut_mgr.bitcount = 16;
//		break;
	case 0x05:
		bcut_mgr.bit = 8;
		bcut_mgr.bitcount = 256;
		break;
	default:
		printf("対応フォーマットは8bitと32bitカラーです\n");
		return false;
	}

	return true;
}

/*========================================================
【機能】tim2のclutデータ読み込み
=========================================================*/
static bool _read_tim2_clut(void)
{
	FILE *cfp;
	uint32 fpos;

	if (bcut_mgr.bit <= 8) {
		// 現在地取得
		fpos = ftell(bcut_mgr.fp);

		bcut_mgr.csize = bcut_mgr.tm2pHead.ClutSize;
		bcut_mgr.clut  = (uint8*)malloc(bcut_mgr.csize);
		if (bcut_mgr.clut == NULL) {
			printf("can't alloc memory!!\n");
			return false;
		}

		// get clut
		uint32 clut_pos = sizeof(TIM2_FILEHEADER) +
			bcut_mgr.tm2pHead.HeaderSize + bcut_mgr.tm2pHead.ImageSize;
		fseek(bcut_mgr.fp, clut_pos, SEEK_SET);
		fread(bcut_mgr.clut, bcut_mgr.tm2pHead.ClutSize, 1, bcut_mgr.fp);

		// output clut
		char tpath[_MAX_PATH] = {0};
		MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "pal", DIR_MODE);
		if (!MtoFileOpen(&cfp, tpath, "wb", NULL)) {
			printf("CLUTが出力できません: %s\n", tpath);
			return false;
		}
		fwrite(bcut_mgr.clut, bcut_mgr.csize, 1, cfp);
		fclose(cfp);

		// 元に戻す
		fseek(bcut_mgr.fp, fpos, SEEK_SET);
	} else {
		bcut_mgr.clut  = NULL;
		bcut_mgr.csize = 0;
	}

	return true;
}

/*========================================================
【機能】BMPヘッダの読み込み＆チェック
=========================================================*/
static bool _read_check_bmp(void)
{
	// get tim2 header
	fread(&bcut_mgr.bmpfHead, sizeof(BITMAPFILEHEADER), 1, bcut_mgr.fp);
	fread(&bcut_mgr.bmpiHead, sizeof(BITMAPINFOHEADER), 1, bcut_mgr.fp);

	// check bmp
	if (bcut_mgr.bmpfHead.bfType != 0x4D42)
	{ // BMP
		printf("BMPではありません\n");
		return false;
	}

	// check picture size
	bcut_mgr.pwh.w = bcut_mgr.bmpiHead.biWidth;
	bcut_mgr.pwh.h = bcut_mgr.bmpiHead.biHeight;

	if ((bcut_mgr.wh.w > bcut_mgr.pwh.w) || (bcut_mgr.wh.h > bcut_mgr.pwh.h))
	{
		printf("指定サイズが画像サイズを超えています\n");
		printf("width :%4d/%4d\n", bcut_mgr.wh.w, bcut_mgr.pwh.w);
		printf("height:%4d/%4d\n", bcut_mgr.wh.h, bcut_mgr.pwh.h);
		return false;
	}

	bcut_mgr.wh.w = bcut_mgr.wh.w ? bcut_mgr.wh.w : bcut_mgr.pwh.w;
	bcut_mgr.wh.h = bcut_mgr.wh.h ? bcut_mgr.wh.h : bcut_mgr.pwh.h;

	// 画像のビット数取得
	switch (bcut_mgr.bmpiHead.biBitCount)
	{
		//	case 16:
		//		bcut_mgr.bit = 16;
		//		bcut_mgr.bitcount = 0;
		//		break;
		//	case 24:
		//		bcut_mgr.bit = 24;
		//		bcut_mgr.bitcount = 0;
		//		break;
	case 32:
		bcut_mgr.bit = 32;
		bcut_mgr.bitcount = 0;
		break;
		//	case 4:
		//		bcut_mgr.bit = 4;
		//		bcut_mgr.bitcount = 16;
		//		break;
	case 8:
		bcut_mgr.bit = 8;
		bcut_mgr.bitcount = 256;
		break;
	default:
		printf("対応フォーマットは8bitと32bitカラーです\n");
		return false;
	}

	return true;
}

/*========================================================
【機能】bmpのclutデータ読み込み
=========================================================*/
static bool _read_bmp_clut(void)
{
	FILE *cfp;
	uint32 fpos;

	if (bcut_mgr.bit <= 8) {
		// 現在地取得
		fpos = ftell(bcut_mgr.fp);

		bcut_mgr.csize = bcut_mgr.bitcount * 4;
		bcut_mgr.clut  = (uint8*)malloc(bcut_mgr.csize);
		if (bcut_mgr.clut == NULL) {
			printf("can't alloc memory!!\n");
			return false;
		}

		// get clut
		uint32 clut_pos = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
		fseek(bcut_mgr.fp, clut_pos, SEEK_SET);
		fread(bcut_mgr.clut, bcut_mgr.csize, 1, bcut_mgr.fp);

		// output clut
		char tpath[_MAX_PATH] = {0};
		MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "pal", DIR_MODE);
		if (!MtoFileOpen(&cfp, tpath, "wb", NULL)) {
			printf("CLUTが出力できません: %s\n", tpath);
			return false;
		}
		fwrite(bcut_mgr.clut, bcut_mgr.csize, 1, cfp);
		fclose(cfp);

		// 元に戻す
		fseek(bcut_mgr.fp, fpos, SEEK_SET);
	} else {
		bcut_mgr.clut  = NULL;
		bcut_mgr.csize = 0;
	}

	return true;
}



/*========================================================
【機能】余白削除
=========================================================*/
int main(int argc, char *argv[])
{
#if defined(_USE_CRTDBG) && !defined(NDEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | _CRTDBG_DELAY_FREE_MEM_DF);
#endif

	// init manager
	memset(&bcut_mgr, 0, sizeof(bcut_mgr));

	// argument check
	if (!(bcut_mgr.infile = _check_option(argc, argv))) {
		_release();
		return 0;
	}

	// Binary cut
	if (bcut_mgr.optc > 0) {
		if (!bcut_mgr.optq) {
			printf("Binary cut start: %dByte\n", bcut_mgr.optc);
		}
		binary_cut(argv[bcut_mgr.infile + ((bcut_mgr.infile + 1) < argc ? 1 : 0)], argv[bcut_mgr.infile], bcut_mgr.optc, bcut_mgr.optq);
		_release();
		return 0;
	}

	// file open
	if (!MtoFileOpen(&bcut_mgr.fp, argv[bcut_mgr.infile], "rb", &bcut_mgr.fsize)) {
		printf("ファイルが読み込めませんでした: %s\n", argv[bcut_mgr.infile]);
		_release();
		return 0;
	}

	// get out file position
	if ((bcut_mgr.infile + 1) < argc) {
		memcpy(bcut_mgr.path, argv[bcut_mgr.infile + 1], sizeof(bcut_mgr.path));
		MtoGetFilePath(bcut_mgr.dir, sizeof(bcut_mgr.dir), bcut_mgr.path);
		MtoGetFileName(bcut_mgr.name, sizeof(bcut_mgr.name), bcut_mgr.path, 0);
	} else {
		MtoGetFilePath(bcut_mgr.dir, sizeof(bcut_mgr.dir), argv[bcut_mgr.infile]);
		MtoGetFileName(bcut_mgr.name, sizeof(bcut_mgr.name), argv[bcut_mgr.infile], 0);

		// make file path
		MtoMakePath(bcut_mgr.path, sizeof(bcut_mgr.path), bcut_mgr.dir, bcut_mgr.name, "blc", DIR_MODE);
	}

	// check pict type
	char ext[4];
	MtoGetExtension(ext, sizeof(ext), argv[bcut_mgr.infile], 0);
	if (strcmp(ext, "tm2") == 0) {
		bcut_mgr.pict = ePICT_TIM2;
	}  else if (strcmp(ext, "bmp") == 0) {
		bcut_mgr.pict = ePICT_BMP;
	} else {
		printf("対応していない画像ファイルです: %s\n", ext);
		_release();
		return 0;
	}

	// read header & clut
	if (bcut_mgr.pict == ePICT_BMP)  {
		if (!_read_check_bmp()) {
			_release();
			return 0;
		}
		if (!_read_bmp_clut()) {
			_release();
			return 0;
		}
	} else {
		// dehault is tim2
		if (!_read_check_tim2()) {
			_release();
			return 0;
		}
		if (!_read_tim2_clut()) {
			_release();
			return 0;
		}
	}

	// search blank and cut blank
	if (!search_blank_output()) {
		_release();
		return 0;
	}

	// end
	_release();

	return 0;
}
