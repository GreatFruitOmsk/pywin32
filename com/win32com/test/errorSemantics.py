# errorSemantics.py

# Test the Python error handling semantics.  Specifically:
#
# * When a Python COM object is called via IDispatch, the nominated
#   scode is placed in the exception tuple, and the HRESULT is
#   DISP_E_EXCEPTION
# * When the same interface is called via IWhatever, the
#   nominated  scode is returned directly (with the scode also
#   reflected in the exception tuple)
# * In all cases, the description etc end up in the exception tuple
# * "Normal" Python exceptions resolve to an E_FAIL "internal error"

import pythoncom
from win32com.server.exception import COMException
from win32com.server.util import wrap
from win32com.client import Dispatch
import winerror
from win32com.test.util import CaptureWriter

class error(Exception):
    def __init__(self, msg, com_exception=None):
        Exception.__init__(self, msg, str(com_exception))

# Our COM server.
class TestServer:
    _public_methods_ = [ 'Clone', 'Commit', 'LockRegion', 'Read']
    _com_interfaces_ = [ pythoncom.IID_IStream ]

    def Clone(self):
        raise COMException("Not today", scode=winerror.E_UNEXPECTED)

    def Commit(self, flags):
        raise "foo"

def test():
    # Call via a native interface.
    com_server = wrap(TestServer(), pythoncom.IID_IStream)
    try:
        com_server.Clone()
    except pythoncom.com_error, com_exc:
        hr, desc, exc, argErr = com_exc
        if hr != winerror.E_UNEXPECTED:
            raise error("Calling the object natively did not yield the correct scode", com_exc)
        if not exc or exc[-1] != winerror.E_UNEXPECTED:
            raise error("The scode element of the exception tuple did not yield the correct scode", com_exc)
        if exc[2] != "Not today":
            raise error("The description in the exception tuple did not yield the correct string", com_exc)
    cap = CaptureWriter()
    try:
        cap.capture()
        try:
            com_server.Commit(0)
        finally:
            cap.release()
    except pythoncom.com_error, com_exc:
        hr, desc, exc, argErr = com_exc
        if hr != winerror.E_FAIL:
            raise error("The hresult was not E_FAIL for an internal error", com_exc)
        if exc[1] != "Python COM Server Internal Error":
            raise error("The description in the exception tuple did not yield the correct string", com_exc)
    # Check we saw a traceback in stderr
    if cap.get_captured().find("Traceback")<0:
        raise error("Could not find a traceback in stderr: %r" % (cap.get_captured(),))

    # Now do it all again, but using IDispatch
    com_server = Dispatch(wrap(TestServer()))
    try:
        com_server.Clone()
    except pythoncom.com_error, com_exc:
        hr, desc, exc, argErr = com_exc
        if hr != winerror.DISP_E_EXCEPTION:
            raise error("Calling the object via IDispatch did not yield the correct scode", com_exc)
        if not exc or exc[-1] != winerror.E_UNEXPECTED:
            raise error("The scode element of the exception tuple did not yield the correct scode", com_exc)
        if exc[2] != "Not today":
            raise error("The description in the exception tuple did not yield the correct string", com_exc)

    cap.clear()
    try:
        cap.capture()
        try:
            com_server.Commit(0)
        finally:
            cap.release()
    except pythoncom.com_error, com_exc:
        hr, desc, exc, argErr = com_exc
        if hr != winerror.DISP_E_EXCEPTION:
            raise error("Calling the object via IDispatch did not yield the correct scode", com_exc)
        if not exc or exc[-1] != winerror.E_FAIL:
            raise error("The scode element of the exception tuple did not yield the correct scode", com_exc)
        if exc[1] != "Python COM Server Internal Error":
            raise error("The description in the exception tuple did not yield the correct string", com_exc)
    # Check we saw a traceback in stderr
    if cap.get_captured().find("Traceback")<0:
        raise error("Could not find a traceback in stderr: %r" % (cap.get_captured(),))

if __name__=='__main__':
    test()
    from util import CheckClean
    CheckClean()
    print "error semantic tests worked"
