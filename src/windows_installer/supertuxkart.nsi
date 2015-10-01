; This installer script assumes the following directory structure:
; STK_ROOT
;  |--STK-ASSETS
;  |--STK-CODE
;     |--BUILD
;  |--STK-EDITOR
;     |--BUILD
;
; To use, build supertuxkart and track editor using VS 2015, 
; then just run this script to compile with NSIS.

;--------------------------------
;Includes

  !include "MUI2.nsh"

  
;--------------------------------
; Variables

  !define AppName "SuperTuxKart"
  !define AppVersion "0.9.1-rc1"

  ; release or debug - uncomment only one
  !define BUILD_TYPE "Release"
  ;!define BUILD_TYPE "Debug"
  
  ; setup vars for install data
  !define STK_ROOT "..\..\..\"
  !define STK_ASSETS "${STK_ROOT}\stk-assets"
  !define STK_CODE "${STK_ROOT}\stk-code"
  !define STK_DATA "${STK_CODE}\data"
  !define STK_EDITOR "${STK_ROOT}\stk-editor"
  !define STK_BUILD_DIR "${STK_CODE}\build\bin\${BUILD_TYPE}"
  !define STK_EDITOR_BUILD_DIR "${STK_EDITOR}\build\${BUILD_TYPE}"

  
;--------------------------------
;General

  ;Name and file
  Name "${AppName}"
  OutFile "${AppName} ${AppVersion} Installer.exe"
  InstallDir "$PROGRAMFILES\${AppName}"
  InstallDirRegKey HKLM "SOFTWARE\${AppName} ${AppVersion}" ""

  ;Required by UAC in Vista and newer
  RequestExecutionLevel admin

  ;Clear NSIS branding text in the bottom corner
  BrandingText " "

  ;Set UI elements
  !define MUI_ICON "install.ico"
  !define MUI_UNICON "uninstall.ico"
  !define MUI_HEADERIMAGE	
  !define MUI_WELCOMEFINISHPAGE_BITMAP "stk_installer.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
  !define MUI_HEADERIMAGE_BITMAP "logo_slim.bmp"
  UninstallIcon "uninstall.ico"

  ;Set the compressor type (tested to give best compression)
  SetCompressor /SOLID lzma


;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING

;--------------------------------
Function validate_dir
  IfFileExists $INSTDIR\data\*.* 0 return
    IfFileExists $INSTDIR\Uninstall.exe 0 dont_uninstall
      MessageBox MB_YESNO "You can't install SuperTuxKart in an existing directory. Do you wish to run the uninstaller in $INSTDIR?"  IDNO dont_uninstall
	; -?=$INSTDIR makes sure that this installer waits for the uninstaller
	; to finish. The uninstaller (and directory) are not removed, but the
	; uninstaller will be overwritten by our installer anyway.
        ExecWait '"$INSTDIR\Uninstall.exe" _?=$INSTDIR'
        goto return
    dont_uninstall:
      MessageBox MB_OK "You can't install SuperTuxKart in an existing directory. Please select a new directory."
      abort
  return:
FunctionEnd
;--------------------------------
;Pages

  ;Installer pages
  !insertmacro MUI_PAGE_WELCOME
  !insertmacro MUI_PAGE_LICENSE "${STK_CODE}\COPYING"
  !define MUI_PAGE_CUSTOMFUNCTION_LEAVE validate_dir
  !insertmacro MUI_PAGE_DIRECTORY
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  ;Uninstaller pages
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

  
  ;!insertmacro MUI_UNPAGE_WELCOME
  ;!insertmacro MUI_UNPAGE_CONFIRM
  ;!insertmacro MUI_UNPAGE_INSTFILES
  ;!insertmacro MUI_UNPAGE_FINISH

;Start Menu Folder Page Configuration
  ;!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
  ;!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\SuperTuxKart"
  ;!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  ;!insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER
  
;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------

;Installer Sections

Section "Main Section" SecMain

  ; files in root dir
  SetOutPath "$INSTDIR"
  ; main program and dlls
  File /r ${STK_BUILD_DIR}\*.*
  ; track editor
  File /nonfatal /oname=supertuxkart_editor.exe ${STK_EDITOR_BUILD_DIR}\stk-editor.exe
  ; license file
  File /oname=License.txt "${STK_CODE}\COPYING"
  ; prereqs
  SetOutPath "$INSTDIR\prerequisities"
  File /r prerequisites\*.*
  ; data + assets
  SetOutPath "$INSTDIR\data\"
  ; exclude svn files, WIP tracks/karts
  File /r /x .svn /x wip-* ${STK_ASSETS}\*.*
  ; exclude linux bash scripts
  File /r /x *.sh ${STK_DATA}\*.*

  ; add registry entries
  WriteRegStr HKLM "SOFTWARE\${AppName} ${AppVersion}" "" $INSTDIR
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${AppName} ${AppVersion}" "DisplayName" "${AppName} ${AppVersion}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${AppName} ${AppVersion}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${AppName} ${AppVersion}" "NoModify" "1"
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${AppName} ${AppVersion}" "NoRepair" "1"
  
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Create shortcuts
  SetShellVarContext all
  CreateDirectory "$SMPROGRAMS\${AppName}"
  CreateShortCut "$SMPROGRAMS\${AppName}\${AppName}.lnk" "$INSTDIR\supertuxkart.exe" ""
  CreateShortCut "$SMPROGRAMS\${AppName}\${AppName} Track Editor.lnk" "$INSTDIR\supertuxkart_editor.exe" ""
  CreateShortCut "$SMPROGRAMS\${AppName}\Uninstall.lnk" "$INSTDIR\Uninstall.exe" ""
  CreateShortCut "$DESKTOP\${AppName}.lnk" "$INSTDIR\supertuxkart.exe" ""


SectionEnd

Section -Prerequisites
  SetOutPath $INSTDIR\Prerequisites
  MessageBox MB_YESNO "Install Microsoft VC++ runtime libraries?" /SD IDYES IDNO endVC
    File "prerequisites\vcredist_x86.exe"
    ExecWait "$INSTDIR\prerequisites\vcredist_x86.exe /q"
    Goto endVC
  endVC:
SectionEnd


;--------------------------------
;Uninstaller Section
Section "Uninstall"

  ; remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${AppName} ${AppVersion}"
  DeleteRegKey HKLM  "SOFTWARE\${AppName}"
  
  ; Remove start menu and desktop shortcuts
  SetShellVarContext all
  Delete "$SMPROGRAMS\${AppName}\*.*"
  RMDir "$SMPROGRAMS\${AppName}"
  Delete "$DESKTOP\${AppName}.lnk"

  ;Remove program files
  RMDir /r $INSTDIR
  
SectionEnd

