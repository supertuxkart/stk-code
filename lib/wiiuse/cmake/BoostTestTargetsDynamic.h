// Small header computed by CMake to set up boost test.
// include AFTER #define BOOST_TEST_MODULE whatever
// but before any other boost test includes.

// Using the Boost UTF dynamic library

#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

