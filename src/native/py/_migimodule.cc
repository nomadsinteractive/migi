#include "py/object.h"
#include "api.h"

#include "intf/console.h"
#include "intf/injector.h"
#include "interceptor.h"
#include "platform/platform.h"

#include "py/object.h"

namespace migi {


template<typename T> class Singleton {
public:

    static T& instance() {
        static T SINGLETON_INSTANCE;
        return SINGLETON_INSTANCE;
    }

};


typedef struct _typeobject {
    PyObject_VAR_HEAD
    const char *tp_name; /* For printing, in format "<module>.<name>" */
    Py_ssize_t tp_basicsize, tp_itemsize; /* For allocation */

    /* Methods to implement standard operations */

    destructor  tp_dealloc;
    nullptr_t   tp_print;
    getattrfunc tp_getattr;
    setattrfunc tp_setattr;
    nullptr_t   tp_as_async; /* formerly known as tp_compare (Python 2)
                                    or tp_reserved (Python 3) */
    reprfunc tp_repr;

    /* Method suites for standard classes */

    nullptr_t tp_as_number;
    nullptr_t tp_as_sequence;
    nullptr_t tp_as_mapping;

    /* More standard operations (here for binary compatibility) */

    hashfunc tp_hash;
    ternaryfunc tp_call;
    reprfunc tp_str;
    getattrofunc tp_getattro;
    setattrofunc tp_setattro;

    /* Functions to access object as input/output buffer */
    nullptr_t  tp_as_buffer;

    /* Flags to define presence of optional/expanded features */
    unsigned long tp_flags;

    const char *tp_doc; /* Documentation string */

    /* Assigned meaning in release 2.0 */
    /* call function for all accessible objects */
    traverseproc tp_traverse;

    /* delete references to contained objects */
    inquiry tp_clear;

    /* Assigned meaning in release 2.1 */
    /* rich comparisons */
    richcmpfunc tp_richcompare;

    /* weak reference enabler */
    Py_ssize_t tp_weaklistoffset;

    /* Iterators */
    getiterfunc tp_iter;
    iternextfunc tp_iternext;

    /* Attribute descriptor and subclassing stuff */
    struct PyMethodDef *tp_methods;
    struct PyMemberDef *tp_members;
    struct PyGetSetDef *tp_getset;
    struct _typeobject *tp_base;
    PyObject *tp_dict;
    descrgetfunc tp_descr_get;
    descrsetfunc tp_descr_set;
    Py_ssize_t tp_dictoffset;
    initproc tp_init;
    allocfunc tp_alloc;
    newfunc tp_new;
    freefunc tp_free; /* Low-level free-memory routine */
    inquiry tp_is_gc; /* For PyObject_IS_GC */
    PyObject *tp_bases;
    PyObject *tp_mro; /* method resolution order */
    PyObject *tp_cache;
    PyObject *tp_subclasses;
    PyObject *tp_weaklist;
    destructor tp_del;

    /* Type attribute cache version tag. Added in version 2.6 */
    unsigned int tp_version_tag;

    destructor tp_finalize;

    char tp_reserved[256];
} _PyTypeObject;


template<typename T> class PyType {
public:
    struct Instance {
        PyObject_HEAD
        T* instance_ptr;

        T& instance() {
            return *instance_ptr;
        }

        const T& instance() const {
            return *instance_ptr;
        }
    };

private:
    static _PyTypeObject* basetype() {
        static PyMethodDef PyType_methods[] = {
            {nullptr, nullptr, 0, nullptr}
        };

        static _PyTypeObject ark_basetype = {
            PyVarObject_HEAD_INIT(nullptr, 0)
            "_migi.Type",
            sizeof(Instance),
            0,
            reinterpret_cast<destructor>(__dealloc__),          /* tp_dealloc */
            nullptr,                                            /* tp_print */
            nullptr,                                            /* tp_getattr */
            nullptr,                                            /* tp_setattr */
            nullptr,                                            /* tp_reserved */
            nullptr,                                            /* tp_repr */
            nullptr,                                            /* tp_as_number */
            nullptr,                                            /* tp_as_sequence */
            nullptr,                                            /* tp_as_mapping */
            reinterpret_cast<hashfunc>(__hash__),               /* tp_hash */
            nullptr,                                            /* tp_call */
            nullptr,                                            /* tp_str */
            nullptr,                                            /* tp_getattro */
            nullptr,                                            /* tp_setattro */
            nullptr,                                            /* tp_as_buffer */
            Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,           /* tp_flags */
            "migi native type object",                          /* tp_doc */
            nullptr,                                            /* tp_traverse */
            nullptr,                                            /* tp_clear */
            nullptr,                                            /* tp_richcompare */
            0,                                                  /* tp_weaklistoffset */
            nullptr,                                            /* tp_iter */
            nullptr,                                            /* tp_iternext */
            PyType_methods,                                     /* tp_methods */
            nullptr,                                            /* tp_members */
            nullptr,                                            /* tp_getset */
            nullptr,                                            /* tp_base */
            nullptr,                                            /* tp_dict */
            nullptr,                                            /* tp_descr_get */
            nullptr,                                            /* tp_descr_set */
            0,                                                  /* tp_dictoffset */
            reinterpret_cast<initproc>(__init__),               /* tp_init */
            nullptr,                                            /* tp_alloc */
            PyType_GenericNew                                   /* tp_new */
        };

        return &ark_basetype;
    }

    static int32_t __init__(Instance* /*self*/, PyObject* /*args*/, PyObject* /*kwargs*/) {
        return 0;
    }

    static void __dealloc__(Instance* self) {
        delete self->instance_ptr;
        self->instance_ptr = nullptr;
        reinterpret_cast<_PyTypeObject*>(Py_TYPE(self))->tp_free(self);
    }

    static Py_hash_t __hash__(Instance* self) {
        return reinterpret_cast<Py_hash_t>(self);
    }

public:
    PyType(std::string name, std::string doc, _PyTypeObject* base, unsigned long flags)
        : _name(std::move(name)), _doc(std::move(doc)) {
        _PyTypeObject pyType = {
            PyVarObject_HEAD_INIT(NULL, 0)
            _name.c_str(),          /*tp_name*/
            sizeof(Instance)        /*tp_basicsize*/
        };

        memset(&pyType.tp_itemsize, 0, sizeof(_PyTypeObject) - offsetof(_PyTypeObject, tp_itemsize));
        pyType.tp_flags = flags;
        pyType.tp_doc = _doc.c_str();
        pyType.tp_base = base ? base : basetype();

        _py_type_object = pyType;
        Py_INCREF(&_py_type_object);
    }

    PyObject* create(T instance) {
        Instance* self = reinterpret_cast<Instance*>(_py_type_object.tp_new(reinterpret_cast<PyTypeObject*>(&_py_type_object), nullptr, nullptr));
        self->instance_ptr = new T(std::move(instance));
        return reinterpret_cast<PyObject*>(self);
    }

    void addToModule(PyObject* module, const char* name) {
        PyType_Ready(reinterpret_cast<PyTypeObject*>(&_py_type_object));
        PyModule_AddObject(module, name, reinterpret_cast<PyObject*>(&_py_type_object));
    }

protected:
    std::string _name;
    std::string _doc;
    _PyTypeObject _py_type_object;
};

class PyConsoleType : public PyType<std::unique_ptr<Console>> {
public:
    PyConsoleType()
        : PyType("_migi.Console", "_migi.Console Type", nullptr, Py_TPFLAGS_DEFAULT) {
        static PyMethodDef PyConsoleType_methods[] = {
            {"show", reinterpret_cast<PyCFunction>(PyConsoleType::show), METH_VARARGS, nullptr},
            {"close", reinterpret_cast<PyCFunction>(PyConsoleType::close), METH_VARARGS, nullptr},
            {"read_line", reinterpret_cast<PyCFunction>(PyConsoleType::read_line), METH_VARARGS, nullptr},
            {"write", reinterpret_cast<PyCFunction>(PyConsoleType::write), METH_VARARGS, nullptr},
            {nullptr}
        };
        _py_type_object.tp_methods = PyConsoleType_methods;
    }

private:
    static PyObject* show(Instance* self, PyObject* /*args*/) {
        self->instance()->show();
        Py_RETURN_NONE;
    }

