# !packhdr $%TEMP%\exehead.tmp `upx -9 "$%TEMP%\exehead.tmp"`

!verbose 4

!define APPLICATION_SHORTNAME "RGhost"
!define APPLICATION_NAME "RGhost Uploader"
!define APPLICATION_VENDOR "RGhost"
!define APPLICATION_EXECUTABLE "rghost_uploader.exe"

; Safe to use Win64's exe version since we require both builds for this combined installer.
!getdllversion "release\${APPLICATION_EXECUTABLE}" expv_
!define VER_MAJOR "${expv_1}"
!define VER_MINOR "${expv_2}"
!define VER_PATCH "${expv_3}"
!define VER_BUILD "${expv_4}"
!define VERSION "${expv_1}.${expv_2}.${expv_3}.${expv_4}"

VIProductVersion "${VERSION}"
VIAddVersionKey "ProductName" "${APPLICATION_NAME}"
VIAddVersionKey "CompanyName" "${APPLICATION_VENDOR}"
VIAddVersionKey "FileVersion" "${VERSION}"

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !define MUI_ICON "images/ghost.ico"
  Name "${APPLICATION_NAME}"
  OutFile "release/RGhostUploaderInstaller.exe"

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
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APPLICATION_SHORTNAME}"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"

  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder

  !insertmacro MUI_PAGE_INSTFILES

  !define MUI_FINISHPAGE_NOAUTOCLOSE
  !define MUI_FINISHPAGE_RUN "$INSTDIR\${APPLICATION_EXECUTABLE}"
  !define MUI_FINISHPAGE_RUN_TEXT "Start ${APPLICATION_NAME}"
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
  WriteRegStr HKCU "Software\RGhost" "InstallDir" $INSTDIR

  File "release\${APPLICATION_EXECUTABLE}"

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uploader.lnk" "$INSTDIR\${APPLICATION_EXECUTABLE}"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_END
SectionEnd

Section "Shell Extension" SecShellEx

  WriteRegStr HKCR "*\shell\RGhost" "" "Upload to ${APPLICATION_SHORTNAME}"
  WriteRegStr HKCR "*\shell\RGhost" "Icon" "$INSTDIR\${APPLICATION_EXECUTABLE}"
  WriteRegStr HKCR "*\shell\RGhost\command" "" '$INSTDIR\${APPLICATION_EXECUTABLE} "%1"'

SectionEnd

;Descriptions

  ;Language strings
  LangString DESC_ShellEx ${LANG_ENGLISH} "Add ${APPLICATION_NAME} to the context menu."

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecShellEx} $(DESC_SecShellEx)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ;ADD YOUR OWN FILES HERE...

  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\${APPLICATION_EXECUTABLE}"

  RMDir "$INSTDIR"

  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder

  Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk"
  Delete "$SMPROGRAMS\$StartMenuFolder\RGhost Uploader.lnk"
  RMDir "$SMPROGRAMS\$StartMenuFolder"

  DeleteRegKey HKCU "Software\${APPLICATION_SHORTNAME}"
  DeleteRegKey HKCR "*\shell\${APPLICATION_SHORTNAME}"
  DeleteRegValue HKLM "Software\Microsoft\Windows\CurrentVersion\Run" "${APPLICATION_NAME}"

SectionEnd

Function LaunchLink
  ExecShell "" "$INSTDIR\${APPLICATION_EXECUTABLE}"
FunctionEnd
