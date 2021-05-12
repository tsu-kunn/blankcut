#include "mto_common.h"
#include "tim2.h"
#include "tga.h"
#include "blankcut.h"
#include "blank.h"

/*
-p 1 -h 160 -g -t s01_kur.tm2 .\output\s01_kur.blc
-p 1 -h 176 -g -t s00_ben.tm2 .\outben\s01_ben.blc
-p 1 -w 160 -h 160 -g -t s07_kae.tm2 .\outkae\s07_kae.blc
*/

/*========================================================
【機能】画像データ取得
=========================================================*/
static bool _get_picture_data(FILE *fp, uint8 *work, uint32 msize, const uint16 loop)
{
	uint32 hsize;

	if (bcut_mgr.pict == ePICT_BMP) {
		hsize = bcut_mgr.bmpfHead.bfOffBits;
	} else if (bcut_mgr.pict == ePICT_TGA) {
		hsize = TGA_HEADER_SIZE + bcut_mgr.tgaHead.IDField + bcut_mgr.bitcount * 4;
	} else {
		hsize = sizeof(TIM2_FILEHEADER) + bcut_mgr.tm2pHead.HeaderSize;
	}

	if (bcut_mgr.wh.w == bcut_mgr.pwh.w) {
		// 幅が同じ？
		if (fread(work, msize, 1, fp) != 1) {
			printf("\n画像読み込み失敗 : %d\n", loop);
			return false;
		}
	} else {
		// 幅が違うならサイズと位置を計算
		int i, nw;
		MPOINT pos;
		uint32 lsize;

		msize = GPITCH(bcut_mgr.wh.w, bcut_mgr.bit);
		lsize = GPITCH(bcut_mgr.pwh.w, bcut_mgr.bit);
		nw    = bcut_mgr.pwh.w / bcut_mgr.wh.w;
		pos.w = loop % nw;
		pos.h = loop / nw;

		// move read position
		fseek(fp, (hsize + lsize * bcut_mgr.wh.h * pos.h + msize * pos.w), SEEK_SET);

		for (i = 0; i < bcut_mgr.wh.h; i++) {
			if (fread(work, msize, 1, fp) != 1) {
				printf("\n画像読み込み失敗 : %d\n", loop);
				return false;
			}

			// next line
			work += msize;
			fseek(fp, (lsize - msize), SEEK_CUR);
		}
	}

	return true;
}

/*========================================================
【機能】余白削除後のサイズを指定倍数に補整
=========================================================*/
static bool _size_revision(BlankCutPixcelHeader *bcp_head, const uint8 rev)
{
	MPOINT txy, twh;

	// 余白を省いた画像サイズ
	twh.w = bcp_head->ofs[1].x - bcp_head->ofs[0].x;
	twh.h = bcp_head->ofs[1].h - bcp_head->ofs[0].h;

	// サイズ補整
	bcp_head->wh.w = BOUND(twh.w, rev);
	bcp_head->wh.h = BOUND(twh.h, rev);

	// 補整サイズ
	txy.x = bcp_head->wh.w - twh.w;
	txy.y = bcp_head->wh.h - twh.h;
	if (txy.x == 0 && txy.y == 0) return true;

	// 保険的にチェック
	uint8 tw = bcut_mgr.wh.w - bcp_head->ofs[1].x;
	uint8 th = bcut_mgr.wh.h - bcp_head->ofs[1].y;
	if ((bcp_head->ofs[0].x + tw) < txy.x) {
		printf("余白を省いた後の幅が不正です\n");
		return false;
	}
	if ((bcp_head->ofs[0].y + th) < txy.y) {
		printf("余白を省いた後の高さが不正です\n");
		return false;
	}

	// width
	uint8 tx[2];
	tx[0] = txy.x / 2;
	tx[1] = txy.x - tx[0];

	if (bcp_head->ofs[0].x <= tx[0]) {
		bcp_head->ofs[1].x += (txy.x - bcp_head->ofs[0].x);
		bcp_head->ofs[0].x -= txy.x - (txy.x - bcp_head->ofs[0].x);
	} else if (tw <= tx[0]) {
		bcp_head->ofs[0].x -= (txy.x - tw);
		bcp_head->ofs[1].x  = bcut_mgr.wh.w;
	} else if (bcut_mgr.optr) { // 右辺を優先に変える
		bcp_head->ofs[1].x += txy.x;
	} else {
		bcp_head->ofs[0].x -= tx[0];
		bcp_head->ofs[1].x += tx[1];
	}

	// height
	uint8 ty[2];
	ty[0] = txy.y / 2;
	ty[1] = txy.y - ty[0];

	if (bcp_head->ofs[0].y <= ty[0]) {
		bcp_head->ofs[1].y += (txy.y - bcp_head->ofs[0].y);
		bcp_head->ofs[0].y -= txy.y - (txy.y - bcp_head->ofs[0].y);
	} else if (th < ty[0]) {
		bcp_head->ofs[0].y -= (txy.y - th);
		bcp_head->ofs[1].y  = bcut_mgr.wh.h;
	} else if (bcp_head->ofs[0].y >= txy.y && bcut_mgr.optb) { // 上辺優先に変える
		bcp_head->ofs[0].y -= txy.y;
	} else {
		bcp_head->ofs[0].y -= ty[1];
		bcp_head->ofs[1].y += ty[0];
	}

	return true;
}

