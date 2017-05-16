//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef ERROR_CODE_HPP
#define ERROR_CODE_HPP

#include <boost/system/error_code.hpp>

namespace nudb {

#if ! GENERATING_DOCS

// local error_code is the same as boost::system::error_code
typedef boost::system::error_code error_code;

#else

/** This library uses error codes defined and implemented by [boost::system::error_code](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/reference.html#Class-error_code). This documentation describes what users need to know about this type in order to use this library effectively. Usage of error_code from within the nudb namespace are forwarded to [boost::system::error_code](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/reference.html#Class-error_code) .

Invoking functions in this library can result in errors. Such errors might be detected by the functions of this library or be detected by other components that this library depends upon such as the C++ standard library or underlying operating system. So all functions in this library return an error code of type [boost::system::error_code](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/reference.html#Class-error_code). This type is capable of holding error codes returned from any possible source. Given this, it's not surprising that it's somewhat more complex than a simple integer value. Full description is beyond the scope of this documentation, but fortunately there are various sources which together provide a good explanation of how to use it.

- [Boost System Library](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/index.html) This is the official documentation for the Boost System Library which includes the description of
    [boost::system::error_code](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/reference.html#Class-error_code) used by this library

- [C++ Standard Library version](http://en.cppreference.com/w/cpp/error) The Boost System Library has been incorporated into the standard library as part of the C++ standard error handling utilities. Except for the substitution of std:: for boost::system:: namespace, the libraries are identical. This link points to the standard library documentation which may be used in addition to the boost version.

- [Thinking Asyncronously in C++](http://blog.think-async.com/2010/04/system-error-support-in-c0x-part-1.html) Another essential reference on the design and usage of the error_code

@par Associated Types

- [boost::system::error::ercc](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/reference.html#Header-error_code) List of values which might be used as values by [boost::system::error_code](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/reference.html#Class-error_code) .

- [error](ref error) list of values which might be used as values NuDB library functions.
    
- [boost::system::error::system_error](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/reference.html#Class-system_error) Construct and exception from an [error_code](http://www.boost.org/doc/libs/1_64_0/libs/system/doc/reference.html#Class-error_code) .

@par Example
@code
#include <boost/system/error_code.hpp>
#include <nudb/error.hpp>
// ...
boost::system::error_code ec;
nudb::store db;
db.open("db.dat", "db.key", "db.log", ec);
if(!ec)
    return; // success !
if(ec == nudb::error::no_key_file)
    std::cerr << "key file not found" << std::endl;
else
if(ec == boost::system::errc::filename_too_long)
    std::cerr << "file name too long" << std::endl;
else
    std::cerr << ec.message() << std::endl;
return; // failure !    
@endcode

*/
struct error_code {
    /// Return string description.
    std::string message() const;
    /// Convert to bool.  true => success, false => failure
    operator bool() const;
};

/** Comparison of error_code
    @relates error_code
    @param lhs error_code.
    @param rhs error_code.
*/
bool operator==( const error_code & lhs, const error_code & rhs ) noexcept;

/** Comparison of error_code
    @relates error_code
    @param lhs error_code.
    @param rhs error_code.
*/
bool operator!=( const error_code & lhs, const error_code & rhs ) noexcept;

/** Convert error_code to an exception
    @relates error_code
    @param error_code.
*/
boost::system::system_error(const error_code & ec);

} // nudb

#endif // GENERATING_DOCS

#endif // ERROR_CODE_HPP
