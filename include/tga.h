#ifndef _TGA_H_
#define _TGA_H_

// イメージタイプ
enum {
	TGA_IMAGE_TYPE_NONE = 0,		// イメージなし
	TGA_IMAGE_TYPE_INDEX,			// 256色
	TGA_IMAGE_TYPE_FULL,			// フルカラー
	TGA_IMAGE_TYPE_GRAY,			// 白黒
	TGA_IMAGE_TYPE_MAX,
	TGA_IMAGE_TYPE_INDEX_RLE = 9,	// 256色(RLE圧縮)
	TGA_IMAGE_TYPE_FULL_RLE,		// フルカラー(RLE圧縮)
	TGA_IMAGE_TYPE_GRAY_RLE,		// 白黒(RLE圧縮)
	TGA_IMAGE_TYPE_RLE_MAX
};

enum {
	TGA_HEADER_SIZE = 0x12,			// ヘッダーサイズ
	TGA_FOOTER_SIZE = 0x1a			// フッターサイズ
};

struct TGAHeader {
	uint8	IDField;			// IDフィールドのサイズ
	uint8	usePalette;			// パレット使用？
	uint8	imageType;			// イメージ形式
	uint16	paletteIndex;		// パレットIndex
	uint16	paletteColor;		// パレットの色数
	uint8	paletteBit;			// 1パレットのビット数
	uint16	imageX;				// イメージX原点
	uint16	imageY;				// イメージY原点
	uint16	imageW;				// イメージ幅
	uint16	imageH;				// イメージ高さ
	uint8	imageBit;			// イメージビット数
	uint8	discripter;			// イメージ記述子
};

struct TGAFooter {
	uint32	filePos;			// ファイルの位置(エクステンションエリアの位置?)
	uint32	fileDev;			// developer directory ファイル位置
	uint8	version[18];		// ”TRUEVISION-TARGA”の文字（version[17]==0x00）
};

#endif
