#include "mto_common.h"

typedef struct tagCompManager {
	uint32 list;	// リストのファイル数

	uint16 name;	//名前の数
	uint16 pad16;
} CompManager;

static CompManager comp_mgr;

/*========================================================
【機能】操作説明
=========================================================*/
void info_draw(void)
{
	printf("複合ツール　by.T.A\n");
	printf("comb [option] <out file> <list file>\n");
	printf("       -c : ファイルの追加結合\n");
	printf("       -n : 名前の数を数える\n");
	printf("       -s : ファイルサイズを得る\n");
	printf("       -t0 : -nで作成されたリストからテーブル作成\n");
	printf("       　1 : -sで作成されたリストからテーブル作成\n");
}

/*========================================================
【機能】オプションの有無を調べる
=========================================================*/
void check_option(int argc, char *argv[])
{
	//オプションがある？
	if (argv[1][0] == '-') {
		switch (argv[1][1]) {
			case 'c': case 'C':
				break;
			case 'n': case 'N':
				break;
			case 's': case 'S':
				break;
			case 't': case 'T':
				switch (argv[1][2]) {
					case '0':
					case '1':
						break;
					default:
						printf("-t0 -t1 を指定してください\n");
						exit(0);
						break;
				}
				break;
			case 'h': case 'H':
				info_draw();
				exit(0);
				break;
			default:
				printf("サポートされていない拡張子です\n");
				printf("『compb』または『compb -h』でヘルプ\n");
				exit(0);
				break;
		}
	} else {
		info_draw();
	}

	//オプションは一つだけ
	if (argv[2][0] == '-') {
		printf("オプションが多すぎです\n");
		exit(1);
	}
}

/*========================================================
【機能】ファイルの読み込み
=========================================================*/
void *f_read(char *fName, uint32 *size)
{
	FILE *fp;
	void *mem;
	
	//ファイルオープン
	if ((fp = fopen(fName, "rb")) == NULL) {
		printf("%sファイルがありません\n", fName);
		return NULL;
	}
	
	//ファイルサイズを調べる
	fseek(fp, 0, SEEK_END);
	*size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//ファイルをメモリに一括読み込み
	if ((mem = malloc(*size)) == NULL) {
		printf("Can't Create Memory!\n");
		fclose(fp);
		return NULL;
	}
	fread(mem, 1, *size, fp);
	fclose(fp);
	
	return mem;
}

/*========================================================
【機能】ファイル名取得
【引数】sname：ファイル名保存先
　　　　path ：フルパス
=========================================================*/
static void get_filename(char *sname, char *path)
{
	char drive[3], dir[100], ext[8];

	_splitpath(path, drive, dir, sname, ext);
}

/*========================================================
【機能】リストファイルからファイル名取得
【引数】fname：リストファイル名
　　　　num　：何番目を取得するか
=========================================================*/
char *getFileName(char *fname, uint16 num)
{
	uint8 *mem, *buf;
	uint32 i, cnt, size;

	cnt = size = 0;
	buf = (uint8*)calloc(256, sizeof(uint8));

	if ((mem = f_read(fname, &size)) == NULL) {
		printf("リストファイルがオープンできません\n");
		return NULL;
	}

	// 指定分進める
	for (i = 0; i < num; i++) {
		do {
			if (cnt >= size) {
				FREE(mem);
				return NULL;
			}
		} while (mem[++cnt] != '\n');
		cnt++;
	}

	// ファイル名取得
	i = 0;
	do {
		i++;
		if ((cnt + i) >= size) {
			FREE(mem);
			return NULL;
		}
	} while (mem[cnt + i] != '\n');
	memcpy(buf, &mem[cnt], (i - 1)); // 改行手前まで

	FREE(mem);

	return buf;
}

/*========================================================
【機能】リストファイルからファイル数取得
【引数】fname：リストファイル名
=========================================================*/
uint32 getFileNum(char *fname)
{
	uint8 *mem;
	uint32 cnt, num, size;

	cnt = num = size = 0;
	if ((mem = f_read(fname, &size)) == NULL) {
		printf("リストファイルがオープンできません\n");
		return -1;
	}

	// 数取得
	do {
		if (mem[cnt] == '\n' ) {
			num++;
		}
	} while (++cnt < size);

	FREE(mem);

	return num;
}

/*========================================================
【機能】名前の数を調べる(">"を探す)
=========================================================*/
void name_count(FILE *fp, int argc, char *argv[])
{
	int i;
	uint8 *mem, *fname;
	uint32 cnt = 0, size;

	for (i = 0; i < comp_mgr.list; i++) {
		// ファイル名取得
		if ((fname = getFileName(argv[3], (uint16)i)) == NULL) {
			FREE(fname);
			printf("リストからファイル名を取得できませんでした\n");
			return;
		}

		mem = (uint8*)f_read(fname, &size);
		if (mem == NULL) {
			FREE(fname);
			return;
		}

		//数を数える
		do {
			if (mem[cnt] == '>') {
				comp_mgr.name++;
			}
			cnt++;
		} while (cnt < size);

		//連結ファイルに書き込み
		printf("%s:%d\n", fname, comp_mgr.name);
		fprintf(fp, "%s:%d\n", fname, comp_mgr.name);

		cnt = 0;
		comp_mgr.name = 0;

		FREE(mem);
		FREE(fname);
	}
}

