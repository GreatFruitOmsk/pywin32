// This file implements the IShellFolder Interface and Gateway for Python.
// Generated by makegw.py

#include "shell_pch.h"
#include "PyIShellFolder.h"

extern void *PyShell_AllocMem(ULONG cb);

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIShellFolder::PyIShellFolder(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIShellFolder::~PyIShellFolder()
{
}

/* static */ IShellFolder *PyIShellFolder::GetI(PyObject *self)
{
	return (IShellFolder *)PyIUnknown::GetI(self);
}

// @pymethod |PyIShellFolder|ParseDisplayName|Description of ParseDisplayName.
PyObject *PyIShellFolder::ParseDisplayName(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm HWND|hwndOwner||Description for hwndOwner
	// @pyparm <o PyIBindCtx>|pbcReserved||Description for pbcReserved
	// @pyparm <o unicode>|lpszDisplayName||Description for lpszDisplayName
	PyObject *obpbcReserved;
	PyObject *oblpszDisplayName;
	HWND hwndOwner;
	IBindCtx * pbcReserved;
	LPOLESTR lpszDisplayName;
	ULONG pchEaten;
	ITEMIDLIST *ppidl;
	ULONG pdwAttributes;
	if ( !PyArg_ParseTuple(args, "lOO:ParseDisplayName", &hwndOwner, &obpbcReserved, &oblpszDisplayName) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyCom_InterfaceFromPyInstanceOrObject(obpbcReserved, IID_IBindCtx, (void **)&pbcReserved, TRUE /* bNoneOK */))
		 bPythonIsHappy = FALSE;
	if (bPythonIsHappy && !PyWinObject_AsBstr(oblpszDisplayName, &lpszDisplayName)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->ParseDisplayName( hwndOwner, pbcReserved, lpszDisplayName, &pchEaten, &ppidl, &pdwAttributes );
	if (pbcReserved) pbcReserved->Release();
	SysFreeString(lpszDisplayName);

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );
	PyObject *obppidl;

	obppidl = PyObject_FromPIDL(ppidl, TRUE);
	PyObject *pyretval = Py_BuildValue("lOl", pchEaten, obppidl, pdwAttributes);
	Py_XDECREF(obppidl);
	return pyretval;
}

// @pymethod |PyIShellFolder|EnumObjects|Description of EnumObjects.
PyObject *PyIShellFolder::EnumObjects(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm HWND|hwndOwner|0|Description for hwndOwner
	// @pyparm int|grfFlags|SHCONTF_FOLDERS\|SHCONTF_NONFOLDERS\|SHCONTF_INCLUDEHIDDEN|Description for grfFlags
	HWND hwndOwner = 0;
	DWORD grfFlags = SHCONTF_FOLDERS|SHCONTF_NONFOLDERS|SHCONTF_INCLUDEHIDDEN;
	IEnumIDList * ppeidl;
	if ( !PyArg_ParseTuple(args, "|ll:EnumObjects", &hwndOwner, &grfFlags) )
		return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->EnumObjects( hwndOwner, grfFlags, &ppeidl );

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );

	return PyCom_PyObjectFromIUnknown(ppeidl, IID_IEnumIDList, FALSE);
}

// @pymethod |PyIShellFolder|BindToObject|Description of BindToObject.
PyObject *PyIShellFolder::BindToObject(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm <o PyIDL>|pidl||Description for pidl
	// @pyparm <o PyIBindCtx>|pbcReserved||Description for pbcReserved
	// @pyparm <o PyIID>|riid||Description for riid
	PyObject *obpidl;
	PyObject *obpbcReserved;
	PyObject *obriid;
	ITEMIDLIST *pidl;
	IBindCtx * pbcReserved;
	IID riid;
	void * out;
	if ( !PyArg_ParseTuple(args, "OOO:BindToObject", &obpidl, &obpbcReserved, &obriid) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsPIDL(obpidl, &pidl)) bPythonIsHappy = FALSE;
	if (bPythonIsHappy && !PyCom_InterfaceFromPyInstanceOrObject(obpbcReserved, IID_IBindCtx, (void **)&pbcReserved, TRUE /* bNoneOK */))
		 bPythonIsHappy = FALSE;
	if (!PyWinObject_AsIID(obriid, &riid)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->BindToObject( pidl, pbcReserved, riid, &out );
	PyObject_FreePIDL(pidl);
	if (pbcReserved) pbcReserved->Release();

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );

	return PyCom_PyObjectFromIUnknown((IUnknown *)out, riid, FALSE);
}

