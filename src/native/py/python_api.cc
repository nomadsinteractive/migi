#include "py/python_api.h"

#include <codecvt>
#include <fstream>
#include <sstream>

#include "migi.h"
#include "intf/console.h"
#include "py/object.h"


namespace migi {
namespace py {

void PyInitialize()
{
    Py_SetProgramName(L"migi");
    Py_InitializeEx(0);
}

void PyFinalize()
{
    if(PyErr_Occurred())
        PyErr_Clear();
    Py_Finalize();
}

class PythonOutputRedirector {
public:
    PythonOutputRedirector(Console& console, bool redirect = true);
    ~PythonOutputRedirector();

    std::string stdoutString();
    std::string stderrString();

private:
    Console& _console;

    bool _redirect;
    Object _stdout;
    Object _stderr;
    Object _stdout_buffer;
    Object _stderr_buffer;
};

static Object PyEvalCode(const std::string& source, const std::string& filename, int32_t start)
{
    Object codeObject = Object::adopt(Py_CompileString(source.c_str(), filename.c_str(), start));
    if(codeObject)
    {
        PyObject* m = PyImport_AddModule("__main__");
        PyObject* globals = m ? PyModule_GetDict(m) : nullptr;
        return Object::adopt(globals ? PyEval_EvalCode(codeObject.object(), globals, globals) : nullptr);
    }
    return Object();
}

std::string PyEval(const std::string& expr, Console& console)
{
    PythonOutputRedirector errCatcher(console, false);
    Object result = PyEvalCode(expr, "__main__", Py_eval_input);
    return result.isNullPtr() ? "<Eval Error>" : PyRepr(result.object());
}

void PyExec(const std::string& expr, Console& console)
{
    PythonOutputRedirector errCatcher(console, true);
    PyEvalCode(expr, "__main__", Py_single_input);
}

std::string PyRepr(PyObject* object)
{
    Object repr = Object::adopt(PyObject_Repr(object));
    std::string expr = PyStr(repr.object());
    return expr;
}

std::string PyStr(PyObject* object)
{
    if (PyBytes_Check(object))
    {
        char* str;
        Py_ssize_t size;
        PyBytes_AsStringAndSize(object, &str, &size);
        return std::string(str, static_cast<size_t>(size));
    }

    if(PyUnicode_Check(object))
        return PyUnicodeAsUTF8(object);

    Object strObj = Object::adopt(PyObject_Str(object));
    return PyUnicodeAsUTF8(strObj.object());
}

std::string PyUnicodeAsUTF8(PyObject* object)
{
    Py_ssize_t size;
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    wchar_t* wstr = PyUnicode_AsWideCharString(object, &size);
    std::wstring wstring(wstr, static_cast<size_t>(size));
    PyMem_Free(wstr);
    return myconv.to_bytes(wstring);
}

PythonOutputRedirector::PythonOutputRedirector(Console& console, bool redirect)
    : _console(console), _redirect(redirect) {
    const Object sysm = PyImportModule("sys");
    _stdout = sysm.attr("stdout");
    _stderr = sysm.attr("stderr");
    const Object stringio = PyImportModule("io").attr("StringIO");
    _stdout_buffer = stringio();
    _stderr_buffer = stringio();
    sysm.setattr("stdout", _stdout_buffer);
    sysm.setattr("stderr", _stderr_buffer);
}

std::string PythonOutputRedirector::stdoutString() {
    _stdout_buffer.attr("seek")(0);
    return _stdout_buffer.attr("read")().str();
}

std::string PythonOutputRedirector::stderrString() {
    _stderr_buffer.attr("seek")(0);
    return _stderr_buffer.attr("read")().str();
}

PythonOutputRedirector::~PythonOutputRedirector() {
    if(PyErr_Occurred()) {
        PyErr_Print();
        PyErr_Clear();
    }

    if(_redirect) {
        _console.write(stdoutString());
        _console.write(stderrString());
    }

    const Object sysm = PyImportModule("sys");
    sysm.setattr("stdout", _stdout);
    sysm.setattr("stderr", _stderr);
}

void PyRunScript(const std::string& filename, Console& console)
{
    std::ifstream fp(filename, std::ios_base::in);
    std::ostringstream sb;
    std::string line;

    DCHECK(fp.good(), "Script file \"%s\" not exists", filename.c_str());

    while(std::getline(fp, line))
        sb << line << std::endl;

    PythonOutputRedirector errCatcher(console, true);
    PyEvalCode(sb.str(), filename, Py_file_input);
}

GILScopedAquire::GILScopedAquire()
{
    _gil_state = PyGILState_Ensure();
}

GILScopedAquire::~GILScopedAquire()
{
    PyGILState_Release(_gil_state);
}

GILScopedRelease::GILScopedRelease()
{
    _thread_state = PyEval_SaveThread();
}

GILScopedRelease::~GILScopedRelease()
{
    PyEval_RestoreThread(_thread_state);
}

Object PyImportModule(const std::string& name)
{
    return Object::adopt(PyImport_ImportModule(name.c_str()));
}

}
}
