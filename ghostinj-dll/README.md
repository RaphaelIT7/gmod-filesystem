# ghostinj.dll

This dll is used by the `-usegh` [command line argument](https://github.com/RaphaelIT7/obsolete-source-engine/blob/gmod/dedicated/sys_common.cpp#L51) and it allows one to do stuff before the dedicated server is loaded.<br>

This will hook into the `dedicated` file and detour `CreateInterface` to redirect it to our own filesystem file<br>