/*========================================================
【機能】余白位置設定
【戻値】0:error 1:cut ok 2:all blank
=========================================================*/
static int _get_blank_position(FILE *fp, uint8 *work, const uint32 msize, BlankCutPixcelHeader *bcp_head)
{
	int i, j;
	uint32 datums, cnt, pitch;

	switch (bcut_mgr.bit) {
	case 8:
		// 基準となるindex取得
		if (bcut_mgr.datums) {
			datums = *(work + msize - 1);
		} else {
			datums = *work;
		}

		// 余白検索(左上)
		cnt = 0;
		while (datums == *(work + cnt)) { // 上→下(Y)
			if (++cnt >= msize) return 2;
		}
		bcp_head->ofs[0].y = cnt / bcut_mgr.wh.w;

		cnt   = 0;
		pitch = GPITCH(bcut_mgr.wh.w, bcut_mgr.bit);
		for (i = 0; i < bcut_mgr.wh.w; i++, cnt++) { // 左→右(X)
			for (j = 0; j < bcut_mgr.wh.h; j++) {
				if (datums != *(work + i + j * pitch)) goto _cut_loop_end1;
			}
		}
_cut_loop_end1:
		bcp_head->ofs[0].x = cnt % bcut_mgr.wh.w;

		// 余白検索(右下)
		cnt = msize - 1;
		while (datums == *(work + cnt)) { // 下→上(Y)
			if (--cnt == 0) return 2;
		}
		bcp_head->ofs[1].y = cnt / bcut_mgr.wh.w + 1; // 下から調べてるので+1

		cnt = msize - 1;
		for (i = (bcut_mgr.wh.w - 1); i >= 0 ; i--, cnt--) { // 右→左(X)
			for (j = (bcut_mgr.wh.h - 1); j >= 0; j--) {
				if (datums != *(work + i + j * pitch)) goto _cut_loop_end2;
			}
		}
_cut_loop_end2:
		bcp_head->ofs[1].x = cnt % bcut_mgr.wh.w + 1;

		// 仕様上8pixcel倍数でないといけないので補整計算
		if (!_size_revision(bcp_head, 8)) return 0;
		break;
	case 32:
		// 基準となるpixcel取得
		if (bcut_mgr.datums) {
			datums = *((uint32*)(work + msize - 4));
		} else {
			datums = *((uint32*)work);
		}

		// 余白検索(左上)
		cnt = 0;
		while (datums == *((uint32*)(work + cnt))) { // 上→下(Y)
			if ((cnt += 4) >= msize) return 2;
		}
		bcp_head->ofs[0].y = (cnt / 4) / bcut_mgr.wh.w;

		cnt   = 0;
		pitch = GPITCH(bcut_mgr.wh.w, bcut_mgr.bit);
		for (i = 0; i < bcut_mgr.wh.w; i++, cnt += 4) { // 左→右(X)
			for (j = 0; j < bcut_mgr.wh.h; j++) {
				if (datums != *((uint32*)(work + i * 4 + j * pitch))) goto _cut_loop_end3;
			}
		}
_cut_loop_end3:
		bcp_head->ofs[0].x = (cnt / 4) % bcut_mgr.wh.w;

		// 余白検索(右下)
		cnt = msize - 4;
		while (datums == *((uint32*)(work + cnt))) { // 下→上(Y)
			if ((cnt -= 4) == 0) return 2;
		}
		bcp_head->ofs[1].y = (cnt / 4) / bcut_mgr.wh.w + 1; // 下から調べてるので+1

		cnt = msize - 4;
		for (i = (bcut_mgr.wh.w - 1); i >= 0 ; i--, cnt -= 4) { // 右→左(X)
			for (j = (bcut_mgr.wh.h - 1); j >= 0; j--) {
				if (datums != *((uint32*)(work + i * 4 + j * pitch))) goto _cut_loop_end4;
			}
		}
_cut_loop_end4:
		bcp_head->ofs[1].x = (cnt / 4) % bcut_mgr.wh.w + 1;

		// 仕様上2pixcel倍数でないといけないので補整計算
		if (!_size_revision(bcp_head, 2)) return 0;
		break;
	}


	return 1;
}

