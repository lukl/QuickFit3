!include "FileAssociation.nsh"

# A few handy definitions to avoid repetition
!define COMPANY_NAME "German Cancer Research Center (DKFZ)"
!define BIT_DEPTH "%%BITDEPTH%%"
!define PRODUCT_NAME "QuickFit 3.0 (${BIT_DEPTH}-bits)"
!define PRODUCT_VERSION " (SVN %%SVNVER%% COMPILEDATE: %%COMPILEDATE%%, %%BITDEPTH%%-bit)"
!define HELPURL "http://www.dkfz.de/Macromol/quickfit/"
!define UPDATEURL "http://www.dkfz.de/Macromol/quickfit/#download"
!define URLInfoAbout "http://www.dkfz.de/Macromol/quickfit/"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\${COMPANY_NAME} ${PRODUCT_NAME}"
!define REG_KEY "Software\QuickFit3_%%BITDEPTH%%"
# Set the installer name and compilation properties
OutFile "%%INSTALLER_BASENAME%%_setup.exe"
RequestExecutionLevel admin

# Set some basic installer properties
Name "${PRODUCT_NAME}"
BrandingText "Copyright (c) 2013-2015 by Jan W. Krieger (German Cancer research Center - DKFZ)"
ShowInstDetails show
ShowUninstDetails show
SetCompressor /FINAL  LZMA 

# we have two install-types:
InstType "default: (imaging) FCS/FCCS + ALEX spFRET"
InstType "minimal (basic plugins only)"
InstType "full"
InstType "FCS/FCCS"
InstType "FCS/FCCS + ALEX spFRET"
InstType "(imaging) FCS/FCCS"

# Set Version Information
#VIProductVersion ${PRODUCT_VERSION}
#VIAddVersionKey "ProductName" "Installer for ${PRODUCT_NAME}"
#VIAddVersionKey "CompanyName" ${COMPANY_NAME}
#VIAddVersionKey "LegalCopyright" ${BrandingText}
#VIAddVersionKey "FileDescription" "Installer Application"
  

# Set the default installation location
InstallDir "%%INSTALLER_INSTDIR%%\${PRODUCT_NAME}"

# Include Multiuser setup
!define MULTIUSER_EXECUTIONLEVEL Highest
!define MULTIUSER_MUI
!define MULTIUSER_INSTALLMODE_COMMANDLINE
!include MultiUser.nsh

# Include the ModernUI 2 library to make everything look nice
!include "MUI2.nsh"
!include "MUI_EXTRAPAGES.nsh"

# Installer pages
# We use definitions described in the ModernUI 2 documentation to customize the UI
Var StartMenuFolder
!define MUI_WELCOMEFINISHPAGE_BITMAP "..\images\splash_installer.bmp"
!define MUI_UNWELCOMEFINISHPAGE_BITMAP  "..\images\splash_installer.bmp"
!define MUI_WELCOMEPAGE_TITLE "Install ${PRODUCT_NAME}, ${BIT_DEPTH}-bit (%%SVNVER%%, %%COMPILEDATE%%)!"
!define MUI_COMPONENTSPAGE
!define MUI_COMPONENTSPAGE_NODESC
!define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKLM" 
!define MUI_STARTMENUPAGE_REGISTRY_KEY ${REG_KEY}
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder" 
!define MUI_STARTMENUPAGE_DEFAULT_FOLDER "${PRODUCT_NAME}"
!define MUI_FINISHPAGE_RUN "quickfit3.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Launch ${PRODUCT_NAME} now!"
!define MUI_FINISHPAGE_SHOWREADME ""
!define MUI_FINISHPAGE_SHOWREADME_CHECKED
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Create Desktop Shortcut"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION finishpageaction
!define MUI_FINISHPAGE_LINK "QuickFit Webpage"
!define MUI_FINISHPAGE_LINK_LOCATION ${URLInfoAbout}

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_README "README.txt"
!insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES

!insertmacro MUI_PAGE_FINISH

!include "FileFunc.nsh"
 
# Remember the installation directory for future updates and the uninstaller
InstallDirRegKey HKLM "${REG_KEY}" "InstallDir"



# Uninstaller pages
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_COMPONENTS
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

# Make sure the appropriate ModernUI language strings are included
!insertmacro MUI_LANGUAGE "English"

  ;Set up install lang strings for 1st lang
  ${ReadmeLanguage} "${LANG_ENGLISH}" \
          "Read Me" \
          "Please review the following important information." \
          "About $(^name):" \
          "$\n  Click on scrollbar arrows or press Page Down to review the entire text."
 
  ;Set up uninstall lang strings for 1st lang
  ${Un.ReadmeLanguage} "${LANG_ENGLISH}" \
          "Read Me" \
          "Please review the following important Uninstall information." \
          "About $(^name) Uninstall:" \
          "$\n  Click on scrollbar arrows or press Page Down to review the entire text."


