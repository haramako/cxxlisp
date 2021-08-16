#pragma once

// Switch GC implementation.
#ifdef CXXLISP_GC_ENABLED
// Use boehm GC.

#include <gc_cpp.h>

#else
// No GC.

class gc {};
class gc_cleanup {};

#endif
