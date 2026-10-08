#include <windows.h>
#include <stdio.h>

int main(void) {
    char curDir[MAX_PATH];
    GetModuleFileNameA(NULL, curDir, MAX_PATH);
    char *p = strrchr(curDir, '\\');
    if (p) *p = '\0';

    char batCmd[MAX_PATH * 2];
    snprintf(batCmd, sizeof(batCmd), "cmd.exe /c \"\"%s\\setup_requirements.bat\"\"", curDir);

    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    if (!CreateProcessA(NULL, batCmd, NULL, NULL, FALSE, 0, NULL, curDir, &si, &pi)) {
        printf("[ERROR] Failed to execute setup_requirements.bat\n");
        return 1;
    }

    WaitForSingleObject(pi.hProcess, INFINITE);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;
}