// @pymethod |PyIShellFolder|BindToStorage|Description of BindToStorage.
PyObject *PyIShellFolder::BindToStorage(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm <o PyIDL>|pidl||Description for pidl
	// @pyparm <o PyIBindCtx>|pbcReserved||Description for pbcReserved
	// @pyparm <o PyIID>|riid||Description for riid
	PyObject *obpidl;
	PyObject *obpbcReserved;
	PyObject *obriid;
	ITEMIDLIST *pidl;
	IBindCtx * pbcReserved;
	IID riid;
	void * out;
	if ( !PyArg_ParseTuple(args, "OOO:BindToStorage", &obpidl, &obpbcReserved, &obriid) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsPIDL(obpidl, &pidl)) bPythonIsHappy = FALSE;
	if (bPythonIsHappy && !PyCom_InterfaceFromPyInstanceOrObject(obpbcReserved, IID_IBindCtx, (void **)&pbcReserved, TRUE /* bNoneOK */))
		 bPythonIsHappy = FALSE;
	if (!PyWinObject_AsIID(obriid, &riid)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->BindToStorage( pidl, pbcReserved, riid, &out );
	PyObject_FreePIDL(pidl);
	if (pbcReserved) pbcReserved->Release();

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );

	return PyCom_PyObjectFromIUnknown((IUnknown *)out, riid, FALSE);
}

// @pymethod int|PyIShellFolder|CompareIDs|Description of CompareIDs.
PyObject *PyIShellFolder::CompareIDs(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm int|lparam||Description for lparam
	// @pyparm <o PyIDL>|pidl1||Description for pidl1
	// @pyparm <o PyIDL>|pidl2||Description for pidl2
	PyObject *obpidl1;
	PyObject *obpidl2;
	long lparam;
	ITEMIDLIST *pidl1;
	ITEMIDLIST *pidl2;
	if ( !PyArg_ParseTuple(args, "lOO:CompareIDs", &lparam, &obpidl1, &obpidl2) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsPIDL(obpidl1, &pidl1)) bPythonIsHappy = FALSE;
	if (bPythonIsHappy && !PyObject_AsPIDL(obpidl2, &pidl2)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->CompareIDs( lparam, pidl1, pidl2 );
	PyObject_FreePIDL(pidl1);
	PyObject_FreePIDL(pidl2);

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );

	// special handling of hresult
	if ((short)HRESULT_CODE(hr) < 0)
		/* pidl1 comes first */
		return PyInt_FromLong(-1);
	else if ((short)HRESULT_CODE(hr) > 0) 
		/* pidl2 comes first */
		return PyInt_FromLong(1);
	else 
	/* the two pidls are equal */
		return PyInt_FromLong(0);
}

