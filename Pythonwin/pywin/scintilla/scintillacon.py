# Generated by h2py from Scintilla.h
def SCINTILLA(obj): return GTK_CHECK_CAST (obj, scintilla_get_type (), ScintillaObject)

def IS_SCINTILLA(obj): return GTK_CHECK_TYPE (obj, scintilla_get_type ())

SCI_START = 2000
SCI_ADDTEXT = SCI_START + 1
SCI_ADDSTYLEDTEXT = SCI_START + 2
SCI_INSERTTEXT = SCI_START + 3
SCI_CLEARALL = SCI_START + 4
SCI_GETLENGTH = SCI_START + 6
SCI_GETCHARAT = SCI_START + 7
SCI_GETCURRENTPOS = SCI_START + 8
SCI_GETANCHOR = SCI_START + 9
SCI_GETSTYLEAT = SCI_START + 10
SCI_REDO = SCI_START + 11
SCI_SETUNDOCOLLECTION = SCI_START + 12
SCI_SELECTALL = SCI_START + 13
SCI_SETSAVEPOINT = SCI_START + 14
SCI_GETSTYLEDTEXT = SCI_START + 15
SCI_CANREDO = SCI_START + 16
SCI_MARKERLINEFROMHANDLE = SCI_START + 17
SCI_MARKERDELETEHANDLE = SCI_START + 18
SC_UNDOCOLLECT_NONE = 0
SC_UNDOCOLLECT_AUTOSTART = 1
SC_UNDOCOLLECT_MANUALSTART = 2
SCI_GETVIEWWS = SCI_START + 20
SCI_SETVIEWWS = SCI_START + 21
SCI_CHANGEPOSITION = SCI_START + 22
SCI_GOTOLINE = SCI_START + 24
SCI_GOTOPOS = SCI_START + 25
SCI_SETANCHOR = SCI_START + 26
SCI_GETCURLINE = SCI_START + 27
SCI_GETENDSTYLED = SCI_START + 28
SCI_GETEOLMODE = SCI_START + 30
SCI_SETEOLMODE = SCI_START + 31
SC_EOL_CRLF = 0
SC_EOL_CR = 1
SC_EOL_LF = 2
SCI_STARTSTYLING = SCI_START + 32
SCI_SETSTYLING = SCI_START + 33
SCI_SETMARGINWIDTH = SCI_START + 34
SCI_SETBUFFEREDDRAW = SCI_START + 35
SCI_SETTABWIDTH = SCI_START + 36
SCI_SETCODEPAGE = SCI_START + 37
SCI_SETLINENUMBERWIDTH = SCI_START + 38
SCI_SETUSEPALETTE = SCI_START + 39
MARKER_MAX = 31
SC_MARK_CIRCLE = 0
SC_MARK_ROUNDRECT = 1
SC_MARK_ARROW = 2
SC_MARK_SMALLRECT = 3
SC_MARK_SHORTARROW = 4
SC_MARK_EMPTY = 5
SCI_MARKERDEFINE = SCI_START + 40
SCI_MARKERSETFORE = SCI_START + 41
SCI_MARKERSETBACK = SCI_START + 42
SCI_MARKERADD = SCI_START + 43
SCI_MARKERDELETE = SCI_START + 44
SCI_MARKERDELETEALL = SCI_START + 45
SCI_MARKERGET = SCI_START + 46
SCI_MARKERNEXT = SCI_START + 47
LEX_STYLE_MAX = 31
STYLE_DEFAULT = 32
STYLE_LINENUMBER = 33
STYLE_BRACELIGHT = 34
STYLE_BRACEBAD = 35
STYLE_CONTROLCHAR = 36
STYLE_MAX = 36
SCI_STYLECLEARALL = SCI_START + 50
SCI_STYLESETFORE = SCI_START + 51
SCI_STYLESETBACK = SCI_START + 52
SCI_STYLESETBOLD = SCI_START + 53
SCI_STYLESETITALIC = SCI_START + 54
SCI_STYLESETSIZE = SCI_START + 55
SCI_STYLESETFONT = SCI_START + 56
SCI_STYLESETEOLFILLED = SCI_START + 57
SCI_STYLERESETDEFAULT = SCI_START + 58
SCI_SETFORE = SCI_START + 60
SCI_SETBACK = SCI_START + 61
SCI_SETBOLD = SCI_START + 62
SCI_SETITALIC = SCI_START + 63
SCI_SETSIZE = SCI_START + 64
SCI_SETFONT = SCI_START + 65
SCI_SETSELFORE = SCI_START + 67
SCI_SETSELBACK = SCI_START + 68
SCI_SETCARETFORE = SCI_START + 69
SCI_ASSIGNCMDKEY = SCI_START + 70
SCI_CLEARCMDKEY = SCI_START + 71
SCI_CLEARALLCMDKEYS = SCI_START + 72
SCI_SETSTYLINGEX = SCI_START + 73
SCI_APPENDUNDOSTARTACTION = SCI_START + 74
SCI_GETCARETPERIOD = SCI_START + 75
SCI_SETCARETPERIOD = SCI_START + 76
SCI_SETWORDCHARS = SCI_START + 77
SCI_BEGINUNDOACTION = SCI_START + 78
SCI_ENDUNDOACTION = SCI_START + 79
INDIC_MAX = 2
INDIC_PLAIN = 0
INDIC_SQUIGGLE = 1
INDIC_TT = 2
INDIC0_MASK = 32
INDIC1_MASK = 64
INDIC2_MASK = 128
INDICS_MASK = (INDIC0_MASK | INDIC1_MASK | INDIC2_MASK)
SCI_INDICSETSTYLE = SCI_START + 80
SCI_INDICGETSTYLE = SCI_START + 81
SCI_INDICSETFORE = SCI_START + 82
SCI_INDICGETFORE = SCI_START + 83
SCI_AUTOCSHOW = SCI_START + 100
SCI_AUTOCCANCEL = SCI_START + 101
SCI_AUTOCACTIVE = SCI_START + 102
SCI_AUTOCPOSSTART = SCI_START + 103
SCI_AUTOCCOMPLETE = SCI_START + 104
SCI_AUTOCSTOPS = SCI_START + 105
SCI_CALLTIPSHOW = SCI_START + 200
SCI_CALLTIPCANCEL = SCI_START + 201
SCI_CALLTIPACTIVE = SCI_START + 202
SCI_CALLTIPPOSSTART = SCI_START + 203
SCI_CALLTIPSETHLT = SCI_START + 204
SCI_LINEDOWN = SCI_START + 300
SCI_LINEDOWNEXTEND = SCI_START + 301
SCI_LINEUP = SCI_START + 302
SCI_LINEUPEXTEND = SCI_START + 303
SCI_CHARLEFT = SCI_START + 304
SCI_CHARLEFTEXTEND = SCI_START + 305
SCI_CHARRIGHT = SCI_START + 306
SCI_CHARRIGHTEXTEND = SCI_START + 307
SCI_WORDLEFT = SCI_START + 308
SCI_WORDLEFTEXTEND = SCI_START + 309
SCI_WORDRIGHT = SCI_START + 310
SCI_WORDRIGHTEXTEND = SCI_START + 311
SCI_HOME = SCI_START + 312
SCI_HOMEEXTEND = SCI_START + 313
SCI_LINEEND = SCI_START + 314
SCI_LINEENDEXTEND = SCI_START + 315
SCI_DOCUMENTSTART = SCI_START + 316
SCI_DOCUMENTSTARTEXTEND = SCI_START + 317
SCI_DOCUMENTEND = SCI_START + 318
SCI_DOCUMENTENDEXTEND = SCI_START + 319
SCI_PAGEUP = SCI_START + 320
SCI_PAGEUPEXTEND = SCI_START + 321
SCI_PAGEDOWN = SCI_START + 322
SCI_PAGEDOWNEXTEND = SCI_START + 323
SCI_EDITTOGGLEOVERTYPE = SCI_START + 324
SCI_CANCEL = SCI_START + 325
SCI_DELETEBACK = SCI_START + 326
SCI_TAB = SCI_START + 327
SCI_BACKTAB = SCI_START + 328
SCI_NEWLINE = SCI_START + 329
SCI_FORMFEED = SCI_START + 330
SCI_VCHOME = SCI_START + 331
SCI_VCHOMEEXTEND = SCI_START + 332
SCI_ZOOMIN = SCI_START + 333
SCI_ZOOMOUT = SCI_START + 334
SCI_DELWORDLEFT = SCI_START + 335
SCI_DELWORDRIGHT = SCI_START + 336
SCI_LINELENGTH = SCI_START + 350
SCI_BRACEHIGHLIGHT = SCI_START + 351
SCI_BRACEBADLIGHT = SCI_START + 352
SCI_BRACEMATCH = SCI_START + 353
SCI_GETVIEWEOL = SCI_START + 355
SCI_SETVIEWEOL = SCI_START + 356
SCI_GRABFOCUS = SCI_START + 400
SCN_STYLENEEDED = 2000
SCN_CHARADDED = 2001
SCN_SAVEPOINTREACHED = 2002
SCN_SAVEPOINTLEFT = 2003
SCN_MODIFYATTEMPTRO = 2004
SCN_KEY = 2005
SCN_DOUBLECLICK = 2006
SCN_CHECKBRACE = 2007
