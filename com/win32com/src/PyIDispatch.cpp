// PyIDispatch and PyIDispathEx implementation

// @doc
#include "stdafx.h"
#include "PythonCOM.h"

// A little helper just for this file
static PyObject* OleSetTypeError(char *msg)
{
	PyErr_SetString(PyExc_TypeError, msg);
	return NULL;
}

static BOOL HandledDispatchFailure(HRESULT hr, EXCEPINFO *einfo, UINT nArgErr, UINT cArgs)
{
	if ( hr == DISP_E_EXCEPTION )
	{
		if ( einfo->scode != DISP_E_TYPEMISMATCH &&
			 einfo->scode != DISP_E_PARAMNOTFOUND )
			nArgErr = (UINT)-1;
		else
			nArgErr = cArgs - nArgErr;	/* convert to usable index */
		PyCom_BuildPyExceptionFromEXCEPINFO(hr, einfo, nArgErr);
		return TRUE;
	}

	if ( FAILED(hr) )
	{
		if ( hr != DISP_E_TYPEMISMATCH && hr != DISP_E_PARAMNOTFOUND )
			nArgErr =(UINT)-1;
		else
			nArgErr = cArgs - nArgErr;	/* convert to usable index */
		PyCom_BuildPyExceptionFromEXCEPINFO(hr, NULL, nArgErr);
		return TRUE;
	}
	return FALSE;
}

PyIDispatch::PyIDispatch(IUnknown *pDisp) :
	PyIUnknown(pDisp)
{
	ob_type = &type;
}

PyIDispatch::~PyIDispatch()
{
}

/*static*/ IDispatch *PyIDispatch::GetI(PyObject *self)
{
	return (IDispatch *)PyIUnknown::GetI(self);
}

// @pymethod (int, ...)/int|PyIDispatch|GetIDsOfNames|Get the DISPID for the passed names.
PyObject *PyIDispatch::GetIDsOfNames(PyObject *self, PyObject *args)
{
	// @rdesc If the first parameter is a sequence, the result will be a tuple of integers
	// for each name in the sequence.  If the first parameter is a single string, the result
	// is a single integer with the ID of requested item.
	UINT i;

	// @pyparm string|name||A name to query for
	// @pyparmalt1 [string, ...]|[name, ...]||A sequence of string names to query
	// @comm Currently the LCID can not be specified, and  LOCALE_SYSTEM_DEFAULT is used.
	int argc = PyObject_Length(args);
	if ( argc == -1 )
		return NULL;
	if ( argc < 1 )
		return OleSetTypeError("At least one argument must be supplied");

	LCID lcid = LOCALE_SYSTEM_DEFAULT;
	UINT offset = 0;
	if ( argc > 1 )
	{
		PyObject *ob = PySequence_GetItem(args, 0);
		if ( !ob )
			return NULL;
		if ( PyInt_Check(ob) )
		{
			lcid = PyInt_AS_LONG((PyIntObject *)ob);
			offset = 1;
		}
		Py_DECREF(ob);
	}

	UINT cNames = argc - offset;
	OLECHAR FAR* FAR* rgszNames = new LPOLESTR[cNames];

	for ( i = 0 ; i < cNames; ++i )
	{
		PyObject *ob = PySequence_GetItem(args, i + offset);
		if ( !ob )
		{
			for (;i>0;i--)
				PyWinObject_FreeBstr(rgszNames[i-1]);
			delete [] rgszNames;
			return NULL;
		}
		if (!PyWinObject_AsBstr(ob, rgszNames+i)) {
			for (;i>0;i--)
				PyWinObject_FreeBstr(rgszNames[i-1]);
			delete [] rgszNames;
			return NULL;
		}
		Py_DECREF(ob);
	}

	DISPID FAR* rgdispid = new DISPID[cNames];
	IDispatch *pMyDispatch = GetI(self);
	if (pMyDispatch==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatch->GetIDsOfNames(IID_NULL, rgszNames, cNames, lcid, rgdispid);
	PY_INTERFACE_POSTCALL;

	for (i=0;i<cNames;i++)
		PyWinObject_FreeBstr(rgszNames[i]);
	delete [] rgszNames;

	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pMyDispatch, IID_IDispatch);

	PyObject *result;

	/* if we have just one name, then return a single DISPID (int) */
	if ( cNames == 1 )
	{
		result = PyInt_FromLong(rgdispid[0]);
	}
	else
	{
		result = PyTuple_New(cNames);
		if ( result )
		{
			for ( i = 0; i < cNames; ++i )
			{
				PyObject *ob = PyInt_FromLong(rgdispid[i]);
				if ( !ob )
				{
					delete [] rgdispid;
					return NULL;
				}
				PyTuple_SET_ITEM(result, i, ob);
			}
		}
	}

	delete [] rgdispid;
	return result;
}

