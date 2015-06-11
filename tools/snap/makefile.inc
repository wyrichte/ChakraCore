$(TARGETNAME).cab :
    echo $(BUILD_PASS)
!if "$(BUILD_PASS)" == "PASS2"
# Check some environment variables to see if we should build the unit tests cab file. We only want to do this on SNAP or build lab builds.
# SNAP_BIN is the root bin directory for SNAP. This should only be set for SNAP builds.
# OFFICIAL_BUILD_MACHINE should indicate primary/secondary/etc status for the current build machine and is only present in build lab.
# TIMEBUILD is a flag indicating that the build is a time build.
# USERNAME is the alias of the dev. For build lab, this is wincbld.
   echo BUILDMSG: SNAP_BIN is "$(SNAP_BIN)", OFFICIAL_BUILD_MACHINE is "$(OFFICIAL_BUILD_MACHINE)", TIMEBUILD is "$(TIMEBUILD)", USERNAME is "$(USERNAME)"
!if "$(SNAP_BIN)" != "" || "$(OFFICIAL_BUILD_MACHINE)" != "" || "$(TIMEBUILD)" != "" || "$(USERNAME)" == "wincbld"
   echo BUILDMSG: Building ProjectionTests cab file via BuildTestCollateralCab.cmd $(OBJ_PATH)\$O $(JSCRIPT_ROOT) $(TARGETNAME).cab
   BuildTestCollateralCab.cmd $(OBJ_PATH)\$O $(JSCRIPT_ROOT) $(TARGETNAME).cab > $(OBJ_PATH)\$O\BuildTestCollateralCab.log 2>&1
!else
   echo BUILDMSG: Building unit tests cab file via set
   set > $(OBJ_PATH)\$O\$(TARGETNAME).cab
!endif
!endif
