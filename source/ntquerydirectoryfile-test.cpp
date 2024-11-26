#include <memory>
#include <Windows.h>


// Typedef for FILE_DIRECTORY_INFORMATION from: <https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/ns-ntifs-_file_directory_information>
typedef struct _FILE_DIRECTORY_INFORMATION {
	ULONG         NextEntryOffset;
	ULONG         FileIndex;
	LARGE_INTEGER CreationTime;
	LARGE_INTEGER LastAccessTime;
	LARGE_INTEGER LastWriteTime;
	LARGE_INTEGER ChangeTime;
	LARGE_INTEGER EndOfFile;
	LARGE_INTEGER AllocationSize;
	ULONG         FileAttributes;
	ULONG         FileNameLength;
	WCHAR         FileName[1];
} FILE_DIRECTORY_INFORMATION, *PFILE_DIRECTORY_INFORMATION;

// Typedef for IO_STATUS_BLOCK from: <https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/ns-wdm-_io_status_block>
typedef struct _IO_STATUS_BLOCK {
	union {
		NTSTATUS Status;
		PVOID    Pointer;
	};
	ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

// Typedef for UNICODE_STRING from: <https://learn.microsoft.com/en-us/windows/win32/api/subauth/ns-subauth-unicode_string>
typedef struct _UNICODE_STRING {
	USHORT Length;
	USHORT MaximumLength;
	PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

// Function pointer typedef and global pointer for NtQueryDirectoryFile
// Function signature from: <https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ntifs/nf-ntifs-ntquerydirectoryfile>
typedef NTSTATUS(WINAPI* PtrTy_NtQueryDirectoryFile)(HANDLE, HANDLE, PVOID, PVOID, PIO_STATUS_BLOCK, PVOID, ULONG, int, BOOLEAN, PUNICODE_STRING, BOOLEAN);
PtrTy_NtQueryDirectoryFile NtQueryDirectoryFile = nullptr;

// Function pointer typedef and global pointer for RtlInitUnicodeString
// Function signature from: <https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/wdm/nf-wdm-rtlinitunicodestring>
typedef void(WINAPI * PtrTy_RtlInitUnicodeString)(PUNICODE_STRING, PCWSTR);
PtrTy_RtlInitUnicodeString RtlInitUnicodeString = nullptr;


// Attempts to open a handle for the specified directory
HANDLE HandleForDirectory(const wchar_t* directory)
{
	return CreateFileW(
		directory,
		GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_BACKUP_SEMANTICS,
		NULL
	);
}

// Performs a query with NtQueryDirectoryFile()
void RunQuery(HANDLE handle, BOOL RestartScan, const wchar_t* mask)
{
	// Init structs for NtQueryDirectoryFile()
	IO_STATUS_BLOCK status;
	ZeroMemory(&status, sizeof(IO_STATUS_BLOCK));
	
	// Convert the filename mask to a UNICODE_STRING if one was specified
	UNICODE_STRING filename;
	if (mask) {
		RtlInitUnicodeString(&filename, mask);
	}
	
	// Allocate a buffer big enough to hold one FILE_DIRECTORY_INFORMATION object with a MAX_PATH length filename
	size_t totalSize = sizeof(FILE_DIRECTORY_INFORMATION) + (MAX_PATH * sizeof(wchar_t));
	std::unique_ptr<uint8_t> infoPlusBuffer(new uint8_t[totalSize]);
	ZeroMemory(infoPlusBuffer.get(), totalSize);
	
	// Perform the query
	NTSTATUS result = NtQueryDirectoryFile(
		handle,
		NULL,
		NULL,
		NULL,
		&status,
		infoPlusBuffer.get(),
		totalSize,
		1, // FileDirectoryInformation
		TRUE,
		(mask ? &filename : NULL),
		RestartScan
	);
	
	// Grab a pointer to the start of the buffer and typecast it to access the struct fields
	FILE_DIRECTORY_INFORMATION* info = (FILE_DIRECTORY_INFORMATION*)(infoPlusBuffer.get());
	
	// Print the query parameters and the result
	printf("RestartScan: %s, Mask: %ws, Result: %X", (RestartScan ? "true" : "false"), (mask ? mask : L"(null)"), result);
	if (result == 0) {
		printf(", Filename: %ws", info->FileName);
	}
	printf("\n");
}

int wmain(int argc, wchar_t* argv[])
{
	// Retrieve the function pointer for NtQueryDirectoryFile
	HMODULE ntdll = GetModuleHandleW(L"ntdll.dll");
	NtQueryDirectoryFile = (PtrTy_NtQueryDirectoryFile)(GetProcAddress(ntdll, "NtQueryDirectoryFile"));
	if (!NtQueryDirectoryFile)
	{
		printf("Error: failed to retrieve function pointer for NtQueryDirectoryFile!\n");
		return 1;
	}
	
	// Retrieve the function pointer for RtlInitUnicodeString
	RtlInitUnicodeString = (PtrTy_RtlInitUnicodeString)(GetProcAddress(ntdll, "RtlInitUnicodeString"));
	if (!RtlInitUnicodeString)
	{
		printf("Error: failed to retrieve function pointer for RtlInitUnicodeString!\n");
		return 1;
	}
	
	// Ensure our test directory and files exist
	system("md C:\\test 2>NUL");
	system("echo hello > C:\\test\\a-file.h");
	system("echo hello > C:\\test\\another-file.h");
	
	// Print the Windows version string
	system("ver");
	printf("\n");
	
	// Open a handle for our test directory
	HANDLE handle = HandleForDirectory(L"C:\\test");
	if (handle == INVALID_HANDLE_VALUE)
	{
		printf("Error: failed to open the handle for the input directory!\n");
		return 1;
	}
	
	// List the contents of the directory
	printf("[First Handle] List the contents of the directory...\n");
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	printf("\n");
	
	// Specify a new mask without restarting the scan
	printf("[First Handle] Specify a new mask without restarting the scan...\n");
	RunQuery(handle, FALSE, L"*.h");
	printf("\n");
	
	// Specify a new mask, restarting the scan
	printf("[First Handle] Specify a new mask, restarting the scan...\n");
	RunQuery(handle, TRUE, L"*.h");
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	printf("\n");
	
	// Restart the scan without a mask
	printf("[First Handle] Restart the scan without a mask...\n");
	RunQuery(handle, TRUE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	printf("\n");
	
	// Restart the scan with an empty mask
	printf("[First Handle] Restart the scan with an empty mask...\n");
	RunQuery(handle, TRUE, L"");
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	printf("\n");
	
	// Restart the scan with a whitespace mask
	printf("[First Handle] Restart the scan with a whitespace mask...\n");
	RunQuery(handle, TRUE, L" ");
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	printf("\n");
	
	// Restart the scan with a mask that doesn't match anything
	printf("[First Handle] Restart the scan with a mask that doesn't match anything...\n");
	RunQuery(handle, TRUE, L"not-a-file.h");
	printf("\n");
	
	// Restart the scan with a mask that matches a single file
	printf("[First Handle] Restart the scan with a mask that matches a single file...\n");
	RunQuery(handle, TRUE, L"a-file.h");
	RunQuery(handle, FALSE, nullptr);
	printf("\n");
	
	// Restart the scan with a mask that matches everything
	printf("[First Handle] Restart the scan with a mask that matches everything...\n");
	RunQuery(handle, TRUE, L"*");
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	printf("\n");
	
	// Close the handle and open a new one to the same directory
	CloseHandle(handle);
	handle = HandleForDirectory(L"C:\\test");
	if (handle == INVALID_HANDLE_VALUE)
	{
		printf("Error: failed to reopen the handle for the input directory!\n");
		return 1;
	}
	
	// Scan with a mask that doesn't match anything
	printf("[Second Handle] Scan with a mask that doesn't match anything...\n");
	RunQuery(handle, TRUE, L"not-a-file.h");
	printf("\n");
	
	// List the contents of the directory
	printf("[Second Handle] List the contents of the directory...\n");
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	RunQuery(handle, FALSE, nullptr);
	printf("\n");
	
	return 0;
}
