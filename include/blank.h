#ifndef _BLANK_H_
#define _BLANK_H_

#ifdef __linux__ 
#define DIR_MODE false
#else
#define DIR_MODE true
#endif

bool search_blank_output(void);

#endif