    static PyObject* close(Instance* self, PyObject* /*args*/) {
        self->instance()->close();
        Py_RETURN_NONE;
    }

    static PyObject* read_line(Instance* self, PyObject* /*args*/) {
        std::string line;
        {
            const py::GILScopedRelease release;
            line = self->instance()->readLine();
        }
        return py::Object::toPyObject(line);
    }

    static PyObject* write(Instance* self, PyObject* args) {
        const char* arg0;
        if(!PyArg_ParseTuple(args, "s", &arg0))
            Py_RETURN_NONE;
        {
            const py::GILScopedRelease release;
            self->instance()->write(arg0);
        }
        Py_RETURN_NONE;
    }

};

class PyInjectorType : public PyType<std::unique_ptr<Injector>> {
public:
    PyInjectorType()
        : PyType("_migi.Injector", "_migi.Injector Type", nullptr, Py_TPFLAGS_DEFAULT) {
        static PyMethodDef PyInjectorType_methods[] = {
            {"alloc_memory", reinterpret_cast<PyCFunction>(PyInjectorType::alloc_memory), METH_VARARGS, nullptr},
            {"write_memory", reinterpret_cast<PyCFunction>(PyInjectorType::write_memory), METH_VARARGS, nullptr},
            {"free_memory", reinterpret_cast<PyCFunction>(PyInjectorType::free_memory), METH_VARARGS, nullptr},
            {"load_library", reinterpret_cast<PyCFunction>(PyInjectorType::load_library), METH_VARARGS, nullptr},
            {nullptr, nullptr, 0, nullptr}
        };
        _py_type_object.tp_methods = PyInjectorType_methods;
    }

private:
    static PyObject* alloc_memory(Instance* self, PyObject* args) {
        Py_ssize_t arg0;
        if(!PyArg_ParseTuple(args, "n", &arg0))
            Py_RETURN_NONE;
        return py::Object::toPyObject(self->instance()->allocMemory(static_cast<size_t>(arg0)));
    }

