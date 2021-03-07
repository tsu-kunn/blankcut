#ifndef _TIM2_H_
#define _TIM2_H_

typedef struct tagTIM2_FILEHEADER {
	sint8  FileId[4];		// ファイルID('T','I','M','2'固定)
	uint8  FormatVersion;	// ファイルフォーマットバージョン番号
	uint8  FormatId;		// フォーマットID
	uint16 Pictures;		// ピクチャデータの個数
	sint8  Reserved[8];		// パディング(0x00固定)
} TIM2_FILEHEADER; //[16byte]

typedef struct tagTIM2_PICTUREHEADER {
	uint32 TotalSize;		// ピクチャデータ全体のバイトサイズ
	uint32 ClutSize;		// CLUTデータ部のバイトサイズ
	uint32 ImageSize;		// イメージデータ部のバイトサイズ
	uint16 HeaderSize;		// ヘッダ部のバイトサイズ
	uint16 ClutColors;		// CLUTデータ部のトータル色数
	uint8  PictFormat;		// ピクチャ形式ID(0固定)
	uint8  MipMapTextures;	// MIPMAPテクスチャ枚数
	uint8  ClutType;		// CLUTデータ部種別
	uint8  ImageType;		// イメージデータ部種別
	uint16 ImageWidth;		// ピクチャ横サイズ
	uint16 ImageHeight;		// ピクチャ縦サイズ
	uint64 GsTex0;			// GS TEX0レジスタデータ
	uint64 GsTex1;			// GS TEX1レジスタデータ
	uint32 GsRegs;			// GS TEXA,FBA,PABEレジスタデータ
	uint32 GsTexClut;		// GS TEXCLUTレジスタデータ
} TIM2_PICTUREHEADER; //[48byte]

typedef struct {
	uint64 GsMiptbp1;		// MIPTBP1(64ビットそのまま)
	uint64 GsMiptbp2;		// MIPTBP2(64ビットそのまま)
	uint32 MMImageSize[0];	// MIPMAP[?]枚目の画像バイトサイズ
} TIM2_MIPMAPHEADER; //[20byte]


// TIM2拡張ヘッダ
typedef struct {
	uint8  ExHeaderId[4];	// 拡張コメントヘッダID('e','X','t','\x00')
	uint32 UserSpaceSize;	// ユーザースペースサイズ
	uint32 UserDataSize;	// ユーザーデータのサイズ
	uint32 Reserved;		// リザーブ
} TIM2_EXHEADER; //[16byte]

#endif
