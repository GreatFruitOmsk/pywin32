// This file implements the IEnumSTATPROPSTG Interface and Gateway for Python.
// Generated by makegw.py

#include "stdafx.h"
#include "PythonCOM.h"

#ifndef NO_PYCOM_ENUMSTATPROPSTG

#include "PyIEnumSTATPROPSTG.h"

// @doc - This file contains autoduck documentation

// ---------------------------------------------------
//
// Interface Implementation

PyIEnumSTATPROPSTG::PyIEnumSTATPROPSTG(IUnknown *pdisp):
	PyIUnknown(pdisp)
{
	ob_type = &type;
}

PyIEnumSTATPROPSTG::~PyIEnumSTATPROPSTG()
{
}

/* static */ IEnumSTATPROPSTG *PyIEnumSTATPROPSTG::GetI(PyObject *self)
{
	return (IEnumSTATPROPSTG *)PyIUnknown::GetI(self);
}

// @pymethod object|PyIEnumSTATPROPSTG|Next|Retrieves a specified number of items in the enumeration sequence.
PyObject *PyIEnumSTATPROPSTG::Next(PyObject *self, PyObject *args)
{
	long celt = 1;
	// @pyparm int|num|1|Number of items to retrieve.
	if ( !PyArg_ParseTuple(args, "|l:Next", &celt) )
		return NULL;

	IEnumSTATPROPSTG *pIESTATPROPSTG = GetI(self);
	if ( pIESTATPROPSTG == NULL )
		return NULL;

	STATPROPSTG *rgVar = new STATPROPSTG[celt];
	if ( rgVar == NULL ) {
		PyErr_SetString(PyExc_MemoryError, "allocating result STATPROPSTGs");
		return NULL;
	}

	int i;
/*	for ( i = celt; i--; )
		// *** possibly init each structure element???
*/

	ULONG celtFetched = 0;
	HRESULT hr = pIESTATPROPSTG->Next(celt, rgVar, &celtFetched);
	if (  HRESULT_CODE(hr) != ERROR_NO_MORE_ITEMS && FAILED(hr) )
	{
		delete [] rgVar;
		return PyCom_BuildPyException(hr);
	}

	PyObject *result = PyTuple_New(celtFetched);
	if ( result != NULL )
	{
		for ( i = celtFetched; i--; )
		{
			PyObject *obName = PyWinObject_FromOLECHAR(rgVar[i].lpwstrName);
			PyObject *ob = Py_BuildValue("Oll", obName, rgVar[i].propid, rgVar[i].vt);
			Py_XDECREF(obName);
			PyTuple_SET_ITEM(result, i, ob);
		}
	}

	for ( i = celtFetched; i--; )
		CoTaskMemFree(rgVar[i].lpwstrName);

	delete [] rgVar;
	return result;
}

// @pymethod |PyIEnumSTATPROPSTG|Skip|Skips over the next specified elementes.
PyObject *PyIEnumSTATPROPSTG::Skip(PyObject *self, PyObject *args)
{
	long celt;
	if ( !PyArg_ParseTuple(args, "l:Skip", &celt) )
		return NULL;

	IEnumSTATPROPSTG *pIESTATPROPSTG = GetI(self);
	if ( pIESTATPROPSTG == NULL )
		return NULL;

	HRESULT hr = pIESTATPROPSTG->Skip(celt);
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod |PyIEnumSTATPROPSTG|Reset|Resets the enumeration sequence to the beginning.
PyObject *PyIEnumSTATPROPSTG::Reset(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":Reset") )
		return NULL;

	IEnumSTATPROPSTG *pIESTATPROPSTG = GetI(self);
	if ( pIESTATPROPSTG == NULL )
		return NULL;

	HRESULT hr = pIESTATPROPSTG->Reset();
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	Py_INCREF(Py_None);
	return Py_None;
}

// @pymethod <o PyIEnumSTATPROPSTG>|PyIEnumSTATPROPSTG|Clone|Creates another enumerator that contains the same enumeration state as the current one
PyObject *PyIEnumSTATPROPSTG::Clone(PyObject *self, PyObject *args)
{
	if ( !PyArg_ParseTuple(args, ":Clone") )
		return NULL;

	IEnumSTATPROPSTG *pIESTATPROPSTG = GetI(self);
	if ( pIESTATPROPSTG == NULL )
		return NULL;

	IEnumSTATPROPSTG *pClone;
	HRESULT hr = pIESTATPROPSTG->Clone(&pClone);
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr);

	return PyCom_PyObjectFromIUnknown(pClone, IID_IEnumSTATPROPSTG, FALSE);
}

// @object PyIEnumSTATPROPSTG|A Python interface to IEnumSTATPROPSTG
static struct PyMethodDef PyIEnumSTATPROPSTG_methods[] =
{
	{ "Next", PyIEnumSTATPROPSTG::Next, 1 },    // @pymeth Next|Retrieves a specified number of items in the enumeration sequence.
	{ "Skip", PyIEnumSTATPROPSTG::Skip, 1 },	// @pymeth Skip|Skips over the next specified elementes.
	{ "Reset", PyIEnumSTATPROPSTG::Reset, 1 },	// @pymeth Reset|Resets the enumeration sequence to the beginning.
	{ "Clone", PyIEnumSTATPROPSTG::Clone, 1 },	// @pymeth Clone|Creates another enumerator that contains the same enumeration state as the current one.
	{ NULL }
};

PyComTypeObject PyIEnumSTATPROPSTG::type("PyIEnumSTATPROPSTG",
		&PyIUnknown::type,
		sizeof(PyIEnumSTATPROPSTG),
		PyIEnumSTATPROPSTG_methods,
		GET_PYCOM_CTOR(PyIEnumSTATPROPSTG));

#endif // NO_PYCOM_ENUMSTATPROPSTG
