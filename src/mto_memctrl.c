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
　・メモリ管理関数
　　最初に使用分のメモリサイズを確保し、その中からメモリの確保を行う
　　プログラムを終了するときに解放が1回で済む、
　　メモリ再配置などで余分な処理を取られることがない。

　　※管理情報、アライメント処理のため、
　　　実際の最大値は指定より小さくなります。

　・プログラム説明
　　memctrl.init()で指定サイズのメモリを確保する。
　　この時cMEM_ALIGINアライメントで確保されます。

　　必要なメモリはmemctrl.alloc()で取得します。
　　この時、指定サイズ(指定アライメントによってはサイズ変化有り)
　　＋MemoryLink多く確保されます。
　　(返す値はMemoryLink分進んだ後のアドレスなので普通は意識しなくてOK)

　　不要になったメモリはmemctrl.free()で解放します。

　・スタック型メモリ管理
　　スタック型はメモリ後方より確保していきます。
　　stack_alloc()でcMEM_ALIGINアライメントサイズ+sizeof(uint32)確保します。
　　(返す値はsizeof(uint32)分進んだ後のアドレスなので普通は意識しなくてOK)

　　主な仕様用途はワークメモリなので簡単な管理しか行いません。
　　解放は確保した順番と逆順のmemctrl.free()と
　　一気にすべてを解放するstack_reset()の二種類のみです。


2004/04/26変更
　　スタック型を止めてチェーン型に変更。
　　確保する再にアドレスとアドレスの空き容量をチェックし、
　　指定サイズが収まるならそこに挿入するようにした。
　　これによりスタック型の穴空きが抑えられるようになった。
　　ただし、断片化が進むと容量があっても確保できなくなるので注意。

2005/02/09変更
　　メモリの確保数のチェックを行うようにした。
　　これによりメモリ管理内のメモリリークを多少検出しやすくなる。
　　define:memallocを追加。

2005/10/27変更
　　解放アトリビュート、reset関数追加。
　　アライメントを指定して確保をallocExに変更。
　　NDS専用の関数追加＆アロケーターに対応。

2005/11/01追加
　　スタック型のメモリ管理機能追加。

2006/06/23変更
　　元アドレスを設定アライメントに合わせ、確保するメモリは
　　設定アライメント単位で確保するように変更。(→psrc追加)
　　これにともないallocExを削除。

2006/09/08
　　Warning部分を修正。

2006/09/12
　　処理をするとき割り込みを禁止にした。（DS処理)

2006/09/29
　　stack_allocの確保サイズバウンディング時にに
　　ヘッダーサイズを含めていなかった点を修正。

2006/10/31
　　認識ヘッダーを追加して解放処理を統一。
　　この作業にともないStackHeaderやIDを追加。
　　これにともないstack_freeを削除。

2006/11/07
　　デバッグ出力設定を行うデバッグ関数追加。

2007/01/20
　　reset()でスタックメモリをリセットしていなかった点を修正。
　　メモリリークがある状態でrelease()を呼んだ場合、
　　リークサイズを出力するようにした。

2007/10/17
　　他で確保したメモリから初期化できるようinitEx関数を追加。
　　初期化の終端アドレス設定でバグがあったのを修正。

2008/01/11
　　DSのC++対応にともなうエラー修正。

2008/06/04
　　mem_mgr.stackの開始アドレスが指定アライメントに
　　なっていなかった不具合を修正。
　　管理用MemoryLinkのattrが未設定だった不具合を修正。
　　これにともない「eMEM_SYSTEM」を追加。

2008/06/24
　　mtolib作成に合わせてファイル名変更。
　　それに応じてバージョンを2.00に変更。
　　基本はC++対応だけど、mtolibではC言語で作成。
-------------------------------------------------------------------*/
typedef struct tagMemoryLink {
	void  *next;		// 次のリンクアドレス
	uint32 head;		// 識別ヘッダー（固定）
	uint32 size;		// 確保メモリサイズ
	uint32 attr;		// 解放アトリビュート
} MemoryLink;

typedef struct tagStackHeader {
	uint32 size;		// 確保メモリサイズ
	uint32 head;		// 認識ヘッダー（固定）
} StatckHeader;

#define LINK_SIZE		(BOUND(sizeof(MemoryLink), cMEM_ALIGIN))
#define STACK_SIZE		(BOUND(sizeof(StatckHeader), cMEM_ALIGIN))
#define LINK_ID			0x004c4e4b // 'LNK'
#define STACK_ID		0x0053544b // 'STK'


typedef struct tagMemoryManager {
	uint8  *psrc;		// 元アドレス
	uint8  *addr;		// アライメント後のアドレス
	uint8  *pos;		// 現在のアドレス
	uint8  *end;		// 終端アドレス
	uint8  *stack;		// スタックのアドレス

	uint32 size;		// メモリサイズ
	sint32 mnum;		// メモリ確保数
	MemoryLink *link;	// リンク用構造体

	sint32 ex_flg;		// initExで初期化？

#ifndef NDEBUG
	sint32 dbgmsg;		// デバッグメッセージ出力？
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
【機能】デバッグメッセージ出力設定
【引数】dbgmsg：出力？
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
【機能】メモリチェック
【備考】メモリが規定範囲内にあるかをチェック（範囲外は停止）
-------------------------------------------------------------------*/
static void _memory_memory_check(const void *cmem)
{
	uint8 *mem = (uint8*)cmem;
	MemoryLink *link = (MemoryLink*)cmem;

	// NULLならチェックしない
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
			sprintf(str, "指定メモリが不正です\ncmem:0x%x\naddr:0x%x\nend :0x%x",
					(uint32)cmem, (uint32)mem_mgr.addr, (uint32)mem_mgr.end);
			// Windowsならエラー表記
			MessageBox(NULL, str, "Error", MB_ICONERROR | MB_OK);
		}
#endif
		while (1); // 無限ループで止める
#endif
	}
}

