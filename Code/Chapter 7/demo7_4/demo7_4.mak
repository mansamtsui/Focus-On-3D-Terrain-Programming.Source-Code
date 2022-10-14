# Microsoft Developer Studio Generated NMAKE File, Based on demo7_4.dsp
!IF "$(CFG)" == ""
CFG=demo7_4 - Win32 Debug
!MESSAGE No configuration specified. Defaulting to demo7_4 - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "demo7_4 - Win32 Release" && "$(CFG)" != "demo7_4 - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "demo7_4.mak" CFG="demo7_4 - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "demo7_4 - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "demo7_4 - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "demo7_4 - Win32 Release"

OUTDIR=.\Release
INTDIR=.\Release
# Begin Custom Macros
OutDir=.\Release
# End Custom Macros

ALL : "$(OUTDIR)\demo7_4.exe"


CLEAN :
	-@erase "$(INTDIR)\camera.obj"
	-@erase "$(INTDIR)\gl_app.obj"
	-@erase "$(INTDIR)\image.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\math_ops.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\ROAM.obj"
	-@erase "$(INTDIR)\terrain.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\demo7_4.exe"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /ML /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\demo7_4.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\demo7_4.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:no /pdb:"$(OUTDIR)\demo7_4.pdb" /machine:I386 /out:"$(OUTDIR)\demo7_4.exe" 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\ROAM.obj" \
	"$(INTDIR)\terrain.obj" \
	"$(INTDIR)\camera.obj" \
	"$(INTDIR)\gl_app.obj" \
	"$(INTDIR)\image.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\math_ops.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\demo7_4.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "demo7_4 - Win32 Debug"

OUTDIR=.\Debug
INTDIR=.\Debug
# Begin Custom Macros
OutDir=.\Debug
# End Custom Macros

ALL : "$(OUTDIR)\demo7_4.exe"


CLEAN :
	-@erase "$(INTDIR)\camera.obj"
	-@erase "$(INTDIR)\gl_app.obj"
	-@erase "$(INTDIR)\image.obj"
	-@erase "$(INTDIR)\log.obj"
	-@erase "$(INTDIR)\main.obj"
	-@erase "$(INTDIR)\math_ops.obj"
	-@erase "$(INTDIR)\resource.res"
	-@erase "$(INTDIR)\ROAM.obj"
	-@erase "$(INTDIR)\terrain.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\demo7_4.exe"
	-@erase "$(OUTDIR)\demo7_4.ilk"
	-@erase "$(OUTDIR)\demo7_4.pdb"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

CPP_PROJ=/nologo /MLd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /Fp"$(INTDIR)\demo7_4.pch" /YX /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC_PROJ=/l 0x409 /fo"$(INTDIR)\resource.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\demo7_4.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /incremental:yes /pdb:"$(OUTDIR)\demo7_4.pdb" /debug /machine:I386 /out:"$(OUTDIR)\demo7_4.exe" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\main.obj" \
	"$(INTDIR)\ROAM.obj" \
	"$(INTDIR)\terrain.obj" \
	"$(INTDIR)\camera.obj" \
	"$(INTDIR)\gl_app.obj" \
	"$(INTDIR)\image.obj" \
	"$(INTDIR)\log.obj" \
	"$(INTDIR)\math_ops.obj" \
	"$(INTDIR)\resource.res"

"$(OUTDIR)\demo7_4.exe" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("demo7_4.dep")
!INCLUDE "demo7_4.dep"
!ELSE 
!MESSAGE Warning: cannot find "demo7_4.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "demo7_4 - Win32 Release" || "$(CFG)" == "demo7_4 - Win32 Debug"
SOURCE=.\main.cpp

"$(INTDIR)\main.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\ROAM.cpp

"$(INTDIR)\ROAM.obj" : $(SOURCE) "$(INTDIR)"


SOURCE=.\terrain.cpp

"$(INTDIR)\terrain.obj" : $(SOURCE) "$(INTDIR)"


SOURCE="..\Base Code\camera.cpp"

"$(INTDIR)\camera.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Base Code\gl_app.cpp"

"$(INTDIR)\gl_app.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Base Code\image.cpp"

"$(INTDIR)\image.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Base Code\log.cpp"

"$(INTDIR)\log.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE="..\Base Code\math_ops.cpp"

"$(INTDIR)\math_ops.obj" : $(SOURCE) "$(INTDIR)"
	$(CPP) $(CPP_PROJ) $(SOURCE)


SOURCE=.\resource.rc

"$(INTDIR)\resource.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)



!ENDIF 

