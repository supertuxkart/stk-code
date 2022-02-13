; Before you start you will need the shelllink plugin for NSIS
; http://nsis.sourceforge.net/ShellLink_plug-in
; Download it from the nsis webpage, and unzip it in the NSIS
; install dir.
;
; To use just put this in a directory below the supertuxkart directory
; which should be called "supertuxkart" and then copy the
; GPL in the supertuxkart directory to 'license.txt'.
; You will then need to make an icon, you can use:
; http://tools.dynamicdrive.com/favicon/ to convert a png to an icon.
; Once you have made an icon put it in the supertuxkart dir and call it
; icon.ico. You will need to do the same for install.ico nd uninstall.ico
; Once there done then all you need to do is compile with NSIS.

;--------------------------------
;Include Modern UI
  !include "MUI2.nsh"

;--------------------------------
;Include LogicLib http://nsis.sourceforge.net/LogicLib
  !include 'LogicLib.nsh'

;--------------------------------
;Include FileFunc lib
  !include "FileFunc.nsh"

;--------------------------------
;General

  ; Version information
  ; TODO get these from the source code directly
  !define VERSION_MAJOR 1
  !define VERSION_MINOR 2
  !define VERSION_REVISION 0
  ; Empty means stable, could be -git, -rc1
  !define VERSION_BUILD ""

  ;Name and file
  !define APPNAME "SuperTuxKart"
  !define APPNAMEANDVERSION "${APPNAME} ${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}${VERSION_BUILD}"
  !define VERSION "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_REVISION}${VERSION_BUILD}"
  !define DESCRIPTION "3D open-source arcade racer with a variety characters, tracks, and modes to play"

  Name "${APPNAMEANDVERSION}"
  OutFile "${APPNAMEANDVERSION} installer-32bit.exe"

  # These will be displayed by the "Click here for support information" link in "Add/Remove Programs"
  # It is possible to use "mailto:" links in here to open the email client
  !define HELPURL "https://supertuxkart.net/" # "Support Information" link
  !define UPDATEURL "https://supertuxkart.net/" # "Product Updates" link
  !define ABOUTURL "https://supertuxkart.net/" # "Publisher" link

  RequestExecutionLevel admin

  ;Default installation folder
  InstallDir "$PROGRAMFILES\${APPNAMEANDVERSION}"

  ;Get installation folder from registry if available
  InstallDirRegKey HKCU "Software\${APPNAMEANDVERSION}" ""

  ;Sets the text in the bottom corner
  BrandingText "${APPNAMEANDVERSION} Installer"

  ;Set the icon
  !define MUI_ICON "install.ico"
  !define MUI_UNICON "uninstall.ico"
  !define MUI_HEADERIMAGE
  !define MUI_WELCOMEFINISHPAGE_BITMAP "stk_installer.bmp"
  !define MUI_WELCOMEFINISHPAGE_BITMAP_NOSTRETCH
  !define MUI_HEADERIMAGE_BITMAP "logo_slim.bmp"
  ;!define MUI_TEXT_INSTALLING_SUBTITLE "Please vote for SuperTuxKart to become SourceForge's Project of the month at vote.supertuxkart.net"
  ;!define MUI_TEXT_FINISH_INFO_TEXT "Please vote for SuperTuxKart to become $\"Project of the Month$\" at vote.supertuxkart.net"

  ; Sets the compressor to /SOLID lzma which when I (hiker) tested was the best.
  ; Between LZMA and zlib there is only a 20 MB difference.
  SetCompressor /SOLID zlib

  ;Vista redirects $SMPROGRAMS to all users without this
  RequestExecutionLevel admin

  ; For the uninstaller in the remove programs
  !define ADD_REMOVE_KEY_NAME "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APPNAMEANDVERSION}"

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
      MessageBox MB_YESNO "You can't install ${APPNAMEANDVERSION} in an existing directory. Do you wish to run the uninstaller in $INSTDIR?"  IDNO dont_uninstall
	; -?=$INSTDIR makes sure that this installer waits for the uninstaller
	; to finish. The uninstaller (and directory) are not removed, but the
	; uninstaller will be overwritten by our installer anyway.
        ExecWait '"$INSTDIR\Uninstall.exe" _?=$INSTDIR'
        goto return
    dont_uninstall:
      MessageBox MB_OK "You can't install ${APPNAMEANDVERSION} in an existing directory. Please select a new directory."
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
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APPNAMEANDVERSION}"
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