// @pymethod |PyIShellFolder|CreateViewObject|Description of CreateViewObject.
PyObject *PyIShellFolder::CreateViewObject(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm HWND|hwndOwner||Description for hwndOwner
	// @pyparm <o PyIID>|riid||Description for riid
	PyObject *obriid;
	HWND hwndOwner;
	IID riid;
	void * out;
	if ( !PyArg_ParseTuple(args, "lO:CreateViewObject", &hwndOwner, &obriid) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (!PyWinObject_AsIID(obriid, &riid)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->CreateViewObject( hwndOwner, riid, &out );

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );

	return PyCom_PyObjectFromIUnknown((IUnknown *)out, riid, FALSE);
}

// @pymethod |PyIShellFolder|GetAttributesOf|Description of GetAttributesOf.
PyObject *PyIShellFolder::GetAttributesOf(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm <o PyIDL>|pidl||Description for pidl
	// @pyparm int|rgfInOut||Description for rgfInOut
	PyObject *obpidl;
	UINT cidl;
	LPCITEMIDLIST *pidl;
	ULONG rgfInOut;
	if ( !PyArg_ParseTuple(args, "Ol:GetAttributesOf", &obpidl, &rgfInOut) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsPIDLArray(obpidl, &cidl, &pidl)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->GetAttributesOf( cidl, pidl, &rgfInOut );
	PyObject_FreePIDLArray(cidl, pidl);

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );
	return PyInt_FromLong(rgfInOut);
}

// @pymethod |PyIShellFolder|GetUIObjectOf|Description of GetUIObjectOf.
PyObject *PyIShellFolder::GetUIObjectOf(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm HWND|hwndOwner||Description for hwndOwner
	// @pyparm <o PyIDL>|pidl||Description for pidl
	// @pyparm <o PyIID>|riid||Description for riid
	// @pyparm int|rgfInOut||Description for rgfInOut
	// @pyparm <o PyIID>|iidout|None|The IID to wrap the result in.  If not specified, riid is used.
	PyObject *obpidl;
	PyObject *obriid;
	PyObject *obiidout = NULL;
	HWND hwndOwner;
	UINT cidl;
	LPCITEMIDLIST *pidl;
	IID riid, iidout;
	UINT rgfInOut;
	void * out;
	if ( !PyArg_ParseTuple(args, "lOOl|O:GetUIObjectOf", &hwndOwner, &obpidl, &obriid, &rgfInOut, &obiidout) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsPIDLArray(obpidl, &cidl, &pidl)) bPythonIsHappy = FALSE;
	if (!PyWinObject_AsIID(obriid, &riid)) bPythonIsHappy = FALSE;
	if (obiidout==NULL)
		iidout = riid;
	else
		if (!PyWinObject_AsIID(obiidout, &iidout)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->GetUIObjectOf( hwndOwner, cidl, pidl, riid, &rgfInOut, &out );
	PyObject_FreePIDLArray(cidl, pidl);

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );
	PyObject *obout;

	obout = PyCom_PyObjectFromIUnknown((IUnknown *)out, iidout, FALSE);
	PyObject *pyretval = Py_BuildValue("lO", rgfInOut, obout);
	Py_XDECREF(obout);
	return pyretval;
}

// @pymethod |PyIShellFolder|GetDisplayNameOf|Description of GetDisplayNameOf.
PyObject *PyIShellFolder::GetDisplayNameOf(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm <o PyIDL>|pidl||Description for pidl
	// @pyparm int|uFlags||Description for uFlags
	PyObject *obpidl;
	ITEMIDLIST *pidl;
	DWORD uFlags;
	if ( !PyArg_ParseTuple(args, "Ol:GetDisplayNameOf", &obpidl, &uFlags) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsPIDL(obpidl, &pidl)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	STRRET out;
	PY_INTERFACE_PRECALL;
	hr = pISF->GetDisplayNameOf( pidl, uFlags, &out );
	PyObject_FreePIDL(pidl);

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );
	return PyObject_FromSTRRET(&out, pidl, TRUE);
}

// @pymethod |PyIShellFolder|SetNameOf|Description of SetNameOf.
PyObject *PyIShellFolder::SetNameOf(PyObject *self, PyObject *args)
{
	IShellFolder *pISF = GetI(self);
	if ( pISF == NULL )
		return NULL;
	// @pyparm HWND|hwndOwner||Description for hwndOwner
	// @pyparm <o PyIDL>|pidl||Description for pidl
	// @pyparm <o unicode>|lpszName||Description for lpszName
	PyObject *obpidl;
	PyObject *oblpszName;
	HWND hwndOwner;
	ITEMIDLIST *pidl;
	ITEMIDLIST *pidlRet;
	LPOLESTR lpszName;
	long flags;
	if ( !PyArg_ParseTuple(args, "lOOl:SetNameOf", &hwndOwner, &obpidl, &oblpszName, &flags) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsPIDL(obpidl, &pidl)) bPythonIsHappy = FALSE;
	if (bPythonIsHappy && !PyWinObject_AsBstr(oblpszName, &lpszName)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pISF->SetNameOf( hwndOwner, pidl, lpszName, (SHGDNF)flags, &pidlRet );
	PyObject_FreePIDL(pidl);
	SysFreeString(lpszName);

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pISF, IID_IShellFolder );
	PyObject *ret = PyObject_FromPIDL(pidlRet, TRUE);
	Py_INCREF(Py_None);
	return Py_None;

}

// @object PyIShellFolder|Description of the interface
static struct PyMethodDef PyIShellFolder_methods[] =
{
	{ "ParseDisplayName", PyIShellFolder::ParseDisplayName, 1 }, // @pymeth ParseDisplayName|Description of ParseDisplayName
	{ "EnumObjects", PyIShellFolder::EnumObjects, 1 }, // @pymeth EnumObjects|Description of EnumObjects
	{ "BindToObject", PyIShellFolder::BindToObject, 1 }, // @pymeth BindToObject|Description of BindToObject
	{ "BindToStorage", PyIShellFolder::BindToStorage, 1 }, // @pymeth BindToStorage|Description of BindToStorage
	{ "CompareIDs", PyIShellFolder::CompareIDs, 1 }, // @pymeth CompareIDs|Description of CompareIDs
	{ "CreateViewObject", PyIShellFolder::CreateViewObject, 1 }, // @pymeth CreateViewObject|Description of CreateViewObject
	{ "GetAttributesOf", PyIShellFolder::GetAttributesOf, 1 }, // @pymeth GetAttributesOf|Description of GetAttributesOf
	{ "GetUIObjectOf", PyIShellFolder::GetUIObjectOf, 1 }, // @pymeth GetUIObjectOf|Description of GetUIObjectOf
	{ "GetDisplayNameOf", PyIShellFolder::GetDisplayNameOf, 1 }, // @pymeth GetDisplayNameOf|Description of GetDisplayNameOf
	{ "SetNameOf", PyIShellFolder::SetNameOf, 1 }, // @pymeth SetNameOf|Description of SetNameOf
	{ NULL }
};

// @pymeth __iter__|Enumerates all objects in this folder.
PyComEnumProviderTypeObject PyIShellFolder::type("PyIShellFolder",
		&PyIUnknown::type,
		sizeof(PyIShellFolder),
		PyIShellFolder_methods,
		GET_PYCOM_CTOR(PyIShellFolder),
		"EnumObjects");
// ---------------------------------------------------
//
// Gateway Implementation
STDMETHODIMP PyGShellFolder::ParseDisplayName(
		/* [unique][in] */ HWND hwndOwner,
		/* [unique][in] */ LPBC pbcReserved,
		/* [unique][in] */ LPOLESTR lpszDisplayName,
		/* [out] */ ULONG __RPC_FAR * pchEaten,
		/* [out] */ LPITEMIDLIST *ppidl,
		/* [out] */ ULONG __RPC_FAR * pdwAttributes)
{
	PY_GATEWAY_METHOD;
	*ppidl = NULL;
	PyObject *obpbcReserved;
	PyObject *oblpszDisplayName;
	obpbcReserved = PyCom_PyObjectFromIUnknown(pbcReserved, IID_IBindCtx, TRUE);
	oblpszDisplayName = MakeOLECHARToObj(lpszDisplayName);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy("ParseDisplayName", &result, "lOO", hwndOwner, obpbcReserved, oblpszDisplayName);
	Py_XDECREF(obpbcReserved);
	Py_XDECREF(oblpszDisplayName);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyObject *obppidl;
	ULONG chEaten, dwAttributes;
	if (!PyArg_ParseTuple(result, "lOl" , &chEaten, &obppidl, &dwAttributes)) 
		return PyCom_SetAndLogCOMErrorFromPyException("ParseDisplayName", IID_IShellFolder);
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsPIDL(obppidl, ppidl)) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) hr = PyCom_SetAndLogCOMErrorFromPyException("ParseDisplayName", IID_IShellFolder);
	if (pchEaten) *pchEaten = chEaten;
	if (pdwAttributes) *pdwAttributes = dwAttributes;
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::EnumObjects(
		/* [unique][in] */ HWND hwndOwner,
		/* [unique][in] */ DWORD grfFlags,
		/* [out] */ IEnumIDList __RPC_FAR ** ppeidl)
{
	PY_GATEWAY_METHOD;
	PyObject *result;
	HRESULT hr=InvokeViaPolicy("EnumObjects", &result, "ll", hwndOwner, grfFlags);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyCom_InterfaceFromPyInstanceOrObject(result, IID_IEnumIDList, (void **)ppeidl, FALSE /* bNoneOK */);
	hr = PyCom_SetAndLogCOMErrorFromPyException("EnumObjects", IID_IShellFolder);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::BindToObject(
		/* [unique][in] */ LPCITEMIDLIST pidl,
		/* [unique][in] */ LPBC pbcReserved,
		/* [unique][in] */ REFIID riid,
		/* [out] */ void ** out)
{
	static const char *szMethodName = "BindToObject";
	PY_GATEWAY_METHOD;
	PyObject *obpidl;
	PyObject *obpbcReserved;
	PyObject *obriid;
	obpidl = PyObject_FromPIDL(pidl, FALSE);
	obpbcReserved = PyCom_PyObjectFromIUnknown(pbcReserved, IID_IBindCtx, TRUE);
	obriid = PyWinObject_FromIID(riid);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy(szMethodName, &result, "OOO", obpidl, obpbcReserved, obriid);
	Py_XDECREF(obpidl);
	Py_XDECREF(obpbcReserved);
	Py_XDECREF(obriid);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyCom_InterfaceFromPyInstanceOrObject(result, riid, out, FALSE /* bNoneOK */);
	hr = PyCom_SetAndLogCOMErrorFromPyException(szMethodName, IID_IShellFolder);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::BindToStorage(
		/* [unique][in] */ LPCITEMIDLIST pidl,
		/* [unique][in] */ LPBC pbcReserved,
		/* [unique][in] */ REFIID riid,
		/* [out] */ void **ppRet)
{
	static const char *szMethodName = "BindToStorage";
	PY_GATEWAY_METHOD;
	PyObject *obpidl;
	PyObject *obpbcReserved;
	PyObject *obriid;
	obpidl = PyObject_FromPIDL(pidl, FALSE);
	obpbcReserved = PyCom_PyObjectFromIUnknown(pbcReserved, IID_IBindCtx, TRUE);
	obriid = PyWinObject_FromIID(riid);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy(szMethodName, &result, "OOO", obpidl, obpbcReserved, obriid);
	Py_XDECREF(obpidl);
	Py_XDECREF(obpbcReserved);
	Py_XDECREF(obriid);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyCom_InterfaceFromPyInstanceOrObject(result, riid, ppRet, FALSE /* bNoneOK */);
	hr = PyCom_SetAndLogCOMErrorFromPyException(szMethodName, IID_IShellFolder);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::CompareIDs(
		/* [unique][in] */ long lparam,
		/* [unique][in] */ const ITEMIDLIST __RPC_FAR * pidl1,
		/* [unique][in] */ const ITEMIDLIST __RPC_FAR * pidl2)
{
	PY_GATEWAY_METHOD;
	PyObject *obpidl1;
	PyObject *obpidl2;
	obpidl1 = PyObject_FromPIDL(pidl1, FALSE);
	obpidl2 = PyObject_FromPIDL(pidl2, FALSE);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy("CompareIDs", &result, "lOO", lparam, obpidl1, obpidl2);
	Py_XDECREF(obpidl1);
	Py_XDECREF(obpidl2);
	if (FAILED(hr)) return hr;
	if (PyInt_Check(result))
		hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, PyInt_AsLong(result));
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::CreateViewObject(
		/* [unique][in] */ HWND hwndOwner,
		/* [unique][in] */ REFIID riid,
		/* [out] */ void **ppRet)
{
	static const char *szMethodName = "CreateViewObject";
	PY_GATEWAY_METHOD;
	PyObject *obriid;
	obriid = PyWinObject_FromIID(riid);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy(szMethodName, &result, "lO", hwndOwner, obriid);
	Py_XDECREF(obriid);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyCom_InterfaceFromPyInstanceOrObject(result, riid, ppRet, FALSE /* bNoneOK */);
	hr = PyCom_SetAndLogCOMErrorFromPyException(szMethodName, IID_IShellFolder);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::GetAttributesOf(
		/* [unique][in] */ UINT cidl,
		/* [unique][in] */ LPCITEMIDLIST *apidl,
		/* [unique][in][out] */ ULONG __RPC_FAR * rgfInOut)
{
	PY_GATEWAY_METHOD;
	PyObject *obpidl;
	obpidl = PyObject_FromPIDLArray(cidl, apidl);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy("GetAttributesOf", &result, "Ol", obpidl, rgfInOut? *rgfInOut : 0);
	Py_XDECREF(obpidl);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	if (!PyArg_Parse(result, "l" , rgfInOut))
		hr = PyCom_SetAndLogCOMErrorFromPyException("GetAttributesOf", IID_IShellFolder);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::GetUIObjectOf(
		/* [unique][in] */ HWND hwndOwner,
		/* [unique][in] */ UINT cidl,
		/* [unique][in] */ LPCITEMIDLIST *apidl,
		/* [unique][in] */ REFIID riid,
		/* [unique][in][out] */ UINT * rgfInOut,
		/* [out] */ void ** ppRet)
{
	static const char *szMethodName = "GetUIObjectOf";
	PY_GATEWAY_METHOD;
	PyObject *obpidl;
	PyObject *obriid;
	obpidl = PyObject_FromPIDLArray(cidl, apidl);
	obriid = PyWinObject_FromIID(riid);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy(szMethodName, &result, "lOOl", hwndOwner, obpidl, obriid, rgfInOut ? *rgfInOut : 0);
	Py_XDECREF(obpidl);
	Py_XDECREF(obriid);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	PyObject *obout;
	UINT inout;
	if (!PyArg_ParseTuple(result, "lO" , &inout, &obout)) return PyCom_SetAndLogCOMErrorFromPyException(szMethodName, IID_IShellFolder);
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyCom_InterfaceFromPyInstanceOrObject(obout, riid, ppRet, FALSE/* bNoneOK */))
		 bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) hr = PyCom_SetAndLogCOMErrorFromPyException(szMethodName, IID_IShellFolder);
	if (rgfInOut) *rgfInOut = inout;
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::GetDisplayNameOf(
		/* [unique][in] */ const ITEMIDLIST __RPC_FAR * pidl,
		/* [unique][in] */ DWORD uFlags,
		/* [out] */ STRRET __RPC_FAR * out)
{
	static const char *szMethodName = "GetDisplayNameOf";
	if (!out)
		return E_POINTER;
	PY_GATEWAY_METHOD;
	PyObject *obpidl;
	obpidl = PyObject_FromPIDL(pidl, FALSE);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy(szMethodName, &result, "Ol", obpidl, uFlags);
	Py_XDECREF(obpidl);
	if (FAILED(hr)) return hr;
	// Process the Python results, and convert back to the real params
	out->uType = STRRET_WSTR;
	if (!PyWinObject_AsPfnAllocatedWCHAR(result, PyShell_AllocMem, &out->pOleStr))
		hr = PyCom_SetAndLogCOMErrorFromPyException(szMethodName, IID_IShellFolder);
	Py_DECREF(result);
	return hr;
}

STDMETHODIMP PyGShellFolder::SetNameOf(
		/* [in] */ HWND hwnd,
		/* [in] */ LPCITEMIDLIST pidl,
		/* [string][in] */ LPCOLESTR pszName,
		/* [in] */ SHGDNF uFlags,
		/* [out] */ LPITEMIDLIST *ppidlOut)
{
	static const char *szMethodName = "SetNameOf";
	PY_GATEWAY_METHOD;
	PyObject *obpidl;
	PyObject *oblpszName;
	obpidl = PyObject_FromPIDL(pidl, FALSE);
	oblpszName = MakeOLECHARToObj(pszName);
	PyObject *result;
	HRESULT hr=InvokeViaPolicy(szMethodName, &result, "lOOl", hwnd, obpidl, oblpszName, uFlags);
	Py_XDECREF(obpidl);
	Py_XDECREF(oblpszName);
	if (FAILED(hr)) return hr;
	PyObject_AsPIDL(result, ppidlOut, FALSE);
	hr = PyCom_SetAndLogCOMErrorFromPyException(szMethodName, IID_IShellFolder);
	Py_DECREF(result);
	return hr;
}

