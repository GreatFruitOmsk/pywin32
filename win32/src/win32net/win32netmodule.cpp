/***********************************************************

win32net.cpp -- module for interface into Network API

NOTE: The Network API for NT uses UNICODE.  Therefore, you
can not simply pass python strings to the API functioms - some
conversion is required.

	Note: The NET functions have their own set of error codes in  2100-2200
	range.  The system error functions do not always apply.
	i.e. GetLastError may be useless.

  REV HISTORY:

  Original version: Mark Hammond's Build 109 distribution.

  October, 98:	rewrote PyNetUserChangePassword - changed error handling - needs testing
				rewrote PyNetUserGetGroups - fixed enumeration - tested
				rewrote PyNetUserGetLocalGroups - fixed enumeration - tested
				added PyNetShareEnum1 - exported as NetShareEnum - check assumptions - tested
				PyNetMessageBufferSend - didn't touch

  November, 98	Cleaned up return Lists (removed Integer count as redundant)
				
******************************************************************/
// @doc

#ifndef UNICODE
#error This project requires a Unicode build.
#endif

#include "windows.h"
#include "lm.h"
#include "lmuseflg.h"
#include "Python.h"
#include "PyWinTypes.h"
#include "win32net.h"

#include "assert.h"
/*****************************************************************************/
/* error helpers */

/* error helper - GetLastError() is provided, but this is for exceptions */
PyObject *ReturnNetError(char *fnName, long err /*=0*/)
{
	if (err==0) err = GetLastError();
	if(err >= NERR_BASE && err <= MAX_NERR) {
		HMODULE hModule;
        hModule = LoadLibraryEx(
            TEXT("netmsg.dll"),
            NULL,
            LOAD_LIBRARY_AS_DATAFILE
            );
        if(hModule != NULL) {
			DWORD dwFormatFlags = FORMAT_MESSAGE_ALLOCATE_BUFFER |
			                      FORMAT_MESSAGE_IGNORE_INSERTS |
								  FORMAT_MESSAGE_FROM_HMODULE |
								  FORMAT_MESSAGE_FROM_SYSTEM |
								  FORMAT_MESSAGE_ALLOCATE_BUFFER;

			// call FormatMessage() to allow for message text to be acquired
			// from the system or the supplied module handle    //
			WCHAR *szMessage;
			DWORD dwResult;
			if(dwResult=FormatMessage(
				dwFormatFlags,
				hModule, // module to get message from (NULL == system)
				err,
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // default language
				(LPTSTR) &szMessage,        
				0,        
				NULL)) {
				// remove the \r\n, like the real API
				if (dwResult>2)
					szMessage[dwResult-2]=_T('\0');
				PyObject *str = PyString_FromUnicode(szMessage);
				LocalFree(szMessage);
				PyObject *v = Py_BuildValue("(izO)", err, fnName, str );
				Py_DECREF(str);
				if (v != NULL) {
					PyErr_SetObject(PyWinExc_ApiError, v);
					Py_DECREF(v);
					return NULL;
				}

			}
		}
	}
	return PyWin_SetAPIError(fnName, err);
};

BOOL FindNET_STRUCT(DWORD level, PyNET_STRUCT *pBase, PyNET_STRUCT **ppRet)
{
	for (;pBase->entries;pBase++) {
		if (level==pBase->level) {
			*ppRet = pBase;
			return TRUE;
		}
	}
	PyErr_SetString(PyExc_ValueError, "This information level is not supported");
	return FALSE;
}