// Convert a PyTuple object into a DISPPARAM structure.
// numArgs specifies which of the LAST args in the tuple are valid.
// To convert all args, pass len(args)
static BOOL PyCom_MakeUntypedDISPPARAMS( PyObject *args, int numArgs, WORD wFlags, DISPPARAMS *pParm)
{
	int argc = PyObject_Length(args);
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	// Clean initialize
	pParm->rgvarg = NULL;
	pParm->rgdispidNamedArgs = NULL;
	pParm->cArgs = numArgs;
	pParm->cNamedArgs = 0;
	if ( pParm->cArgs )
	{
		pParm->rgvarg = new VARIANTARG[pParm->cArgs];
		for ( UINT i = 0; i < pParm->cArgs; ++i )
		{
			VariantInit(&pParm->rgvarg[i]);
			// args in reverse order.
			if ( !PyCom_VariantFromPyObject(PyTuple_GET_ITEM(args, argc-i-1), &pParm->rgvarg[i]) )
			{
				if ( !PyErr_Occurred() )
					OleSetTypeError("Bad argument");
				while ( i-- > 0 )
					VariantClear(&pParm->rgvarg[i]);
				return FALSE;
			}
		}

		/* puts and putrefs need a named argument */
		if ( wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF) )
		{
			pParm->rgdispidNamedArgs = new DISPID[1];
			pParm->rgdispidNamedArgs[0] = DISPID_PROPERTYPUT;
			pParm->cNamedArgs = 1;
		}
	}
	return TRUE;
}

static void PyCom_FreeUntypedDISPPARAMS( DISPPARAMS *pParm )
{
	if ( pParm->rgvarg )
	{
		for ( UINT i = pParm->cArgs; i--; )
			VariantClear(&pParm->rgvarg[i]);
		delete [] pParm->rgvarg;
	}
	delete [] pParm->rgdispidNamedArgs;
}