    static PyObject* write_memory(Instance* self, PyObject* args) {
        Py_ssize_t arg0;
        PyObject* arg1;
        if(!PyArg_ParseTuple(args, "nO", &arg0, &arg1))
            Py_RETURN_NONE;
        const std::string content = py::Object::toCppObject<std::string>(arg1);
        return py::Object::toPyObject(self->instance()->writeMemory(static_cast<uintptr_t>(arg0), content.c_str(), content.size()));
    }

    static PyObject* free_memory(Instance* self, PyObject* args) {
        Py_ssize_t arg0;
        if(!PyArg_ParseTuple(args, "n", &arg0))
            Py_RETURN_NONE;
        self->instance()->freeMemory(static_cast<uintptr_t>(arg0));
        Py_RETURN_NONE;
    }

    static PyObject* load_library(Instance* self, PyObject* args) {
        const char* arg0;
        Py_ssize_t arg1;
        if(!PyArg_ParseTuple(args, "sn", &arg0, &arg1))
            Py_RETURN_NONE;
        return py::Object::toPyObject(self->instance()->loadLibrary(std::filesystem::path(arg0), static_cast<uintptr_t>(arg1)));
    }

};

class PyDeviceType : public PyType<std::unique_ptr<Device>> {
public:
    PyDeviceType()
        : PyType("_migi.Device", "_migi.Device Type", nullptr, Py_TPFLAGS_DEFAULT) {
        static PyMethodDef PyDeviceType_methods[] = {
            {"create_console", reinterpret_cast<PyCFunction>(PyDeviceType::create_console), METH_VARARGS, nullptr},
            {"create_injector", reinterpret_cast<PyCFunction>(PyDeviceType::create_injector), METH_VARARGS, nullptr},
            {"get_process_architecture", reinterpret_cast<PyCFunction>(PyDeviceType::get_process_architecture), METH_VARARGS, nullptr},
            {"find_process_by_name", reinterpret_cast<PyCFunction>(PyDeviceType::find_process_by_name), METH_VARARGS, nullptr},
            {nullptr, nullptr, 0, nullptr}
        };
        _py_type_object.tp_methods = PyDeviceType_methods;
    }

private:
    static PyObject* create_console(Instance* self, PyObject* /*args*/) {
        return Singleton<PyConsoleType>::instance().create(std::unique_ptr<Console>(self->instance()->createConsole()));
    }

