#include <windows.h>
#include <stdio.h>
#include <string.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    char basePath[MAX_PATH];
    GetModuleFileNameA(NULL, basePath, MAX_PATH);
    char *p = strrchr(basePath, '\\');
    if (p) *p = '\0';

    char otvdmPath[MAX_PATH];
    char cmdLine[MAX_PATH * 3];
    char workDir[MAX_PATH];

    snprintf(otvdmPath, sizeof(otvdmPath), "%s\\tools\\otvdm\\otvdmw.exe", basePath);
    snprintf(cmdLine, sizeof(cmdLine), "\"%s\\tools\\otvdm\\otvdmw.exe\" \"%s\\chips_challenge\\CHIPS.EXE\"", basePath, basePath);
    snprintf(workDir, sizeof(workDir), "%s\\chips_challenge", basePath);

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    if (!CreateProcessA(otvdmPath, cmdLine, NULL, NULL, FALSE, 0, NULL, workDir, &si, &pi)) {
        MessageBoxA(NULL, "Gagal menjalankan WineVDM (otvdmw.exe)!\nPastikan Anda telah menjalankan setup_requirements.bat.", "Error Launcher", MB_ICONERROR);
        return 1;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