void PyObject_FreeNET_STRUCT(PyNET_STRUCT *pI, BYTE *pBuf)
{
	if (pBuf==NULL) return;
	// Free all the strings.
	PyNET_STRUCT_ITEM *pItem;
	for( pItem=pI->entries;pItem->attrname != NULL;pItem++) {
		switch (pItem->type) {
			case NSI_WSTR:
				if (*((WCHAR **)(pBuf+pItem->off)))
					PyWinObject_FreeWCHAR(*((WCHAR **)(pBuf+pItem->off)));
				break;
			case NSI_HOURS:
				if (*((char **)(pBuf+pItem->off)))
					free(*((char **)(pBuf+pItem->off)));
				break;
			case NSI_SID:
				if (*((SID **)(pBuf+pItem->off)))
					free(*((SID **)(pBuf+pItem->off)));
			case NSI_SECURITY_DESCRIPTOR:
				if (*((SECURITY_DESCRIPTOR **)(pBuf+pItem->off)))
					free(*((SECURITY_DESCRIPTOR **)(pBuf+pItem->off)));
			default:
				break;
		}
	}
	free(pBuf);
}
BOOL PyObject_AsNET_STRUCT( PyObject *ob, PyNET_STRUCT *pI, BYTE **ppRet )
{
	BOOL ok = FALSE;
	if (!PyMapping_Check(ob)) {
		PyErr_SetString(PyExc_TypeError, "The object must be a mapping");
		return FALSE;
	}
	char *szAttrName = NULL;
	// allocate the structure, and wipe it to zero.
	BYTE *buf = (BYTE *)malloc(pI->structsize);
	memset(buf, 0, pI->structsize);
	PyNET_STRUCT_ITEM *pItem;
	for( pItem=pI->entries;pItem->attrname != NULL;pItem++) {
		if (szAttrName) {
			PyWinObject_FreeString(szAttrName);
			szAttrName = NULL;
		}
		if (!PyWin_WCHAR_AsString(pItem->attrname, -1, &szAttrName))
			goto done;
		PyObject *subob = PyMapping_GetItemString(ob, szAttrName);

		if (subob==NULL) {
			PyErr_Clear();
			// See if it is OK.
			if (pItem->reqd) {
				PyErr_Format(PyExc_ValueError, "The mapping does not have the required attribute '%s'", szAttrName);
				goto done;
			}
		} else {
			switch (pItem->type) {
				case NSI_WSTR:
					WCHAR *wsz;
					if (!PyWinObject_AsWCHAR(subob, &wsz, !pItem->reqd)) {
						Py_DECREF(subob);
						goto done;
					}
					*((WCHAR **)(buf+pItem->off)) = wsz;
					break;
				case NSI_DWORD:
					if (!PyInt_Check(subob)) {
						PyErr_Format(PyExc_TypeError, "The mapping attribute '%s' must be an integer", szAttrName);
						Py_DECREF(subob);
						goto done;
					}
					*((DWORD *)(buf+pItem->off)) = (DWORD)PyInt_AsLong(subob);
					break;
				case NSI_LONG:
					if (!PyInt_Check(subob)) {
						PyErr_Format(PyExc_TypeError, "The mapping attribute '%s' must be an integer", szAttrName);
						Py_DECREF(subob);
						goto done;
					}
					*((LONG *)(buf+pItem->off)) = (LONG)PyInt_AsLong(subob);
					break;
				case NSI_BOOL:
					if (!PyInt_Check(subob)) {
						PyErr_Format(PyExc_TypeError, "The mapping attribute '%s' must be an integer", szAttrName);
						Py_DECREF(subob);
						goto done;
					}
					*((BOOL *)(buf+pItem->off)) = (BOOL)PyInt_AsLong(subob);
					break;
				case NSI_HOURS:
					if (subob != Py_None) {
						if (!PyString_Check(subob) || PyString_Size(subob)!=21) {
							PyErr_Format(PyExc_TypeError, "The mapping attribute '%s' must be a string of exactly length 21", szAttrName);
							Py_DECREF(subob);
							goto done;
						}
						*((char **)(buf+pItem->off)) = (char *)malloc(21);
						memcpy(*((char **)(buf+pItem->off)), PyString_AsString(subob), 21);
					}
					break;
				case NSI_SID: {
					PSID pSIDsrc;
					if (!PyWinObject_AsSID(subob, &pSIDsrc ,TRUE)) {
						Py_DECREF(subob);
						goto done;
					}
					PSID *ppSIDdest = ((PSID *)(buf+pItem->off));
					size_t len = GetLengthSid(pSIDsrc);
					*ppSIDdest = (SID *)malloc(len);
					memcpy(*ppSIDdest, pSIDsrc, len);
					}
					break;
				case NSI_SECURITY_DESCRIPTOR: {
					PSECURITY_DESCRIPTOR pSDsrc;
					if (!PyWinObject_AsSECURITY_DESCRIPTOR(subob, &pSDsrc ,TRUE)) {
						Py_DECREF(subob);
						goto done;
					}
					PSECURITY_DESCRIPTOR *ppSDdest = ((PSECURITY_DESCRIPTOR *)(buf+pItem->off));
					size_t len = GetSecurityDescriptorLength(pSDsrc);
					*ppSDdest = (PSECURITY_DESCRIPTOR)malloc(len);
					memcpy(*ppSDdest, pSDsrc, len);
					}
					break;
				
				default:
					PyErr_SetString(PyExc_RuntimeError, "invalid internal data type");
					Py_DECREF(subob);
					goto done;
			}
			Py_DECREF(subob);
		}
	}
	ok = TRUE;
done:
	if (szAttrName) PyWinObject_FreeString(szAttrName);
	if (!ok ) {
		PyObject_FreeNET_STRUCT(pI, buf);
		return FALSE;
	}
	*ppRet = buf;
	return TRUE;
}

