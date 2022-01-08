#ifndef EPXORT_H
#define EPXORT_H

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