/*========================================================
【機能】BMPの出力
=========================================================*/
static bool _output_bmp(const uint8 *mem, const BlankCutPixcelHeader *bcp_head)
{
	FILE *fp;
	int pcount;
	uint32 bsize, hsize, isize, psize;
	BITMAPFILEHEADER bmHead;
	BITMAPINFOHEADER bmInfo;

	char tpath[_MAX_PATH] = {0};
	char tname[_MAX_PATH] = {0};
	snprintf(tname, sizeof(tname), "%s_%03d", bcut_mgr.name, bcut_mgr.bc_head.PixcelNum);
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, tname, "bmp", DIR_MODE);

	// BMP作成
	memset(&bmHead, 0, sizeof(bmHead));
	memset(&bmInfo, 0, sizeof(bmInfo));

	pcount = bcut_mgr.bit == 4 ? 16 : 256;
	if (bcut_mgr.bit >= 24) pcount = 0; // Releaseでクリティカルエラーが出るので変更

	hsize = sizeof(BITMAPFILEHEADER);
	isize = sizeof(BITMAPINFOHEADER);
	psize = sizeof(RGBAQUAD) * pcount;
	bsize = bcp_head->size + hsize + isize + psize;

	bmHead.bfType = 0x4D42; //BM
	bmHead.bfSize = bsize;
	bmHead.bfReserved1 = 0;
	bmHead.bfReserved2 = 0;
	bmHead.bfOffBits = hsize + isize + psize;

	bmInfo.biSize = isize;
	bmInfo.biWidth = bcp_head->wh.w;
	bmInfo.biHeight = bcp_head->wh.h;
	bmInfo.biPlanes = 1;
	bmInfo.biBitCount = bcut_mgr.bit;

	// BMP出力
	if (MtoFileOpen(&fp, tpath, "wb", NULL)) {
		fwrite(&bmHead, hsize, 1, fp);
		fwrite(&bmInfo, isize, 1, fp);
		if (bcut_mgr.csize) fwrite(bcut_mgr.clut, bcut_mgr.csize, 1, fp);
		fwrite(mem, bcp_head->size, 1, fp);
		fclose(fp);
	}

	return true;
}

