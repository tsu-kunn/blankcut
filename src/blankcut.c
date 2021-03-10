#include "mto_common.h"
#include "tim2.h"
#include "blankcut.h"
#include "blank.h"

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
	printf("blankcut.exe Version 1.03            (c)T.Araki-NOV 15 2004-\n");
	printf("\n");
	printf("blankcut [option] [in file] [out file]\n");
	printf("    [option]\n");
	printf("       -?       : ヘルプ出力\n");
	printf("       -p [pos] : 基準位置\n");
	printf("                  ０：左上(default)\n");
	printf("                  １：右下\n");
	printf("       -w       : チェックする幅　(省略時は画像幅)\n");
	printf("       -h       : チェックする高さ(省略時は画像高さ)\n");
	printf("       -r       : 幅補正の時、右辺を優先的に変更する※\n");
	printf("       -b       : 高さ補正の時、上辺を優先的に変更する※\n");
	printf("       -t       : テーブル出力\n");
	printf("       -g       : 余白を削ったtim2出力\n");
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
		case '?':
			_info_draw();
			return 0;
		default:
			printf("サポートされていない拡張子です\n");
			printf("『blankcut -?』でヘルプ\n");
			return 0;
		}

		if (++opt > (argc - 2)) { // -2:入力、出力ファイル
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
【機能】clutデータ読み込み
=========================================================*/
static bool _read_clut(void)
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
		char tpath[_MAX_PATH];
		_makepath(tpath, bcut_mgr.drive, bcut_mgr.dir, bcut_mgr.name, "pal");
		if (!fileopen(&cfp, tpath, "wb", NULL)) {
			printf("パレットが出力できません\n");
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

	// file open
	if (!fileopen(&bcut_mgr.fp, argv[bcut_mgr.infile], "rb", &bcut_mgr.fsize)) {
		_release();
		return 0;
	}

	// get out file position
	if ((bcut_mgr.infile + 1) < argc) {
		memcpy(bcut_mgr.path, argv[bcut_mgr.infile + 1], sizeof(bcut_mgr.path));
		_splitpath(bcut_mgr.path, bcut_mgr.drive, bcut_mgr.dir, bcut_mgr.name, bcut_mgr.ext);
	} else {
		// get in file path
		_splitpath(argv[bcut_mgr.infile], bcut_mgr.drive, bcut_mgr.dir, bcut_mgr.name, bcut_mgr.ext);

		// make file path
		_makepath(bcut_mgr.path, bcut_mgr.drive, bcut_mgr.dir, bcut_mgr.name, "blc");
	}

	// read tim2 header
	if (!_read_check_tim2()) {
		_release();
		return 0;
	}
	if (!_read_clut()) {
		_release();
		return 0;
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
