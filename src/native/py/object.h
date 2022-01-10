#ifndef MIGI_PY_OBJECT_H_
#define MIGI_PY_OBJECT_H_

#include <stdint.h>

#include <memory>
#include <utility>
#include <string>

#include "api.h"

#include "py/forwarding.h"
#include "py/python_api.h"


namespace migi {
namespace py {

class List;

class Object {
private:

    struct Ref {
        Ref();
        Ref(PyObject* object, bool isOwnedInstance);
        Ref(const Ref& other) noexcept;
        Ref(Ref&& other) noexcept;
        ~Ref();

        Ref& operator = (const Ref& other) noexcept;
        Ref& operator = (Ref&& other) noexcept;

        PyObject* incref();

        PyObject* object;
        bool is_owned_instance;
    };

public:
    Object() = default;

    DEFAULT_COPY_AND_ASSIGN_NOEXCEPT(Object);

    static Object borrow(PyObject* object);
    static Object adopt(PyObject* object);
    static Object own(PyObject* object);

    explicit operator bool() const;

    bool isNone() const;
    bool isNullPtr() const;

    std::string str() const;

    Object attr(const std::string& name) const;
    void setattr(const std::string& name, PyObject* value) const;

    template<typename T> void setattr(const std::string& name, T value) const {
        setattr(name, toPyObject(value));
    }

    template<typename... Args> Object operator() (Args&&... args) const {
        Object pyargs = Object::adopt(PyTuple_New(sizeof...(args)));
        packOneArgument(pyargs._object_ref.object, 0, std::forward<Args...>(args)...);
        return Object::adopt(PyObject_Call(_object_ref.object, pyargs._object_ref.object, nullptr));
    }

    template<typename T> static PyObject* toPyObject(T cppobj) {
        return toPyObject_sfinae(std::forward<T>(cppobj), nullptr);
    }

    template<typename T> static T toCppObject(PyObject* pyobj) {
        return toCppObject_sfinae<T>(pyobj, nullptr);
    }

    PyObject* object() const;

private:
    Object(PyObject* object, bool isOwnedInstance);

    template<typename T, typename... Args> static void packOneArgument(PyObject* tuple, size_t index, T one, Args&&... args) {
        PyTuple_SetItem(tuple, index, toPyObject(one));
        packOneArgument(tuple, index + 1, std::forward<Args...>(args)...);
    }

    static void packOneArgument(PyObject* /*tuple*/, size_t /*index*/) {
    }

    template<typename T> struct remove_cvref {
        typedef std::remove_cv_t<std::remove_reference_t<T>> type;
    };
    template<typename T> using remove_cvref_t = typename remove_cvref<T>::type;

    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, typename std::enable_if_t<std::is_same<remove_cvref_t<T>, Object>::value>*) {
        return cppobj._object_ref.incref();
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, typename std::enable_if_t<std::is_same<remove_cvref_t<T>, List>::value>*) {
        return cppobj._object_ref.incref();
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::enable_if_t<std::is_enum<remove_cvref_t<T>>::value>*) {
        return PyLong_FromLong(static_cast<int32_t>(cppobj));
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::enable_if_t<std::is_integral<remove_cvref_t<T>>::value && !std::is_same<remove_cvref_t<T>, bool>::value && std::is_signed<remove_cvref_t<T>>::value>*) {
        return sizeof(T) < sizeof(long long) ? PyLong_FromLong(cppobj) : PyLong_FromLongLong(cppobj);
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::enable_if_t<std::is_integral<remove_cvref_t<T>>::value && !std::is_same<remove_cvref_t<T>, bool>::value && std::is_unsigned<remove_cvref_t<T>>::value>*) {
        return sizeof(T) < sizeof(long long) ? PyLong_FromUnsignedLong(cppobj) : PyLong_FromUnsignedLongLong(cppobj);
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::enable_if_t<std::is_floating_point<remove_cvref_t<T>>::value>*) {
        return PyFloat_FromDouble(cppobj);
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::enable_if_t<std::is_same<remove_cvref_t<T>, bool>::value>*) {
        return PyBool_FromLong(static_cast<int32_t>(cppobj));
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::enable_if_t<std::is_same<remove_cvref_t<T>, char*>::value || std::is_same<remove_cvref_t<T>, const char*>::value>*) {
        return PyUnicode_FromString(cppobj);
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::enable_if_t<std::is_same<remove_cvref_t<T>, std::string>::value>*) {
        return PyUnicode_FromString(cppobj.c_str());
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::enable_if_t<std::is_same<remove_cvref_t<T>, std::wstring>::value>*) {
        return PyUnicode_FromWideChar(cppobj.c_str(), 0);
    }
    template<typename T> static PyObject* toPyObject_sfinae(T cppobj, std::remove_reference_t<decltype(cppobj.begin()->second)>*) {
        PyObject* pyDict = PyDict_New();
        for(const auto& i : cppobj)
            PyDict_SetItem(pyDict, toPyObject(i.first), toPyObject(i.second));
        return pyDict;
    }

    template<typename T> static T toCppObject_sfinae(PyObject* pyobj, std::enable_if_t<std::is_same<remove_cvref_t<T>, std::string>::value>*) {
        return PyStr(pyobj);
    }
    template<typename T> static T toCppObject_sfinae(PyObject* pyobj, std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, bool>::value && std::is_signed<T>::value>*) {
        return static_cast<T>(sizeof(T) < sizeof(long long) ? PyLong_AsLong(pyobj) : PyLong_AsLongLong(pyobj));
    }
    template<typename T> static T toCppObject_sfinae(PyObject* pyobj, std::enable_if_t<std::is_integral<T>::value && !std::is_same<T, bool>::value && !std::is_signed<T>::value>*) {
        return static_cast<T>(sizeof(T) < sizeof(long long) ? PyLong_AsUnsignedLong(pyobj) : PyLong_AsUnsignedLongLong(pyobj));
    }

    template<typename T> static T toCppObject_sfinae(PyObject* obj, std::enable_if_t<!(std::is_same<T, std::string>::value || std::is_same<T, std::wstring>::value), decltype(std::declval<T>().push_back(std::declval<typename T::value_type>()))>*) {
        typedef typename T::value_type U;
        Py_ssize_t len = PyObject_Length(obj);
        T col;
        DCHECK(len != -1, "Object has no length");
        for(Py_ssize_t i = 0; i < len; ++i) {
            PyObject* key = PyLong_FromLong(i);
            PyObject* item = PyObject_GetItem(obj, key);
            col.push_back(toCppObject<U>(item));
            Py_XDECREF(item);
            Py_DECREF(key);
        }
        return col;
    }

private:
    Ref _object_ref;

    friend class List;
};


class List {
public:
    List(Py_ssize_t size = 0);

    Object operator[] (Py_ssize_t index);

    void append(const Object& item);

    template<typename T> void append(T cppobj) {
        append(Object::adopt(Object::toPyObject<T>(cppobj)));
    }

private:
    Object::Ref _object_ref;

    friend class Object;
};

}
}

#endif