// @pymethod object|PyIDispatch|Invoke|Invokes a DISPID, using the passed arguments.
PyObject * PyIDispatch::Invoke(PyObject *self, PyObject *args)
{
	/* Invoke(dispid, lcid, wflags, bResultWanted, arg1, arg2...) */
	// should be no need to clear this error - but for the next few release
	// I will keep it in place for release builds, and assert in debug
#ifdef _DEBUG
	assert(!PyErr_Occurred());
#else
	PyErr_Clear();
#endif
	int argc = PyObject_Length(args);
	if ( argc == -1 )
		return NULL;
	if ( argc < 4 )
		return OleSetTypeError("not enough arguments (at least 4 needed)");

	// @pyparm int|dispid||The dispid to use.  Typically this value will come from <om PyIDispatch.GetIDsOfNames> or from a type library.
	DISPID dispid = PyInt_AsLong(PyTuple_GET_ITEM(args, 0));
	// @pyparm int|lcid||The locale id to use.
	LCID lcid = PyInt_AsLong(PyTuple_GET_ITEM(args, 1));
	// @pyparm int|flags||The flags for the call.  The following flags can be used.
	// @flagh Flag|Description
	// @flag DISPATCH_METHOD|The member is invoked as a method. If a property has the same name, both this and the DISPATCH_PROPERTYGET flag may be set. 
	// @flag DISPATCH_PROPERTYGET|The member is retrieved as a property or data member.
	// @flag DISPATCH_PROPERTYPUT|The member is changed as a property or data member. 
	// @flag DISPATCH_PROPERTYPUTREF|The member is changed by a reference assignment, rather than a value assignment. This flag is valid only when the property accepts a reference to an object. 
	UINT wFlags = PyInt_AsLong(PyTuple_GET_ITEM(args, 2));
	// @pyparm int|bResultWanted||Indicates if the result of the call should be requested.
	// @pyparm object, ...|params, ...||The parameters to pass.
	BOOL bResultWanted = (BOOL)PyInt_AsLong(PyTuple_GET_ITEM(args, 3));
	if ( PyErr_Occurred() )
		return NULL;

	IDispatch *pMyDispatch = GetI(self);
	if ( pMyDispatch==NULL )
		return NULL;

	DISPPARAMS dispparams;
	if (!PyCom_MakeUntypedDISPPARAMS(args, argc-4, wFlags, &dispparams ))
		return NULL;

	VARIANT varResult;
	VARIANT *pVarResultUse;
	if ( bResultWanted ) {
		VariantInit(&varResult);
		pVarResultUse = &varResult;
	} else
		pVarResultUse = NULL;

	// initialize EXCEPINFO struct
	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);

	UINT nArgErr = (UINT)-1;  // initialize to invalid arg
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatch->Invoke(dispid, IID_NULL, lcid, wFlags, &dispparams, pVarResultUse, &excepInfo, &nArgErr);
	PY_INTERFACE_POSTCALL;

	PyCom_FreeUntypedDISPPARAMS(&dispparams);

	if ( HandledDispatchFailure(hr, &excepInfo, nArgErr, dispparams.cArgs) )
	{
		if ( pVarResultUse )
			VariantClear(pVarResultUse);
		return NULL;
	}
	// @rdesc If the bResultWanted parameter is False, then the result will be None.
	// Otherwise, the result is determined by the COM object itself (and may still be None)
	PyObject *result;
	if (pVarResultUse) {
		result = PyCom_PyObjectFromVariant(pVarResultUse);
		VariantClear(pVarResultUse);
	} else {
		result = Py_None;
		Py_INCREF(result);
	}
	return result;
}

