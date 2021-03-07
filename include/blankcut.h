#ifndef _BLANK_CUT_H_
#define _BLANK_CUT_H_

/*----------------------------------------------------------------------------
�@�E�t�@�C���\��
     ������������������
     ���t�@�C���w�b�_��
     ������������������
     �� Pixcel�w�b�_ ��
     ������������������
     �� Pixcel�f�[�^ ��
     ������������������
-----------------------------------------------------------------------------*/
typedef struct tagBlankCutHeader {
	char	id[4];			// [ 4] ID('B', 'L', 'C', '\0')
	uint32	PixcelNum;		// [ 4] �s�N�Z���f�[�^��
	uint32	PixcelHeadPos;	// [ 4] �s�N�Z���w�b�_�̈ʒu
	uint32	PixcelDataPos;	// [ 4] �s�N�Z���f�[�^�ʒu
	uint32	WidthMax;		// [ 4] �ő啝
	uint32	HeightMax;		// [ 4] �ő卂��
	uint32	SizeMax;		// [ 4] �ő�T�C�Y
	uint32	padding;		// [ 4]
} BlankCutHeader;			// [32byte]

typedef struct tagBlankCutPixcelHeader {
	uint32	size;			// [ 4] �f�[�^�T�C�Y
	uint32	pixpos;			// [ 4] pixcel�f�[�^�̈ʒu(pixcel�f�[�^�ʒu����)
	MPOINT	wh;				// [ 8] �摜�T�C�Y
	MPOINT	fwh;			// [ 8] ���摜�T�C�Y
	MPOINT	ofs[2];			// [16] �I�t�Z�b�g(0:���� 1:�E��)
	uint32	pad[2];			// [ 8]
} BlankCutPixcelHeader;		// [48]

typedef struct tagBlankCutManager {
	FILE	*fp;
	uint32	fsize;		// �t�@�C���T�C�Y

	uint8	*clut;		// clut data
	uint32	csize;		// clut size

	uint8	datums;		// ��_
	uint8	infile;		// ���̓t�@�C���̈ʒu
	uint8	pad0[2];

	uint8	pad1;
	uint8	bit;		// �摜�r�b�g��
	uint16	bitcount;	// �p���b�g��

	uint8	optq;		// �W���o�͂ւ̏o�͐���
	uint8	optt;		// �e�[�u���o��
	uint8	optg;		// tim2�o��
	uint8	optb:4;		// �␳�^�C�v�i���/���Ӂj
	uint8	optr:4;		// �␳�^�C�v�i����/�E�Ӂj

	MPOINT	pwh;		// �摜�T�C�Y
	MPOINT	wh;			// �`�F�b�N����T�C�Y

	TIM2_FILEHEADER    tm2fHead;
	TIM2_PICTUREHEADER tm2pHead;

	BlankCutHeader		bc_head;

	// �t�@�C���p�X�p
	char	path[_MAX_PATH];
	char	drive[_MAX_DRIVE];
	char	dir[_MAX_DIR];
	char	name[_MAX_FNAME];
	char	ext[_MAX_EXT];
} BlankCutManager;

extern BlankCutManager bcut_mgr;

#endif
