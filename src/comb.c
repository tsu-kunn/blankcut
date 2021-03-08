#include "mto_common.h"

typedef struct tagCompManager {
	uint32 list;	// ���X�g�̃t�@�C����

	uint16 name;	//���O�̐�
	uint16 pad16;
} CompManager;

static CompManager comp_mgr;

/*========================================================
�y�@�\�z�������
=========================================================*/
void info_draw(void)
{
	printf("�����c�[���@by.T.A\n");
	printf("comb [option] <out file> <list file>\n");
	printf("       -c : �t�@�C���̒ǉ�����\n");
	printf("       -n : ���O�̐��𐔂���\n");
	printf("       -s : �t�@�C���T�C�Y�𓾂�\n");
	printf("       -t0 : -n�ō쐬���ꂽ���X�g����e�[�u���쐬\n");
	printf("       �@1 : -s�ō쐬���ꂽ���X�g����e�[�u���쐬\n");
}

/*========================================================
�y�@�\�z�I�v�V�����̗L���𒲂ׂ�
=========================================================*/
void check_option(int argc, char *argv[])
{
	//�I�v�V����������H
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
						printf("-t0 -t1 ���w�肵�Ă�������\n");
						exit(0);
						break;
				}
				break;
			case 'h': case 'H':
				info_draw();
				exit(0);
				break;
			default:
				printf("�T�|�[�g����Ă��Ȃ��g���q�ł�\n");
				printf("�wcompb�x�܂��́wcompb -h�x�Ńw���v\n");
				exit(0);
				break;
		}
	} else {
		info_draw();
	}

	//�I�v�V�����͈����
	if (argv[2][0] == '-') {
		printf("�I�v�V�������������ł�\n");
		exit(1);
	}
}

