Unicode true
!include "MUI2.nsh"
!define APP "Mongolian Phonetic Keyboard"
!define EXE "mongolian-phonetic.exe"
Name "${APP}"
OutFile "..\build\MongolianPhonetic-Setup.exe"
RequestExecutionLevel user
InstallDir "$LOCALAPPDATA\Programs\MongolianPhonetic"
!define MUI_ICON "..\assets\on.ico"
!define MUI_UNICON "..\assets\on.ico"
!define MUI_FINISHPAGE_RUN "$INSTDIR\${EXE}"
!define MUI_FINISHPAGE_RUN_TEXT "Start now (tray icon, Ctrl+Alt+M toggles on/off)"
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "Mongolian"

Section "Install"
  nsExec::Exec 'taskkill /F /IM ${EXE}'
  Sleep 300
  SetOutPath "$INSTDIR"
  File "..\build\${EXE}"
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  CreateDirectory "$SMPROGRAMS\${APP}"
  CreateShortcut "$SMPROGRAMS\${APP}\${APP}.lnk" "$INSTDIR\${EXE}"
  CreateShortcut "$SMPROGRAMS\${APP}\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "MongolianPhonetic" '"$INSTDIR\${EXE}"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MongolianPhonetic" "DisplayName" "${APP}"
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MongolianPhonetic" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MongolianPhonetic" "DisplayIcon" "$INSTDIR\${EXE}"
  WriteRegDWORD HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MongolianPhonetic" "NoModify" 1
SectionEnd

Section "Uninstall"
  nsExec::Exec 'taskkill /F /IM ${EXE}'
  Sleep 300
  Delete "$INSTDIR\${EXE}"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  Delete "$SMPROGRAMS\${APP}\*.lnk"
  RMDir "$SMPROGRAMS\${APP}"
  DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "MongolianPhonetic"
  DeleteRegKey HKCU "Software\Microsoft\Windows\CurrentVersion\Uninstall\MongolianPhonetic"
SectionEnd
