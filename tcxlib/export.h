#ifndef __TCX__EPXORT_H__
#define __TCX__EPXORT_H__

#if WIN32 && BUILD_SHARED_LIBS
	#ifdef DLL_EXPORT
		#define DLL_API __declspec(dllexport)
	#else
		#define DLL_API __declspec(dllimport)
	#endif
#else
	#define DLL_API
#endif

#endif //EPXORT_H
