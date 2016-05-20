//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "suite.hpp"
#include <nudb/detail/varint.hpp>
#include <array>
#include <iostream>
#include <vector>

namespace nudb {
namespace test {

class varint_test : public suite
{
public:
    void
    test_varints (std::vector<std::size_t> vv)
    {
        for (auto const v : vv)
        {
            std::array<std::uint8_t,
                detail::varint_traits<
                    std::size_t>::max> vi;
            auto const n0 =
                detail::write_varint(
                    vi.data(), v);
            expect (n0 > 0, "write error");
            std::size_t v1;
            auto const n1 =
                detail::read_varint(
                    vi.data(), n0, v1);
            expect(n1 == n0, "read error");
            expect(v == v1, "wrong value");
        }
    }

    void
    run() override
    {
        test_varints({
                0,     1,     2,
              126,   127,   128,
              253,   254,   255,
            16127, 16128, 16129,
            0xff,
            0xffff,
            0xffffffff,
            0xffffffffffffUL,
            0xffffffffffffffffUL});
    }
};

} // test
} // nudb

int main()
{
    std::cout << "varint_test:" << std::endl;
    nudb::test::varint_test t;
    return t(std::cerr) ? EXIT_SUCCESS : EXIT_FAILURE;
}
