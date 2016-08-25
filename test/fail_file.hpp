//
// Copyright(c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_TEST_FAIL_FILE_HPP
#define NUDB_TEST_FAIL_FILE_HPP

#include <nudb/common.hpp>
#include <nudb/error.hpp>
#include <atomic>
#include <cstddef>
#include <string>
#include <utility>

namespace nudb {
namespace test {

/// Test error codes.
enum class test_error
{
    /// No error
    success = 0,

    /// Simulated failure
    failure
};

/// Returns the error category used for test error codes.
inline
error_category const&
test_category()
{
    struct cat_t : public error_category
    {
        char const*
        name() const noexcept override
        {
            return "nudb";
        }

        std::string
        message(int ev) const override
        {
            switch(static_cast<error>(ev))
            {
            case test_error::failure:
                return "test failure";

            default:
                return "test error";
            }
        }

        error_condition
        default_error_condition(int ev) const noexcept override
        {
            return error_condition{ev, *this};
        }

        bool
        equivalent(int ev,
            error_condition const& ec) const noexcept override
        {
            return ec.value() == ev && &ec.category() == this;
        }

        bool
        equivalent(error_code const& ec, int ev) const noexcept override
        {
            return ec.value() == ev && &ec.category() == this;
        }
    };
    static cat_t const cat{};
    return cat;
}

/// Returns a database error code.
inline
error_code
make_error_code(test_error ev)
{
    return error_code{static_cast<int>(ev), test_category()};
}

} // test
} // nudb

namespace boost {
namespace system {
template<>
struct is_error_code_enum<nudb::test::test_error>
{
    static bool const value = true;
};
} // system
} // boost

namespace nudb {
namespace test {

/// Thrown when a test failure occurs
struct fail_error : std::exception
{
    char const*
    what() const noexcept override
    {
        return "test failure";
    }
};

/** Countdown to test failure modue. */
class fail_counter
{
private:
    std::size_t target_;
    std::atomic<std::size_t> count_;

public:
    fail_counter(fail_counter const&) = delete;
    fail_counter& operator=(fail_counter const&) = delete;

    explicit
    fail_counter(std::size_t target = 0)
    {
        reset(target);
    }

    /** Reset the counter to fail at the nth step, or 0 for no failure. */
    void
    reset(std::size_t n = 0)
    {
        target_ = n;
        count_.store(0);
    }

    bool
    fail()
    {
        return target_ &&(++count_ >= target_);
    }
};

/** Wrapper to simulate file system failures. */
template <class File>
class fail_file
{
private:
    File f_;
    fail_counter* c_ = nullptr;

public:
    fail_file() = default;
    fail_file(fail_file const&) = delete;
    fail_file& operator=(fail_file const&) = delete;
    ~fail_file() = default;

    fail_file(fail_file&&);

    fail_file&
    operator=(fail_file&& other);

    explicit
    fail_file(fail_counter& c);

    bool
    is_open() const
    {
        return f_.is_open();
    }

    path_type const&
    path() const
    {
        return f_.path();
    }

    std::size_t
    actual_size() const
    {
        return f_.actual_size();
    }

    void
    close()
    {
        f_.close();
    }

    bool
    create(file_mode mode,
        path_type const& path)
    {
        return f_.create(mode, path);
    }

    bool
    open(file_mode mode,
        path_type const& path)
    {
        return f_.open(mode, path);
    }

    static
    void
    erase(path_type const& path)
    {
        File::erase(path);
    }

    void
    read(std::size_t offset,
        void* buffer, std::size_t bytes)
    {
        f_.read(offset, buffer, bytes);
    }

    void
    write(std::size_t offset,
        void const* buffer, std::size_t bytes);

    void
    sync();

    void
    trunc(std::size_t length);

private:
    bool
    fail();

    void
    do_fail();
};

template <class File>
fail_file<File>::fail_file(fail_file&& other)
    : f_(std::move(other.f_))
    , c_(other.c_)
{
    other.c_ = nullptr;
}

template <class File>
fail_file<File>&
fail_file<File>::operator=(fail_file&& other)
{
    f_ = std::move(other.f_);
    c_ = other.c_;
    other.c_ = nullptr;
    return *this;
}

template <class File>
fail_file<File>::fail_file(fail_counter& c)
    : c_(&c)
{
}

template <class File>
void
fail_file<File>::write(std::size_t offset,
    void const* buffer, std::size_t bytes)
{
    if(fail())
        do_fail();
    if(fail())
    {
        f_.write(offset, buffer,(bytes + 1) / 2);
        do_fail();
    }
    f_.write(offset, buffer, bytes);
}

template <class File>
void
fail_file<File>::sync()
{
    if(fail())
        do_fail();
    // We don't need a real sync for
    // testing, it just slows things down.
    //f_.sync();
}

template <class File>
void
fail_file<File>::trunc(std::size_t length)
{
    if(fail())
        do_fail();
    f_.trunc(length);
}

template <class File>
bool
fail_file<File>::fail()
{
    if(c_)
        return c_->fail();
    return false;
}

template <class File>
void
fail_file<File>::do_fail()
{
    throw fail_error();
}

} // test
} // nudb

#endif

