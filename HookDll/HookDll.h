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

#ifdef HOOKDLL_EXPORTS
#define HOOKDLL_API __declspec(dllexport)
#else
#define HOOKDLL_API __declspec(dllimport)
#endif

// Starts the keyboard hook to lock keys and returns true if done.
HOOKDLL_API BOOL StartHook();

// Cancels the keyboard hook and let's keys work as normal.
HOOKDLL_API void StopHook();

// The window class used for the main app's top level window.
#define HOOK_DLL_APP_WINDOW_CLASS L"VirtualLearningAppWndClass"

// WM_HOTKEY code that the hook dll sends to the app window when user
// types in the unlock code.
#define HOOK_DLL_HOTKEY_UNLOCK_CODE 1001