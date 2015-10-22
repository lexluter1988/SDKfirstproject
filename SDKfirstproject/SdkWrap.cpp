///////////////////////////////////////////////////////////////////////////////
///
/// @file SdkWrap.cpp
///
/// This file is the part of parallels public sdk library
/// Really simple C++ wrapper over the DLL.
/// Requires no special libraries and works on win/lib/mac.
///
/// @author maximk
///
/// Copyright (c) 2005-2007 Parallels Software International, Inc.
/// All rights reserved.
/// http://www.parallels.com
///
///////////////////////////////////////////////////////////////////////////////

#include "SdkWrap.h"

#include <string>

#include <string.h> // for strcmp
#include <stdio.h>

#if defined(_WIN_)

	#define DLL_HANDLE hInstance
	#define DLL_LOAD( a ) LoadLibraryA( a )
	#define DLL_GETSYM( a, b ) GetProcAddress( a, b )
	#define DLL_UNLOAD( a ) !FreeLibrary( a )
	struct HINSTANCE__;
	typedef struct HINSTANCE__* hInstance;

	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>

#elif defined(_LIN_) || defined(_MAC_)

	#define DLL_HANDLE void*
	#define DLL_LOAD( a ) dlopen( a, RTLD_LAZY )
	#define DLL_GETSYM( a, b ) dlsym( a, b )
	#define DLL_UNLOAD( a ) dlclose( a )

	#include <dlfcn.h>
#endif

#if defined(_WIN_)
#define LIB_FILE_EXT ".dll"
#elif defined(_LIN_)
#define LIB_FILE_EXT ".so"
#elif defined(_MAC_)
#define LIB_FILE_EXT ".dylib"
#else
	#error unknown platform
#endif

