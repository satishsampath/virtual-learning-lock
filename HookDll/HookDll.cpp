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

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
#include <windows.h>
#include "HookDll.h"

// Password that user types to disable the virtual learning lock.
#define MASTER_UNLOCK_PASSWORD L"OPENSESAME"
#define MASTER_UNLOCK_PASSWORD_LENGTH (ARRAYSIZE(MASTER_UNLOCK_PASSWORD) - 1)

HINSTANCE hInstance;
HHOOK hHook = NULL;

// Queue of the last N characters typed, to check for password.
WCHAR wcKeyQueue[MASTER_UNLOCK_PASSWORD_LENGTH] = { 0 };

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ulReason, LPVOID lpReserved) {
  switch (ulReason) {
  case DLL_PROCESS_ATTACH:
    hInstance = (HINSTANCE) hModule;
    break;
  case DLL_THREAD_ATTACH:
  case DLL_THREAD_DETACH:
  case DLL_PROCESS_DETACH:
    break;
  }
  return TRUE;
}

LRESULT CALLBACK LLKeyboardHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
  if (nCode == HC_ACTION) {
    if (wParam == WM_KEYDOWN) {
      // Insert this keycode as the last character into our password queue.
      KBDLLHOOKSTRUCT* hs = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);
      memcpy(wcKeyQueue, wcKeyQueue + 1, (MASTER_UNLOCK_PASSWORD_LENGTH - 1) * sizeof(WCHAR));
      wcKeyQueue[MASTER_UNLOCK_PASSWORD_LENGTH - 1] = static_cast<WCHAR>(hs->vkCode);

      // Did the user just finish typing the unlock password?
      if (memcmp(wcKeyQueue, MASTER_UNLOCK_PASSWORD, sizeof(wcKeyQueue)) == 0) {
        // Yes! Cancel our hook and inform the main app to unlock.
        StopHook();
        HWND appWindow = FindWindow(HOOK_DLL_APP_WINDOW_CLASS, NULL);
        if (appWindow != NULL) {
          PostMessage(appWindow, WM_HOTKEY, HOOK_DLL_HOTKEY_UNLOCK_CODE, 0);
        }
      }
    }
    return 1;  // Don't let any key go to other applications, until we are unlocked.
  }
  return CallNextHookEx(hHook, nCode, wParam, lParam);
}

HOOKDLL_API BOOL StartHook() {
  hHook = SetWindowsHookEx(WH_KEYBOARD_LL, LLKeyboardHookProc, hInstance, 0);
  return hHook != NULL;
}

HOOKDLL_API void StopHook() {
  UnhookWindowsHookEx(hHook);
}