/*========================================================
【機能】TGAの出力
=========================================================*/
static bool _output_tga(const uint8 *mem, const BlankCutPixcelHeader *bcp_head)
{
	FILE *fp;
	struct TGAHeader tgaHead;

	char tpath[_MAX_PATH] = {0};
	char tname[_MAX_PATH] = {0};
	snprintf(tname, sizeof(tname), "%s_%03d", bcut_mgr.name, bcut_mgr.bc_head.PixcelNum);
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, tname, "tga", DIR_MODE);

	// TGA作成
	tgaHead.IDField      = bcut_mgr.tgaHead.IDField;
	tgaHead.usePalette   = bcut_mgr.tgaHead.usePalette;
	tgaHead.imageType    = bcut_mgr.tgaHead.imageType;
	tgaHead.paletteIndex = bcut_mgr.tgaHead.paletteIndex;
	tgaHead.paletteColor = bcut_mgr.tgaHead.paletteColor;
	tgaHead.paletteBit   = bcut_mgr.tgaHead.paletteBit;
	tgaHead.imageX       = bcut_mgr.tgaHead.imageX;
	tgaHead.imageY       = bcut_mgr.tgaHead.imageY;
	tgaHead.imageW       = bcp_head->wh.w;
	tgaHead.imageH       = bcp_head->wh.h;
	tgaHead.imageBit     = bcut_mgr.tgaHead.imageBit;
	tgaHead.discripter   = bcut_mgr.tgaHead.discripter;

	// TGA出力(フッターなし)
	if (MtoFileOpen(&fp, tpath, "wb", NULL)) {
		// アライメントに沿っていないので個別出力
		fwrite(&tgaHead.IDField     , sizeof(tgaHead.IDField)     , 1, fp);
		fwrite(&tgaHead.usePalette  , sizeof(tgaHead.usePalette)  , 1, fp);
		fwrite(&tgaHead.imageType   , sizeof(tgaHead.imageType)   , 1, fp);
		fwrite(&tgaHead.paletteIndex, sizeof(tgaHead.paletteIndex), 1, fp);
		fwrite(&tgaHead.paletteColor, sizeof(tgaHead.paletteColor), 1, fp);
		fwrite(&tgaHead.paletteBit  , sizeof(tgaHead.paletteBit)  , 1, fp);
		fwrite(&tgaHead.imageX      , sizeof(tgaHead.imageX)      , 1, fp);
		fwrite(&tgaHead.imageY      , sizeof(tgaHead.imageY)      , 1, fp);
		fwrite(&tgaHead.imageW      , sizeof(tgaHead.imageW)      , 1, fp);
		fwrite(&tgaHead.imageH      , sizeof(tgaHead.imageH)      , 1, fp);
		fwrite(&tgaHead.imageBit    , sizeof(tgaHead.imageBit)    , 1, fp);
		fwrite(&tgaHead.discripter  , sizeof(tgaHead.discripter)  , 1, fp);

		if (bcut_mgr.csize) fwrite(bcut_mgr.clut, bcut_mgr.csize, 1, fp);
		fwrite(mem, bcp_head->size, 1, fp);
		fclose(fp);
	}

	return true;
}

/*========================================================
【機能】TIM2の出力
=========================================================*/
static bool _output_tim2(const uint8 *mem, const BlankCutPixcelHeader *bcp_head)
{
	FILE *ft2;
	TIM2_PICTUREHEADER tm2_pic;

	char tpath[_MAX_PATH] = {0};
	char tname[_MAX_PATH] = {0};
	snprintf(tname, sizeof(tname), "%s_%03d", bcut_mgr.name, bcut_mgr.bc_head.PixcelNum);
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, tname, "tm2", DIR_MODE);

	if (MtoFileOpen(&ft2, tpath, "wb", NULL)) {
		memcpy(&tm2_pic, &bcut_mgr.tm2pHead, sizeof(tm2_pic));
		tm2_pic.TotalSize   = bcp_head->size + tm2_pic.ClutSize + tm2_pic.HeaderSize;
		tm2_pic.ImageSize   = bcp_head->size;
		tm2_pic.ImageWidth  = bcp_head->wh.w;
		tm2_pic.ImageHeight = bcp_head->wh.h;

		fwrite(&bcut_mgr.tm2fHead, sizeof(TIM2_FILEHEADER), 1, ft2);
		fwrite(&tm2_pic, sizeof(tm2_pic), 1, ft2);
		fwrite(mem, bcp_head->size, 1, ft2);
		if (bcut_mgr.csize) fwrite(bcut_mgr.clut, bcut_mgr.csize, 1, ft2);
		fclose(ft2);
	}

	return true;
}