PyObject *PyObject_FromNET_STRUCT(PyNET_STRUCT *pI, BYTE *buf)
{
	PyObject *ret = PyDict_New();
	PyNET_STRUCT_ITEM *pItem;
	for( pItem=pI->entries;pItem->attrname != NULL;pItem++) {
		PyObject *newObj = NULL;
		switch (pItem->type) {
			case NSI_WSTR:
				newObj = PyWinObject_FromWCHAR(*((WCHAR **)(buf+pItem->off)));
				break;
			case NSI_DWORD:
				newObj = PyInt_FromLong(*((DWORD *)(buf+pItem->off)));
				break;
			case NSI_LONG:
				newObj = PyInt_FromLong(*((LONG *)(buf+pItem->off)));
				break;
			case NSI_BOOL:
				newObj = *((BOOL *)(buf+pItem->off)) ? Py_True : Py_False;
				Py_INCREF(newObj);
				break;
			case NSI_HOURS: {
				char *data = *((char **)(buf+pItem->off));
				if (data) {
					newObj = PyString_FromStringAndSize(data,21);
				} else {
					newObj = Py_None;
					Py_INCREF(Py_None);
				}
				break;
				}
			case NSI_SID:
				newObj = PyWinObject_FromSID(*((SID **)(buf+pItem->off)));
				break;
			case NSI_SECURITY_DESCRIPTOR:
				newObj = PyWinObject_FromSECURITY_DESCRIPTOR(*((PSECURITY_DESCRIPTOR *)(buf+pItem->off)));
				break;
			default:
				PyErr_SetString(PyExc_RuntimeError, "invalid internal data");
				break;
		}
		if (newObj==NULL) {
			Py_DECREF(ret);
			return NULL;
		}
		char *szAttrName;
		if (!PyWin_WCHAR_AsString(pItem->attrname, -1, &szAttrName)) {
			Py_DECREF(ret);
			return NULL;
		}
		PyMapping_SetItemString(ret, szAttrName, newObj);
		Py_DECREF(newObj);
		PyWinObject_FreeString(szAttrName);
	}
	return ret;
}

PyObject *PyDoSimpleEnum(PyObject *self, PyObject *args, PFNSIMPLEENUM pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL;
	PyObject *obServer;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = MAX_PREFERRED_LENGTH;
	DWORD level;
	BOOL ok = FALSE;
	DWORD resumeHandle = 0;
	DWORD numRead, i;
	PyObject *list;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	if (!PyArg_ParseTuple(args, "Oi|ii", &obServer, &level, &resumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

	if (!FindNET_STRUCT(level, pInfos, &pInfo))
		goto done;

	Py_BEGIN_ALLOW_THREADS
	/* Bad resume handles etc can cause access violations here - catch them. */
	__try {
		err = (*pfn)(szServer, level, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
	}
#if defined(__MINGW32__) || defined(MAINWIN)
		catch(...)
#else
		__except( EXCEPTION_EXECUTE_HANDLER )
#endif
	{
		err = ERROR_INVALID_PARAMETER;
	}
	Py_END_ALLOW_THREADS
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError(fnname,err);
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SetItem(list, i, sub);
	}
	resumeHandle = err==0 ? 0 : resumeHandle;
	ret = Py_BuildValue("Oll", list, totalEntries, resumeHandle);
	Py_DECREF(list);
	ok = TRUE;
done:
	if (buf) NetApiBufferFree(buf);
	if (!ok) {
		Py_XDECREF(ret);
		ret = NULL;
	}
	PyWinObject_FreeWCHAR(szServer);
	return ret;
}

PyObject *PyDoNamedEnum(PyObject *self, PyObject *args, PFNNAMEDENUM pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL, *szGroup = NULL;
	PyObject *obServer, *obGroup;
	PyObject *ret = NULL;
	PyNET_STRUCT *pInfo;
	DWORD err;
	DWORD dwPrefLen = 4096;
	DWORD level;
	BOOL ok = FALSE;
	DWORD resumeHandle = 0;
	DWORD numRead, i;
	PyObject *list;
	BYTE *buf = NULL;
	DWORD totalEntries = 0;
	if (!PyArg_ParseTuple(args, "OOi|ii", &obServer, &obGroup, &level, &resumeHandle, &dwPrefLen))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

	if (!PyWinObject_AsWCHAR(obGroup, &szGroup, FALSE))
		goto done;

	if (!FindNET_STRUCT(level, pInfos, &pInfo))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szGroup, level, &buf, dwPrefLen, &numRead, &totalEntries, &resumeHandle);
    Py_END_ALLOW_THREADS
	if (err!=0 && err != ERROR_MORE_DATA) {
		ReturnNetError(fnname,err);
		goto done;
	}
	list = PyList_New(numRead);
	if (list==NULL) goto done;
	for (i=0;i<numRead;i++) {
		PyObject *sub = PyObject_FromNET_STRUCT(pInfo, buf+(i*pInfo->structsize));
		if (sub==NULL) goto done;
		PyList_SetItem(list, i, sub);
	}
	resumeHandle = err==0 ? 0 : resumeHandle;
	ret = Py_BuildValue("Oll", list, totalEntries, resumeHandle);
	Py_DECREF(list);
	ok = TRUE;
done:
	if (buf) NetApiBufferFree(buf);
	if (!ok) {
		Py_XDECREF(ret);
		ret = NULL;
	}
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szGroup);
	return ret;
}

