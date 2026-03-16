#pragma once
#ifndef DECLSPEC
#ifdef _MSC_VER
#ifdef BC_EXPORT_DLL
#define DECLSPEC __declspec(dllexport)
#else
#define DECLSPEC __declspec(dllimport) 
#endif

#define DLLOCAL


#else
#define DECLSPEC __attribute__((visibility("default")))
#define DLLOCAL __attribute__((visibility("hidden")))

#endif
#endif // !DECLSPEC