    static PyObject* create_injector(Instance* self, PyObject* args) {
        uint32_t arg0;
        if(!PyArg_ParseTuple(args, "I", &arg0))
            Py_RETURN_NONE;
        return Singleton<PyInjectorType>::instance().create(std::unique_ptr<Injector>(self->instance()->createInjector(arg0)));
    }

    static PyObject* get_process_architecture(Instance* self, PyObject* args) {
        uint32_t arg0;
        if(!PyArg_ParseTuple(args, "I", &arg0))
            Py_RETURN_NONE;
        return py::Object::toPyObject(self->instance()->getProcessArchitecture(arg0));
    }

    static PyObject* find_process_by_name(Instance* self, PyObject* args) {
        const char* arg0;
        if(!PyArg_ParseTuple(args, "s", &arg0))
            Py_RETURN_NONE;
        return py::Object::toPyObject(self->instance()->findProcessByName(arg0));
    }

};

class PyInterceptorType : public PyType<std::shared_ptr<Interceptor>> {
public:
    PyInterceptorType()
        : PyType("_migi.Interceptor", "_migi.Interceptor Type", nullptr, Py_TPFLAGS_DEFAULT) {
        static PyMethodDef PyInterceptorType_methods[] = {
            {"intercept", reinterpret_cast<PyCFunction>(PyInterceptorType::intercept), METH_VARARGS, nullptr},
            {"restore", reinterpret_cast<PyCFunction>(PyInterceptorType::restore), METH_VARARGS, nullptr},
            {nullptr, nullptr, 0, nullptr}
        };
        static PyGetSetDef PyInterceptorType_getseters[] = {
            {"function_entry", reinterpret_cast<getter>(PyInterceptorType::function_entry), nullptr, "function_entry", nullptr},
            {nullptr, nullptr, nullptr, nullptr, nullptr}
        };
        _py_type_object.tp_methods = PyInterceptorType_methods;
        _py_type_object.tp_getset = PyInterceptorType_getseters;
    }

private:
    static PyObject* intercept(Instance* self, PyObject* /*args*/) {
        return py::Object::toPyObject(self->instance()->intercept());
    }

    static PyObject* restore(Instance* self, PyObject* /*args*/) {
        return py::Object::toPyObject(self->instance()->restore());
    }