PyObject *
PyDoGroupSet(PyObject *self, PyObject *args, PFNGROUPSET pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL;
	WCHAR *szGroup = NULL;
	PyObject *obServer, *obGroup, *obData;
	PyObject *ret = NULL;
	DWORD level;
	DWORD err = 0;
	BYTE *buf = NULL;
	BYTE **ppTempObjects = NULL;
	int i, numEntries;
	PyNET_STRUCT *pI;
	if (!PyArg_ParseTuple(args, "OOiO", &obServer, &obGroup, &level, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obGroup, &szGroup, FALSE))
		goto done;
	if (!FindNET_STRUCT(level, pInfos, &pI))
		goto done;

	if (!PySequence_Check(obData)) {
		PyErr_SetString(PyExc_TypeError, "Data must be a sequence of dictionaries");
		goto done;
	}
	numEntries = PySequence_Length(obData);
	ppTempObjects = new BYTE *[numEntries];
	memset(ppTempObjects, 0, sizeof(BYTE *) * numEntries);
	for (i=0;i<numEntries;i++) {
		PyObject *sub = PySequence_GetItem(obData, i);
		if (sub==NULL) goto done;
		if (!PyObject_AsNET_STRUCT(sub, pI, ppTempObjects+i)) {
			Py_DECREF(sub);
			goto done;
		}
	}
	// OK - all objects are ok, and we are holding the buffers.
	// copy to our own buffer
	buf = new BYTE[numEntries*pI->structsize];
	if (buf==NULL)	{
		PyErr_SetString(PyExc_MemoryError, "Allocating buffer for members");
		goto done;
	}
	for (i=0;i<numEntries;i++)
		memcpy(buf+(i*pI->structsize), ppTempObjects[i], pI->structsize);
    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szGroup, level, buf, numEntries);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);
		goto done;
	}
	ret = Py_None;
	Py_INCREF(Py_None);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szGroup);
	if (ppTempObjects) {
		for (i=0;i<numEntries;i++) {
			PyObject_FreeNET_STRUCT(pI, ppTempObjects[i]);
		}
		delete [] ppTempObjects;
	}
	delete [] buf;
	return ret;
}

PyObject *PyDoGetInfo(PyObject *self, PyObject *args, PFNGETINFO pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	PyObject *obName, *obServer;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err;
	if (!PyArg_ParseTuple(args, "OOi", &obServer, &obName, &typ))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
		goto done;
	if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;
    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szName, typ, &buf);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);
		goto done;
	}
	ret= PyObject_FromNET_STRUCT(pInfo, buf);
done:
	if (buf) NetApiBufferFree(buf);
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);
	return ret;
}

PyObject *PyDoGetModalsInfo(PyObject *self, PyObject *args, PFNGETMODALSINFO pfn, char *fnname, PyNET_STRUCT *pInfos) 
{
	WCHAR *szServer = NULL;
	PyObject *obServer;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err;
	if (!PyArg_ParseTuple(args, "Oi", &obServer, &typ))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;
    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, typ, &buf);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);
		goto done;
	}
	ret= PyObject_FromNET_STRUCT(pInfo, buf);
done:
	if (buf) NetApiBufferFree(buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
}


/*****************************************************************************/

// @pymethod |win32net|NetMessageBufferSend|sends a string to a registered message alias.
/******************************************************************************/

PyObject *
PyNetMessageBufferSend( PyObject *self, PyObject *args)
{
	DWORD rc;
	TCHAR *serverName = NULL;
	TCHAR *msgName = NULL;
	TCHAR *fromName = NULL;
	TCHAR *message = NULL;
	PyObject *obServerName, *obMsgName, *obFromName, *obMessage;
	PyObject *ret = NULL;
	DWORD msgLen;
	if (!PyArg_ParseTuple(args, "OOOO:NetMessageBufferSend", 
	          &obServerName,  // @pyparm string|domain||Specifies the name of the remote server on which the function is to execute. None or empty string the local computer.
	          &obMsgName, // @pyparm string|userName||Specifies the message name to which the message buffer should be sent.
	          &obFromName, // @pyparm string|fromName||The user the message is to come from, or None for the current user.
	          &obMessage)) // @pyparm string|message||The message text
		return NULL;

	if (!PyWinObject_AsTCHAR(obServerName, &serverName, TRUE))
		goto done;

	if (!PyWinObject_AsTCHAR(obMsgName, &msgName, FALSE))
		goto done;

	if (!PyWinObject_AsTCHAR(obFromName, &fromName, TRUE))
		goto done;

	if (!PyWinObject_AsTCHAR(obMessage, &message, FALSE, &msgLen))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	// message is "BYTE *", but still expects Unicode?  Wonder why not LPTSTR like the other string args?
	rc=NetMessageBufferSend( serverName, msgName, fromName, (BYTE *)message, msgLen * sizeof(TCHAR));
    Py_END_ALLOW_THREADS
	if (rc) {
		ReturnNetError("NetMessageBufferSend",rc);	// @pyseeapi NetMessageBufferSend
		goto done;
	}

	Py_INCREF(Py_None);
	ret = Py_None;
done:
	PyWinObject_FreeTCHAR(serverName);
	PyWinObject_FreeTCHAR(msgName);
	PyWinObject_FreeTCHAR(fromName);
	PyWinObject_FreeTCHAR(message);
	return ret;
}