/*-------------------------------------------------------------------
【機能】メモリ初期化
【引数】size:メモリ確保サイズ
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

	// stack_allocの開始位置が指定アライメントになるように調整
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
【機能】他で確保したメモリで初期化
【引数】addr:メモリアドレス
        size:メモリサイズ
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

	// stack_allocの開始位置が指定アライメントになるように調整
	stack_size = BOUND(mem_mgr.end, cMEM_ALIGIN);
	if ((uint32)mem_mgr.end != stack_size) {
		mem_mgr.stack = (uint8*)(stack_size - cMEM_ALIGIN);
	} else {
		mem_mgr.stack = mem_mgr.end;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
【機能】メモリ確保
【引数】size  :確保サイズ
　　　　attr  :解放アトリビュート
【備考】cMEM_ALIGINアライメント固定。
        LINK_SIZE多く確保し、そこにMemoryLinkを埋め込む
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

	// 指定アライメント単位で確保
	size = BOUND(size, cMEM_ALIGIN) + LINK_SIZE;

	// 空いている位置を探す
	mlink = mem_mgr.link;
	while (mlink != NULL) {
		if (mlink->next == NULL) break; // 終端

		mfree = (uint32)mlink->next - ((uint32)mlink + mlink->size);
		if (size <= mfree) { // 空き発見
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

	// 新規確保
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
【機能】メモリ解放
【引数】addr:メモリアドレス
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

		// 確保サイズ取得
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
			if (slink->next == NULL) { // it end？
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
【機能】メモリの空き容量
-------------------------------------------------------------------*/
static uint32 _memory_get_quantity(void)
{
	uint32 mfree;
	MemoryLink *mlink;

	// 基本
	mfree = (uint32)(mem_mgr.stack - mem_mgr.pos);

	// 間にある空きを加算
	mlink = mem_mgr.link;
	while (mlink != NULL) {
		if (mlink->next == NULL) break; // 終端
		mfree += (uint32)mlink->next - ((uint32)mlink + mlink->size);
		mlink = (MemoryLink*)mlink->next;
	}

	return mfree;
}

/*-------------------------------------------------------------------
【機能】断片化したメモリの空き容量
-------------------------------------------------------------------*/
static uint32 _memory_get_fragment(void)
{
	uint32 mfree;
	MemoryLink *mlink;

	// 基本
	mfree = 0;

	// 間にある空きを加算
	mlink = mem_mgr.link;
	while (mlink != NULL) {
		if (mlink->next == NULL) break; // 終端
		mfree += (uint32)mlink->next - ((uint32)mlink + mlink->size);
		mlink = (MemoryLink*)mlink->next;
	}

	return mfree;
}

/*-------------------------------------------------------------------
【機能】eMEM_UNLOCKメモリ解放
【備考】この関数を読んだ後、既に解放されているアドレスでfreeを呼ぶと
　　　　Errorになるので注意。（回避策を考え中）
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
			if (slink->next == NULL) { // it end？
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

	// スタックメモリリセット
	mem_mgr.stack = mem_mgr.end;

#ifdef _NDS
	OS_RestoreInterrupts(old);
#endif
#endif
}

/*-------------------------------------------------------------------
【機能】メモリ解放
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
【機能】スタックメモリ確保
【引数】size  :確保サイズ
-------------------------------------------------------------------*/
static void *_memory_stack_alloc(uint32 size)
{
	void *ptr = NULL;
	StatckHeader *stack;

#ifdef _NDS
	OSIntrMode old = OS_DisableInterrupts();
#endif

	// 指定アライメント単位で確保
	size = BOUND(size, cMEM_ALIGIN) + STACK_SIZE;

	// メモリ確保可能？
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
【機能】スタックメモリリセット
【備考】スタック位置を初期位置に戻しますので、
　　　　既に確保したメモリは使用しない＆stack_free()に渡さないこと!!
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
【機能】NDS用メモリ確保
-------------------------------------------------------------------*/
static void *_memory_nds_alloc(NNSFndAllocator *pAllocator, uint32 size)
{
	NOTHING(pAllocator);
	return _memory_alloc(size, eMEM_LOCK);
}

/*-------------------------------------------------------------------
【機能】NDS用メモリ解放
-------------------------------------------------------------------*/
static void _memory_nds_free(NNSFndAllocator *pAllocator, void *addr)
{
	NOTHING(pAllocator);
	_memory_free(addr);
}

/*-------------------------------------------------------------------
【機能】NDS用メモリ初期化
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

	// メモリ管理マネージャー初期化
	mem_mgr.addr  = mem_mgr.psrc; // アライメントされている
	mem_mgr.pos   = mem_mgr.addr + LINK_SIZE;
	mem_mgr.end   = mem_mgr.psrc + heapSize;
	mem_mgr.size  = heapSize;
	mem_mgr.link  = (MemoryLink*)mem_mgr.addr;
	mem_mgr.link->next = NULL;
	mem_mgr.link->size = LINK_SIZE;

	// stack_allocの開始位置が指定アライメントになるように調整
	stack_size = BOUND(mem_mgr.end, cMEM_ALIGIN);
	if ((uint32)mem_mgr.end != stack_size) {
		mem_mgr.stack = (uint8*)(stack_size - cMEM_ALIGIN);
	} else {
		mem_mgr.stack = mem_mgr.end;
	}

	return TRUE;
}

/*-------------------------------------------------------------------
【機能】NDS用アロケーター構造体取得
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
