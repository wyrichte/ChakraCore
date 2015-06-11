!if "$(MANAGED_RESOURCE_FILES)" != ""

$(MANAGED_RESOURCE_FILES):
!if "$(BUILD_PASS)" == "PASS0"
	$(URTRUN_20) resgen /useSourcePath /compile $@
!endif

!endif

# $(_NTBINDIR)\tools\managed\v4.0\mt.exe -manifest $(ProjectDir)$(TargetFileName).manifest -outputresource:$(TargetDir)$(TargetFileName);1

MERGE_MANIFEST_FILE:
!if "$(BUILD_PASS)" == "PASS2"
    rem dir z:\fbl_ie_script_dev.obj.x86chk\inetcore\jscript\tools\jsetwconsole\objchk\i386
	echo $(URTRUN_20) mt -manifest %CD%\$(TARGETNAME).manifest -outputresource:%_NTTREE%\$(TARGET_DESTINATION)\$(TARGETNAME);1
	$(URTRUN_20) mt -manifest %CD%\$(TARGETNAME).exe.manifest -outputresource:%_NTTREE%\$(TARGET_DESTINATION)\$(TARGETNAME).exe;1
!endif
