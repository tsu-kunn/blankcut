/*=============================================================================
// Fast 2004/04/01
// Last 2008/06/24  Ver2.00                                          (c)T.Araki
=============================================================================*/
#if defined(_NDS) || defined(_PLASTATION2)
#include "cyn_application.h"
#include "cyn_common.h"
#else
#include "mto_common.h"
#include "mto_memctrl.h"
#endif

/*-------------------------------------------------------------------
�@�E�������Ǘ��֐�
�@�@�ŏ��Ɏg�p���̃������T�C�Y���m�ۂ��A���̒����烁�����̊m�ۂ��s��
�@�@�v���O�������I������Ƃ��ɉ����1��ōςށA
�@�@�������Ĕz�u�Ȃǂŗ]���ȏ���������邱�Ƃ��Ȃ��B

�@�@���Ǘ����A�A���C�����g�����̂��߁A
�@�@�@���ۂ̍ő�l�͎w���菬�����Ȃ�܂��B

�@�E�v���O��������
�@�@memctrl.init()�Ŏw��T�C�Y�̃��������m�ۂ���B
�@�@���̎�cMEM_ALIGIN�A���C�����g�Ŋm�ۂ���܂��B

�@�@�K�v�ȃ�������memctrl.alloc()�Ŏ擾���܂��B
�@�@���̎��A�w��T�C�Y(�w��A���C�����g�ɂ���Ă̓T�C�Y�ω��L��)
�@�@�{MemoryLink�����m�ۂ���܂��B
�@�@(�Ԃ��l��MemoryLink���i�񂾌�̃A�h���X�Ȃ̂ŕ��ʂ͈ӎ����Ȃ���OK)

�@�@�s�v�ɂȂ�����������memctrl.free()�ŉ�����܂��B

�@�E�X�^�b�N�^�������Ǘ�
�@�@�X�^�b�N�^�̓�����������m�ۂ��Ă����܂��B
�@�@stack_alloc()��cMEM_ALIGIN�A���C�����g�T�C�Y+sizeof(uint32)�m�ۂ��܂��B
�@�@(�Ԃ��l��sizeof(uint32)���i�񂾌�̃A�h���X�Ȃ̂ŕ��ʂ͈ӎ����Ȃ���OK)

�@�@��Ȏd�l�p�r�̓��[�N�������Ȃ̂ŊȒP�ȊǗ������s���܂���B
�@�@����͊m�ۂ������ԂƋt����memctrl.free()��
�@�@��C�ɂ��ׂĂ��������stack_reset()�̓��ނ݂̂ł��B


2004/04/26�ύX
�@�@�X�^�b�N�^���~�߂ă`�F�[���^�ɕύX�B
�@�@�m�ۂ���ĂɃA�h���X�ƃA�h���X�̋󂫗e�ʂ��`�F�b�N���A
�@�@�w��T�C�Y�����܂�Ȃ炻���ɑ}������悤�ɂ����B
�@�@����ɂ��X�^�b�N�^�̌��󂫂��}������悤�ɂȂ����B
�@�@�������A�f�Љ����i�ނƗe�ʂ������Ă��m�ۂł��Ȃ��Ȃ�̂Œ��ӁB

2005/02/09�ύX
�@�@�������̊m�ې��̃`�F�b�N���s���悤�ɂ����B
�@�@����ɂ�胁�����Ǘ����̃��������[�N�𑽏����o���₷���Ȃ�B
�@�@define:memalloc��ǉ��B

2005/10/27�ύX
�@�@����A�g���r���[�g�Areset�֐��ǉ��B
�@�@�A���C�����g���w�肵�Ċm�ۂ�allocEx�ɕύX�B
�@�@NDS��p�̊֐��ǉ����A���P�[�^�[�ɑΉ��B

2005/11/01�ǉ�
�@�@�X�^�b�N�^�̃������Ǘ��@�\�ǉ��B

2006/06/23�ύX
�@�@���A�h���X��ݒ�A���C�����g�ɍ��킹�A�m�ۂ��郁������
�@�@�ݒ�A���C�����g�P�ʂŊm�ۂ���悤�ɕύX�B(��psrc�ǉ�)
�@�@����ɂƂ��Ȃ�allocEx���폜�B

2006/09/08
�@�@Warning�������C���B

2006/09/12
�@�@����������Ƃ����荞�݂��֎~�ɂ����B�iDS����)

2006/09/29
�@�@stack_alloc�̊m�ۃT�C�Y�o�E���f�B���O���ɂ�
�@�@�w�b�_�[�T�C�Y���܂߂Ă��Ȃ������_���C���B

2006/10/31
�@�@�F���w�b�_�[��ǉ����ĉ�������𓝈�B
�@�@���̍�ƂɂƂ��Ȃ�StackHeader��ID��ǉ��B
�@�@����ɂƂ��Ȃ�stack_free���폜�B

2006/11/07
�@�@�f�o�b�O�o�͐ݒ���s���f�o�b�O�֐��ǉ��B

2007/01/20
�@�@reset()�ŃX�^�b�N�����������Z�b�g���Ă��Ȃ������_���C���B
�@�@���������[�N�������Ԃ�release()���Ă񂾏ꍇ�A
�@�@���[�N�T�C�Y���o�͂���悤�ɂ����B

2007/10/17
�@�@���Ŋm�ۂ������������珉�����ł���悤initEx�֐���ǉ��B
�@�@�������̏I�[�A�h���X�ݒ�Ńo�O���������̂��C���B

2008/01/11
�@�@DS��C++�Ή��ɂƂ��Ȃ��G���[�C���B

2008/06/04
�@�@mem_mgr.stack�̊J�n�A�h���X���w��A���C�����g��
�@�@�Ȃ��Ă��Ȃ������s����C���B
�@�@�Ǘ��pMemoryLink��attr�����ݒ肾�����s����C���B
�@�@����ɂƂ��Ȃ��ueMEM_SYSTEM�v��ǉ��B

2008/06/24
�@�@mtolib�쐬�ɍ��킹�ăt�@�C�����ύX�B
�@�@����ɉ����ăo�[�W������2.00�ɕύX�B
�@�@��{��C++�Ή������ǁAmtolib�ł�C����ō쐬�B
-------------------------------------------------------------------*/
typedef struct tagMemoryLink {
	void  *next;		// ���̃����N�A�h���X
	uint32 head;		// ���ʃw�b�_�[�i�Œ�j
	uint32 size;		// �m�ۃ������T�C�Y
	uint32 attr;		// ����A�g���r���[�g
} MemoryLink;

