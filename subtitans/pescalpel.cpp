#include "subtitans.h"
#include "pescalpel.h"
#include <ImageHlp.h>

// I'd rather apply these modifications to this PE when it's not running,
// but I don't want to modify and distribute the game executable so there's that. 

// Grab the reloc section, we're in luck as the game hasn't been stripped of it
static IMAGE_SECTION_HEADER* FindRelocSection(IMAGE_NT_HEADERS* nt)
{
    IMAGE_SECTION_HEADER* sections = IMAGE_FIRST_SECTION(nt);

    for (int i = 0; i < nt->FileHeader.NumberOfSections; ++i)
    {
        char name[9] = {};
        memcpy(name, sections[i].Name, 8);

        if (strcmp(name, ".reloc") == 0)
            return &sections[i];
    }

    return nullptr;
}

// Godspeed, little executable.
static bool SwitchDynamicBaseAndNXCompatFlags(BYTE* base, bool enable)
{
    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
    {
        GetLogger()->Error("What are we trying to open here?\n");
        return false;
    }

    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE)
    {
        GetLogger()->Error("We're lacking new technology.\n");
        return false;
    }

    auto& relocDir = nt->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC];
    if (enable)
    {
        nt->OptionalHeader.DllCharacteristics |= IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
        nt->OptionalHeader.DllCharacteristics |= IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
    }
    else
    {
        nt->OptionalHeader.DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_DYNAMIC_BASE;
        nt->OptionalHeader.DllCharacteristics &= ~IMAGE_DLLCHARACTERISTICS_NX_COMPAT;
    }

    return true;
}

static bool UpdateChecksum(const wchar_t* applicationPath, BYTE* base)
{
    DWORD headerSum;
    DWORD checkSum;

    if (MapFileAndCheckSumW(applicationPath, &headerSum, &checkSum) != 0)
    {
        GetLogger()->Error("Failed to calculate checksum\n");
        return false;
    }
    
    GetLogger()->Debug("Header sum: %i, Check sum: %i\n", &headerSum, &checkSum);

    IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)base;
    if (dos->e_magic != IMAGE_DOS_SIGNATURE)
    {
        GetLogger()->Error("What are we trying to open here?\n");
        return false;
    }

    IMAGE_NT_HEADERS* nt = (IMAGE_NT_HEADERS*)(base + dos->e_lfanew);
    if (nt->Signature != IMAGE_NT_SIGNATURE)
    {
        GetLogger()->Error("We're lacking new technology.\n");
        return false;
    }

    GetLogger()->Debug("PE header check sum: %i\n", nt->OptionalHeader.CheckSum);

    // Original PE contains a value of 0, GOG seems to contain an incorrect value?
    // For now we'll just skip updating the check sum...

//    nt->OptionalHeader.CheckSum = newChecksum;

    return true;
}

static bool ApplyChanges(wchar_t* applicationPath, bool enable)
{
    const wchar_t* bakExecutable = L"submarinetitans.bak";
    const wchar_t* tmpExecutable = L"submarinetitans.tmp";
    bool fileCopied = CopyFileW(applicationPath, tmpExecutable, false);
    if (!fileCopied)
    {
        GetLogger()->Error("Failed to create submarinetitans.tmp\n");
        return false;
    }

    // Step 1: Set the flags in the header, write to disk
    // Step 2: Calculate Checksum and write to header
    for (int i = 0; i < 2; ++i)
    {
        HANDLE hFile = CreateFileW(
            tmpExecutable,
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
            nullptr,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            nullptr
        );

        if (hFile == INVALID_HANDLE_VALUE)
            return false;

        HANDLE hMap = CreateFileMappingW(hFile, nullptr, PAGE_READWRITE, 0, 0, nullptr);
        if (!hMap)
        {
            CloseHandle(hFile);
            return false;
        }

        BYTE* base = (BYTE*)MapViewOfFile(hMap, FILE_MAP_WRITE, 0, 0, 0);
        if (!base)
        {
            CloseHandle(hMap);
            CloseHandle(hFile);
            return false;
        }

        bool result = false;
        if (i == 0)
            result = SwitchDynamicBaseAndNXCompatFlags(base, enable);
        else
            result = UpdateChecksum(tmpExecutable, base);

        if (result)
            FlushViewOfFile(base, 0);

        UnmapViewOfFile(base);

        CloseHandle(hMap);
        CloseHandle(hFile);

        if (!result)
            return false;
    }

    // Move the current running process
    if (!MoveFileExW(applicationPath, bakExecutable, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        GetLogger()->Error("Failed to backup executable\n");
        return false;
    }

    // Move the modified exe in place
    if (!MoveFileExW(tmpExecutable, applicationPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH))
    {
        // Try to restore the running process
        MoveFileExW(bakExecutable, applicationPath, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

        GetLogger()->Error("Failed to apply update\n");
        return false;
    }

    // Delete the original PE later
    MoveFileExW(bakExecutable, nullptr, MOVEFILE_REPLACE_EXISTING | MOVEFILE_DELAY_UNTIL_REBOOT);

    return true;
}

bool PE::Scalpel::EnableDynamicBaseAndNXCompat()
{
    wchar_t applicationPath[MAX_PATH] = {};

    auto hModule = GetModuleHandle(NULL);
    if (GetModuleFileNameW(hModule, applicationPath, MAX_PATH) == 0)
        return false;

    return ApplyChanges(applicationPath, true);
}

bool PE::Scalpel::DisableDynamicBaseAndNXCompat()
{
    wchar_t applicationPath[MAX_PATH] = {};

    auto hModule = GetModuleHandle(NULL);
    if (GetModuleFileNameW(hModule, applicationPath, MAX_PATH) == 0)
        return false;

    return ApplyChanges(applicationPath, false);
}
