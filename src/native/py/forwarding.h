#ifndef MIGI_PY_FORWARDING_H_
#define MIGI_PY_FORWARDING_H_

#ifdef _DEBUG
  #undef _DEBUG
  #include <Python.h>
  #define _DEBUG
#else
  #include <Python.h>
#endif

namespace migi {
namespace py {

class Object;

}
}

#endif