/*========================================================
�y�@�\�z�t�@�C���̓ǂݍ���
=========================================================*/
void *f_read(char *fName, uint32 *size)
{
	FILE *fp;
	void *mem;
	
	//�t�@�C���I�[�v��
	if ((fp = fopen(fName, "rb")) == NULL) {
		printf("%s�t�@�C��������܂���\n", fName);
		return NULL;
	}
	
	//�t�@�C���T�C�Y�𒲂ׂ�
	fseek(fp, 0, SEEK_END);
	*size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	//�t�@�C�����������Ɉꊇ�ǂݍ���
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
�y�@�\�z�t�@�C�����擾
�y�����zsname�F�t�@�C�����ۑ���
�@�@�@�@path �F�t���p�X
=========================================================*/
static void get_filename(char *sname, char *path)
{
	char drive[3], dir[100], ext[8];

	_splitpath(path, drive, dir, sname, ext);
}

/*========================================================
�y�@�\�z���X�g�t�@�C������t�@�C�����擾
�y�����zfname�F���X�g�t�@�C����
�@�@�@�@num�@�F���Ԗڂ��擾���邩
=========================================================*/
char *getFileName(char *fname, uint16 num)
{
	uint8 *mem, *buf;
	uint32 i, cnt, size;

	cnt = size = 0;
	buf = (uint8*)calloc(256, sizeof(uint8));

	if ((mem = f_read(fname, &size)) == NULL) {
		printf("���X�g�t�@�C�����I�[�v���ł��܂���\n");
		return NULL;
	}

	// �w�蕪�i�߂�
	for (i = 0; i < num; i++) {
		do {
			if (cnt >= size) {
				FREE(mem);
				return NULL;
			}
		} while (mem[++cnt] != '\n');
		cnt++;
	}

	// �t�@�C�����擾
	i = 0;
	do {
		i++;
		if ((cnt + i) >= size) {
			FREE(mem);
			return NULL;
		}
	} while (mem[cnt + i] != '\n');
	memcpy(buf, &mem[cnt], (i - 1)); // ���s��O�܂�

	FREE(mem);

	return buf;
}

/*========================================================
�y�@�\�z���X�g�t�@�C������t�@�C�����擾
�y�����zfname�F���X�g�t�@�C����
=========================================================*/
uint32 getFileNum(char *fname)
{
	uint8 *mem;
	uint32 cnt, num, size;

	cnt = num = size = 0;
	if ((mem = f_read(fname, &size)) == NULL) {
		printf("���X�g�t�@�C�����I�[�v���ł��܂���\n");
		return -1;
	}

	// ���擾
	do {
		if (mem[cnt] == '\n' ) {
			num++;
		}
	} while (++cnt < size);

	FREE(mem);

	return num;
}

/*========================================================
�y�@�\�z���O�̐��𒲂ׂ�(">"��T��)
=========================================================*/
void name_count(FILE *fp, int argc, char *argv[])
{
	int i;
	uint8 *mem, *fname;
	uint32 cnt = 0, size;

	for (i = 0; i < comp_mgr.list; i++) {
		// �t�@�C�����擾
		if ((fname = getFileName(argv[3], (uint16)i)) == NULL) {
			FREE(fname);
			printf("���X�g����t�@�C�������擾�ł��܂���ł���\n");
			return;
		}

		mem = (uint8*)f_read(fname, &size);
		if (mem == NULL) {
			FREE(fname);
			return;
		}

		//���𐔂���
		do {
			if (mem[cnt] == '>') {
				comp_mgr.name++;
			}
			cnt++;
		} while (cnt < size);

		//�A���t�@�C���ɏ�������
		printf("%s:%d\n", fname, comp_mgr.name);
		fprintf(fp, "%s:%d\n", fname, comp_mgr.name);

		cnt = 0;
		comp_mgr.name = 0;

		FREE(mem);
		FREE(fname);
	}
}

/*========================================================
�y�@�\�z�t�@�C���T�C�Y�𒲂ׂ�
=========================================================*/
void file_size(FILE *fp, int argc, char *argv[])
{
	int i;
	uint8 *mem, *fname;
	uint32 size;

	for (i = 0; i < comp_mgr.list; i++) {
		// �t�@�C�����擾
		if ((fname = getFileName(argv[3], (uint16)i)) == NULL) {
			FREE(fname);
			printf("���X�g����t�@�C�������擾�ł��܂���ł���\n");
			return;
		}

		// �t�@�C���T�C�Y�̎擾
		mem = (uint8*)f_read(fname, &size);
		if (mem == NULL) {
			FREE(fname);
			return;
		}

		//�A���t�@�C���ɏ�������
		printf("%s:0x%x\n", fname, size);
		fprintf(fp, "%s:%d\n", fname, size);

		FREE(mem);
		FREE(fname);
	}
}

/*========================================================
�y�@�\�z�o�͂��ꂽ���O�̐�����e�[�u���쐬
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

	//���𐔂���
	total = cfile = 0;
	cnt   = ncnt  = 0;
	do {
		if (mem[cnt] == ':') {
			//���O�̎擾
			memset(str, 0, sizeof(str));
			memcpy(str, &mem[ncnt], (cnt - ncnt));
			//:�̎��̈ʒu��ۑ�
			scnt = cnt + 1;
		}

		if (mem[cnt] == '\n') {
			//���̎擾
			memset(buf, 0, sizeof(buf));
			memcpy(buf, &mem[scnt], (cnt - scnt));

			//�����񂩂琮���l�ɕϊ�
			name = atoi(buf);

			//�t�@�C�������o��
			if (type == 0) {
				fprintf(fp, "\t%6d,\t\t // %s(%d - %d)\n", total, str, total, (total + name - 1));
			} else {
				fprintf(fp, "\t%#08x,\t\t// %s\n", name, str);
			}

			//�g�[�^���v�Z
			total += atoi(buf);

			//���̐擪�ʒu��ۑ�
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
�y�@�\�z�t�@�C������������
=========================================================*/
void comp_file(FILE *fp, int argc, char *argv[])
{
	int i;
	uint8 *mem, *fname;
	uint32 size;

	for (i = 0; i < comp_mgr.list; i++) {
		// �t�@�C�����擾
		if ((fname = getFileName(argv[3], (uint16)i++)) == NULL) {
			FREE(fname);
			printf("���X�g����t�@�C�������擾�ł��܂���ł���\n");
			return;
		}

		mem = (uint8*)f_read(fname, &size);
		if (mem == NULL) {
			FREE(fname);
			return;
		}

		//�A���t�@�C���ɏ�������
		fwrite(mem, 1, size, fp);

		//�J��
		FREE(mem);
		FREE(fname);
	}
}

/*========================================================
�y�@�\�z���C��
=========================================================*/
int main(int argc, char *argv[])
{
	FILE *fp;
	
	if (argc <= 1) {
		info_draw();
		exit(1);
	}

	//�I�v�V�����̗L���𒲂ׂ�
	check_option(argc, argv);

	//�o�̓t�@�C���̃I�[�v��
	if ((fp = fopen(argv[2], "wb")) == NULL) {
		printf("%s�t�@�C�����I�[�v���ł��܂���\n", argv[1]);
		exit(0);
	}

	// �t�@�C�������擾
	comp_mgr.list = getFileNum(argv[3]);

	//�I�v�V�����ŏ����𕦂���
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

	//�o�̓t�@�C�������
	fclose(fp);

	return 0;
}