    static PyObject* function_entry(Instance* self, PyObject* /*args*/) {
        return py::Object::toPyObject(reinterpret_cast<uintptr_t>(self->instance()->functionEntry()));
    }
};

static PyObject* migi_is_mocked(PyObject* /*self*/, PyObject* /*args*/)
{
    if(is_mocked())
        Py_RETURN_TRUE;
    Py_RETURN_FALSE;
}

static PyObject* migi_sizeof_void_p(PyObject* /*self*/, PyObject* /*args*/)
{
    return PyLong_FromUnsignedLong(sizeof_void_p());
}

static PyObject* migi_sleep_for(PyObject* /*self*/, PyObject* args)
{
    float arg0 = 0;
    if(!PyArg_ParseTuple(args, "f", &arg0))
        Py_RETURN_NONE;
    sleep_for(arg0);
    Py_RETURN_NONE;
}

static PyObject* migi_create_device(PyObject* /*self*/, PyObject* args)
{
    int32_t arg0 = 0;
    if(!PyArg_ParseTuple(args, "i", &arg0))
        Py_RETURN_NONE;
    return Singleton<PyDeviceType>::instance().create(std::unique_ptr<Device>(Platform::createDevice(static_cast<Device::DeviceType>(arg0))));
}

static PyObject* migi_dump_bytes(PyObject* /*self*/, PyObject* args)
{
    PyObject* arg0;
    if(!PyArg_ParseTuple(args, "O", &arg0))
        Py_RETURN_NONE;
    return py::Object::toPyObject(dump_bytes(py::PyStr(arg0)));
}

static PyObject* migi_load_library(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    uint32_t arg1;
    if(!PyArg_ParseTuple(args, "OI", &arg0, &arg1))
        Py_RETURN_NONE;
    return py::Object::toPyObject(load_library(arg0, arg1));
}

static PyObject* migi_free_library(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    if(!PyArg_ParseTuple(args, "n", &arg0))
        Py_RETURN_NONE;
    free_library(arg0);
    Py_RETURN_NONE;
}

static PyObject* migi_make_call(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    PyObject* arg1;
    uint32_t arg2 = 0;
    if(!PyArg_ParseTuple(args, "nO|I", &arg0, &arg1, &arg2))
        Py_RETURN_NONE;
    return py::Object::toPyObject(make_call(arg0, py::Object::toCppObject<std::vector<uintptr_t>>(arg1), arg2));
}

static PyObject* migi_make_thiscall(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    uintptr_t arg1;
    PyObject* arg2;
    uint32_t arg3 = 0;
    if(!PyArg_ParseTuple(args, "nnO|I", &arg0, &arg1, &arg2, &arg3))
        Py_RETURN_NONE;
    return py::Object::toPyObject(make_thiscall(arg0, arg1, py::Object::toCppObject<std::vector<uintptr_t>>(arg2), arg3));
}

static PyObject* migi_make_fastcall(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    uintptr_t arg1;
    uintptr_t arg2;
    PyObject* arg3;
    uint32_t arg4 = 0;
    if(!PyArg_ParseTuple(args, "nnnO|I", &arg0, &arg1, &arg2, &arg3, &arg4))
        Py_RETURN_NONE;
    return py::Object::toPyObject(make_fastcall(arg0, arg1, arg2, py::Object::toCppObject<std::vector<uintptr_t>>(arg3), arg4));
}

static PyObject* migi_stdcall_to_fastcall(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    bool arg1 = true;
    uintptr_t arg2 = 0;
    uintptr_t arg3 = 0;
    if(!PyArg_ParseTuple(args, "n|pnn", &arg0, &arg1, &arg2, &arg3))
        Py_RETURN_NONE;
    return py::Object::toPyObject(stdcall_to_fastcall(arg0, arg1, arg2, arg3));
}

static PyObject* migi_stdcall_to_fastcall_recycle(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    if(!PyArg_ParseTuple(args, "n", &arg0))
        Py_RETURN_NONE;
    stdcall_to_fastcall_recycle(arg0);
    Py_RETURN_NONE;
}

static PyObject* migi_stdcall_to_thiscall(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    bool arg1 = true;
    uintptr_t arg2 = 0;
    uintptr_t arg3 = 0;
    if(!PyArg_ParseTuple(args, "n|pnn", &arg0, &arg1, &arg2, &arg3))
        Py_RETURN_NONE;
    return py::Object::toPyObject(stdcall_to_thiscall(arg0, arg1, arg2, arg3));
}

static PyObject* migi_stdcall_to_thiscall_recycle(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    if(!PyArg_ParseTuple(args, "n", &arg0))
        Py_RETURN_NONE;
    stdcall_to_thiscall_recycle(arg0);
    Py_RETURN_NONE;
}

static PyObject* migi_make_interceptor(PyObject* /*self*/, PyObject* args)
{
    uintptr_t arg0;
    uintptr_t arg1;
    if(!PyArg_ParseTuple(args, "nn", &arg0, &arg1))
        Py_RETURN_NONE;
    return Singleton<migi::PyInterceptorType>::instance().create(make_interceptor(arg0, arg1));
}

static PyObject* migi_start_console(PyObject* /*self*/, PyObject* args)
{
    PyObject* arg0;
    if(!PyArg_ParseTuple(args, "O", &arg0))
        Py_RETURN_NONE;
    start_console(py::Object::toCppObject<std::vector<std::string>>(arg0));
    Py_RETURN_NONE;
}

static PyObject* migi_get_module_file_path(PyObject* /*self*/, PyObject* /*args*/)
{
    return py::Object::toPyObject(get_module_file_path());
}

static PyObject* migi_logd(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    if(!PyArg_ParseTuple(args, "s", &arg0))
        Py_RETURN_NONE;
    logd(arg0);
    Py_RETURN_NONE;
}

static PyObject* migi_get_module_addr(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    uintptr_t arg1;
    if(!PyArg_ParseTuple(args, "sn", &arg0, &arg1))
        Py_RETURN_NONE;
    return py::Object::toPyObject(get_module_address(arg0, arg1));
}

static PyObject* migi_get_module_proc(PyObject* /*self*/, PyObject* args)
{
    const char* arg0;
    const char* arg1;
    if(!PyArg_ParseTuple(args, "ss", &arg0, &arg1))
        Py_RETURN_NONE;
    return py::Object::toPyObject(get_module_proc(arg0, arg1));
}

static PyObject* migi_get_properties(PyObject* /*self*/, PyObject* /*args*/)
{
    return py::Object::toPyObject(get_properties());
}

static PyObject* migi_detach(PyObject* /*self*/, PyObject* /*args*/)
{
    detach();
    Py_RETURN_NONE;
}

}


