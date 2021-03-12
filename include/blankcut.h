#ifndef _BLANK_CUT_H_
#define _BLANK_CUT_H_

/*----------------------------------------------------------------------------
  ・ファイル構造
     ┌───────┐
     │ファイルヘッダ│
     ├───────┤
     │ Pixcelヘッダ │
     ├───────┤
     │ Pixcelデータ │
     └───────┘
-----------------------------------------------------------------------------*/
typedef struct tagBlankCutHeader {
	char	id[4];			// [ 4] ID('B', 'L', 'C', '\0')
	uint32	PixcelNum;		// [ 4] ピクセルデータ数
	uint32	PixcelHeadPos;	// [ 4] ピクセルヘッダの位置
	uint32	PixcelDataPos;	// [ 4] ピクセルデータ位置
	uint32	WidthMax;		// [ 4] 最大幅
	uint32	HeightMax;		// [ 4] 最大高さ
	uint32	SizeMax;		// [ 4] 最大サイズ
	uint32	padding;		// [ 4]
} BlankCutHeader;			// [32byte]

typedef struct tagBlankCutPixcelHeader {
	uint32	size;			// [ 4] データサイズ
	uint32	pixpos;			// [ 4] pixcelデータの位置(pixcelデータ位置から)
	MPOINT	wh;				// [ 8] 画像サイズ
	MPOINT	fwh;			// [ 8] 元画像サイズ
	MPOINT	ofs[2];			// [16] オフセット(0:左上 1:右下)
	uint32	pad[2];			// [ 8]
} BlankCutPixcelHeader;		// [48]

typedef struct tagBlankCutManager {
	FILE	*fp;
	uint32	fsize;		// ファイルサイズ

	uint8	*clut;		// clut data
	uint32	csize;		// clut size

	uint8	datums;		// 基準点
	uint8	infile;		// 入力ファイルの位置
	uint8	pad0[2];

	uint8	pad1;
	uint8	bit;		// 画像ビット数
	uint16	bitcount;	// パレット数

	uint8	optq;		// 標準出力への出力制御
	uint8	optt;		// テーブル出力
	uint8	optg;		// tim2出力
	uint8	optb:4;		// 補正タイプ（上辺/下辺）
	uint8	optr:4;		// 補正タイプ（左辺/右辺）

	MPOINT	pwh;		// 画像サイズ
	MPOINT	wh;			// チェックするサイズ

	TIM2_FILEHEADER    tm2fHead;
	TIM2_PICTUREHEADER tm2pHead;

	BlankCutHeader		bc_head;

	// ファイルパス用
	char	path[_MAX_PATH];
	char	dir[_MAX_DIR];
	char	name[_MAX_FNAME];
} BlankCutManager;

extern BlankCutManager bcut_mgr;

#endif