namespace PrlSdkWrapNamespace {
/**
 * Each SDK method is instantiated as an instance of the
 * variable that points to the proper method somehow
 */
#ifdef PRL_SDK_WRAP_FOR_EACH_ITERATOR
    #undef PRL_SDK_WRAP_FOR_EACH_ITERATOR
    #undef PRL_SDK_WRAP_FOR_EACH_DEPR_ITERATOR
#endif
#define PRL_SDK_WRAP_FOR_EACH_ITERATOR( name ) \
	name##wrap##_Ptr name;
#define PRL_SDK_WRAP_FOR_EACH_DEPR_ITERATOR( name ) \
	name##wrap##_Ptr name;

#include "PrlApiMacro.h"

PRL_SDK_WRAP_FOR_EACH()

/// Handle of the loaded library
static PRL_VOID_PTR s_DllHandle = 0;


/**
 * Find symbol in the loaded library.
 *
 * @param symbol name to find
 * @return pointer to the symbol, NULL on failure
 */
PRL_VOID_PTR SdkWrap_GetSymbol ( PRL_CONST_STR symbol )
{
	if ( !s_DllHandle )
		return 0;

	return (PRL_VOID_PTR) DLL_GETSYM( (DLL_HANDLE) s_DllHandle, symbol );
}


/**
 * Unload previously loaded dynamic library.
 *
 * @return PRL_RESULT
 */
PRL_RESULT SdkWrap_Unload (  )
{
	if ( !s_DllHandle )
		return PRL_ERR_UNINITIALIZED;

	if ( DLL_UNLOAD( (DLL_HANDLE) s_DllHandle ) )
		return PRL_ERR_FAILURE;

	#ifdef PRL_SDK_WRAP_FOR_EACH_ITERATOR
		#undef PRL_SDK_WRAP_FOR_EACH_ITERATOR
		#undef PRL_SDK_WRAP_FOR_EACH_DEPR_ITERATOR
	#endif

	#define PRL_SDK_WRAP_FOR_EACH_ITERATOR( name ) \
		name = 0;
	#define PRL_SDK_WRAP_FOR_EACH_DEPR_ITERATOR( name ) \
		name = 0;

	#include "PrlApiMacro.h"

	PRL_SDK_WRAP_FOR_EACH()

	s_DllHandle = 0;

	return PRL_ERR_SUCCESS;
}


/**
 * Load dynamic library pointer my the file name
 *
 * @param utf-8 encoded file name
 * @return PRL_RESULT
 */
PRL_RESULT SdkWrap_Load( PRL_CONST_STR cname, bool bDebugMode )
{
	std::string name = cname;

	// Need to be sure everything is unloaded
	SdkWrap_Unload();

	#if defined(_WIN_)
		// LoadLibrary() does add .dll automatically
	#elif defined(_LIN_) || defined(_MAC_)
        // dlopen() does not add lib file extention to the filename
        std::string::size_type spos = name.find_last_of('/');
        if ( name.find(LIB_FILE_EXT, (spos == std::string::npos)?0:spos) == std::string::npos )
			name += LIB_FILE_EXT ;
        // to be able to pass here names like 'prl_sdk' or something without prefix
        if ( spos == std::string::npos && name.substr(0,3) != "lib")
            name = "lib" + name;
	#endif

	if ( bDebugMode )
		fprintf(stderr, "Trying to load SDK lib '%s'\n", name.c_str());

    s_DllHandle = (PRL_VOID_PTR) DLL_LOAD( name.c_str() );

	// Checking error status
    if( !s_DllHandle )
	{
		if ( bDebugMode )
		{
#ifndef _WIN_
			fprintf(stderr, "Failed to load SDK lib '%s' due an error: '%s'\n", name.c_str(), dlerror());
#else
//FIXME: need implementation for Windows case
#endif
		}
		return PRL_ERR_FILE_NOT_FOUND;
	}

	#ifdef PRL_SDK_WRAP_FOR_EACH_ITERATOR
		#undef PRL_SDK_WRAP_FOR_EACH_ITERATOR
		#undef PRL_SDK_WRAP_FOR_EACH_DEPR_ITERATOR
	#endif

	bool some_method_not_found = false;

    /**
     * For each function in the SDK we attempt to load it.
     * If some methods are left unitialized - then probably it's an error,
     * but we can consider leaving it NULL in the future - will see
     */
#ifdef _WIN_
	#define PRL_SDK_WRAP_FOR_EACH_ITERATOR( name ) \
		if ( !( name = ( name##wrap##_Ptr) SdkWrap_GetSymbol( #name ) ) ) { \
			fprintf(stderr, "Failed to locate SDK symbol '%s' on initialization (!)\n", #name ); \
			some_method_not_found = true; \
        }
	#define PRL_SDK_WRAP_FOR_EACH_DEPR_ITERATOR( name ) \
		if ( !( name = ( name##wrap##_Ptr) SdkWrap_GetSymbol( #name ) ) ) { \
        }
#else
	#define PRL_SDK_WRAP_FOR_EACH_ITERATOR( name ) \
		if ( !( name = ( name##wrap##_Ptr) SdkWrap_GetSymbol( #name ) ) ) { \
			if ( bDebugMode ) \
				fprintf(stderr, "Failed to locate SDK symbol '%s' on initialization (!)\n", #name ); \
			some_method_not_found = true; \
        }
	#define PRL_SDK_WRAP_FOR_EACH_DEPR_ITERATOR( name ) \
		if ( !( name = ( name##wrap##_Ptr) SdkWrap_GetSymbol( #name ) ) ) { \
			if ( bDebugMode ) \
				fprintf(stderr, "Failed to locate deprecated SDK symbol '%s' on initialization\n", #name ); \
        }
#endif
	#include "PrlApiMacro.h"

	PRL_SDK_WRAP_FOR_EACH()

    if (some_method_not_found) {
		if ( bDebugMode )
			fprintf(stderr, "Failed with SDK library '%s'.\n", name.c_str() ); \
		return PRL_ERR_FAILURE;
    }

	return PRL_ERR_SUCCESS;
}


/**
 * Return boolean status that identifies loaded state of the dll.
 *
 * @return boolean loaded status
 */
PRL_BOOL SdkWrap_IsLoaded ()
{
	return (s_DllHandle != 0);
}

PRL_BOOL SdkWrap_LoadLibFromStdPaths( bool bDebugMode )
{
#ifdef _WIN_
/* Fixme:
  http://bugzilla.parallels.com/show_bug.cgi?id=7355
[TARGETDIR]\Application\prl_sdk_py.dll

[TARGETDIR] is a base directory for Parallels Server.
It can be determine via Windows registry key:

HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\{F005D13D-B44F-4B5B-B856-D5494ABF72B5}\InstallLocation
*/
	#define SDK_LIB_PATH ""
	#define SDK_LIB_NAME "prl_sdk"

#elif defined(_LIN_)
	#define SDK_LIB_NAME "libprl_sdk"

#if defined (PRL_PROD_WORKSTATION_EX)
	#define SDK_LIB_PATH "/usr/lib/parallels-workstation/libs/libprl_sdk.so"
#elif defined (PRL_PROD_DESKTOP_WL)
	#define SDK_LIB_PATH "/usr/lib/parallels-desktop/libs/libprl_sdk.so"
#else
	#define SDK_LIB_PATH \
		"${ORIGIN}/../../../parallels-virtualization-sdk/libs/libprl_sdk.so." PARALLELS_SDK_LIB_MAJ "/"
	#define SDK_LIB_NAME_WITH_MAJOR SDK_LIB_NAME LIB_FILE_EXT "." PARALLELS_SDK_LIB_MAJ
#endif

#elif defined(_MAC_)

	#define SDK_LIB_PATH "/Library/Parallels/Parallels Service.app/Contents/Frameworks/ParallelsVirtualizationSDK.framework/Versions/Current/Libraries/"
	#define SDK_LIB_PATH2 "/Library/Frameworks/ParallelsVirtualizationSDK.framework/Libraries/"
	#define SDK_LIB_NAME "libprl_sdk"

#endif

    if (!SdkWrap_IsLoaded())
        if (PRL_FAILED(SdkWrap_Load("./" SDK_LIB_NAME, bDebugMode)))
#if defined(SDK_LIB_NAME_WITH_MAJOR)
			if (PRL_FAILED(SdkWrap_Load(SDK_LIB_NAME_WITH_MAJOR, bDebugMode)))
#endif
            if (PRL_FAILED(SdkWrap_Load(SDK_LIB_PATH SDK_LIB_NAME, bDebugMode)))
                if (PRL_FAILED(SdkWrap_Load(SDK_LIB_NAME, bDebugMode)))
#if defined(SDK_LIB_PATH2)
                    if (PRL_FAILED(SdkWrap_Load(SDK_LIB_PATH2 SDK_LIB_NAME, bDebugMode)))
#endif
                        return (PRL_FALSE);
	return (PRL_TRUE);
}

}//namesapce PrlSdkWrapNamespace

