#ifndef MIGI_PY_PYTHON_API_H_
#define MIGI_PY_PYTHON_API_H_

#include <string>
#include <memory>

#include "api.h"
#include "forwarding.h"

#include "py/forwarding.h"

namespace migi {
namespace py {

void PyInitialize();
void PyFinalize();

std::string PyEval(const std::string& expr, Console& console);
void PyExec(const std::string&, Console& console);
void PyRunScript(const std::string& filename, Console& console);

std::string PyRepr(PyObject* object);
std::string PyStr(PyObject* object);
std::string PyUnicodeAsUTF8(PyObject* object);


Object PyImportModule(const std::string& name);

class GILScopedAquire {
public:
    GILScopedAquire();
    ~GILScopedAquire();

private:
    PyGILState_STATE _gil_state;
};

class GILScopedRelease {
public:
    GILScopedRelease();
    ~GILScopedRelease();

private:
    PyThreadState *_thread_state;
};


}
}

#endif