typedef struct tagStackHeader {
	uint32 size;		// �m�ۃ������T�C�Y
	uint32 head;		// �F���w�b�_�[�i�Œ�j
} StatckHeader;

#define LINK_SIZE		(BOUND(sizeof(MemoryLink), cMEM_ALIGIN))
#define STACK_SIZE		(BOUND(sizeof(StatckHeader), cMEM_ALIGIN))
#define LINK_ID			0x004c4e4b // 'LNK'
#define STACK_ID		0x0053544b // 'STK'


typedef struct tagMemoryManager {
	uint8  *psrc;		// ���A�h���X
	uint8  *addr;		// �A���C�����g��̃A�h���X
	uint8  *pos;		// ���݂̃A�h���X
	uint8  *end;		// �I�[�A�h���X
	uint8  *stack;		// �X�^�b�N�̃A�h���X

	uint32 size;		// �������T�C�Y
	sint32 mnum;		// �������m�ې�
	MemoryLink *link;	// �����N�p�\����

	sint32 ex_flg;		// initEx�ŏ������H

#ifndef NDEBUG
	sint32 dbgmsg;		// �f�o�b�O���b�Z�[�W�o�́H
#endif

#ifdef _NDS
	NNSFndAllocator allocator;
#endif
} MemoryManager;

static MemoryManager mem_mgr;

#ifndef NDEBUG
#define DBG_PRINT_SIZE(asize, addr)	\
	if (!mem_mgr.dbgmsg) { \
		char str[256]; \
		sprintf(str, "alloc size:%d(%#x) addr:%d(%#x)\n", asize, asize, addr, addr); \
		DBG_PRINT(str); \
	}
#else
#define DBG_PRINT_SIZE(asize, addr)	
#endif



