// This file implements the IContextMenu Interface and Gateway for Python.
// Generated by makegw.py

#include "shell_pch.h"
#include "PyIContextMenu.h"

// @doc - This file contains autoduck documentation
// ---------------------------------------------------
//
// Interface Implementation

PyIContextMenu::PyIContextMenu(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIContextMenu::~PyIContextMenu()
{
}

/* static */ IContextMenu *PyIContextMenu::GetI(PyObject *self)
{
	return (IContextMenu *)PyIUnknown::GetI(self);
}

// @pymethod |PyIContextMenu|QueryContextMenu|Description of QueryContextMenu.
PyObject *PyIContextMenu::QueryContextMenu(PyObject *self, PyObject *args)
{
	IContextMenu *pICM = GetI(self);
	if ( pICM == NULL )
		return NULL;
	// @pyparm int|hmenu||Description for hmenu
	// @pyparm int|indexMenu||Description for indexMenu
	// @pyparm int|idCmdFirst||Description for idCmdFirst
	// @pyparm int|idCmdLast||Description for idCmdLast
	// @pyparm int|uFlags||Description for uFlags
	INT hmenu;
	UINT indexMenu;
	UINT idCmdFirst;
	UINT idCmdLast;
	UINT uFlags;
	if ( !PyArg_ParseTuple(args, "iiiii:QueryContextMenu", &hmenu, &indexMenu, &idCmdFirst, &idCmdLast, &uFlags) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pICM->QueryContextMenu( (HMENU)hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags );

	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pICM, IID_IContextMenu );
	return PyInt_FromLong(hr);
}

// @pymethod |PyIContextMenu|InvokeCommand|Description of InvokeCommand.
PyObject *PyIContextMenu::InvokeCommand(PyObject *self, PyObject *args)
{
	IContextMenu *pICM = GetI(self);
	if ( pICM == NULL )
		return NULL;
	CMINVOKECOMMANDINFO ci;
	PyObject *oblpici;
	if ( !PyArg_ParseTuple(args, "O:InvokeCommand", &oblpici) )
		return NULL;
	BOOL bPythonIsHappy = TRUE;
	if (bPythonIsHappy && !PyObject_AsCMINVOKECOMMANDINFO( oblpici, &ci )) bPythonIsHappy = FALSE;
	if (!bPythonIsHappy) return NULL;
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pICM->InvokeCommand( &ci );
	PyObject_FreeCMINVOKECOMMANDINFO(&ci);
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pICM, IID_IContextMenu );
	Py_INCREF(Py_None);
	return Py_None;

}

// @pymethod |PyIContextMenu|GetCommandString|Description of GetCommandString.
PyObject *PyIContextMenu::GetCommandString(PyObject *self, PyObject *args)
{
	IContextMenu *pICM = GetI(self);
	if ( pICM == NULL )
		return NULL;
	// @pyparm int|idCmd||Description for idCmd
	// @pyparm int|uType||Description for uType
	// @pyparm int|cchMax|2048|Description for cchMax
	UINT idCmd;
	UINT uType;
	UINT cchMax = 2048;
	if ( !PyArg_ParseTuple(args, "ii|i:GetCommandString", &idCmd, &uType, &cchMax) )
		return NULL;

	char *buf = (char *)malloc(cchMax);
	if (!buf)
		return PyErr_NoMemory();
	HRESULT hr;
	PY_INTERFACE_PRECALL;
	hr = pICM->GetCommandString( idCmd, uType, NULL, buf, cchMax );
	PY_INTERFACE_POSTCALL;

	if ( FAILED(hr) ) {
		free(buf);
		return PyCom_BuildPyException(hr, pICM, IID_IContextMenu );
	}
	PyObject *ret = PyString_FromString(buf);
	free(buf);
	return ret;

}

// @object PyIContextMenu|Description of the interface
static struct PyMethodDef PyIContextMenu_methods[] =
{
	{ "QueryContextMenu", PyIContextMenu::QueryContextMenu, 1 }, // @pymeth QueryContextMenu|Description of QueryContextMenu
	{ "InvokeCommand", PyIContextMenu::InvokeCommand, 1 }, // @pymeth InvokeCommand|Description of InvokeCommand
	{ "GetCommandString", PyIContextMenu::GetCommandString, 1 }, // @pymeth GetCommandString|Description of GetCommandString
	{ NULL }
};

PyComTypeObject PyIContextMenu::type("PyIContextMenu",
		&PyIUnknown::type,
		sizeof(PyIContextMenu),
		PyIContextMenu_methods,
		GET_PYCOM_CTOR(PyIContextMenu));
// ---------------------------------------------------
//
// Gateway Implementation
STDMETHODIMP PyGContextMenu::QueryContextMenu(
		/* [unique][in] */ HMENU hmenu,
		/* [unique][in] */ UINT indexMenu,
		/* [unique][in] */ UINT idCmdFirst,
		/* [unique][in] */ UINT idCmdLast,
		/* [unique][in] */ UINT uFlags)
{
	PY_GATEWAY_METHOD;
	PyObject *ret;
	HRESULT hr=InvokeViaPolicy("QueryContextMenu", &ret, "iiiii", hmenu, indexMenu, idCmdFirst, idCmdLast, uFlags);
	if (FAILED(hr)) return hr;
	if (PyInt_Check(ret))
		hr = MAKE_HRESULT(SEVERITY_SUCCESS, 0, PyInt_AsLong(ret));
	return hr;
}

STDMETHODIMP PyGContextMenu::InvokeCommand(
		/* [unique][in] */ CMINVOKECOMMANDINFO __RPC_FAR * lpici)
{
	PY_GATEWAY_METHOD;
	PyObject *oblpici = PyObject_FromCMINVOKECOMMANDINFO(lpici);
	if (oblpici==NULL) return MAKE_PYCOM_GATEWAY_FAILURE_CODE("InvokeCommand");
	HRESULT hr=InvokeViaPolicy("InvokeCommand", NULL, "(O)", oblpici);
	Py_DECREF(oblpici);
	return hr;
}

STDMETHODIMP PyGContextMenu::GetCommandString(
		/* [unique][in] */ UINT idCmd,
		/* [unique][in] */ UINT uFlags,
		/* [unique][in] */ UINT * pwReserved,
		/* [unique][in] */ LPSTR pszName,
		/* [unique][in] */ UINT cchMax)
{
	PyObject *result;
	PY_GATEWAY_METHOD;
	HRESULT hr=InvokeViaPolicy("GetCommandString", &result, "ii", idCmd, uFlags);
	if (FAILED(hr))
		return hr;
	if (result && (PyString_Check(result) || PyUnicode_Check(result))) {
		if (uFlags==GCS_HELPTEXTW || uFlags==GCS_VERBW) {
			WCHAR *szResult;
			if (PyWinObject_AsWCHAR(result, &szResult, FALSE, NULL)) {
				wcsncpy((WCHAR *)pszName, szResult, cchMax);
				PyWinObject_FreeWCHAR(szResult);
			}
		} else {
			char *szResult;
			if (PyWinObject_AsString(result, &szResult, FALSE, NULL)) {
				strncpy(pszName, szResult, cchMax);
				PyWinObject_FreeString(szResult);
			}
		}
		hr = S_OK;
	} else if (result && PyInt_Check(result)) {
		hr = PyInt_AsLong(result) ? S_OK : S_FALSE;
	}
	Py_DECREF(result);
	return hr;
}

