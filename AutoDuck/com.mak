# MAKEFILE
# Builds documentation for Pythonwin using the AUTODUCK tool
#

!include "common_top.mak"

TARGET  = win32com
GENDIR  = ..\build\Temp\Help
TITLE   = $(TARGET) Help
DOCHDR  = $(TARGET) Reference

#AD      = autoduck.exe /SPythonWin.fmt
#ADLOG   = "/L$(GENDIR)\$(TARGET).LOG" /N
#ADHLP   = /RH "/C$(GENDIR)\$(TARGET).LOG" "/O$(GENDIR)\$(TARGET).RTF" /D "title=$(TITLE)"
#ADDOC   = /RD "/O$(GENDIR)\$(TARGET).DOC" /D "doc_header=$(DOCHDR)"
#ADTAB   = 8
#HC      = hcw /a /c 


WIN32COM_DIR = ../com/win32com
HELP_DIR = ../com/help
WIN32COMEXT_DIR = ../com/win32comext
MAPI_DIR = $(WIN32COMEXT_DIR)/mapi

SOURCE  = $(WIN32COM_DIR)\src\*.cpp \
	$(WIN32COM_DIR)\src\extensions\*.cpp \
	$(WIN32COMEXT_DIR)\axscript\src\*.cpp \
	$(WIN32COMEXT_DIR)\axdebug\src\*.cpp \
	$(WIN32COMEXT_DIR)\axcontrol\src\*.cpp \
	$(WIN32COMEXT_DIR)\shell\src\*.cpp \
	$(WIN32COMEXT_DIR)\internet\src\*.cpp \
	$(WIN32COM_DIR)\src\include\*.h \
	$(MAPI_DIR)\src\*.cpp \
	$(GENDIR)\mapi.d \
	$(GENDIR)\PyIABContainer.d \
	$(GENDIR)\PyIAddrBook.d \
	$(GENDIR)\PyIAttach.d \
	$(GENDIR)\PyIDistList.d \
	$(GENDIR)\PyIMailUser.d \
	$(GENDIR)\PyIMAPIContainer.d \
	$(GENDIR)\PyIMAPIFolder.d \
	$(GENDIR)\PyIMAPIProp.d \
	$(GENDIR)\PyIMAPISession.d \
	$(GENDIR)\PyIMAPITable.d \
	$(GENDIR)\PyIMessage.d \
	$(GENDIR)\PyIMsgServiceAdmin.d \
	$(GENDIR)\PyIMsgStore.d \
	$(GENDIR)\PyIProfAdmin.d \
	$(GENDIR)\PyIProfSect.d \
	$(GENDIR)\exchange.d \
	$(GENDIR)\exchdapi.d \
	$(HELP_DIR)\adsi.d \


# Help and Doc targets

help : ..\$(TARGET).hlp $(GENDIR)

doc : $(TARGET).doc

clean: cleanad

$(GENDIR)\mapi.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIABContainer.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIContainer $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIAddrBook.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIAttach.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIDistList.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMailUser.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIContainer $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIContainer.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIFolder.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPIProp.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPISession.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMAPITable.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMessage.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMsgServiceAdmin.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIMsgStore.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIProfAdmin.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIUnknown $(MAPI_DIR)/src/$(*B).i

$(GENDIR)\PyIProfSect.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d -p PyIMAPIProp $(MAPI_DIR)/src/$(*B).i

# Exchange stuff.
$(GENDIR)\exchange.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

# Exchange stuff.
$(GENDIR)\exchdapi.d: $(MAPI_DIR)/src/$(*B).i
	makedfromi.py -o$*.d $(MAPI_DIR)/src/$(*B).i

!include "common.mak"

