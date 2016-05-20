//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_TEST_SUITE_HPP
#define NUDB_TEST_SUITE_HPP

#include <nudb/common.hpp>
#include <cstddef>
#include <ostream>
#include <string>

namespace nudb {
namespace test {

class suite
{
    std::size_t n_ = 0;
    std::size_t fail_ = 0;
    std::ostream* os_ = nullptr;

public:
    suite() = default;

    bool operator()(std::ostream& os)
    {
        os_ = &os;
        run();
        return fail_ == 0;
    }

    virtual void run() = 0;

    std::ostream& log()
    {
        return *os_;
    }

    template<class Condition, class String>
    bool
    expect(Condition const& shouldBeTrue,
        String const& reason)
    {
        if(! shouldBeTrue)
        {
            fail(reason);
            return false;
        }
        pass();
        return true;
    }

    template<class Condition>
    bool
    expect(Condition const& shouldBeTrue)
    {
        if(! shouldBeTrue)
        {
            fail("");
            return false;
        }
        pass();
        return true;
    }

    void pass()
    {
        ++n_;
    }

    void fail(std::string const& s)
    {
        ++n_;
        ++fail_;
        if(! s.empty())
            *os_ << '#' << n_ << " failed: " << s << std::endl;
        else
            *os_ << '#' << n_ << " failed" << std::endl;
    }
};

} // test
} // nudb

#endif