PyObject *PyDoSetInfo(PyObject *self, PyObject *args, PFNSETINFO pfn, char *fnname, PyNET_STRUCT *pInfos)
{
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	PyObject *obName, *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OOiO", &obServer, &obName, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
		goto done;

	if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szName, typ, buf, NULL);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);
	return ret;
}

PyObject *PyDoSetModalsInfo(PyObject *self, PyObject *args, PFNSETMODALSINFO pfn, char *fnname, PyNET_STRUCT *pInfos)
{
	WCHAR *szServer = NULL;	
	PyObject *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OiO", &obServer, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;

  if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, typ, buf, NULL);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);	
	return ret;
}


PyObject *PyDoAdd(PyObject *self, PyObject *args, PFNADD pfn, char *fnname, PyNET_STRUCT *pInfos)
{
	WCHAR *szServer = NULL;
	PyObject *obServer, *obData;
	PyNET_STRUCT *pInfo;
	BYTE *buf = NULL;
	PyObject *ret = NULL;
	int typ;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OiO", &obServer, &typ, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!FindNET_STRUCT(typ, pInfos, &pInfo))
		goto done;

	if (!PyObject_AsNET_STRUCT(obData, pInfo, &buf))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, typ, buf, NULL);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);	
		goto done;
	}
	ret= Py_None;
	Py_INCREF(ret);
done:
	if (buf) PyObject_FreeNET_STRUCT(pInfo, buf);
	PyWinObject_FreeWCHAR(szServer);
	return ret;
}

PyObject *PyDoDel(PyObject *self, PyObject *args, PFNDEL pfn, char *fnname)
{
	WCHAR *szServer = NULL;
	WCHAR *szName = NULL;
	PyObject *obName, *obServer;
	PyObject *ret = NULL;
	DWORD err = 0;
	if (!PyArg_ParseTuple(args, "OO", &obServer, &obName))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obName, &szName, FALSE))
		goto done;

    Py_BEGIN_ALLOW_THREADS
	err = (*pfn)(szServer, szName);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError(fnname,err);	
		goto done;
	}
	ret = Py_None;
	Py_INCREF(Py_None);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szName);
	return ret;
}

PyObject *
PyDoGroupDelMembers(PyObject *self, PyObject *args)
{
	WCHAR *szServer = NULL;
	WCHAR *szGroup = NULL;
	PyObject *obServer, *obGroup, *obData;
	PyObject *ret = NULL;
	DWORD err = 0;
	BYTE *buf = NULL;
	int i, numEntries;
    DWORD level = 3;
    LOCALGROUP_MEMBERS_INFO_3 *plgrminfo;

	if (!PyArg_ParseTuple(args, "OOO", &obServer, &obGroup, &obData))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obGroup, &szGroup, FALSE))
		goto done;

	if (!PySequence_Check(obData)) {
		PyErr_SetString(PyExc_TypeError, "Data must be a sequence of dictionaries");
		goto done;
	}

	numEntries = PySequence_Length(obData);
    plgrminfo = new LOCALGROUP_MEMBERS_INFO_3[numEntries];
	// XXX - todo - we should allow a list of LOCALGROUP_MEMBER_INFO items *or* strings
	memset(plgrminfo, 0, sizeof(LOCALGROUP_MEMBERS_INFO_3) * numEntries);
	for (i = 0; i < numEntries; i++) {
		PyObject *sub = PySequence_GetItem(obData, i);
		if (sub==NULL)
            goto done;
        PyWinObject_AsWCHAR(sub, &plgrminfo[i].lgrmi3_domainandname);
        if (!plgrminfo[i].lgrmi3_domainandname) {
			Py_DECREF(sub);
			goto done;
		}
	}

    Py_BEGIN_ALLOW_THREADS
    err = NetLocalGroupDelMembers(szServer, szGroup, 3, (BYTE *)plgrminfo, numEntries);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetLocalGroupDelMembers", err);
		goto done;
	}

	ret = Py_None;
	Py_INCREF(Py_None);

done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szGroup);
	if (plgrminfo) {
		for (i=0;i<numEntries;i++) {
            if (plgrminfo[i].lgrmi3_domainandname) {
			    PyWinObject_FreeWCHAR(plgrminfo[i].lgrmi3_domainandname);
            }
		}
	}
	return ret;
}

/* Other misc functions */
// @method <o PyUnicode>|win32net|NetGetDCName|Returns the name of the primary domain controller (PDC).
PyObject *PyNetGetDCName(PyObject *self, PyObject *args)
{
	PyObject *obServer = Py_None, *obDomain = Py_None;
	WCHAR *szServer = NULL, *szDomain = NULL, *result = NULL;
	PyObject *ret = NULL;
	NET_API_STATUS err;

	// @pyparm <o PyUnicode>|server|None|Specifies the name of the remote server on which the function is to execute. If this parameter is None, the local computer is used.
	// @pyparm <o PyUnicode>|domain|None|Specifies the name of the domain. If this parameter is None, the name of the domain controller for the primary domain is used.
	if (!PyArg_ParseTuple(args, "|OO", &obServer, &obDomain))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;
    Py_BEGIN_ALLOW_THREADS
    err = NetGetDCName(szServer, szDomain, (LPBYTE *)&result);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetGetDCName", err);
		goto done;
	}
	ret = PyWinObject_FromWCHAR(result);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szDomain);
	NetApiBufferFree(result);
	return ret;
}

