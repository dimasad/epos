PROJ_DEFINE = /D DEBUG

all: eposcmd.exe epos.dll

eposcmd.exe: eposcmd.obj epos.obj
	link /OUT:$@ $**


eposcmd.obj: eposcmd.c
	$(CC) $(PROJ_DEFINE) /c $**

epos.obj: epos.c
	$(CC) $(PROJ_DEFINE) /c $**

epos.dll: epos.c
	$(CC) /D_USRDLL /D_WINDLL $** /link /DLL /OUT:$@ /DEF:epos.def
