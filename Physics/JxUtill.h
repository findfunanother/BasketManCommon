// ***************************************************************
//  JxUtill.h
// ***************************************************************
// History :	2019/03/14 - Created By findfun
// ***************************************************************

#pragma once
 
#ifndef SAFE_DELETE
#define SAFE_DELETE(p)			{ if(p) { delete (p);     (p)=NULL; } }
#endif

#ifndef SAFE_DELETE_ARRAY
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p);   (p)=NULL; } }
#endif

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p)=NULL; } }
#endif

#ifndef INIT_MEMORY
#define INIT_MEMORY(p,s)		memset(p,0x00,s)
#endif

#ifndef	STRNCPY
#define STRNCPY(p,max_p,d)		strncpy_s((char*)p,max_p + 1,(char*)d,max_p)
#endif

#ifndef	TSTRNCPY
#define TSTRNCPY(p,max_p,d)		_tcsncpy_s((TCHAR*)p,max_p + 1,(TCHAR*)d,max_p)
#endif


  


