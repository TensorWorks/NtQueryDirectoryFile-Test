# NtQueryDirectoryFile filename mask reset test

This repository contains a test application to call the [NtQueryDirectoryFile](https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntquerydirectoryfile) Windows API function with various arguments and print the results. Specifically, the test application is designed to verify the behaviour described in Microsoft's documentation regarding the `FileName` filename mask parameter:

> `[in, optional] FileName`
> 
> An optional pointer to a caller-allocated Unicode string containing the name of a file (or multiple files, if wildcards are used) within the directory specified by FileHandle. This parameter is optional and can be NULL.
> 
> If FileName is not NULL, only files whose names match the FileName string are included in the directory scan. If FileName is NULL, all files are included.
> 
> **The FileName is used as a search expression and is captured on the very first call to NtQueryDirectoryFile for a given handle. Subsequent calls to NtQueryDirectoryFile will use the search expression set in the first call. The FileName parameter passed to subsequent calls will be ignored.**


## Test results

### Raw output

The [results](./results/) subdirectory contains the output captured when running the test application under various versions of Windows:

- [Windows XP SP3](./results/result_xp.txt)
- [Windows Vista SP2](./results/result_vista.txt)
- [Windows 7 SP1](./results/result_win7.txt)
- [Windows 8](./results/result_win8.txt)
- [Windows 8.1](./results/result_win81.txt)
- [Windows 10 version 22H2](./results/result_win10.txt)
- [Windows 10 version 22H2 with Compatibility Mode set to Windows XP SP3](./results/result_win10_compatibility_xp.txt)
- [Windows 10 version 22H2 with Compatibility Mode set to Windows Vista SP2](./results/result_win10_compatibility_vista.txt)
- [Windows 10 version 22H2 with Compatibility Mode set to Windows 7](./results/result_win10_compatibility_win7.txt)
- [Windows 11 version 23H2](./results/result_win11.txt)
- [Windows 11 version 23H2 with Compatibility Mode set to Windows XP SP3](./results/result_win11_compatibility_xp.txt)
- [Windows 11 version 23H2 with Compatibility Mode set to Windows Vista SP2](./results/result_win11_compatibility_vista.txt)
- [Windows 11 version 23H2 with Compatibility Mode set to Windows 7](./results/result_win11_compatibility_win7.txt)

### Observed differences

Comparing the results from different versions of Windows highlights the following differences:

- Under versions of Windows prior to Windows 8, the filename mask is indeed captured by the first call to `NtQueryDirectoryFile()` for a given handle and subsequent changes to the mask are ignored, even when restarting the scan. This matches the behaviour described in Microsoft's documentation.

- Under Windows 8 and newer, changes to the filename mask are reflected in the output when restarting the scan. This contradicts the behaviour described in Microsoft's documentation, and some modern applications may rely on this undocumented behaviour in order to function correctly.

- Even when Compatibility Mode is enabled for the test executable and the compatibility target is set to a version of Windows prior to Windows 8, newer versions of Windows do not reproduce the documented behaviour.


## Building the test application from source

Building the test application requires the following:

- [CMake](https://cmake.org/) 3.22 or newer
- [Visual Studio](https://visualstudio.microsoft.com/) 2017 or newer
- [Windows XP platform toolset](https://learn.microsoft.com/en-us/cpp/build/configuring-programs-for-windows-xp?view=msvc-170) (under Visual Studio 2022, the component is called **"C++ Windows XP Support for VS 2017 (v141) tools [Deprecated]"**)

To build a 32-bit executable for the test application with support for Windows XP, run the following commands:

```batch
cd build
cmake -A Win32 -T v141_xp -DCMAKE_INSTALL_PREFIX=.. ..\source
cmake --build . --target install --config Release
```

Once the build process is complete, the built executable will be placed in the `bin` subdirectory.


## Legal

Copyright &copy; 2024, Amazon.com and Affiliates. Licensed under the Apache License, Version 2.0. See the file [LICENSE](./LICENSE) for details.
