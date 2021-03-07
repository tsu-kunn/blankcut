#ifndef _MTO_MEMCTRL_H_
#define _MTO_MEMCTRL_H_

#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

#define _USE_MEM_CTRL  // use memory control ?


enum {
	eMEM_LOCK = 0,					// �\�t�g�E�F�A���Z�b�g�ŉ������Ȃ� 
	eMEM_UNLOCK,					// �\�t�g�E�F�A���Z�b�g�ŉ�������
	eMEM_SYSTEM						// �Ǘ������i�����Ɏw�肵�Ȃ��ł��������j
};



#ifdef _USE_MEM_CTRL
#define CR_FREE(x)		if (x != NULL) {memctrl.free(x);x = NULL;}
#define memalloc(x)		memctrl.alloc((x), (uint32)eMEM_UNLOCK)
#else
#define CR_FREE(x)		if (x != NULL) {free(x);x = NULL;}
#define memalloc(x)		malloc((x))
#endif // _USE_MEM_CTRL

#define cMEM_ALIGIN		32						// �A���C�����g(4,8,16,32,64,128)
#define cMEM_SIZE(x)	((x) * 1024 * 1024)		// 1M�P��



typedef struct tagMemoryControl {
	sint32 (*init)        (const uint32 size);
	sint32 (*initEx)      (void *addr, const uint32 size);

	void  *(*alloc)       (uint32 size, const uint32 attr);
	sint32 (*free)        (const void *addr);

	uint32 (*quantity)    (void);
	uint32 (*fragment)    (void);

	void   (*reset)       (void);
	void   (*release)     (void);

	void  *(*stack_alloc) (uint32 size);
	void   (*stack_reset) (void);

	void   (*check)       (const void *cmem);
	void   (*dbgmsg)      (const sint32 dbgmsg);

#ifdef _NDS
	sint32 (*nds_init)  (void);
	void  *(*nds_alloc) (NNSFndAllocator *pAllocator, uint32 size);
	void   (*nds_free)  (NNSFndAllocator *pAllocator, void *addr);
	NNSFndAllocator (*nds_get_allocator) (NNSFndAllocator *pAllocator);
#endif
} MemoryControl;

extern const MemoryControl memctrl;


#if defined(_LANGUAGE_C_PLUS_PLUS) || defined(__cplusplus) || defined(c_plusplus)
}
#endif
#endif //_MTO_MEMCTRL_H_
