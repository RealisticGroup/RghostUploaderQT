!define Packhdr upx
!include Packhdr.nsh

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !define MUI_ICON "images/ghost.ico"
  Name "RGhost uploader"
  OutFile "RGhostUploader.exe"

  InstallDir "$PROGRAMFILES\RGhost"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\RGhost" "InstallDir"

  ;Request application privileges for Windows Vista
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var StartMenuFolder

;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "License.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\RGhost"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

  !insertmacro MUI_PAGE_INSTFILES

  !define MUI_FINISHPAGE_NOAUTOCLOSE
  !define MUI_FINISHPAGE_RUN "$INSTDIR\rghost_uploader.exe"
  !define MUI_FINISHPAGE_RUN_TEXT "Start rghost uploader"
  !define MUI_FINISHPAGE_RUN_FUNCTION "LaunchLink"
  !insertmacro MUI_PAGE_FINISH

  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
;Installer Sections

Section "RGhost Uploader" SecUploader

  SetOutPath "$INSTDIR"
  WriteRegStr HKCU "Software\RhHost" "InstallDir" $INSTDIR

  File "Release\rghost_uploader.exe"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uploader.lnk" "$INSTDIR\rghost_uploader.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "Shell Extension" SecShellEx

  WriteRegStr HKCR "*\shell\RGhost" "" "Upload to RGhost"
  WriteRegStr HKCR "*\shell\RGhost\command" "" '$INSTDIR\rghost_uploader.exe "%1"'

SectionEnd

;Descriptions

  ;Language strings
  LangString DESC_ShellEx ${LANG_ENGLISH} "Add rghost uploader to the context menu."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShellEx} $(DESC_SecShellEx)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\rghost_uploader.exe"

  RMDir "$INSTDIR"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\Uploader.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey HKCU "Software\RGhost"
  DeleteRegKey HKCR "*\shell\RGhost"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "RGhost uploader"

SectionEnd

Function LaunchLink
  ExecShell "" "$INSTDIR\rghost_uploader.exe"
FunctionEnd
