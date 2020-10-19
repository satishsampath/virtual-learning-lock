; Virtual Learning Lock
; Copyright (C) 2020 Satish Sampath, All Rights Reserved
;
; This program is free software; you can redistribute it and/or modify
; it under the terms of the GNU General Public License as published by
; the Free Software Foundation; either version 2 of the License, or
; (at your option) any later version.
;
; This program is distributed in the hope that it will be useful,
; but WITHOUT ANY WARRANTY; without even the implied warranty of
; MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
; GNU General Public License for more details.
;
; You should have received a copy of the GNU General Public License along
; with this program; if not, write to the Free Software Foundation, Inc.,
; 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
; http://www.gnu.org/copyleft/gpl.html

Unicode true

;---- These two lines are for making the installer show up sharp & DPI aware on hi DPI displays and monitors
;---- They came from http://forums.winamp.com/showthread.php?t=452632
ManifestDPIAware System ; System DPI on Vista/7/8/8.1/10(<10.1607(AU))
ManifestDPIAwareness "PerMonitorV2,System" ; PMv2 on 10.1703(CU)+, System on 10.1607(AU) with NSIS 3.03+

Name "Virtual Learning Lock"
OutFile "VirtualLearningLockSetup.exe"
BrandingText " "
SetCompressor lzma

Page license
Page directory
Page instfiles

AutoCloseWindow true

LicenseText "Please accept the software license agreement before installing."
LicenseData "LICENSE"

InstallDir "$PROGRAMFILES\Virtual Learning Lock"
InstallDirRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Virtual Learning Lock" "InstallDir"
DirText "Installation Directory" "Select where to install Virtual Learning Lock"

;----------- begin sections ----------------

Section "" ; (default section)
	StrCpy $R0 "$INSTDIR"

	;---- Check if we have registry priviledges
	ClearErrors
	WriteRegStr HKEY_LOCAL_MACHINE "SOFTWARE\Virtual Learning Lock" "InstallDir" "$R0"
	IfErrors 0 CanWriteToReg
	MessageBox MB_OK "Could not write to the registry. Please check if you have admin priviledges."
	Abort
CanWriteToReg:

	SetOutPath "$R0"
	File LICENSE
	File Release\*.exe
	File Release\*.dll
	WriteUninstaller "$R0\uninst.exe"

	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Virtual Learning Lock" "DisplayName" "Virtual Learning Lock"
	WriteRegStr HKEY_LOCAL_MACHINE "Software\Microsoft\Windows\CurrentVersion\Uninstall\Virtual Learning Lock" "UninstallString" '"$R0\uninst.exe"'

	CreateDirectory "$SMPROGRAMS\Virtual Learning Lock"
	SetOutPath "$SMPROGRAMS\Virtual Learning Lock"
	CreateShortcut "$SMPROGRAMS\Virtual Learning Lock\Virtual Learning Lock.lnk" "$R0\VirtualLearningLock.exe"
	CreateShortcut "$SMPROGRAMS\Virtual Learning Lock\About.lnk" "$R0\VirtualLearningLock.exe" "/about"
	CreateShortcut "$SMPROGRAMS\Virtual Learning Lock\Uninstall.lnk" "$R0\uninst.exe"
SectionEnd ; end of default section

Function .onInstSuccess
	MessageBox MB_OK "Successfully installed."
FunctionEnd
  
; begin uninstall settings/section
UninstallText "This will uninstall Virtual Learning Lock from your system"

Section Uninstall
	DeleteRegKey HKEY_LOCAL_MACHINE "SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\Virtual Learning Lock"
	DeleteRegKey HKEY_CURRENT_USER "SOFTWARE\Virtual Learning Lock"

	Delete "$INSTDIR\LICENSE"
	Delete "$INSTDIR\*.exe*"
	Delete "$INSTDIR\*.dll"
	RMDir "$INSTDIR"
	RMDir /r "$SMPROGRAMS\Virtual Learning Lock"
SectionEnd ; end of uninstall section

; eof