/*-------------------------------------------------------------------
�y�@�\�z�f�o�b�O���b�Z�[�W�o�͐ݒ�
�y�����zdbgmsg�F�o�́H
-------------------------------------------------------------------*/
static void _memory_set_dbgmsg(const sint32 dbgmsg)
{
#ifndef NDEBUG
	mem_mgr.dbgmsg = !dbgmsg;
#else
	NOTHING(dbgmsg);
#endif
}

/*-------------------------------------------------------------------
�y�@�\�z�������`�F�b�N
�y���l�z���������K��͈͓��ɂ��邩���`�F�b�N�i�͈͊O�͒�~�j
-------------------------------------------------------------------*/
static void _memory_memory_check(const void *cmem)
{
	uint8 *mem = (uint8*)cmem;
	MemoryLink *link = (MemoryLink*)cmem;

	// NULL�Ȃ�`�F�b�N���Ȃ�
	if (cmem == NULL) return;

	if (mem < mem_mgr.addr || mem > mem_mgr.pos || (uint8*)link->next > mem_mgr.pos || (link->next != NULL && mem > (uint8*)link->next))
	{
#ifndef NDEBUG
		DBG_ASSERT(0, "memory release error!!");
#else
#ifdef _WINDOWS_
		{
			char str[256];
			memset(str, 0, sizeof(str));
			sprintf(str, "�w�胁�������s���ł�\ncmem:0x%x\naddr:0x%x\nend :0x%x",
					(uint32)cmem, (uint32)mem_mgr.addr, (uint32)mem_mgr.end);
			// Windows�Ȃ�G���[�\�L
			MessageBox(NULL, str, "Error", MB_ICONERROR | MB_OK);
		}
#endif
		while (1); // �������[�v�Ŏ~�߂�
#endif
	}
}

/*-------------------------------------------------------------------
�y�@�\�z������������
�y�����zsize:�������m�ۃT�C�Y
-------------------------------------------------------------------*/
static sint32 _memory_init(const uint32 size)
{
	uint32 alloc_size;
	uint32 stack_size;

	memset(&mem_mgr, 0, sizeof(mem_mgr));

#if defined(_USE_MEM_CTRL) && !defined(_NDS)
	// memory alloc
	alloc_size = BOUND((size + cMEM_ALIGIN), cMEM_ALIGIN);
	if ((mem_mgr.psrc = (uint8*)malloc(alloc_size)) == NULL) {
#ifndef NDEBUG
		char buf[64];
		sprintf(buf, "can't alloc memory!! : %d byte\n", size);
		DBG_PRINT(buf);
#endif
		return FALSE;
	}

	mem_mgr.addr  = (uint8*)BOUND(mem_mgr.psrc, cMEM_ALIGIN);
	mem_mgr.pos   = mem_mgr.addr + LINK_SIZE;
	mem_mgr.end   = mem_mgr.psrc + size;
	mem_mgr.size  = size;
	mem_mgr.link  = (MemoryLink*)mem_mgr.addr;
	mem_mgr.link->next = NULL;
	mem_mgr.link->size = LINK_SIZE;
	mem_mgr.link->attr = eMEM_SYSTEM;
	mem_mgr.ex_flg = FALSE;

	// stack_alloc�̊J�n�ʒu���w��A���C�����g�ɂȂ�悤�ɒ���
	stack_size = BOUND(mem_mgr.end, cMEM_ALIGIN);
	if ((uint32)mem_mgr.end != stack_size) {
		mem_mgr.stack = (uint8*)(stack_size - cMEM_ALIGIN);
	} else {
		mem_mgr.stack = mem_mgr.end;
	}
#else
	NOTHING(size);
	NOTHING(alloc_size);
	NOTHING(stack_size);
#endif

	return TRUE;
}