/*========================================================
【機能】余白削除
=========================================================*/
static uint8 *_blank_cut(uint8 *work, const uint32 msize, BlankCutPixcelHeader *bcp_head)
{
	int w, h;
	uint8 *mem, *mem8;
	uint32 *mem32;

	bcp_head->size = GPITCH((bcp_head->wh.w * bcp_head->wh.h), bcut_mgr.bit);
	mem = (uint8*)malloc(bcp_head->size);
	if (mem == NULL) {
		printf("can't alloc memory!!\n");
		return NULL;
	}
	mem8  = mem;
	mem32 = (uint32*)mem;

	// blank cut
	if (bcut_mgr.bit == 8) {
		for (h = 0; h < bcut_mgr.wh.h; h++) {
			if (h >= bcp_head->ofs[1].y) break;

			for (w = 0; w < bcut_mgr.wh.w; w++) {
				if (bcp_head->ofs[0].x <= w && w < bcp_head->ofs[1].x && bcp_head->ofs[0].y <= h) {
					*mem8++ = *work;
				}
				work++;
			}
		}
	} else {
		for (h = 0; h < bcut_mgr.wh.h; h++) {
			if (h >= bcp_head->ofs[1].y) break;

			for (w = 0; w < bcut_mgr.wh.w; w++) {
				if (bcp_head->ofs[0].x <= w && w < bcp_head->ofs[1].x && bcp_head->ofs[0].y <= h) {
					*mem32++ = *((uint32*)work);
				}
				work += 4;
			}
		}
	}

	// bmp/tim2 output?
	if (bcut_mgr.optg) {
		if (bcut_mgr.pict == ePICT_BMP) {
			_output_bmp(mem, bcp_head);
		} else if (bcut_mgr.pict == ePICT_TGA) {
			_output_tga(mem, bcp_head);
		} else {
			_output_tim2(mem, bcp_head);
		}
	}

	return mem;
}

/*========================================================
【機能】テーブル出力
=========================================================*/
static bool _output_table(void)
{
	if (!bcut_mgr.optt) return true; // テーブル出力なし

	int i;
	FILE *tfp;
	uint8 *mem;
	uint32 fsize;
	BlankCutPixcelHeader *bcph;

	// table open
	char tpath[_MAX_PATH] = {0};
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "tbl", DIR_MODE);
	if (!MtoFileOpen(&tfp, tpath, "w", NULL)) {
		return false;
	}

	// pixcel header
	memcls(tpath, sizeof(tpath));
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "hed", DIR_MODE);
	if ((mem = (uint8*)MtoFileRead(tpath, &fsize)) == NULL) {
		printf("can't alloc memory!!\n");
		fclose(tfp);
		return false;
	}
	bcph = (BlankCutPixcelHeader*)mem;

	// get max width and height to picture
	bcut_mgr.bc_head.SizeMax   = 0;
	bcut_mgr.bc_head.WidthMax  = 0;
	bcut_mgr.bc_head.HeightMax = 0;
	for (i = 0; i < bcut_mgr.bc_head.PixcelNum; i++) {
		if (bcut_mgr.bc_head.SizeMax   < bcph->size) bcut_mgr.bc_head.SizeMax   = bcph->size;
		if (bcut_mgr.bc_head.WidthMax  < bcph->wh.w) bcut_mgr.bc_head.WidthMax  = bcph->wh.w;
		if (bcut_mgr.bc_head.HeightMax < bcph->wh.h) bcut_mgr.bc_head.HeightMax = bcph->wh.h;
		bcph++;
	}
	bcph = (BlankCutPixcelHeader*)mem;

	// output table;
	fprintf(tfp, "// max size   : %#x\n", bcut_mgr.bc_head.SizeMax);
	fprintf(tfp, "// max widht  : %3d\n", bcut_mgr.bc_head.WidthMax);
	fprintf(tfp, "// max height : %3d\n", bcut_mgr.bc_head.HeightMax);
	fprintf(tfp, "static const BlankCutPixcelHeader _%s_tbl[] = {\n", bcut_mgr.name);
	for (i = 0; i < bcut_mgr.bc_head.PixcelNum; i++) {
		fprintf(tfp, "\t{0x%08x, 0x%08x, {%3d, %3d}, {%3d, %3d}, {{%3d, %3d}, {%3d, %3d}}, {0, 0}},\t// %3d\n",
			bcph->size, bcph->pixpos, bcph->wh.w, bcph->wh.h, bcph->fwh.w, bcph->fwh.h, 
			bcph->ofs[0].x, bcph->ofs[0].y, bcph->ofs[1].x, bcph->ofs[1].y, i);
		bcph++;
	}
	fprintf(tfp, "};\n");

	SAFE_FREE(mem);
	fclose(tfp);

	return true;
}

