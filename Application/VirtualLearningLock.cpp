/**
 * Virtual Learning Lock
 * Copyright (C) 2020 Satish Sampath, All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 * http://www.gnu.org/copyleft/gpl.html
 */

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN  // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include "resource.h"
#include "../Hookdll/Hookdll.h"

enum {
  TIMER_ID_ANIMATE_FADE = 1,
  TIMER_ID_POST_QUIT,
};

HINSTANCE ghInst;

// Current and target window alpha/opacity values, used during animation.
int giCurrentAlpha;
int giTargetAlpha;

/**
 * Fades the given window alpha from startAlpha to endAlpha using WM_TIMER events.
 */
void FadeInOutWindow(HWND hWnd, int startAlpha, int endAlpha) {
  giCurrentAlpha = startAlpha;
  giTargetAlpha = endAlpha;
  SetTimer(hWnd, TIMER_ID_ANIMATE_FADE, USER_TIMER_MINIMUM, NULL);
}

/**
 * Shows the given message in a standard MessageBox
 */
void ShowMessageBox(int iStringId) {
  WCHAR wcMessage[100];
  WCHAR wcTitle[100];
  LoadStringW(ghInst, iStringId, wcMessage, ARRAYSIZE(wcMessage));
  LoadStringW(ghInst, IDS_APP_TITLE, wcTitle, ARRAYSIZE(wcTitle));
  MessageBoxW(NULL, wcMessage, wcTitle, MB_OK);
}

/**
 * Standard WindowProc
 */
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_TIMER:
    if (wParam == TIMER_ID_ANIMATE_FADE) {
      // Animate the alpha for this window forward or backward depending on the target alpha.
      // NOTE: We don't ever set 0 as the alpha value because in that case mouse events are sent
      // to the window below ours. So the lowest alpha value we set is 1, so we still get the mouse
      // events to our own window (because it has the WS_EX_TOPMOST extended style).
      SetLayeredWindowAttributes(hWnd, 0, giCurrentAlpha == 0 ? 1 : giCurrentAlpha, LWA_ALPHA);
      if (giCurrentAlpha == giTargetAlpha) {
        KillTimer(hWnd, wParam);  // done with animation.
      } else {
        giCurrentAlpha += (giCurrentAlpha < giTargetAlpha ? 10 : -10);
      }
    } else if (wParam == TIMER_ID_POST_QUIT) {
      PostQuitMessage(0);
    }
    break;

  case WM_HOTKEY:
    if (wParam == HOOK_DLL_HOTKEY_UNLOCK_CODE) {
      // Hook dll sends this when user keys in the unlock password. We show the window flashing
      // animation and then quit.
      FadeInOutWindow(hWnd, 0, 250);
      SetTimer(hWnd, TIMER_ID_POST_QUIT, 500, 0);
    }
    break;

  case WM_SYSCOMMAND:
    if (wParam == SC_CLOSE)
      return 0;  // Don't allow closing this app/window with Alt+F4 or other such means.
    // follow through for rest of sys-commands.

  default:
      return DefWindowProc(hWnd, message, wParam, lParam);
  }
  return 0;
}

BOOL CALLBACK AboutDlgProc(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK:
    case IDCANCEL:
      EndDialog(hwndDlg, wParam);
      return TRUE;
    }
  }
  return FALSE;
}

/**
 * Standard application entry function
 */
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
  // Did the user click on 'About ...' link in the start menu?
  if (lpCmdLine != NULL && _wcsicmp(lpCmdLine, L"/about") == 0) {
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUT), NULL, (DLGPROC)AboutDlgProc);
    return TRUE;
  }

  ghInst = hInstance;

  // Register window class
  WNDCLASSEXW wcex = { 0 };
  wcex.cbSize = sizeof(WNDCLASSEX);
  wcex.lpszClassName = HOOK_DLL_APP_WINDOW_CLASS;
  wcex.style = CS_HREDRAW | CS_VREDRAW;
  wcex.lpfnWndProc = WndProc;
  wcex.hInstance = hInstance;
  wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_VIRTUALLEARNINGLOCK));
  wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
  wcex.hbrBackground = (HBRUSH)(COLOR_ACTIVECAPTION + 1);

  // Create window
  HWND hWnd = NULL;
  if (RegisterClassExW(&wcex)) {
    WCHAR wcTaskbarButtonString[100];
    LoadStringW(ghInst, IDS_LOCK_ON, wcTaskbarButtonString, ARRAYSIZE(wcTaskbarButtonString));
    hWnd = CreateWindowExW(
      WS_EX_LAYERED | WS_EX_TOPMOST, HOOK_DLL_APP_WINDOW_CLASS, wcTaskbarButtonString,
      WS_OVERLAPPED, CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);
  }
  if (!hWnd) {
    ShowMessageBox(IDS_INITIALIZATION_ERROR);
    return FALSE;
  }

  // Hook/capture the keyboard to lock out all keys until password is typed in.
  if (!StartHook()) {
    ShowMessageBox(IDS_HOOK_ERROR);
    return FALSE;
  }

  // Remove all borders, title bar etc. so that this looks like a plain overlay over other windows.
  LONG lStyle = GetWindowLong(hWnd, GWL_STYLE);
  lStyle &= ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU);
  SetWindowLong(hWnd, GWL_STYLE, lStyle);
  ShowWindow(hWnd, SW_MAXIMIZE);
  UpdateWindow(hWnd);

  // Fade the window out of view, so that user can see what's behind it while we capture keys and
  // mouse events.
  FadeInOutWindow(hWnd, 250, 0);

  // Application message loop.
  MSG msg;
  while (GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  // User entered password or application was quit through other means. Stop the keyboard hook and exit.
  StopHook();
  return (int)msg.wParam;
}