// @pymethod object|PyIDispatch|InvokeTypes|Invokes a DISPID, using the passed arguments and type descriptions.
PyObject * PyIDispatch::InvokeTypes(PyObject *self, PyObject *args)
{
	/* InvokeType(dispid, lcid, wflags, ELEMDESC resultType, ELEMDESC[] argTypes, arg1, arg2...) */
	PyErr_Clear();
	int argc = PyObject_Length(args);
	if ( argc == -1 )
		return NULL;
	if ( argc < 5 )
		return OleSetTypeError("not enough arguments (at least 5 needed)");

	DISPID dispid = PyInt_AsLong(PyTuple_GET_ITEM(args, 0)); // @pyparm int|dispid||The dispid to use.  Please see <om PyIDispatch.Invoke>.
	LCID lcid = PyInt_AsLong(PyTuple_GET_ITEM(args, 1)); // @pyparm int|lcid||The locale ID.  Please see <om PyIDispatch.Invoke>.
	UINT wFlags = PyInt_AsLong(PyTuple_GET_ITEM(args, 2)); // @pyparm int|wFlags||Flags for the call.  Please see <om PyIDispatch.Invoke>.
	PyObject *resultElemDesc = PyTuple_GET_ITEM(args, 3); // @pyparm tuple|resultTypeDesc||A tuple describing the type of the result.
	PyObject *argsElemDescArray = PyTuple_GET_ITEM(args, 4); // @pyparm  (tuple, ...)|typeDesc||A sequence of tuples describing the types of the parameters for the function.
	// @pyparm object, ...|args||The args to the function.
	if ( PyErr_Occurred() )
		return NULL;
	int numArgs;
	int argTypesLen = PyObject_Length(argsElemDescArray);
	if (!PyTuple_Check(argsElemDescArray) || argTypesLen<argc-5)
		return OleSetTypeError("The array of argument types must be a tuple whose size is <= to the number of arguments.");
	// See how many _real_ entries - count until end or
	// first param marked as Missing.
	for (numArgs = 0;numArgs<argc-5; numArgs++) {
		if (PyTuple_GET_ITEM(args, numArgs+5)->ob_type==&PyOleMissingType) {
			break;
		}
	}

	// these will all be cleared before returning
	PythonOleArgHelper *ArgHelpers = NULL;
	PyObject *result = NULL;
	DISPID dispidNamed = DISPID_PROPERTYPUT;
	DISPPARAMS dispparams = { NULL, NULL, 0, 0 };
	PythonOleArgHelper resultArgHelper;

	// This gets confusing.  If we have typeinfo for a byref arg, but the
	// arg is not specified by the user, then we _do_ present the arg to
	// COM.  If the arg does not exist, and it is not byref, then we do
	// not present it _at_all - ie, the arg count does _not_ include it.
	// So - first we must loop over the arg types, using this info to 
	// decide how big the arg array is!

	// If we have type info for an arg but not specified by the user, we will still process
	// the arg fully.
	// Note numArgs can not be > argTypesLen (as checked above)
	UINT numArgArray = 0;
	UINT i;

	if (argTypesLen>0) {
		ArgHelpers = new PythonOleArgHelper[argTypesLen]; // new may! except.
		if (ArgHelpers==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating ArgHelpers array");
			return NULL;
		}
		for ( i = 0; i < (UINT)argTypesLen; i++ ) {
			if (!ArgHelpers[i].ParseTypeInformation(PyTuple_GET_ITEM(argsElemDescArray,i)))
				goto error;
			// We ignore "in" params specified as "Missing", but
			// for byref (ie, "IsOut") args we still must process it.
			if (i<(UINT)numArgs || ArgHelpers[i].m_bIsOut)
				numArgArray++;
		}
	}

	dispparams.cArgs = numArgArray;
	if ( dispparams.cArgs ) {
		dispparams.rgvarg = new VARIANTARG[dispparams.cArgs];
		if (dispparams.rgvarg==NULL) {
			PyErr_SetString(PyExc_MemoryError, "Allocating dispparams.rgvarg array");
			goto error;
		}

		for ( i = dispparams.cArgs; i--; )
			VariantInit(&dispparams.rgvarg[i]);

		for ( i = 0; i < dispparams.cArgs; ++i ) {
			// args in reverse order.
			// arg-helpers in normal order.
			UINT offset = dispparams.cArgs - i - 1;
			// See if the user actually specified this arg.
			PyObject *arg = i>=(UINT)numArgs ? Py_None : PyTuple_GET_ITEM(args, i+5);
			if ( !ArgHelpers[i].MakeObjToVariant(arg, &dispparams.rgvarg[offset], PyTuple_GET_ITEM(argsElemDescArray,i)) )
				goto error;
		}
	}

	/* puts and putrefs need a named argument */
	if ( wFlags & (DISPATCH_PROPERTYPUT | DISPATCH_PROPERTYPUTREF) )
	{
		dispparams.rgdispidNamedArgs = &dispidNamed;
		dispparams.cNamedArgs = 1;
	}

	if (!resultArgHelper.ParseTypeInformation(resultElemDesc)) {
		PyCom_BuildInternalPyException("The return type information could not be parsed");
		goto error;
	}

	BOOL bResultWanted;
	bResultWanted = (resultArgHelper.m_reqdType != VT_VOID && resultArgHelper.m_reqdType != VT_EMPTY);

	VARIANT varResult;
	VARIANT *pVarResultUse;
	if ( bResultWanted ) {
		VariantInit(&varResult);
		pVarResultUse = &varResult;
	} else
		pVarResultUse = NULL;

	// initialize EXCEPINFO struct
	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);

	HRESULT hr;
	UINT nArgErr;
	IDispatch *pMyDispatch;

	pMyDispatch = GetI(self);
	if (pMyDispatch==NULL) goto error;
	nArgErr = (UINT)-1;  // initialize to invalid arg
	{
	PY_INTERFACE_PRECALL;
	hr = pMyDispatch->Invoke(dispid, IID_NULL, lcid, wFlags, &dispparams, pVarResultUse, &excepInfo, &nArgErr);
	PY_INTERFACE_POSTCALL;
	}

	if ( !HandledDispatchFailure(hr, &excepInfo, nArgErr, dispparams.cArgs) )
	{
		// Now get fancy with the args.  Any args specified as BYREF get returned
		// to Python.
		int retSize = 0;
		if (pVarResultUse)
			retSize++;
		for (UINT arg=0;arg<numArgArray;arg++)
			if (ArgHelpers[arg].m_bIsOut)
				retSize++;
		if (retSize==0) {  // result is None.
			result = Py_None;
			Py_INCREF(result);
		} else if (retSize==1) { // result is a simple object.
			if (pVarResultUse) { // only retval is actual result.
				result = resultArgHelper.MakeVariantToObj(pVarResultUse);
			} else { // only result in one of the params - seek it.
				for (UINT arg=0;arg<numArgArray;arg++) {
					if (ArgHelpers[arg].m_bIsOut) {
						result = ArgHelpers[arg].MakeVariantToObj(dispparams.rgvarg+(numArgArray-arg-1));
						break;
					}
				}
			}
		} else { // result is a tuple.
			result = PyTuple_New(retSize);
			int tupleItem = 0;
			if (pVarResultUse) {
				PyTuple_SetItem(result, tupleItem++, resultArgHelper.MakeVariantToObj(pVarResultUse));
			}
			// Loop over all the args, reverse order, setting the byrefs.
			for (int arg=numArgArray-1;arg>=0;arg--)
				if (ArgHelpers[numArgArray-arg-1].m_bIsOut)
					PyTuple_SetItem(result, tupleItem++, ArgHelpers[numArgArray-arg-1].MakeVariantToObj(dispparams.rgvarg+(arg)));
		}
	}
	if (pVarResultUse) VariantClear(pVarResultUse); // wipe the result.

  error:
	if ( dispparams.rgvarg )
	{
		for ( i = dispparams.cArgs; i--; )
			VariantClear(&dispparams.rgvarg[i]);
		delete [] dispparams.rgvarg;
	}
	delete [] ArgHelpers;
	return result;
}

