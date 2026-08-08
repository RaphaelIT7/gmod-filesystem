#include <Platform.hpp>
#include <stdio.h>
#include <detouring/detours.h>
#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/Symbol.hpp>
#include <scanning/symbolfinder.hpp>
#include <tier1/interface.h>

// Realized after adding windows support that usegh was linux only... well just in case for the future I guess... :sob:
#ifdef SYSTEM_WINDOWS
#include <Windows.h>
#undef GetObject
#undef GetClassName
#define DLL_Handle HMODULE
#define DLL_LoadModule(name, _) LoadLibrary(name)
#define DLL_UnloadModule(handle) FreeLibrary((DLL_Handle)handle)
#define DLL_GetAddress(handle, name) GetProcAddress((DLL_Handle)handle, name)
#define DLL_LASTERROR "LINUXONLY"
#define DLL_EXTENSION ".dll"
#else
#include <dlfcn.h>
#define DLL_Handle void*
#define DLL_LoadModule(name, type) dlopen(name, type)
#define DLL_UnloadModule(handle) dlclose(handle)
#define DLL_GetAddress(handle, name) dlsym(handle, name)
#define DLL_LASTERROR dlerror()
#define DLL_EXTENSION ".so"
#endif

DLL_Handle filesystem_stdio = NULL;
CreateInterfaceFn filesystem_stdioFn = nullptr;

// If we include filesystem.h we would need to also compile tier0? which I do not want...
#define FILESYSTEM_INTERFACE_VERSION			"VFileSystem022"
// Hammer uses this one
#define BASEFILESYSTEM_INTERFACE_VERSION		"VBaseFileSystem011"

Detouring::Hook detour_CreateInterface;
void* hook_CreateInterface( const char *pName, int *pReturnCode )
{
	if ( ( strcmp( pName, FILESYSTEM_INTERFACE_VERSION ) == 0 || strcmp( pName, BASEFILESYSTEM_INTERFACE_VERSION ) == 0 ) && filesystem_stdioFn )
		return filesystem_stdioFn(pName, pReturnCode);

	return detour_CreateInterface.GetTrampoline<CreateInterfaceFn>()( pName, pReturnCode );
}

Detouring::Hook detour_AppSystemCreateInterface;
void* hook_AppSystemCreateInterface( const char *pName, int *pReturnCode )
{
	if ( ( strcmp( pName, FILESYSTEM_INTERFACE_VERSION ) == 0 || strcmp( pName, BASEFILESYSTEM_INTERFACE_VERSION ) == 0 ) && filesystem_stdioFn )
		return filesystem_stdioFn( pName, pReturnCode );

	return detour_AppSystemCreateInterface.GetTrampoline<CreateInterfaceFn>()( pName, pReturnCode );
}

using FSReturnCode_t = int;
class IFileSystem;
class CFSMountContentInfo
{
public:
	bool				m_bToolsMode;
	const char			*m_pDirectoryName;
	IFileSystem			*m_pFileSystem;
};

Detouring::Hook detour_FileSystem_MountContent;
using FileSystem_MountContentFunc = FSReturnCode_t (*)( CFSMountContentInfo &mountContentInfo );
FSReturnCode_t hook_FileSystem_MountContent( CFSMountContentInfo &mountContentInfo )
{
	int pReturnCode = 0;
	mountContentInfo.m_pFileSystem = (IFileSystem*)filesystem_stdioFn( FILESYSTEM_INTERFACE_VERSION, &pReturnCode );
	printf( "CFSMountContentInfo.m_pFileSystem = %p\n", mountContentInfo.m_pFileSystem );

	return detour_FileSystem_MountContent.GetTrampoline<FileSystem_MountContentFunc>()( mountContentInfo );
}