// @method <o PyUnicode>|win32net|NetGetAnyDCName|Returns the name of any domain controller trusted by the specified server.
PyObject *PyNetGetAnyDCName(PyObject *self, PyObject *args)
{
	PyObject *obServer = Py_None, *obDomain = Py_None;
	WCHAR *szServer = NULL, *szDomain = NULL, *result = NULL;
	PyObject *ret = NULL;
	NET_API_STATUS err;

	// @pyparm <o PyUnicode>|server|None|Specifies the name of the remote server on which the function is to execute. If this parameter is None, the local computer is used.
	// @pyparm <o PyUnicode>|domain|None|Specifies the name of the domain. If this parameter is None, the name of the domain controller for the primary domain is used.
	if (!PyArg_ParseTuple(args, "|OO", &obServer, &obDomain))
		return NULL;
	if (!PyWinObject_AsWCHAR(obServer, &szServer, TRUE))
		goto done;
	if (!PyWinObject_AsWCHAR(obDomain, &szDomain, TRUE))
		goto done;
    Py_BEGIN_ALLOW_THREADS
    err = NetGetAnyDCName(szServer, szDomain, (LPBYTE *)&result);
    Py_END_ALLOW_THREADS
	if (err) {
		ReturnNetError("NetGetAnyDCName", err);
		goto done;
	}
	ret = PyWinObject_FromWCHAR(result);
done:
	PyWinObject_FreeWCHAR(szServer);
	PyWinObject_FreeWCHAR(szDomain);
	NetApiBufferFree(result);
	return ret;
}

/*************************************************************************************************************
**



*************************************************************************************************************/
extern PyObject *PyNetUserAdd(PyObject *self, PyObject *args);
extern PyObject *PyNetUserSetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetUserGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetUserDel(PyObject *self, PyObject *args);
extern PyObject *PyNetUserEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetUserChangePassword(PyObject *self, PyObject *args);
extern PyObject *PyNetUserGetLocalGroups( PyObject *self, PyObject *args);
extern PyObject *PyNetUserGetGroups( PyObject *self, PyObject *args);

extern PyObject *PyNetUserModalsGet(PyObject *self, PyObject *args);
extern PyObject *PyNetUserModalsSet(PyObject *self, PyObject *args);