#Macro to create URL shortcut
!macro CreateInternetShortcut FILENAME URL ICONFILE ICONINDEX
WriteINIStr "${FILENAME}.url" "InternetShortcut" "URL" "${URL}"
WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconFile" "${ICONFILE}"
WriteINIStr "${FILENAME}.url" "InternetShortcut" "IconIndex" "${ICONINDEX}"
!macroend

!macro VerifyUserIsAdmin
UserInfo::GetAccountType
pop $0
${If} $0 != "admin" ;Require admin rights on NT4+
        messageBox mb_iconstop "Administrator rights required!"
        setErrorLevel 740 ;ERROR_ELEVATION_REQUIRED
        quit
${EndIf}
!macroend


#function to create desktop shortcut when the  user selects it
Function finishpageaction
	CreateShortCut "$desktop\${PRODUCT_NAME}.lnk" "$INSTDIR\quickfit3.exe"
FunctionEnd

SectionGroup "${PRODUCT_NAME} Basic Install"

# This installs the main application
Section "${PRODUCT_NAME} ${PRODUCT_VERSION}" sec_main
	Push $OUTDIR ; Store previous output directory
	SetOutPath "$INSTDIR\" ; Set output directory

	SectionIn 1 RO
	SectionIn 2 RO
	SectionIn 3 RO
	SectionIn 4 RO
	SectionIn 5 RO
	SectionIn 6 RO
	
	#CreateShortCut "$SMPROGRAMS${COMPANY_NAME}${PRODUCT_NAME}.lnk" "$OUTDIRquickfit3.exe"
	
	# automatically created list of install files
	%%INSTALLER_DIRS%%
	%%INSTALLER_FILES%%

	# Create an uninstaller and add it to the Add an Add/Remove programs list
	WriteUninstaller Uninstall.exe
	WriteRegStr HKLM  "${UNINSTALL_KEY}" DisplayName "${PRODUCT_NAME}"
	WriteRegStr HKLM  "${UNINSTALL_KEY}" DisplayIcon "$OUTDIR\Uninstall.exe"
	WriteRegStr HKLM  "${UNINSTALL_KEY}" UninstallString "$OUTDIR\Uninstall.exe"
	
	
	
	WriteRegStr HKLM  "${UNINSTALL_KEY}" "DisplayName" "${PRODUCT_NAME}"
	WriteRegStr HKLM  "${UNINSTALL_KEY}" "UninstallString" "$\"$INSTDIR\Uninstall.exe$\""
	WriteRegStr HKLM  "${UNINSTALL_KEY}" "QuietUninstallString" "$\"$INSTDIR\Uninstall.exe$\" /S"
	WriteRegStr HKLM  "${UNINSTALL_KEY}" "InstallLocation" "$\"$INSTDIR$\""
	WriteRegStr HKLM  "${UNINSTALL_KEY}" "DisplayIcon" "$\"$INSTDIR\qf3icon.ico$\""
	WriteRegStr HKLM  "${UNINSTALL_KEY}" "Publisher" "$\"${COMPANY_NAME}$\""
	WriteRegStr HKLM  "${UNINSTALL_KEY}" "URLUpdateInfo" "$\"${UPDATEURL}$\""
	WriteRegStr HKLM  "${UNINSTALL_KEY}" "URLInfoAbout" "$\"${HELPURL}$\""
	
	# There is no option for modifying or repairing the install
	WriteRegDWORD HKLM  "${UNINSTALL_KEY}" "NoModify" 1
	WriteRegDWORD HKLM  "${UNINSTALL_KEY}" "NoRepair" 1
	
	# Set the INSTALLSIZE constant (!defined at the top of this script) so Add/Remove Programs can accurately report the size
	 ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
	IntFmt $0 "0x%08X" $0
	WriteRegDWORD HKLM  "${UNINSTALL_KEY}" "EstimatedSize" "$0"

	!insertmacro MUI_STARTMENU_WRITE_BEGIN Application
	# write start menu entries
	createDirectory "$SMPROGRAMS\$StartMenuFolder"
	createShortCut "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}.lnk" "$INSTDIR\quickfit3.exe" "" "$INSTDIR\qf3icon.ico"
	!insertmacro CreateInternetShortcut \
		  "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME} homepage" \
		  "${HELPURL}" \
		  "" "0"
	createShortCut "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}_releasenotes.lnk" "$INSTDIR\releasenotes.html" "" "" 0 SW_SHOWNORMAL "" "Release Notes" 
	createShortCut "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}_license.lnk" "$INSTDIR\LICENSE.txt" "" "" 0 SW_SHOWNORMAL "" "License" 
	createShortCut "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}_readme.lnk" "$INSTDIR\README.txt" "" "" 0 SW_SHOWNORMAL "" "Read Me" 
	createShortCut "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}_uninstall.lnk" "$INSTDIR\Uninstall.exe" "" 
	
	
	# store to registry, whether special version
	SetRegView 64
	WriteRegStr HKLM  "${REG_KEY}" 'SpecialVersion' '%%STORESPECIALVERSION%%'
	WriteRegStr HKCU  "${REG_KEY}" 'SpecialVersion' '%%STORESPECIALVERSION%%'
	SetRegView 32
	WriteRegStr HKLM  "${REG_KEY}" 'SpecialVersion' '%%STORESPECIALVERSION%%'
	WriteRegStr HKCU  "${REG_KEY}" 'SpecialVersion' '%%STORESPECIALVERSION%%'

	
