PROJ_DEFINE = /D DEBUG

all: epos2cmd.exe epos.dll

epos2cmd.exe: epos2cmd.obj epos2.obj
	link /OUT:$@ $**


epos2cmd.obj: epos2cmd.c
	$(CC) $(PROJ_DEFINE) /c $**

epos2.obj: epos2.c
	$(CC) $(PROJ_DEFINE) /c $**

epos.dll: epos2.c
	$(CC) /D_USRDLL /D_WINDLL $** /link /DLL /OUT:$@ /DEF:epos.def