// @pymethod <o PyITypeInfo>|PyIDispatch|GetTypeInfo|Get type information for the object.
PyObject *PyIDispatch::GetTypeInfo(PyObject *self, PyObject *args)
{
	LCID locale = LOCALE_USER_DEFAULT;
	// @pyparm int|locale|LOCALE_USER_DEFAULT|The locale to use.
	if (!PyArg_ParseTuple(args, "|i:GetTypeInfo", &locale))
		return NULL;

	IDispatch *pMyDispatch = GetI(self);
	if (pMyDispatch==NULL) return NULL;
	ITypeInfo *pti = NULL;
	PY_INTERFACE_PRECALL;
	SCODE sc = pMyDispatch->GetTypeInfo(0, locale, &pti);
	PY_INTERFACE_POSTCALL;
	if (sc!=S_OK) // S_OK is only acceptable result.
		return PyCom_BuildPyException(sc, pMyDispatch, IID_IDispatch);
	return PyCom_PyObjectFromIUnknown(pti, IID_ITypeInfo);
}

// @pymethod int|PyIDispatch|GetTypeInfoCount|Retrieves the number of <o PyITypeInfo>s the object provides.
PyObject *PyIDispatch::GetTypeInfoCount(PyObject *self, PyObject *args)
{
	if (!PyArg_ParseTuple(args,":GetTypeInfoCount"))
		return NULL;
	unsigned int ret;

	IDispatch *pMyDispatch = GetI(self);
	if (pMyDispatch==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatch->GetTypeInfoCount(&ret);
	PY_INTERFACE_POSTCALL;
	if ( FAILED(hr) )
		return PyCom_BuildPyException(hr, pMyDispatch, IID_IDispatch);
	return Py_BuildValue("i", ret);
}

// @object PyIDispatch|A OLE automation client object.
static struct PyMethodDef PyIDispatch_methods[] =
{
	{"Invoke",         PyIDispatch::Invoke,  1}, // @pymeth Invoke|Invokes a DISPID, using the passed arguments.
	{"InvokeTypes",    PyIDispatch::InvokeTypes,  1}, // @pymeth InvokeTypes|Invokes a DISPID, using the passed arguments and type descriptions.
	{"GetIDsOfNames",  PyIDispatch::GetIDsOfNames,  1}, // @pymeth GetIDsOfNames|Get the DISPID for the passed names.
	{"GetTypeInfo",    PyIDispatch::GetTypeInfo,  1}, // @pymeth GetTypeInfo|Get type information for the object.
	{"GetTypeInfoCount",PyIDispatch::GetTypeInfoCount,  1}, // @pymeth GetTypeInfoCount|Retrieves the number of <o PyITypeInfo>s the object provides.
	{NULL,  NULL}        
};

PyComTypeObject PyIDispatch::type("PyIDispatch",
                 &PyIUnknown::type, // @base PyIDispatch|PyIUnknown
                 sizeof(PyIDispatch),
                 PyIDispatch_methods,
				 GET_PYCOM_CTOR(PyIDispatch));

#ifndef NO_PYCOM_IDISPATCHEX

//////////////////////////////////////////////////////////////////
//
// PyIDispatchEx

PyIDispatchEx::PyIDispatchEx(IUnknown *pDisp) :
	PyIDispatch(pDisp)
{
	ob_type = &type;
}

PyIDispatchEx::~PyIDispatchEx()
{
}

/*static*/ IDispatchEx *PyIDispatchEx::GetI(PyObject *self)
{
	return (IDispatchEx *)PyIDispatch::GetI(self);
}

// @method int|PyIDispatchEx|GetDispID|
PyObject *PyIDispatchEx::GetDispID(PyObject *self, PyObject *args)
{
	long fdex;
	PyObject *obName;
	if (!PyArg_ParseTuple(args, "Ol:GetDispId",
		&obName, // @pyparm <o PyUnicode>|name||Passed in name to be mapped
		&fdex)) // @pyparm int|fdex||Determines the options for obtaining the member identifier. This can be a combination of the fdex* constants:
		return NULL;
	PyWin_AutoFreeBstr name;
	if (!PyWinObject_AsAutoFreeBstr(obName, &name))
		return NULL;
	IDispatchEx *pMyDispatchEx = GetI(self);
	if (pMyDispatchEx==NULL) return NULL;
	DISPID dispid;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatchEx->GetDispID(name, (DWORD)fdex, &dispid);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return SetPythonCOMError(self, hr);
	return PyInt_FromLong(dispid);
}

// @method object|PyIDispatchEx|InvokeEx|Provides access to properties and methods exposed by a <o PyIDispatchEx> object. 
PyObject *PyIDispatchEx::InvokeEx(PyObject *self, PyObject *args)
{
	long dispid;
	long lcid;
	int flags;
	PyObject *invokeArgs;
	PyObject *types = Py_None;
	PyObject *obReturnDesc = Py_None;
	PyObject *obCaller = Py_None;
	if (!PyArg_ParseTuple(args, "lliO|OOO:InvokeEx",
		&dispid, // @pyparm int|dispid||
		&lcid,  // @pyparm int|lcid||
		&flags, // @pyparm int|flags||
		&invokeArgs,  // @pyparm [object, ...]|args||The arguments.
		&types, // @pyparm [object, ...]|types|None|A tuple of type description object, or None if type descriptions are not available.
		&obReturnDesc, // @pyparm object\|int|returnDesc|1|If types==None, should be a BOOL indicating if the result is needed.  If types is a tuple, then should a be type description.
		&obCaller)) // @pyparm <o PyIServiceProvider>|serviceProvider|None|A service provider object supplied by the caller which allows the object to obtain services from the caller. Can be None.
		return NULL;

	if (!PyTuple_Check(invokeArgs)) {
		PyErr_SetString(PyExc_TypeError, "The arguments must be a tuple.");
		return NULL;
	}

	// TODO - We do not yet support the Type Description here
	// (Im not even sure if we need it!)
	if (types != Py_None || obReturnDesc != Py_None) {
		PyErr_SetString(PyExc_TypeError, "Type descriptions are not yet supported.");
		return NULL;
	}
	// TODO - Add support for PyIServiceProvider
	if (obCaller != Py_None) {
		PyErr_SetString(PyExc_TypeError, "If you really need IServiceProvider support, you are going to have to add it!.");
		return NULL;
	}
	BOOL bResultWanted = TRUE;

	IDispatchEx *pMyDispatch = GetI(self);
	if ( pMyDispatch==NULL )
		return NULL;

	DISPPARAMS dispparams;
	if (!PyCom_MakeUntypedDISPPARAMS(invokeArgs, PyObject_Length(invokeArgs), flags, &dispparams ))
		return NULL;

	VARIANT varResult;
	VARIANT *pVarResultUse;
	if ( bResultWanted ) {
		VariantInit(&varResult);
		pVarResultUse = &varResult;
	} else
		pVarResultUse = NULL;

	// initialize EXCEPINFO struct
	EXCEPINFO excepInfo;
	memset(&excepInfo, 0, sizeof excepInfo);

	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatch->InvokeEx((DISPID)dispid, (LCID)lcid, (WORD)flags, &dispparams, pVarResultUse, &excepInfo, NULL);
	PY_INTERFACE_POSTCALL;

	PyCom_FreeUntypedDISPPARAMS(&dispparams);

	if ( HandledDispatchFailure(hr, &excepInfo, (UINT)-1, dispparams.cArgs) )
	{
		if ( pVarResultUse )
			VariantClear(pVarResultUse);
		return NULL;
	}

	PyObject *result;
	if (pVarResultUse) {
		result = PyCom_PyObjectFromVariant(pVarResultUse);
		VariantClear(pVarResultUse);
	} else {
		result = Py_None;
		Py_INCREF(result);
	}
	return result;
}
// @method |PyIDispatchEx|DeleteMemberByName|
PyObject *PyIDispatchEx::DeleteMemberByName(PyObject *self, PyObject *args)
{
	long fdex;
	PyObject *obName;
	if (!PyArg_ParseTuple(args, "Ol:DeleteMemberByName",
		&obName, // @pyparm <o PyUnicode>|name||Passed in name to be mapped
		&fdex)) // @pyparm int|fdex||Determines the options 
		return NULL;
	PyWin_AutoFreeBstr name;
	if (!PyWinObject_AsAutoFreeBstr(obName, &name))
		return NULL;
	IDispatchEx *pMyDispatchEx = GetI(self);
	if (pMyDispatchEx==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatchEx->DeleteMemberByName(name, (DWORD)fdex);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return SetPythonCOMError(self, hr);
	Py_INCREF(Py_None);
	return Py_None;
}
// @method |PyIDispatchEx|DeleteMemberByDispID|
PyObject *PyIDispatchEx::DeleteMemberByDispID(PyObject *self, PyObject *args)
{
	long dispid;
	if (!PyArg_ParseTuple(args, "ll:DeleteMemberByDispID",
		&dispid)) // @pyparm int|dispid||
		return NULL;
	IDispatchEx *pMyDispatchEx = GetI(self);
	if (pMyDispatchEx==NULL) return NULL;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatchEx->DeleteMemberByDispID((DISPID)dispid);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return SetPythonCOMError(self, hr);
	Py_INCREF(Py_None);
	return Py_None;
}
// @method int|PyIDispatchEx|GetMemberProperties|
PyObject *PyIDispatchEx::GetMemberProperties(PyObject *self, PyObject *args)
{
	long fdex;
	long dispid;
	if (!PyArg_ParseTuple(args, "ll:GetMemberProperties",
		&dispid, // @pyparm int|dispid||
		&fdex)) // @pyparm int|fdex||Determines the options
		return NULL;
	IDispatchEx *pMyDispatchEx = GetI(self);
	if (pMyDispatchEx==NULL) return NULL;
	DWORD props;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatchEx->GetMemberProperties((DISPID)dispid, (DWORD)fdex, &props);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return SetPythonCOMError(self, hr);
	return PyInt_FromLong(props);
}
// @method int|PyIDispatchEx|GetMemberName|
PyObject *PyIDispatchEx::GetMemberName(PyObject *self, PyObject *args)
{
	long fdex;
	long dispid;
	if (!PyArg_ParseTuple(args, "ll:GetMemberName",
		&dispid, // @pyparm int|dispid||
		&fdex)) // @pyparm int|fdex||Determines the options
		return NULL;
	IDispatchEx *pMyDispatchEx = GetI(self);
	if (pMyDispatchEx==NULL) return NULL;
	DWORD props;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatchEx->GetMemberProperties((DISPID)dispid, (DWORD)fdex, &props);
	PY_INTERFACE_POSTCALL;
	if (FAILED(hr)) return SetPythonCOMError(self, hr);
	return PyInt_FromLong(props);
}

// @method int|PyIDispatchEx|GetNextDispID|
PyObject *PyIDispatchEx::GetNextDispID(PyObject *self, PyObject *args)
{
	long fdex;
	long dispid;
	if (!PyArg_ParseTuple(args, "ll:GetNextDispID",
		&fdex, // @pyparm int|fdex||Determines the options
		&dispid)) // @pyparm int|dispid||Identifies the current member. GetNextDispID will retrieve the item in the enumeration after this one. 
		return NULL;
	IDispatchEx *pMyDispatchEx = GetI(self);
	if (pMyDispatchEx==NULL) return NULL;
	DISPID retDispid;
	PY_INTERFACE_PRECALL;
	HRESULT hr = pMyDispatchEx->GetNextDispID((DWORD)fdex, (DISPID)dispid, &retDispid);
	PY_INTERFACE_POSTCALL;
	if (hr != S_OK) return SetPythonCOMError(self, hr);
	return PyInt_FromLong(retDispid);
}

// @object PyIDispatchEx|A OLE automation client object that uses the IDispatchEx scripting interface..
static struct PyMethodDef PyIDispatchEx_methods[] =
{
	{"GetDispID",           PyIDispatchEx::GetDispID, 1 }, // @pymeth GetDispID|
	{"InvokeEx",            PyIDispatchEx::InvokeEx, 1 }, // @pymeth InvokeEx|Provides access to properties and methods exposed by a <o PyIDispatchEx> object. 
	{"DeleteMemberByName",  PyIDispatchEx::DeleteMemberByName, 1 }, // @pymeth DeleteMemberByName|
	{"DeleteMemberByDispID",PyIDispatchEx::DeleteMemberByDispID, 1 }, // @pymeth DeleteMemberByDispID|
	{"GetMemberProperties", PyIDispatchEx::GetMemberProperties, 1 }, // @pymeth GetMemberProperties|
	{"GetMemberName",       PyIDispatchEx::GetMemberName, 1 }, // @pymeth GetMemberName|
	{"GetNextDispID",       PyIDispatchEx::GetNextDispID, 1 }, // @pymeth GetNextDispID|
	{NULL,  NULL}        
};

PyComTypeObject PyIDispatchEx::type("PyIDispatchEx",
                 &PyIDispatch::type, // @base PyIDispatchEx|PyIDispatch
                 sizeof(PyIDispatchEx),
                 PyIDispatchEx_methods,
				 GET_PYCOM_CTOR(PyIDispatchEx));

#endif // NO_PYCOM_IDISPATCHEX
