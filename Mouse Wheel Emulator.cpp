#include <windows.h>
#include <shlobj.h>
#include <tchar.h>

#define WM_TRAYICON (WM_USER + 1)

HINSTANCE hInst;
NOTIFYICONDATA nid;
UINT keyUp = VK_UP;
UINT keyDown = VK_DOWN;

void emulateMouseWheelScroll(int delta) {
	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.dwFlags = MOUSEEVENTF_WHEEL;
	input.mi.mouseData = delta;
	SendInput(1, &input, sizeof(INPUT));
}

void registerHotkeys(HWND hwnd) {
	UnregisterHotKey(hwnd, 1);
	UnregisterHotKey(hwnd, 2);
	RegisterHotKey(hwnd, 1, 0, keyUp);
	RegisterHotKey(hwnd, 2, 0, keyDown);
}

bool addToStartup() {
	TCHAR path[MAX_PATH];
	if (GetModuleFileName(NULL, path, MAX_PATH) == 0) {
		return false;
	}
	HKEY hKey;
	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey) != ERROR_SUCCESS) {
		return false;
	}
	if (RegSetValueEx(hKey, TEXT("Mouse Wheel Emulator"), 0, REG_SZ, (BYTE*)path, (_tcslen(path) + 1) * sizeof(TCHAR)) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return false;
	}
	RegCloseKey(hKey);
	return true;
}

bool removeFromStartup() {
	HKEY hKey;
	if (RegOpenKey(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey) != ERROR_SUCCESS) {
		return false;
	}
	if (RegDeleteValue(hKey, TEXT("Mouse Wheel Emulator")) != ERROR_SUCCESS) {
		RegCloseKey(hKey);
		return false;
	}
	RegCloseKey(hKey);
	return true;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_HOTKEY) {
		if (wParam == 1) {
			emulateMouseWheelScroll(120);
		}
		else if (wParam == 2) {
			emulateMouseWheelScroll(-120);
		}
	}
	else if (msg == WM_TRAYICON) {
		if (lParam == WM_RBUTTONDOWN) {
			HMENU hMenu = CreatePopupMenu();
			AppendMenu(hMenu, MF_STRING, 1, TEXT("About"));
			AppendMenu(hMenu, MF_STRING, 2, TEXT("Add to Startup"));
			AppendMenu(hMenu, MF_STRING, 3, TEXT("Remove from Startup"));
			AppendMenu(hMenu, MF_STRING, 4, TEXT("Exit"));
			POINT pt;
			GetCursorPos(&pt);
			SetForegroundWindow(hwnd);
			int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD | TPM_NONOTIFY, pt.x, pt.y, 0, hwnd, NULL);
			if (cmd == 1) {
				MessageBox(hwnd, TEXT("Mouse Wheel Emulator\nVersion:1.0\nCreated by:weapon_smg1"), TEXT("About"), MB_ICONINFORMATION | MB_OK);
			}
			else if (cmd == 2) {
				if (addToStartup()) {
					MessageBox(hwnd, TEXT("Program added to startup."), TEXT("Mouse Wheel Emulator"), MB_OK | MB_ICONINFORMATION);
				}
				else {
					MessageBox(hwnd, TEXT("Failed to add program to startup."), TEXT("Mouse Wheel Emulator"), MB_OK | MB_ICONERROR);
				}
			}
			else if (cmd == 3) {
				if (removeFromStartup()) {
					MessageBox(hwnd, TEXT("Program removed from startup."), TEXT("Mouse Wheel Emulator"), MB_OK | MB_ICONINFORMATION);
				}
				else {
					MessageBox(hwnd, TEXT("Failed to remove program from startup."), TEXT("Mouse Wheel Emulator"), MB_OK | MB_ICONERROR);
				}
			}
			else if (cmd == 4) {
				DestroyWindow(hwnd);
			}
			DestroyMenu(hMenu);
		}
	}
	else if (msg == WM_DESTROY) {
		Shell_NotifyIcon(NIM_DELETE, &nid);
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

void createTrayIcon(HWND hwnd) {
	ZeroMemory(&nid, sizeof(nid));
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hwnd;
	nid.uID = 1;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_TRAYICON;
	nid.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	lstrcpy(nid.szTip, TEXT("Mouse Wheel Emulator"));
	Shell_NotifyIcon(NIM_ADD, &nid);
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	hInst = hInstance;

	WNDCLASS wc = { 0 };
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = TEXT("TrayApp");

	RegisterClass(&wc);

	HWND hwnd = CreateWindow(wc.lpszClassName, TEXT("Mouse Wheel Emulator"), 0, 0, 0, 0, 0, NULL, NULL, hInstance, NULL);
	createTrayIcon(hwnd);

	MessageBox(hwnd, TEXT("Mouse Wheel Emulator has started"), TEXT("Mouse Wheel Emulator"), MB_OK | MB_ICONINFORMATION);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		if (msg.message == WM_KEYDOWN) {
			if (msg.wParam == keyUp) {
				emulateMouseWheelScroll(120);
			}
			else if (msg.wParam == keyDown) {
				emulateMouseWheelScroll(-120);
			}
			else if (msg.wParam == VK_ESCAPE) {
				DestroyWindow(hwnd);
			}
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
