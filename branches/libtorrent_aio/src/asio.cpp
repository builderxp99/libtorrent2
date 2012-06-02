// builds all boost.asio source as a separate compilation unit
#include <boost/version.hpp>

#if BOOST_VERSION >= 104500
#include <boost/asio/impl/src.hpp>
#elif BOOST_VERSION >= 104400
#include <boost/asio/impl/src.cpp>
#endif