/*========================================================
【機能】ファイルサイズを調べる
=========================================================*/
void file_size(FILE *fp, int argc, char *argv[])
{
	int i;
	uint8 *mem, *fname;
	uint32 size;

	for (i = 0; i < comp_mgr.list; i++) {
		// ファイル名取得
		if ((fname = getFileName(argv[3], (uint16)i)) == NULL) {
			FREE(fname);
			printf("リストからファイル名を取得できませんでした\n");
			return;
		}

		// ファイルサイズの取得
		mem = (uint8*)f_read(fname, &size);
		if (mem == NULL) {
			FREE(fname);
			return;
		}

		//連結ファイルに書き込み
		printf("%s:0x%x\n", fname, size);
		fprintf(fp, "%s:%d\n", fname, size);

		FREE(mem);
		FREE(fname);
	}
}

/*========================================================
【機能】出力された名前の数からテーブル作成
=========================================================*/
void name_table(FILE *fp, char *argv[], uint8 type)
{
	char tname[64];
	uint8 *mem, str[64], buf[8];
	uint32 name, total, cfile;
	uint32 cnt, ncnt, scnt, size;

	mem = (uint8*)f_read(argv[3], &size);
	if (mem == NULL) return;
/*
	if (type == 0) {
		fprintf(fp, "static const unsigned short msg_tbl[] = {\n");
	} else {
		fprintf(fp, "static const unsigned int obj_tbl[] = {\n");
	}
*/
	memset(tname, 0, sizeof(tname));
	get_filename(tname, argv[2]);
	fprintf(fp, "static const unsigned int %s[] = {\n", tname);

	//数を数える
	total = cfile = 0;
	cnt   = ncnt  = 0;
	do {
		if (mem[cnt] == ':') {
			//名前の取得
			memset(str, 0, sizeof(str));
			memcpy(str, &mem[ncnt], (cnt - ncnt));
			//:の次の位置を保存
			scnt = cnt + 1;
		}

		if (mem[cnt] == '\n') {
			//数の取得
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &mem[scnt], (cnt - scnt));

			//文字列から整数値に変換
			name = atoi(buf);

			//ファイル書き出し
			if (type == 0) {
				fprintf(fp, "\t%6d,\t\t // %s(%d - %d)\n", total, str, total, (total + name - 1));
			} else {
				fprintf(fp, "\t%#08x,\t\t// %s\n", name, str);
			}

			//トータル計算
			total += atoi(buf);

			//次の先頭位置を保存
			ncnt = cnt + 1;
			cfile++;
		}
		cnt++;
	} while (cnt < size);

	fprintf(fp, "};\n\n");

	if (type == 0) {
		printf("total file:%d\n", total);
		printf("the number of files:%d\n", cfile);
		fprintf(fp, "//total file:%d\n", total);
		fprintf(fp, "//the number of files:%d\n", cfile);
	}

	FREE(mem);
}

/*========================================================
【機能】ファイルを結合する
=========================================================*/
void comp_file(FILE *fp, int argc, char *argv[])
{
	int i;
	uint8 *mem, *fname;
	uint32 size;

	for (i = 0; i < comp_mgr.list; i++) {
		// ファイル名取得
		if ((fname = getFileName(argv[3], (uint16)i++)) == NULL) {
			FREE(fname);
			printf("リストからファイル名を取得できませんでした\n");
			return;
		}

		mem = (uint8*)f_read(fname, &size);
		if (mem == NULL) {
			FREE(fname);
			return;
		}

		//連結ファイルに書き込み
		fwrite(mem, 1, size, fp);

		//開放
		FREE(mem);
		FREE(fname);
	}
}

/*========================================================
【機能】メイン
=========================================================*/
int main(int argc, char *argv[])
{
	FILE *fp;
	
	if (argc <= 1) {
		info_draw();
		exit(1);
	}

	//オプションの有無を調べる
	check_option(argc, argv);

	//出力ファイルのオープン
	if ((fp = fopen(argv[2], "wb")) == NULL) {
		printf("%sファイルがオープンできません\n", argv[1]);
		exit(0);
	}

	// ファイル数を取得
	comp_mgr.list = getFileNum(argv[3]);

	//オプションで処理を沸ける
	switch (argv[1][1]) {
		case 'c': case 'C':
			comp_file(fp, argc, argv);
			break;
		case 'n': case 'N':
			name_count(fp, argc, argv);
			break;
		case 's': case 'S':
			file_size(fp, argc, argv);
			break;
		case 't': case 'T':
			switch (argv[1][2]) {
				case '0':
					name_table(fp, argv, 0);
					break;
				case '1':
					name_table(fp, argv, 1);
					break;
			}
			break;
	}

	//出力ファイルを閉じる
	fclose(fp);

	return 0;
}