extern PyObject *PyNetGroupGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupSetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupAdd(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupAddUser(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupDel(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupDelUser(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetGroupGetUsers(PyObject *self, PyObject *args);
extern PyObject * PyNetGroupSetUsers(PyObject *self, PyObject *args) ;

extern PyObject *PyNetLocalGroupGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupSetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupAdd(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupAddMembers(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupDelMembers(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupDel(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetLocalGroupGetMembers(PyObject *self, PyObject *args) ;
extern PyObject *PyNetLocalGroupSetMembers(PyObject *self, PyObject *args) ;

extern PyObject *PyNetServerEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetServerGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetServerSetInfo(PyObject *self, PyObject *args);

extern PyObject *PyNetShareAdd(PyObject *self, PyObject *args);
extern PyObject *PyNetShareDel(PyObject *self, PyObject *args);
extern PyObject *PyNetShareEnum(PyObject *self, PyObject *args);
extern PyObject *PyNetShareGetInfo(PyObject *self, PyObject *args);
extern PyObject *PyNetShareSetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetShareCheck(PyObject *self, PyObject *args);

extern PyObject *PyNetWkstaUserEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaGetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaSetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaTransportEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaTransportAdd(PyObject *self, PyObject *args);
extern PyObject * PyNetWkstaTransportDel(PyObject *self, PyObject *args);
extern PyObject * PyNetServerDiskEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetStatisticsGet(PyObject *self, PyObject *args);

// NetUse Functions
extern PyObject * PyNetUseAdd(PyObject *self, PyObject *args);
extern PyObject * PyNetUseDel(PyObject *self, PyObject *args);
extern PyObject * PyNetUseEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetUseGetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetSessionEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetSessionDel(PyObject *self, PyObject *args);
extern PyObject * PyNetSessionGetInfo(PyObject *self, PyObject *args);
extern PyObject * PyNetFileEnum(PyObject *self, PyObject *args);
extern PyObject * PyNetFileClose(PyObject *self, PyObject *args);
extern PyObject * PyNetFileGetInfo(PyObject *self, PyObject *args);

/* List of functions exported by this module */
// @module win32net|A module encapsulating the Windows Network API.
static struct PyMethodDef win32net_functions[] = {
	{"NetGroupGetInfo",         PyNetGroupGetInfo,          1}, // @pymeth NetGroupGetInfo|Retrieves information about a particular group on a server.
	{"NetGroupGetUsers",        PyNetGroupGetUsers,         1}, // @pymeth NetGroupGetUsers|Enumerates the users in a group.
	{"NetGroupSetUsers",        PyNetGroupSetUsers,         1}, // @pymeth NetGroupSetUsers|Sets the users in a group on server.
	{"NetGroupSetInfo",         PyNetGroupSetInfo,          1}, // @pymeth NetGroupSetInfo|Sets information about a particular group account on a server.
	{"NetGroupAdd",             PyNetGroupAdd,              1}, // @pymeth NetGroupAdd|Creates a new group.
	{"NetGroupAddUser",         PyNetGroupAddUser,          1}, // @pymeth NetGroupAddUser|Adds a user to a group
	{"NetGroupDel",             PyNetGroupDel,              1}, // @pymeth NetGroupDel|Deletes a group.
	{"NetGroupDelUser",         PyNetGroupDelUser,          1}, // @pymeth NetGroupDelUser|Deletes a user from the group
	{"NetGroupEnum",            PyNetGroupEnum,             1}, // @pymeth NetGroupEnum|Enumerates the groups.

	{"NetLocalGroupAdd",        PyNetLocalGroupAdd,              1}, // @pymeth NetGroupAdd|Creates a new group.
	{"NetLocalGroupAddMembers", PyNetLocalGroupAddMembers,  1}, // @pymeth NetLocalGroupAddMembers|Adds users to a local group.
    {"NetLocalGroupDelMembers", PyNetLocalGroupDelMembers,  1}, // @pymeth NetLocalGroupDelMembers|Deletes users from a local group.
	{"NetLocalGroupDel",        PyNetLocalGroupDel,              1}, // @pymeth NetGroupDel|Deletes a group.
	{"NetLocalGroupEnum",       PyNetLocalGroupEnum,             1}, // @pymeth NetGroupEnum|Enumerates the groups.
	{"NetLocalGroupGetInfo",    PyNetLocalGroupGetInfo,          1}, // @pymeth NetGroupGetInfo|Retrieves information about a particular group on a server.
	{"NetLocalGroupGetMembers", PyNetLocalGroupGetMembers,  1}, // @pymeth NetLocalGroupGetMembers|Enumerates the members in a local group.
	{"NetLocalGroupSetInfo",    PyNetLocalGroupSetInfo,          1}, // @pymeth NetGroupSetInfo|Sets information about a particular group account on a server.
	{"NetLocalGroupSetMembers", PyNetLocalGroupSetMembers,  1}, // @pymeth NetLocalGroupSetMembers|Sets the members of a local group.  Any existing members not listed are removed.

	{"NetMessageBufferSend",	PyNetMessageBufferSend,		1}, // @pymeth NetMessageBufferSend|sends a string to a registered message alias.

	{"NetServerEnum",           PyNetServerEnum,            1}, // @pymeth NetServerEnum|Retrieves information about all servers of a specific type
	{"NetServerGetInfo",        PyNetServerGetInfo,         1}, // @pymeth NetServerGetInfo|Retrieves information about a particular server.
	{"NetServerSetInfo",        PyNetServerSetInfo,         1}, // @pymeth NetServerSetInfo|Sets information about a particular server.

	{"NetShareAdd",             PyNetShareAdd,              1}, // @pymeth NetShareAdd|Creates a new share.
	{"NetShareDel",             PyNetShareDel,              1}, // @pymeth NetShareDel|Deletes a share
	{"NetShareCheck",           PyNetShareCheck,            1}, // @pymeth NetShareCheck|Checks if server is sharing a device
	{"NetShareEnum",			PyNetShareEnum,				1,	"Obsolete Function,Level 1 call"},	// @pymeth NetShareEnum|Retrieves information about each shared resource on a server. 
	{"NetShareGetInfo",         PyNetShareGetInfo,          1}, // @pymeth NetShareGetInfo|Retrieves information about a particular share on a server.
	{"NetShareSetInfo",         PyNetShareSetInfo,          1}, // @pymeth NetShareSetInfo|Sets information about a particular share on a server.

	{"NetUserAdd",              PyNetUserAdd,               1}, // @pymeth NetUserAdd|Creates a new user.
	{"NetUserChangePassword",	PyNetUserChangePassword,	1}, // @pymeth NetUserChangePassword|Changes a users password on the specified domain.
	{"NetUserEnum",             PyNetUserEnum,              1}, // @pymeth NetUserEnum|Enumerates all users.
	{"NetUserGetGroups",		PyNetUserGetGroups,			1,	"Updated - New Behavior"}, // @pymeth NetUserGetGroups|Returns a list of groups,attributes for all groups for the user.
	{"NetUserGetInfo",          PyNetUserGetInfo,           1}, // @pymeth NetUserGetInfo|Retrieves information about a particular user account on a server.
	{"NetUserGetLocalGroups",	PyNetUserGetLocalGroups,	1,	"Updated - New Behavior"}, // @pymeth NetUserGetLocalGroups|Retrieves a list of local groups to which a specified user belongs.
	{"NetUserSetInfo",          PyNetUserSetInfo,           1}, // @pymeth NetUserSetInfo|Sets information about a particular user account on a server.
	{"NetUserDel",              PyNetUserDel,               1}, // @pymeth NetUserDel|Deletes a user.

	{"NetUserModalsGet",          PyNetUserModalsGet,           1}, // @pymeth NetUserModalsGet|Retrieves global user information on a server.
  {"NetUserModalsSet",          PyNetUserModalsSet,           1}, // @pymeth NetUserModalsSet|Sets global user information on a server.

    {"NetWkstaUserEnum",        PyNetWkstaUserEnum,         1}, // @pymeth NetWkstaUserEnum|Retrieves information about all users currently logged on to the workstation.
    {"NetWkstaGetInfo",         PyNetWkstaGetInfo,          1}, // @pymeth NetWkstaGetInfo|returns information about the configuration elements for a workstation.
    {"NetWkstaSetInfo",         PyNetWkstaSetInfo,          1}, // @pymeth NetWkstaSetInfo|Sets information about the configuration elements for a workstation.
    {"NetWkstaTransportEnum",   PyNetWkstaTransportEnum,    1}, // @pymeth NetWkstaTransportEnum|Retrieves information about transport protocols that are currently managed by the redirector.
    {"NetWkstaTransportAdd",    PyNetWkstaTransportAdd,    1}, // @pymeth NetWkstaTransportAdd|binds the redirector to a transport.
    {"NetWkstaTransportDel",    PyNetWkstaTransportDel,    1}, // @pymeth NetWkstaTransportDel|unbinds transport protocol from the redirector.
    {"NetServerDiskEnum",       PyNetServerDiskEnum,       1}, // @pymeth NetServerDiskEnum|Retrieves the list of disk drives on a server.
    
    {"NetUseAdd",               PyNetUseAdd,               1}, // @pymeth NetUseAdd|Establishes connection between local or NULL device name and a shared resource through redirector.
    {"NetUseDel",               PyNetUseDel,               1}, // @pymeth NetUseDel|Ends connection to a shared resource.
    {"NetUseEnum",              PyNetUseEnum,               1}, // @pymeth NetUseEnum|Enumerates connection between local machine and shared resources on remote computers.
    {"NetUseGetInfo",           PyNetUseGetInfo,           1}, // @pymeth NetUseGetInfo|Get information about locally mapped shared resource on remote computer.

	{"NetGetAnyDCName",         PyNetGetAnyDCName,         1}, // @pymeth NetGetAnyDCName|Returns the name of any domain controller trusted by the specified server.
	{"NetGetDCName",            PyNetGetDCName,            1}, // @pymeth NetGetDCName|Returns the name of the primary domain controller (PDC).

	{"NetSessionEnum",          PyNetSessionEnum,          1}, // @pymeth NetSessionEnum|Returns network session for the server, limited to single client and/or user if specified.
	{"NetSessionDel",           PyNetSessionDel,           1}, // @pymeth NetSessionDel|Delete network session for specified server, client computer and user. Returns None on success.
	{"NetSessionGetInfo",       PyNetSessionGetInfo,       1}, // @pymeth NetSessionGetInfo|Get network session information.
	{"NetFileEnum",             PyNetFileEnum,             1}, // @pymeth NetFileEnum|Returns open file resources for server (single client and/or user may also be passed as criteria).
	{"NetFileClose",            PyNetFileClose,            1}, // @pymeth NetFileClose|Closes file for specified server and file id.
	{"NetFileGetInfo",          PyNetFileGetInfo,          1}, // @pymeth NetFileGetInfo|Get info about files open on the server.
	{"NetStatisticsGet",		PyNetStatisticsGet,		   1}, // @pymeth NetStatisticsGet|Return server or workstation stats
	{NULL,			NULL}
};

static void AddConstant(PyObject *dict, char *name, long val)
{
  PyObject *nv = PyInt_FromLong(val);
  PyDict_SetItemString(dict, name, nv );
  Py_XDECREF(nv);
}

extern "C" __declspec(dllexport) void
initwin32net(void)
{
  PyObject *dict, *module;
  module = Py_InitModule("win32net", win32net_functions);
  if (!module) return;
  dict = PyModule_GetDict(module);
  if (!dict) return;
  PyWinGlobals_Ensure();
  PyDict_SetItemString(dict, "error", PyWinExc_ApiError);
  PyDict_SetItemString(dict, "SERVICE_SERVER", PyUnicode_FromWideChar(SERVICE_SERVER,wcslen(SERVICE_SERVER)));
  PyDict_SetItemString(dict, "SERVICE_WORKSTATION", PyUnicode_FromWideChar(SERVICE_WORKSTATION,wcslen(SERVICE_WORKSTATION)));
  
  Py_INCREF(PyWinExc_ApiError);
  AddConstant(dict, "USE_NOFORCE", USE_NOFORCE);
  AddConstant(dict, "USE_FORCE", USE_FORCE);
  AddConstant(dict, "USE_LOTS_OF_FORCE", USE_LOTS_OF_FORCE);
}