DLL_Handle ghostinj2 = NULL;
typedef void ( *plugin_main )();
void Load()
{
	printf( "--- FileSystemOverride-GhostInj Loading ---\n" );

	filesystem_stdio = DLL_LoadModule( "filesystem_stdio_new" DLL_EXTENSION, RTLD_NOW );
	if ( filesystem_stdio )
	{
		filesystem_stdioFn = (CreateInterfaceFn)DLL_GetAddress( filesystem_stdio, CREATEINTERFACE_PROCNAME );

		void* pFileSystem = filesystem_stdioFn( FILESYSTEM_INTERFACE_VERSION, nullptr );
		printf( "pFileSystem = %p\n", pFileSystem );

		printf( "Found and loaded filesystem_stdio_new%s\n", DLL_EXTENSION );
		SourceSDK::FactoryLoader dedicated_loader( "dedicated" );
		void* CreateInterfaceFnAddr = DLL_GetAddress( dedicated_loader.GetModule(), CREATEINTERFACE_PROCNAME );
		if ( CreateInterfaceFnAddr )
		{
			detour_CreateInterface.Create( CreateInterfaceFnAddr, (void*)hook_CreateInterface );
			if ( detour_CreateInterface.IsValid() )
			{
				detour_CreateInterface.Enable();
				printf( "Successfully detoured dedicated %s!\n", CREATEINTERFACE_PROCNAME );
			}
			else
				printf( "Failed to detour dedicated %s!\n", CREATEINTERFACE_PROCNAME );
		} else
			printf( "Failed to find dedicated %s!\n", CREATEINTERFACE_PROCNAME );

		// Can be found using "System (%s) failed during stage %s\n" (CAppSystemGroup::ReportStartupFailure -> CAppSystemGroup::ConnectSystems)
		Symbol AppSystemCreateInterfaceFnSym = Symbol::FromName( "_Z26AppSystemCreateInterfaceFnPKcPi" );
		// Can be found using "Should not be using filesystem_steam anymore!"
		Symbol FileSystem_MountContentSym = Symbol::FromName( "_Z23FileSystem_MountContentR19CFSMountContentInfo" );

		SymbolFinder symfinder;
		void* AppSystemCreateInterfaceFnAddr = symfinder.Resolve( dedicated_loader.GetModule(), AppSystemCreateInterfaceFnSym.name.c_str(), AppSystemCreateInterfaceFnSym.length );
		if ( AppSystemCreateInterfaceFnAddr )
		{
			detour_AppSystemCreateInterface.Create( AppSystemCreateInterfaceFnAddr, (void*)hook_AppSystemCreateInterface );
			if ( detour_AppSystemCreateInterface.IsValid() )
			{
				detour_AppSystemCreateInterface.Enable();
				printf( "Successfully detoured dedicated AppSystemCreateInterfaceFn!\n" );
			}
			else
				printf( "Failed to detour dedicated AppSystemCreateInterfaceFn!\n" );
		} else
			printf( "Failed to find dedicated AppSystemCreateInterfaceFn!\n" );

		void* FileSystem_MountContentAddr = symfinder.Resolve( dedicated_loader.GetModule(), FileSystem_MountContentSym.name.c_str(), FileSystem_MountContentSym.length );
		if ( FileSystem_MountContentAddr )
		{
			detour_FileSystem_MountContent.Create( FileSystem_MountContentAddr, (void*)hook_FileSystem_MountContent );
			if ( detour_FileSystem_MountContent.IsValid() )
			{
				detour_FileSystem_MountContent.Enable();
				printf( "Successfully detoured dedicated FileSystem_MountContentAddr!\n" );
			}
			else
				printf( "Failed to detour dedicated FileSystem_MountContentAddr!\n" );
		} else
			printf( "Failed to find dedicated FileSystem_MountContentAddr!\n" );
	}

	ghostinj2 = DLL_LoadModule( "ghostinj2.dll", RTLD_NOW );
	if ( ghostinj2 )
		printf( "Found and loaded ghostinj2.dll\n" );

	printf( "--- FileSystemOverride-GhostInj loaded ---\n" );
}

void Unload()
{
	printf( "--- FileSystemOverride-GhostInj unloading ---\n" );

	if ( detour_CreateInterface.IsEnabled() )
		detour_CreateInterface.Destroy();

	if ( filesystem_stdio )
		DLL_UnloadModule( filesystem_stdio );

	if ( ghostinj2 )
		DLL_UnloadModule( ghostinj2 );

	printf( "--- FileSystemOverride-GhostInj unloaded ---\n" );
}

#ifdef SYSTEM_WINDOWS
#include <windows.h>
BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved )
{
	switch( fdwReason ) 
	{ 
		case DLL_PROCESS_ATTACH:
			Load();
			break;
		case DLL_PROCESS_DETACH:
			if ( lpvReserved )
				break;

			Unload();
			break;
	}

	return TRUE;
}
#else
void __attribute__((constructor)) SO_load()
{
	Load();
}

void __attribute__((destructor)) SO_unload()
{
	Unload();
}
#endif