!insertmacro MUI_STARTMENU_WRITE_END
	
	Pop $OUTDIR ; Restore the original output directory
SectionEnd



# This section does the file extension association
Section "associate file extensions (*.qfp/qfpz)" sec_assoc
	Push $OUTDIR ; Store previous output directory
	SetOutPath "$INSTDIR\" ; Set output directory
	
	SectionIn 1
	SectionIn 2
	SectionIn 3 
	SectionIn 4
	SectionIn 5 
	SectionIn 6 
	
	${registerExtension} "$INSTDIR\quickfit3.exe" ".qfp" "QFP_File"
	${registerExtension} "$INSTDIR\quickfit3.exe" ".qfpz" "QFPZ_File"
	${registerExtension} "$INSTDIR\quickfit3.exe" ".qfp.gz" "QFPGZ_File"

	Pop $OUTDIR ; Restore the original output directory
SectionEnd

SectionGroupEnd

SectionGroup "Additional Plugins"

%%ADDITIONAL_SECTIONS%%

SectionGroupEnd





function .onInit
	  ReadRegStr $R0 HKLM \
	  "${UNINSTALL_KEY}" \
	  "UninstallString"
	  StrCmp $R0 "" done
	 
	  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
	  "${PRODUCT_NAME} is already installed. $\n$\nClick `OK` to remove the \
	  previous version or `Cancel` to cancel this upgrade." \
	  IDOK uninst
	  Abort
	 
	;Run the uninstaller
	uninst:
	  ClearErrors
	  SetRebootFlag false
	  ExecWait '$R0 _?=$INSTDIR' ;Do not copy the uninstaller to a temp file
	 
	  IfErrors no_remove_uninstaller done
		;You can either use Delete /REBOOTOK in the uninstaller or add some code
		;here to remove the uninstaller. Use a registry key to check
		;whether the user has chosen to uninstall. If you are using an uninstaller
		;components page, make sure all sections are uninstalled.
	  no_remove_uninstaller:
	 
	done:


  	  setShellVarContext all
	  !insertmacro VerifyUserIsAdmin
	  !insertmacro MULTIUSER_INIT
	  IntOp $0 ${SF_SELECTED} | ${SF_RO}
	  IntOp $0 $0 | ${SF_BOLD}
      SectionSetFlags ${sec_main} $0

	  %%SEC_MODES%%

functionEnd


#Uninstaller
function un.onInit
	SetShellVarContext all
	SetRebootFlag false
 
	#Verify the uninstaller - last chance to back out
	MessageBox MB_OKCANCEL "Permanantly remove ${PRODUCT_NAME}?" IDOK next
		Abort
	next:
	!insertmacro VerifyUserIsAdmin
	!insertmacro MULTIUSER_UNINIT
functionEnd


# This uninstalls the main application and everything else
Section "un.${PRODUCT_NAME} ${PRODUCT_VERSION}" sec_un_main

	!insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
	
	# Remove the program and Start menu shortcut
	Delete "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}_uninstall.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME} homepage.url"
	Delete "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}_releasenotes.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}_license.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}_readme.lnk"
	Delete "$SMPROGRAMS\$StartMenuFolder\${PRODUCT_NAME}.lnk"
	RMDir "$SMPROGRAMS\$StartMenuFolder"

	# automatically created list of install files
	%%UNINSTALLER_FILES%%
	
	RMDir "$SMPROGRAMS${COMPANY_NAME}"
	
	# Remove the uninstaller and Add/Remove programs information
	Delete "$INSTDIR\Uninstall.exe"
	DeleteRegKey HKLM  "${UNINSTALL_KEY}"
	
	# Remove the installation directory if empty
	RMDir /r "$INSTDIR"
SectionEnd