#ifndef _TIM2_H_
#define _TIM2_H_

typedef struct tagTIM2_FILEHEADER {
	sint8  FileId[4];		// �t�@�C��ID('T','I','M','2'�Œ�)
	uint8  FormatVersion;	// �t�@�C���t�H�[�}�b�g�o�[�W�����ԍ�
	uint8  FormatId;		// �t�H�[�}�b�gID
	uint16 Pictures;		// �s�N�`���f�[�^�̌�
	sint8  Reserved[8];		// �p�f�B���O(0x00�Œ�)
} TIM2_FILEHEADER; //[16byte]

typedef struct tagTIM2_PICTUREHEADER {
	uint32 TotalSize;		// �s�N�`���f�[�^�S�̂̃o�C�g�T�C�Y
	uint32 ClutSize;		// CLUT�f�[�^���̃o�C�g�T�C�Y
	uint32 ImageSize;		// �C���[�W�f�[�^���̃o�C�g�T�C�Y
	uint16 HeaderSize;		// �w�b�_���̃o�C�g�T�C�Y
	uint16 ClutColors;		// CLUT�f�[�^���̃g�[�^���F��
	uint8  PictFormat;		// �s�N�`���`��ID(0�Œ�)
	uint8  MipMapTextures;	// MIPMAP�e�N�X�`������
	uint8  ClutType;		// CLUT�f�[�^�����
	uint8  ImageType;		// �C���[�W�f�[�^�����
	uint16 ImageWidth;		// �s�N�`�����T�C�Y
	uint16 ImageHeight;		// �s�N�`���c�T�C�Y
	uint64 GsTex0;			// GS TEX0���W�X�^�f�[�^
	uint64 GsTex1;			// GS TEX1���W�X�^�f�[�^
	uint32 GsRegs;			// GS TEXA,FBA,PABE���W�X�^�f�[�^
	uint32 GsTexClut;		// GS TEXCLUT���W�X�^�f�[�^
} TIM2_PICTUREHEADER; //[48byte]

typedef struct {
	uint64 GsMiptbp1;		// MIPTBP1(64�r�b�g���̂܂�)
	uint64 GsMiptbp2;		// MIPTBP2(64�r�b�g���̂܂�)
	uint32 MMImageSize[0];	// MIPMAP[?]���ڂ̉摜�o�C�g�T�C�Y
} TIM2_MIPMAPHEADER; //[20byte]


// TIM2�g���w�b�_
typedef struct {
	uint8  ExHeaderId[4];	// �g���R�����g�w�b�_ID('e','X','t','\x00')
	uint32 UserSpaceSize;	// ���[�U�[�X�y�[�X�T�C�Y
	uint32 UserDataSize;	// ���[�U�[�f�[�^�̃T�C�Y
	uint32 Reserved;		// ���U�[�u
} TIM2_EXHEADER; //[16byte]

#endif
