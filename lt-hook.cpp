#include <windows.h>

void sendKey(DWORD keyCodePoint);
LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);

struct KeyChange {
    const DWORD input;
    const DWORD outputLowercase;
    const DWORD outputUppercase;

    KeyChange(DWORD input, DWORD outputLowercase, DWORD outputUppercase) : 
        input(input), outputLowercase(outputLowercase), outputUppercase(outputUppercase) {}
};

const DWORD changeKey = VK_SNAPSHOT; //print screen key
const KeyChange changedKeys[] = {
    KeyChange(0x34, 0x34, 0x20AC), //$ to € when uppercase
    KeyChange(0x41, 0x0105, 0x0104), //a to ą
    KeyChange(0x43, 0x010d, 0x010c), //c to č
    KeyChange(0x45, 0x0119, 0x0118), //e to ę
    KeyChange(0x57, 0x0117, 0x0116), //w to ė
    KeyChange(0x49, 0x012f, 0x012e), //i to į
    KeyChange(0x53, 0x0161, 0x0160), //s to š
    KeyChange(0x55, 0x0173, 0x0172), //u to ų
    KeyChange(0x59, 0x016b, 0x016a), //y to ū
    KeyChange(0x5A, 0x017e, 0x017d), //z to ž
};

HHOOK hhook = NULL;
bool changeKeyDown = false;

INT CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, INT nCmdShow)
{
    hhook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, hInstance, NULL);

    MSG msg;
    while (!GetMessage(&msg, NULL, NULL, NULL)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UnhookWindowsHookEx(hhook);
    return 0;
}

void sendKey(DWORD keyCodePoint);

LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    bool eatKeyStroke = false;

    if (nCode == HC_ACTION) {
        PKBDLLHOOKSTRUCT p = (PKBDLLHOOKSTRUCT)lParam;

        switch (wParam) {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
                if (p->vkCode == changeKey) {
                    changeKeyDown = true;
                }
                for (const KeyChange& entry : changedKeys) {
                    if (changeKeyDown && p->vkCode == entry.input) {
                        eatKeyStroke = true;
                        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) { //shift is currently held down
                            sendKey(entry.outputUppercase);
                        }
                        else {
                            sendKey(entry.outputLowercase);
                        }
                    }
                }
                break;
            case WM_KEYUP:
            case WM_SYSKEYUP:
                if (p->vkCode == changeKey) {
                    changeKeyDown = false;
                }
        }

    }

    return eatKeyStroke ? 1 : CallNextHookEx(hhook, nCode, wParam, lParam);
}

void sendKey(DWORD keyCodePoint) {
    KEYBDINPUT kb = { 0 };
    INPUT Input = { 0 };

    // down
    kb.wScan = (WORD) keyCodePoint;
    kb.dwFlags = KEYEVENTF_UNICODE;
    Input.type = INPUT_KEYBOARD;
    Input.ki = kb;
    ::SendInput(1, &Input, sizeof(Input));

    // up
    kb.wScan = (WORD) keyCodePoint;
    kb.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    Input.type = INPUT_KEYBOARD;
    Input.ki = kb;
    ::SendInput(1, &Input, sizeof(Input));
}
