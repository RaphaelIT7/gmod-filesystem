#include <Platform.hpp>
#include <stdio.h>
#include <detouring/detours.h>
#include <GarrysMod/FactoryLoader.hpp>
#include <GarrysMod/Symbol.hpp>
#include <scanning/symbolfinder.hpp>
#include <tier1/interface.h>

// Realized after adding windows support that usegh was linux only... well just in case for the future I guess... :sob:
#ifdef WIN32
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

Detouring::Hook detour_CreateInterface;
void* CreateInterface(const char *pName, int *pReturnCode)
{
	if (strcmp(pName, FILESYSTEM_INTERFACE_VERSION) && filesystem_stdioFn)
		return filesystem_stdioFn(pName, pReturnCode);

	return detour_CreateInterface.GetTrampoline<CreateInterfaceFn>()(pName, pReturnCode);
}

DLL_Handle ghostinj2 = NULL;
SymbolFinder symfinder;
typedef void ( *plugin_main )();
void Load()
{
	printf( "--- FileSystemOverride-GhostInj Loading ---\n" );

	filesystem_stdio = DLL_LoadModule( "filesystem_stdio_new" DLL_EXTENSION, RTLD_NOW );
	if ( filesystem_stdio )
	{
		Symbol CreateInterfaceSym = Symbol::FromName( CREATEINTERFACE_PROCNAME );
		filesystem_stdioFn = (CreateInterfaceFn)symfinder.Resolve( filesystem_stdio, CreateInterfaceSym.name.c_str(), CreateInterfaceSym.length );

		printf( "Found and loaded filesystem_stdio_new%s\n", DLL_EXTENSION );
		SourceSDK::FactoryLoader dedicated_loader( "dedicated" );
		void* CreateInterfaceFn = symfinder.Resolve( dedicated_loader.GetModule(), CreateInterfaceSym.name.c_str(), CreateInterfaceSym.length );
		if ( CreateInterfaceFn )
		{
			detour_CreateInterface.Create( CreateInterfaceFn, CreateInterface );
			if ( detour_CreateInterface.IsValid() )
				detour_CreateInterface.Enable();
			else
				printf( "Failed to detour dedicated %s!\n", CREATEINTERFACE_PROCNAME );
		} else {
			printf( "Failed to find dedicated %s!\n", CREATEINTERFACE_PROCNAME );
		}
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

#if SYSTEM_WINDOWS
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