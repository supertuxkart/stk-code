; Before you start you will need the shelllink plugin for NSIS
; Download it from the nsis webpage, and unzip it in the NSIS
; install dir.
;
; To use just put this in a directory below the supertuxkart directory
; which should be called "supertuxkart" and then copy the
; GPL in the supertuxkart directory to 'license.txt'.
; Next to supertuxkart create a subdirectory called 'prerequisites'
; and copy the VC++ (vcredist_x86.exe) and OpenAL (oalinst.exe) 
; redistributables.
; You will then need to make an icon, you can use:
; http://tools.dynamicdrive.com/favicon/ to convert a png to an icon.
; Once you have made an icon put it in the supertuxkart dir and call it
; icon.ico. You will need to do the same for install.ico nd uninstall.ico
; Once there done then all you need to do is compile with NSIS.

;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"

;--------------------------------
;General

  ;Name and file
  Name "SuperTuxKart"
  OutFile "supertuxkart-installer.exe"

  RequestExecutionLevel admin

  ;Default installation folder
  InstallDir "$PROGRAMFILES\SuperTuxKart"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\SuperTuxKart" ""

  ;Sets the text in the bottom corner
  BrandingText "SuperTuxKart Installer"

  ;Set the icon
  !define MUI_ICON "install.ico"
  !define MUI_UNICON "uninstall.ico"
  !define MUI_HEADERIMAGE	
  !define MUI_WELCOMEFINISHPAGE_BITMAP "stk_installer.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
  !define MUI_HEADERIMAGE_BITMAP "logo_slim.bmp"
  ;!define MUI_TEXT_INSTALLING_SUBTITLE "Please vote for SuperTuxKart to become SourceForge's Project of the month at vote.supertuxkart.net"
  ;!define MUI_TEXT_FINISH_INFO_TEXT "Please vote for SuperTuxKart to become $\"Project of the Month$\" at vote.supertuxkart.net"

  ;Sets the compressor to /SOLID lzma which when I tested was the best
  SetCompressor /SOLID lzma

  ;Vista redirects $SMPROGRAMS to all users without this
  RequestExecutionLevel admin

;--------------------------------
;Variables

  Var MUI_TEMP
  Var STARTMENU_FOLDER

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
  !insertmacro MUI_PAGE_LICENSE "..\..\COPYING"

  !define MUI_PAGE_CUSTOMFUNCTION_LEAVE validate_dir
  !insertmacro MUI_PAGE_DIRECTORY

  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\SuperTuxKart"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  !insertmacro MUI_PAGE_STARTMENU Application $STARTMENU_FOLDER

  !insertmacro MUI_PAGE_INSTFILES
  ;!define MUI_FINISHPAGE_LINK "Please vote for SuperTuxkart here"
  ;!define MUI_FINISHPAGE_LINK_LOCATION "http://vote.supertuxkart.net"
  !insertmacro MUI_PAGE_FINISH


  ;Uninstaller pages
  !insertmacro MUI_UNPAGE_WELCOME
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES
  !insertmacro MUI_UNPAGE_FINISH

;--------------------------------
;Languages

  !insertmacro MUI_LANGUAGE "English"

;--------------------------------

;Installer Sections

Section "Main Section" SecMain

  SetOutPath "$INSTDIR"
  ; files in root dir
  File /r ..\..\..\build\bin\release\*.*
  File *.ico 
  ; prereqs
  SetOutPath "$INSTDIR\prerequisities"
  File /r prerequisites\*.*
  ; data + assets
  SetOutPath "$INSTDIR\data\"
  File /r /x .svn /x wip-* ..\..\..\stk-assets\*.*
  File /r /x *.sh ..\..\data\*.*

  

  ;Store installation folder
  WriteRegStr HKCU "Software\SuperTuxKart" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\uninstall.ico"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\SuperTuxKart.lnk" "$INSTDIR\supertuxkart.exe" "" "$INSTDIR\icon.ico"
    ;CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\supertuxkart_editor.lnk" "$INSTDIR\supertuxkart_editor.exe" "" "$INSTDIR\supertuxkart_editor.ico"
    ShellLink::SetShortCutShowMode $SMPROGRAMS\$STARTMENU_FOLDER\SuperTuxKart.lnk 0

  !insertmacro MUI_STARTMENU_WRITE_END

SectionEnd

Section -Prerequisites
  ;SetOutPath $INSTDIR\Prerequisites
  MessageBox MB_YESNO "Install Microsoft VC++ runtime libraries?" /SD IDYES IDNO endVC
    File "prerequisites\vcredist_x86.exe"
    ExecWait "$INSTDIR\prerequisites\vcredist_x86.exe /q"
    Goto endVC
  endVC:
  MessageBox MB_YESNO "Install OpenAL sound libraries?" /SD IDYES IDNO endOA
    File "prerequisites\oalinst.exe"
    ExecWait "$INSTDIR\prerequisites\oalinst.exe"
    Goto endOA
  endOA:
SectionEnd
;--------------------------------
;Uninstaller Section

Section "Uninstall"redist

  ;Removes all the supertuxkart data files
  RMDir /r /REBOOTOK $INSTDIR\*.*
  RMDir "$INSTDIR"

  SetShellVarContext all

  ;Remove start menu items
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\supertuxkart.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\supertuxkart_editor.lnk"

  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

  startMenuDeleteLoop:
	ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."

    IfErrors startMenuDeleteLoopDone

    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:


  DeleteRegKey /ifempty HKCU "Software\SuperTuxKart"

SectionEnd