/*========================================================
【機能】終了処理
=========================================================*/
static void _end_process(FILE *tfp, FILE *hfp)
{
	if (tfp != NULL) fclose(tfp);
	if (hfp != NULL) fclose(hfp);

	// remove work files
	char tpath[_MAX_PATH] = {0};
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "tmp", DIR_MODE);
	remove(tpath);
	memcls(tpath, sizeof(tpath));
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "hed", DIR_MODE);
	remove(tpath);
}



/*========================================================
【機能】余白を削除し出力
=========================================================*/
bool search_blank_output(void)
{
	FILE *ofp, *tfp, *hfp;
	int ret;
	uint8 *mem, *omem;
	uint16 loop, lpmax;
	uint32 msize;
	BlankCutPixcelHeader bcp_head;

	// work file
	char tpath[_MAX_PATH] = {0};
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "tmp", DIR_MODE);
	if (!MtoFileOpen(&tfp, tpath, "wb", NULL)) {
		printf("tmpファイルが作成できません: %s\n", tpath);
		return false;
	}

	memcls(tpath, sizeof(tpath));
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "hed", DIR_MODE);
	if (!MtoFileOpen(&hfp, tpath, "wb", NULL)) {
		fclose(tfp);
		printf("hedファイルが作成できません: %s\n", tpath);
		return false;
	}

	// alloc work memory
	msize = GPITCH((bcut_mgr.wh.w * bcut_mgr.wh.h), bcut_mgr.bit);
	mem   = (uint8*)malloc(msize);
	if (mem == NULL) {
		printf("can't alloc memory!!\n");
		_end_process(tfp, hfp);
		return false;
	}
	memset(mem, 0, msize);

	lpmax = (bcut_mgr.pwh.w / bcut_mgr.wh.w) * (bcut_mgr.pwh.h / bcut_mgr.wh.h);
	loop  = 0;

	if (lpmax == 0) {
		printf("画像データ、または指定サイズが不正です\n");
		SAFE_FREE(mem);
		_end_process(tfp, hfp);
		return false;
	}

	// init hader
	memset(&bcp_head, 0, sizeof(bcp_head));
	memset(&bcut_mgr.bc_head, 0, sizeof(bcut_mgr.bc_head));
	strcpy(bcut_mgr.bc_head.id, "BLC");
	bcp_head.fwh.w = bcut_mgr.wh.w;
	bcp_head.fwh.h = bcut_mgr.wh.h;

	// set position is pixcel data
	if (bcut_mgr.pict == ePICT_BMP) {
		fseek(bcut_mgr.fp, bcut_mgr.bmpfHead.bfOffBits, SEEK_SET);
	} else if (bcut_mgr.pict == ePICT_TGA) {
		fseek(bcut_mgr.fp, (TGA_HEADER_SIZE + bcut_mgr.tgaHead.IDField + bcut_mgr.bitcount * 4), SEEK_SET);
	} else {
		fseek(bcut_mgr.fp, (sizeof(TIM2_FILEHEADER) + bcut_mgr.tm2pHead.HeaderSize), SEEK_SET);
	}

	// search & cut & output
	do {
		// processing situation
		if (!bcut_mgr.optq) {
			int i, e, n;
			printf("\r%s : |", bcut_mgr.name);
			e = (loop * 10) / lpmax;
			n = 10 - e;
			for (i = 0; i < e; i++) printf("*");
			for (i = 0; i < n; i++) printf("-");
			printf("| %3.2f%%", ((float)loop * 100.0f / (float)lpmax));
		}

		// get data
		if (!_get_picture_data(bcut_mgr.fp, mem, msize, loop)) {
			SAFE_FREE(mem);
			_end_process(tfp, hfp);
			return false;
		}

		// get blank position
		ret = _get_blank_position(bcut_mgr.fp, mem, msize, &bcp_head);
		if (ret == 0) {
			SAFE_FREE(mem);
			_end_process(tfp, hfp);
			return false;
		} else if (ret == 1) {
			// blank cut
			omem = _blank_cut(mem, msize, &bcp_head);
			if (omem == NULL) {
				SAFE_FREE(mem);
				_end_process(tfp, hfp);
				return false;
			}

			// output data
			fwrite(omem, bcp_head.size, 1, tfp);
			fwrite(&bcp_head, sizeof(bcp_head), 1, hfp);
			SAFE_FREE(omem);

			bcp_head.pixpos += bcp_head.size;
			bcut_mgr.bc_head.PixcelNum++;
		}
	} while (++loop < lpmax);

	if (!bcut_mgr.optq) {
		printf("\r%s : |**********| %3.2f%%\n", bcut_mgr.name, ((float)loop * 100.0f / (float)lpmax));
	}

	// cut end
	fclose(tfp);
	fclose(hfp);
	SAFE_FREE(mem);

	// make file header
	uint32 fsize;
	bcut_mgr.bc_head.PixcelHeadPos = sizeof(BlankCutHeader);
	bcut_mgr.bc_head.PixcelDataPos = sizeof(BlankCutHeader) + sizeof(BlankCutPixcelHeader) * bcut_mgr.bc_head.PixcelNum;

	// output table
	_output_table();

	// output file
	if (!MtoFileOpen(&ofp, bcut_mgr.path, "wb", NULL)) {
		printf("出力ファイルがオープンできませんでした。\n");
		_end_process(NULL, NULL);
		return false;
	}

	// file header
	fwrite(&bcut_mgr.bc_head, sizeof(BlankCutHeader), 1, ofp);

	// pixcel header
	memcls(tpath, sizeof(tpath));
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "hed", DIR_MODE);
	if ((mem = (uint8*)MtoFileRead(tpath, &fsize)) == NULL) {
		printf("can't alloc memory!!\n");
		_end_process(NULL, NULL);
		return false;
	}
	fwrite(mem, fsize, 1, ofp);
	SAFE_FREE(mem);

	// pixcel data
	memcls(tpath, sizeof(tpath));
	MtoMakePath(tpath, sizeof(tpath), bcut_mgr.dir, bcut_mgr.name, "tmp", DIR_MODE);
	if ((mem = (uint8*)MtoFileRead(tpath, &fsize)) == NULL) {
		printf("can't alloc memory!!\n");
		_end_process(NULL, NULL);
		return false;
	}
	fwrite(mem, fsize, 1, ofp);
	SAFE_FREE(mem);

	// end process
	_end_process(NULL, NULL);

	return true;
}