/*-------------------------------------------------------------------
�y�@�\�z���Ŋm�ۂ����������ŏ�����
�y�����zaddr:�������A�h���X
        size:�������T�C�Y
-------------------------------------------------------------------*/
static sint32 _memory_initEx(void *addr, const uint32 size)
{
	uint32 stack_size;

	memset(&mem_mgr, 0, sizeof(mem_mgr));

	if (addr == NULL) {
#ifndef NDEBUG
		_ASSERT(0);
#endif
		return FALSE;
	}

	mem_mgr.psrc  = (uint8*)addr;
	mem_mgr.addr  = (uint8*)BOUND(mem_mgr.psrc, cMEM_ALIGIN);
	mem_mgr.pos   = mem_mgr.addr + LINK_SIZE;
	mem_mgr.end   = mem_mgr.psrc + size;
	mem_mgr.size  = size;
	mem_mgr.link  = (MemoryLink*)mem_mgr.addr;
	mem_mgr.link->next = NULL;
	mem_mgr.link->size = LINK_SIZE;
	mem_mgr.link->attr = eMEM_SYSTEM;
	mem_mgr.ex_flg = TRUE;

	// stack_alloc�̊J�n�ʒu���w��A���C�����g�ɂȂ�悤�ɒ���
	stack_size = BOUND(mem_mgr.end, cMEM_ALIGIN);
	if ((uint32)mem_mgr.end != stack_size) {
		mem_mgr.stack = (uint8*)(stack_size - cMEM_ALIGIN);
	} else {
		mem_mgr.stack = mem_mgr.end;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
�y�@�\�z�������m��
�y�����zsize  :�m�ۃT�C�Y
�@�@�@�@attr  :����A�g���r���[�g
�y���l�zcMEM_ALIGIN�A���C�����g�Œ�B
        LINK_SIZE�����m�ۂ��A������MemoryLink�𖄂ߍ���
-------------------------------------------------------------------*/
static void *_memory_alloc(uint32 size, const uint32 attr)
{
	uint8 *mem;

#ifdef _USE_MEM_CTRL
	uint32 mfree;
	MemoryLink *tlink, *mlink, *slink;

#ifdef _NDS
	OSIntrMode old = OS_DisableInterrupts();
#endif

#ifndef NDEBUG
	DBG_ASSERT(attr <= eMEM_UNLOCK, "attrib error!!");
#endif

	// �w��A���C�����g�P�ʂŊm��
	size = BOUND(size, cMEM_ALIGIN) + LINK_SIZE;

	// �󂢂Ă���ʒu��T��
	mlink = mem_mgr.link;
	while (mlink != NULL) {
		if (mlink->next == NULL) break; // �I�[

		mfree = (uint32)mlink->next - ((uint32)mlink + mlink->size);
		if (size <= mfree) { // �󂫔���
			// save next link addres
			tlink = (MemoryLink*)mlink->next;
			mem   = (uint8*)mlink + mlink->size;

			// address & size save
			slink = (MemoryLink*)mem;
			slink->size = size;
			slink->attr = attr;
			slink->head = LINK_ID;
			mem += LINK_SIZE;

			// Insert Link
			mlink->next = slink;
			slink->next = tlink;

			// add memory alloc num
			mem_mgr.mnum++;
			DBG_PRINT_SIZE(size, mem);

#ifdef _NDS
			OS_RestoreInterrupts(old);
#endif
			return (void*)mem;
		}
		mlink = (MemoryLink*)mlink->next;
		_memory_memory_check(mlink);
	}

	// �V�K�m��
	// memory free ok?
	if ((uint32)(mem_mgr.stack - mem_mgr.pos) < size) {
#ifndef NDEBUG
		char buf[64];
		sprintf(buf, "not free size!! %d byte\n", size);
		DBG_PRINT(buf);
#endif
#ifdef _NDS
		OS_RestoreInterrupts(old);
#endif
		return NULL;
	}

	// set address
	mem = mem_mgr.pos;

	// address & size save & attrib
	slink = (MemoryLink*)mem;
	slink->next = NULL;
	slink->size = size;
	slink->attr = attr;
	slink->head = LINK_ID;
	mem += LINK_SIZE;

	// pos address move
	mem_mgr.pos += size;
	mlink->next  = slink;
#else
	mem = (uint8*)malloc(BOUND(size, cMEM_ALIGIN));
#endif

	// add memory alloc num
	mem_mgr.mnum++;
	DBG_PRINT_SIZE(size, mem);

#ifdef _NDS
	OS_RestoreInterrupts(old);
#endif

	return (void*)mem;
}

/*-------------------------------------------------------------------
�y�@�\�z���������
�y�����zaddr:�������A�h���X
-------------------------------------------------------------------*/
static sint32 _memory_free(const void *addr)
{
#ifdef _USE_MEM_CTRL
	uint8 *mem;
	MemoryLink *mlink, *slink;
	StatckHeader *stack;
#ifdef _NDS
	OSIntrMode old;
#endif


	// stack memory ?
	stack = (StatckHeader*)((uint8*)addr - STACK_SIZE);
	if (stack->head == STACK_ID) {
		uint32 size;

#ifdef _NDS
		old = OS_DisableInterrupts();
#endif

		// �m�ۃT�C�Y�擾
		size = *((uint32*)((uint8*)addr - STACK_SIZE));
		mem_mgr.stack += size;

#ifndef NDEBUG
		DBG_ASSERT(((uint32)mem_mgr.stack <= (uint32)mem_mgr.end), "stack memory size error!!");
#endif
#ifdef _NDS
		OS_RestoreInterrupts(old);
#endif
		return TRUE;
	}

	// link memory
#ifdef _NDS
	old = OS_DisableInterrupts();
#endif
	mem   = (uint8*)addr - LINK_SIZE;
	slink = (MemoryLink*)mem;

	// memory release ok ?
	_memory_memory_check(mem);

	// serch addres
	mlink = mem_mgr.link;
	while (mlink != NULL) {
		if (slink == mlink->next) {
			if (slink->next == NULL) { // it end�H
				mem_mgr.pos = (uint8*)mlink + mlink->size;
			}
			mlink->next = (MemoryLink*)slink->next;
			mem_mgr.mnum--;
#ifndef NDEBUG
			if (mem_mgr.mnum < 0) {
				DBG_ASSERT(0, "memory free over!!");
			}
#endif
#ifdef _NDS
			OS_RestoreInterrupts(old);
#endif
			return TRUE;
		}
		mlink = (MemoryLink*)mlink->next;
		_memory_memory_check(mlink);
	}

	// release error
#ifndef NDEBUG
	DBG_ASSERT(0, "memory release error!!");
#endif
#ifdef _NDS
	OS_RestoreInterrupts(old);
#endif

	return FALSE;
#else
	FREE(addr);
	mem_mgr.mnum--;
#ifndef NDEBUG
	if (mem_mgr.mnum < 0) {
		DBG_ASSERT(0, "memory free over!!");
	}
#endif //NDEBUG

	return TRUE;
#endif
}

/*-------------------------------------------------------------------
�y�@�\�z�������̋󂫗e��
-------------------------------------------------------------------*/
static uint32 _memory_get_quantity(void)
{
	uint32 mfree;
	MemoryLink *mlink;

	// ��{
	mfree = (uint32)(mem_mgr.stack - mem_mgr.pos);

	// �Ԃɂ���󂫂����Z
	mlink = mem_mgr.link;
	while (mlink != NULL) {
		if (mlink->next == NULL) break; // �I�[
		mfree += (uint32)mlink->next - ((uint32)mlink + mlink->size);
		mlink = (MemoryLink*)mlink->next;
	}

	return mfree;
}

/*-------------------------------------------------------------------
�y�@�\�z�f�Љ������������̋󂫗e��
-------------------------------------------------------------------*/
static uint32 _memory_get_fragment(void)
{
	uint32 mfree;
	MemoryLink *mlink;

	// ��{
	mfree = 0;

	// �Ԃɂ���󂫂����Z
	mlink = mem_mgr.link;
	while (mlink != NULL) {
		if (mlink->next == NULL) break; // �I�[
		mfree += (uint32)mlink->next - ((uint32)mlink + mlink->size);
		mlink = (MemoryLink*)mlink->next;
	}

	return mfree;
}

/*-------------------------------------------------------------------
�y�@�\�zeMEM_UNLOCK���������
�y���l�z���̊֐���ǂ񂾌�A���ɉ������Ă���A�h���X��free���ĂԂ�
�@�@�@�@Error�ɂȂ�̂Œ��ӁB�i�������l�����j
-------------------------------------------------------------------*/
static void _memory_reset(void)
{
#ifdef _USE_MEM_CTRL
	MemoryLink *mlink, *slink;

#ifdef _NDS
	OSIntrMode old = OS_DisableInterrupts();
#endif

	mlink = mem_mgr.link;
	slink = (MemoryLink*)mlink->next;

	// serch attrib eMEM_UNLOCK
	while (mlink != NULL && slink != NULL) {
		if (slink->attr == eMEM_UNLOCK) {
			if (slink->next == NULL) { // it end�H
				mem_mgr.pos = (uint8*)mlink + mlink->size;
			}
			mlink->next = (MemoryLink*)slink->next;
			mem_mgr.mnum--;

#ifndef NDEBUG
			if (mem_mgr.mnum < 0) {
				DBG_ASSERT(0, "memory free over!!");
			}
#endif
			// next link
			slink = (MemoryLink*)mlink->next;
			_memory_memory_check(slink);
		} else {
			mlink = slink;
			slink = (MemoryLink*)mlink->next;
			_memory_memory_check(slink);
		}
	}

	// �X�^�b�N���������Z�b�g
	mem_mgr.stack = mem_mgr.end;

#ifdef _NDS
	OS_RestoreInterrupts(old);
#endif
#endif
}

/*-------------------------------------------------------------------
�y�@�\�z���������
-------------------------------------------------------------------*/
static void _memory_release(void)
{
#ifndef NDEBUG
	if (mem_mgr.mnum != 0) {
		int i;
		char buf[128];
		MemoryLink *mlink;

		sprintf(buf, "--- memory leak!!:%d ---\n", mem_mgr.mnum);
		DBG_PRINT(buf);

		i = 0;
		mlink = mem_mgr.link;
		while (mlink != NULL) {
			sprintf(buf, " %3d:LeakSize:%d(%#x) attr:%d\n", i, mlink->size, mlink->size, mlink->attr);
			DBG_PRINT(buf);
			mlink = (MemoryLink*)mlink->next;
			i++;
		}

		if (mem_mgr.stack != mem_mgr.end) {
			uint32 leak = (uint32)(mem_mgr.end - mem_mgr.stack);
			sprintf(buf, " StackLeak:%d(%#x)\n", leak, leak);
			DBG_PRINT(buf);
		}
	}
#endif // NDEBUG

#if defined(_USE_MEM_CTRL) && !defined(_NDS)
	if (mem_mgr.psrc != NULL) {
		if (!mem_mgr.ex_flg) {
			free(mem_mgr.psrc);
		}
		mem_mgr.psrc = NULL;
		mem_mgr.addr = NULL;
	}
#endif
}



	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/



/*-------------------------------------------------------------------
�y�@�\�z�X�^�b�N�������m��
�y�����zsize  :�m�ۃT�C�Y
-------------------------------------------------------------------*/
static void *_memory_stack_alloc(uint32 size)
{
	void *ptr = NULL;
	StatckHeader *stack;

#ifdef _NDS
	OSIntrMode old = OS_DisableInterrupts();
#endif

	// �w��A���C�����g�P�ʂŊm��
	size = BOUND(size, cMEM_ALIGIN) + STACK_SIZE;

	// �������m�ۉ\�H
	if (((uint32)mem_mgr.stack - size) > (uint32)mem_mgr.pos) {
		mem_mgr.stack -= size;
		stack = (StatckHeader*)mem_mgr.stack;
		stack->size = size;
		stack->head = STACK_ID;
		DBG_PRINT_SIZE(size, ((uint8*)mem_mgr.stack + STACK_SIZE));

		ptr = (void*)((uint8*)mem_mgr.stack + STACK_SIZE);
	}

#ifdef _NDS
	OS_RestoreInterrupts(old);
#endif
	return ptr;
}

/*-------------------------------------------------------------------
�y�@�\�z�X�^�b�N���������Z�b�g
�y���l�z�X�^�b�N�ʒu�������ʒu�ɖ߂��܂��̂ŁA
�@�@�@�@���Ɋm�ۂ����������͎g�p���Ȃ���stack_free()�ɓn���Ȃ�����!!
-------------------------------------------------------------------*/
static void _memory_stack_rest(void)
{
#ifndef NDEBUG
	DBG_PRINT("stack position inislize.\n");
#endif
	mem_mgr.stack = mem_mgr.end;
}



	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/



#ifdef _NDS
/*-------------------------------------------------------------------
�y�@�\�zNDS�p�������m��
-------------------------------------------------------------------*/
static void *_memory_nds_alloc(NNSFndAllocator *pAllocator, uint32 size)
{
	NOTHING(pAllocator);
	return _memory_alloc(size, eMEM_LOCK);
}

/*-------------------------------------------------------------------
�y�@�\�zNDS�p���������
-------------------------------------------------------------------*/
static void _memory_nds_free(NNSFndAllocator *pAllocator, void *addr)
{
	NOTHING(pAllocator);
	_memory_free(addr);
}

/*-------------------------------------------------------------------
�y�@�\�zNDS�p������������
-------------------------------------------------------------------*/
static sint32 _memory_nds_init(void)
{
	u32 arenaLow   = ROUND_UP  (OS_GetMainArenaLo(), cMEM_ALIGIN);
	u32 arenaHigh  = ROUND_DOWN(OS_GetMainArenaHi(), cMEM_ALIGIN);
	u32 heapSize   = arenaHigh - arenaLow;
	u32 stack_size;

	static const NNSFndAllocatorFunc sAllocatorFunc = {
		_memory_nds_alloc,
		_memory_nds_free,
	};

	// Initalize MemoryManager
	 MI_CpuFill8(&mem_mgr, 0, sizeof(mem_mgr));

	// memory alloc
	mem_mgr.psrc = (uint8*)OS_AllocFromMainArenaLo(heapSize, cMEM_ALIGIN);
	if (mem_mgr.psrc == NULL) {
#ifndef NDEBUG
		SDK_ASSERTMSG(0, "can't alloc memory!! : %d byte\n", heapSize);
#endif
		return FALSE;
	}

#ifndef NDEBUG
	DBG_PRINT("\n------ _memory_nds_init ------\n");
	DBG_PRINT("arenaLow :%12d\n", arenaLow);
	DBG_PRINT("arenaHigh:%12d\n", arenaHigh);
	DBG_PRINT("heapSize :%12d\n", heapSize);
	DBG_PRINT("heapSize :%12d\n", (uint32)mem_mgr.psrc);
	DBG_PRINT("------------------------------\n\n");
#endif

	// allocator init
	mem_mgr.allocator.pFunc = &sAllocatorFunc;
	mem_mgr.allocator.pHeap = NULL;
	mem_mgr.allocator.heapParam1 = (u32)cMEM_ALIGIN;
	mem_mgr.allocator.heapParam2 = 0; // no use

	// �������Ǘ��}�l�[�W���[������
	mem_mgr.addr  = mem_mgr.psrc; // �A���C�����g����Ă���
	mem_mgr.pos   = mem_mgr.addr + LINK_SIZE;
	mem_mgr.end   = mem_mgr.psrc + heapSize;
	mem_mgr.size  = heapSize;
	mem_mgr.link  = (MemoryLink*)mem_mgr.addr;
	mem_mgr.link->next = NULL;
	mem_mgr.link->size = LINK_SIZE;

	// stack_alloc�̊J�n�ʒu���w��A���C�����g�ɂȂ�悤�ɒ���
	stack_size = BOUND(mem_mgr.end, cMEM_ALIGIN);
	if ((uint32)mem_mgr.end != stack_size) {
		mem_mgr.stack = (uint8*)(stack_size - cMEM_ALIGIN);
	} else {
		mem_mgr.stack = mem_mgr.end;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
�y�@�\�zNDS�p�A���P�[�^�[�\���̎擾
-------------------------------------------------------------------*/
static NNSFndAllocator _memory_nds_get_allocator(NNSFndAllocator *pAllocator)
{
	if (pAllocator != NULL) {
		MI_CpuCopy8(&mem_mgr.allocator, pAllocator, sizeof(mem_mgr.allocator));
	}

	return mem_mgr.allocator;
}
#endif // _NDS



	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/
	/*------------------------------------------------------------*/



const MemoryControl memctrl = {
	_memory_init,
	_memory_initEx,

	_memory_alloc,
	_memory_free,

	_memory_get_quantity,
	_memory_get_fragment,

	_memory_reset,
	_memory_release,

	_memory_stack_alloc,
	_memory_stack_rest,

	_memory_memory_check,
	_memory_set_dbgmsg,

#ifdef _NDS
	_memory_nds_init,
	_memory_nds_alloc,
	_memory_nds_free,
	_memory_nds_get_allocator,
#endif
};
