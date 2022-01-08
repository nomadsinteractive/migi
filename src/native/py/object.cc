#include "py/object.h"

#include "py/python_api.h"

namespace migi {
namespace py {

Object::Ref::Ref()
    : object(nullptr), is_owned_instance(false)
{
}

Object::Ref::Ref(PyObject* object, bool isOwnedInstance)
    : object(object), is_owned_instance(isOwnedInstance)
{
}

Object::Ref::Ref(const Ref& other) noexcept
    : object(other.object), is_owned_instance(true)
{
    Py_XINCREF(object);
}

Object::Ref::Ref(Ref&& other) noexcept
    : object(other.object), is_owned_instance(other.is_owned_instance)
{
    other.object = nullptr;
}

Object::Ref::~Ref()
{
    if(is_owned_instance)
        Py_XDECREF(object);
}

Object::Ref& Object::Ref::operator =(const Ref& other) noexcept
{
    object = other.object;
    is_owned_instance = other.is_owned_instance;
    if(is_owned_instance)
        Py_XINCREF(object);
    return *this;
}

Object::Ref& Object::Ref::operator =(Ref&& other) noexcept
{
    object = other.object;
    is_owned_instance = other.is_owned_instance;
    other.object = nullptr;
    return *this;
}

Object Object::borrow(PyObject* object)
{
    return Object(object, false);
}

Object Object::adopt(PyObject* object)
{
    return Object(object, true);
}

Object Object::own(PyObject* object)
{
    Py_XINCREF(object);
    return Object(object, true);
}

Object::operator bool() const
{
    return _object_ref.object && !isNone();
}

bool Object::isNone() const
{
    return _object_ref.object == Py_None;
}

bool Object::isNullPtr() const
{
    return _object_ref.object == nullptr;
}

std::string Object::str() const
{
    return PyStr(_object_ref.object);
}

Object Object::attr(const std::string& name) const
{
    return Object::adopt(PyObject_GetAttrString(_object_ref.object, name.c_str()));
}

void Object::setattr(const std::string& name, PyObject* value) const
{
    PyObject_SetAttrString(_object_ref.object, name.c_str(), value);
}

PyObject* Object::object() const
{
    return _object_ref.object;
}

Object::Object(PyObject* object, bool isOwnedInstance)
    : _object_ref(object, isOwnedInstance)
{
}

PyObject* Object::Ref::incref()
{
    Py_XINCREF(object);
    return object;
}

List::List(Py_ssize_t size)
    : _object_ref(PyList_New(size), true)
{
}

Object List::operator[](Py_ssize_t index)
{
    return Object::borrow(PyList_GetItem(_object_ref.object, index));
}

void List::append(const Object& item)
{
    PyList_Append(_object_ref.object, item._object_ref.object);
}

}
}