; ---------------------------------------------------------------------------
; based on http://nsis.sourceforge.net/Check_if_a_file_exists_at_compile_time
; Sets the variable _VAR_NAME to _FILE_NAME if _VAR_NAME is not defined yet
; and _FILE_NAME exists:
!macro !setIfUndefinedAndExists _VAR_NAME _FILE_NAME
    !ifndef ${_VAR_NAME}
	!tempfile _TEMPFILE
	!ifdef NSIS_WIN32_MAKENSIS
		; Windows - cmd.exe
		!system 'if exist "${_FILE_NAME}" echo !define ${_VAR_NAME} "${_FILE_NAME}" > "${_TEMPFILE}"'
	!else
		; Posix - sh
		!system 'if [ -e "${_FILE_NAME}" ]; then echo "!define ${_VAR_NAME} ${_FILE_NAME}" > "${_TEMPFILE}"; fi'
	!endif
	!include '${_TEMPFILE}'
	!delfile '${_TEMPFILE}'
	!undef _TEMPFILE
   !endif
!macroend
!define !setIfUndefinedAndExists "!insertmacro !setIfUndefinedAndExists"

;--------------------------------

;Installer Sections

Section "Install" SecMain

  SetOutPath "$INSTDIR"
  ; files in root dir

  ; Try to find the binary directory in a list of 'typical' names:
  ; The first found directory is used
  ${!setIfUndefinedAndExists} EXEC_PATH ..\..\bld\bin\RelWithDebInfo\*.*
  ${!setIfUndefinedAndExists} EXEC_PATH ..\..\bld\bin\Release\*.*
  ${!setIfUndefinedAndExists} EXEC_PATH ..\..\build\bin\RelWithDebInfo\*.*
  ${!setIfUndefinedAndExists} EXEC_PATH ..\..\build\bin\Release\*.*
  ${!setIfUndefinedAndExists} EXEC_PATH ..\..\cmake_build\bin\RelWithDebInfo\*.*
  ${!setIfUndefinedAndExists} EXEC_PATH ..\..\cmake_build\bin\Release\*.*
  ; This failed to run in mxe nsis
  ;File /x *.ilk ${EXEC_PATH}

  File /x *.ilk ../../build-mingw/bin/*.*

  ; Check various options for the editor. Note that us devs mostly use 'bld',
  ; but documented is the name 'build'
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\editor\bld\RelWithDebInfo
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\editor\bld\Release
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\stk-editor\bld\RelWithDebInfo
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\stk-editor\bld\Release
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\supertuxkart-editor\bld\RelWithDebInfo
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\supertuxkart-editor\bld\Release
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\editor\build\RelWithDebInfo
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\editor\build\Release
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\stk-editor\build\RelWithDebInfo
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\stk-editor\build\Release
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\supertuxkart-editor\build\RelWithDebInfo
  ${!setIfUndefinedAndExists} EDITOR_PATH ..\..\..\supertuxkart-editor\build\Release

  !ifdef EDITOR_PATH
    File ${EDITOR_PATH}\supertuxkart-editor.exe ${EDITOR_PATH}\supertuxkart-editor.pdb
    File ${EDITOR_PATH}\..\..\supertuxkart-editor.ico
  !endif

  File *.ico

  ; data + assets
  SetOutPath "$INSTDIR\data\"
  File /r /x .svn /x wip-* ..\..\..\stk-assets\*.*
  File /r /x *.sh ..\..\data\*.*


  ;Store installation folder
  WriteRegStr HKCU "Software\${APPNAMEANDVERSION}" "" $INSTDIR

  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    SetShellVarContext all
    CreateDirectory "$SMPROGRAMS\$STARTMENU_FOLDER"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\Uninstall ${APPNAMEANDVERSION}.lnk" "$INSTDIR\Uninstall.exe" "" "$INSTDIR\uninstall.ico"
    CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\${APPNAMEANDVERSION}.lnk" "$INSTDIR\supertuxkart.exe" "" "$INSTDIR\icon.ico"
    !ifdef EDITOR_PATH
      CreateShortCut "$SMPROGRAMS\$STARTMENU_FOLDER\supertuxkart-editor (beta).lnk" "$INSTDIR\supertuxkart-editor.exe" "" "$INSTDIR\supertuxkart-editor.ico"
    !endif
    ShellLink::SetShortCutShowMode $SMPROGRAMS\$STARTMENU_FOLDER\SuperTuxKart.lnk 0

  !insertmacro MUI_STARTMENU_WRITE_END

  ; Registry information for add/remove programs
  ; See http://nsis.sourceforge.net/Add_uninstall_information_to_Add/Remove_Programs
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" \
                 "DisplayName" "${APPNAMEANDVERSION} - ${DESCRIPTION}"
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "Publisher" "${APPNAME}"
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "DisplayIcon" "$\"$INSTDIR\icon.ico$\""
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "DisplayVersion" "${VERSION}"
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "HelpLink" "$\"${HELPURL}$\""
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "URLUpdateInfo" "$\"${UPDATEURL}$\""
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "URLInfoAbout" "$\"${ABOUTURL}$\""
  # There is no option for modifying or repairing the install
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "NoModify" 1
  WriteRegStr HKLM "${ADD_REMOVE_KEY_NAME}" "NoRepair" 1

  ; Write size
  ; [...copy all files here, before GetSize...]
  ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
  IntFmt $0 "0x%08X" $0
  WriteRegDWORD HKLM "${ADD_REMOVE_KEY_NAME}" "EstimatedSize" "$0"

SectionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall" redist

  ;Removes all the supertuxkart data files
  ; DO NOT USE RMDIR ... $INSTDIR\*.*  - if someone should e.g.
  ; install supertuxkart in c:\Program Files  (note: no subdirectory)
  ; this could remove all files in Program Files!!!!!!!!!!!!!!!!!!!

  RMDir /r /REBOOTOK $INSTDIR\data

  DELETE /REBOOTOK "$INSTDIR\install.ico"
  DELETE /REBOOTOK "$INSTDIR\icon.ico"
  DELETE /REBOOTOK "$INSTDIR\libbz2.dll"
  DELETE /REBOOTOK "$INSTDIR\libcurl.dll"
  DELETE /REBOOTOK "$INSTDIR\libeay32.dll"
  DELETE /REBOOTOK "$INSTDIR\License.txt"
  DELETE /REBOOTOK "$INSTDIR\libfreetype-6.dll"
  DELETE /REBOOTOK "$INSTDIR\libharfbuzz-0.dll"
  DELETE /REBOOTOK "$INSTDIR\libharfbuzz-subset-0.dll"
  DELETE /REBOOTOK "$INSTDIR\libidn-11.dll"
  DELETE /REBOOTOK "$INSTDIR\libjpeg-62.dll"
  DELETE /REBOOTOK "$INSTDIR\libogg-0.dll"
  DELETE /REBOOTOK "$INSTDIR\libopenglrecorder.dll"
  DELETE /REBOOTOK "$INSTDIR\libpng16.dll"
  DELETE /REBOOTOK "$INSTDIR\libturbojpeg.dll"
  DELETE /REBOOTOK "$INSTDIR\libvorbis-0.dll"
  DELETE /REBOOTOK "$INSTDIR\libvorbisenc-2.dll"
  DELETE /REBOOTOK "$INSTDIR\libvorbisfile-3.dll"
  DELETE /REBOOTOK "$INSTDIR\libvpx.dll"
  DELETE /REBOOTOK "$INSTDIR\OpenAL32.dll"
  DELETE /REBOOTOK "$INSTDIR\SDL2.dll"
  DELETE /REBOOTOK "$INSTDIR\SDL2.pdb"
  DELETE /REBOOTOK "$INSTDIR\physfs.dll"
  DELETE /REBOOTOK "$INSTDIR\ssleay32.dll"
  DELETE /REBOOTOK "$INSTDIR\supertuxkart.exe"
  DELETE /REBOOTOK "$INSTDIR\supertuxkart.ico"
  DELETE /REBOOTOK "$INSTDIR\supertuxkart.icon"
  DELETE /REBOOTOK "$INSTDIR\supertuxkart.pdb"
  DELETE /REBOOTOK "$INSTDIR\supertuxkart-editor.exe"
  DELETE /REBOOTOK "$INSTDIR\supertuxkart-editor.ico"
  DELETE /REBOOTOK "$INSTDIR\supertuxkart-editor.pdb"
  DELETE /REBOOTOK "$INSTDIR\uninstall.ico"
  Delete /REBOOTOK "$INSTDIR\Uninstall.exe"
  DELETE /REBOOTOK "$INSTDIR\zlib1.dll"
  RMDir "$INSTDIR"

  SetShellVarContext all

  ;Remove start menu items
  !insertmacro MUI_STARTMENU_GETFOLDER Application $MUI_TEMP

  Delete "$SMPROGRAMS\$MUI_TEMP\Uninstall ${APPNAMEANDVERSION}.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\${APPNAMEANDVERSION}.lnk"
  Delete "$SMPROGRAMS\$MUI_TEMP\supertuxkart-editor (beta).lnk"

  ;Delete empty start menu parent diretories
  StrCpy $MUI_TEMP "$SMPROGRAMS\$MUI_TEMP"

  startMenuDeleteLoop:
	ClearErrors
    RMDir $MUI_TEMP
    GetFullPathName $MUI_TEMP "$MUI_TEMP\.."

    IfErrors startMenuDeleteLoopDone

    StrCmp $MUI_TEMP $SMPROGRAMS startMenuDeleteLoopDone startMenuDeleteLoop
  startMenuDeleteLoopDone:

  DeleteRegKey /ifempty HKCU "Software\${APPNAMEANDVERSION}"
  DeleteRegKey HKLM "${ADD_REMOVE_KEY_NAME}"

SectionEnd