PyMODINIT_FUNC PyInit__migi(void) {
    static PyMethodDef sPyMigiMethods[] = {
        {"is_mocked",  migi::migi_is_mocked, METH_VARARGS, "is_mocked"},
        {"sizeof_void_p",  migi::migi_sizeof_void_p, METH_VARARGS, "sizeof_void_p"},
        {"sleep_for",  migi::migi_sleep_for, METH_VARARGS, "sleep_for"},
        {"create_device",  migi::migi_create_device, METH_VARARGS, "create_device"},
        {"dump_bytes",  migi::migi_dump_bytes, METH_VARARGS, "dump_bytes"},
        {"load_library",  migi::migi_load_library, METH_VARARGS, "load_library"},
        {"free_library",  migi::migi_free_library, METH_VARARGS, "free_library"},
        {"make_call",  migi::migi_make_call, METH_VARARGS, "make_call"},
        {"make_thiscall",  migi::migi_make_thiscall, METH_VARARGS, "make_thiscall"},
        {"make_fastcall",  migi::migi_make_fastcall, METH_VARARGS, "make_fastcall"},
        {"stdcall_to_fastcall",  migi::migi_stdcall_to_fastcall, METH_VARARGS, "stdcall_to_fastcall"},
        {"stdcall_to_fastcall_recycle",  migi::migi_stdcall_to_fastcall_recycle, METH_VARARGS, "stdcall_to_fastcall_recycle"},
        {"stdcall_to_thiscall",  migi::migi_stdcall_to_thiscall, METH_VARARGS, "stdcall_to_thiscall"},
        {"stdcall_to_thiscall_recycle",  migi::migi_stdcall_to_thiscall_recycle, METH_VARARGS, "stdcall_to_thiscall_recycle"},
        {"make_interceptor",  migi::migi_make_interceptor, METH_VARARGS, "make_interceptor"},
        {"start_console",  migi::migi_start_console, METH_VARARGS, "start_console"},
        {"get_module_file_path",  migi::migi_get_module_file_path, METH_VARARGS, "get_module_file_path"},
        {"logd",  migi::migi_logd, METH_VARARGS, "logd"},
        {"get_module_addr",  migi::migi_get_module_addr, METH_VARARGS, "get_module_addr"},
        {"get_module_proc",  migi::migi_get_module_proc, METH_VARARGS, "get_module_proc"},
        {"get_properties",  migi::migi_get_properties, METH_VARARGS, "get_properties"},
        {"detach",  migi::migi_detach, METH_VARARGS, "detach"},
        {nullptr, nullptr, 0, nullptr}
    };

    static struct PyModuleDef sPyModMigi = {
        PyModuleDef_HEAD_INIT,
        "_migi",                    /* name of module */
        "migi Python bindings",     /* module documentation, may be NULL */
        -1,                         /* size of per-interpreter state of the module, or -1 if the module keeps state in global variables. */
        sPyMigiMethods,
        nullptr,
        nullptr,
        nullptr,
        nullptr
    };

    PyObject* module = PyModule_Create(&sPyModMigi);
    migi::Singleton<migi::PyConsoleType>::instance().addToModule(module, "Console");
    migi::Singleton<migi::PyDeviceType>::instance().addToModule(module, "Device");
    migi::Singleton<migi::PyInjectorType>::instance().addToModule(module, "Injector");
    migi::Singleton<migi::PyInterceptorType>::instance().addToModule(module, "Interceptor");
    return module;
}
