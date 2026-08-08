--[[
	This premake file is also loaded by our development/module/premake5.lua
	If this happens the DEVELOPMENT will be set to true
	I did this to have 1 premake file instead of 2 separate ones.
]]
PROJECT_GENERATOR_VERSION = 3

EXCLUDE_COMMON_PROJECT = true
if not DEVELOPMENT then
	newoption({
		trigger = "gmcommon",
		description = "Sets the path to the garrysmod_common (https://github.com/danielga/garrysmod_common) directory",
		default = "../garrysmod_common"
	})

	newoption({
		trigger = "dedicated",
		description = "Build for Windows dedicated server (defines DEDICATED)"
	})

	DEDICATED = _OPTIONS["dedicated"] and true or false
end

local gmcommon = assert(_OPTIONS.gmcommon or os.getenv("GARRYSMOD_COMMON"),
	"you didn't provide a path to your garrysmod_common (https://github.com/danielga/garrysmod_common) directory")
include(gmcommon)

local rootDir = ""
local sourcePath = "source/"
if DEVELOPMENT then
	rootDir = "../../"
	sourcePath = rootDir .. sourcePath
end
include(rootDir .. "overrides.lua")

include(sourcePath .. "bootil/premake5.lua")

--[[
	Project setup
	We MUST use abi as else many GMod structs do not match!
]]
CreateWorkspace({name = "filesystem_stdio", abi_compatible = true})
	-- Serverside module (gmsv prefix)
	-- Can define "source_path", where the source files are located
	-- Can define "manual_files", which allows you to manually add files to the project,
	-- instead of automatically including them from the "source_path"
	-- Can also define "abi_compatible", for project specific compatibility
	CreateProject({serverside = true, manual_files = false, source_path = sourcePath:sub(0, -2)})
		kind("SharedLib")
		symbols("On")
		targetsuffix("")

		-- Remove some or all of these includes if they're not needed
		--IncludeHelpersExtended()
		--IncludeLuaShared()
		IncludeSDKCommon()
		IncludeSDKTier0()
		IncludeSDKTier1()
		IncludeSDKTier2()
		--IncludeSDKTier3()
		--IncludeSDKMathlib()
		--IncludeSDKRaytrace()
		--IncludeSDKBitmap()
		--IncludeSDKVTF()
		IncludeSteamAPI()
		--IncludeDetouring()
		--IncludeScanning()
		IncludeBootil()

		if DEDICATED then
			defines("SWDS=1")
		end

		defines("GMOD")
		defines("NO_VCR")

		-- Filesystem defs
		defines({"FILESYSTEM_STDIO_EXPORTS", "DONT_PROTECT_FILEIO_FUNCTIONS", "PROTECTED_THINGS_ENABLE"})
		-- GMod specific def
		defines("SUPPORT_PACKED_STORE")

		if GMOD_X86_64 then
			defines("GMOD_X86_64")
		end

		files({
			gmcommon .. [[/sourcesdk-minimal/public/zip_utils.cpp]],
			gmcommon .. [[/sourcesdk-minimal/public/kevvaluescompiler.cpp]],
			rootDir .. "README.md",
			rootDir .. ".github/workflows/**.yml",
			sourcePath .. "garrysmod/**",
			sourcePath .. "vpklib/**",
		})

		vpaths({
			["Source files/sourcesdk/"] = gmcommon .. "/**.*",
			["README"] = rootDir .. "README.md",
			["Workflows"] = rootDir .. ".github/workflows/**.yml",
		})

		filter("system:windows")
			defines({"IVP_NO_MATH_INL", "COMPILER_MSVC"})
			disablewarnings({"4101"})
			files(sourcePath .. "tier0/platform.cpp")

		filter({"system:windows", "platforms:x86_64"})
			defines({"COMPILER_MSVC64", "WIN64"})

		filter({"system:linux", "platforms:x86_64"})
			buildoptions({"-mcx16"}) -- Should solve this: undefined reference to `__sync_bool_compare_and_swap_16'

		filter({"platforms:x86_64"})
			defines("PLATFORM_64BITS")

		filter("system:windows")
			removefiles(sourcePath .. "linux_support.cpp")
			if HOLYLIB_DEDICATED then
				defines("DEDICATED")
			else
				defines("NOT_DEDICATED") -- Windows client build
			end

		filter("system:linux")
			files(sourcePath .. "tier0/platform_posix.cpp")
			disablewarnings({"unused-variable"})
			targetextension(".so")
			links({"dl", "tier0", "pthread"}) -- this fixes the undefined reference to `dlopen' errors.
			defines({"DEDICATED", "POSIX"}) -- All linux build focus Linux dedicated servers.