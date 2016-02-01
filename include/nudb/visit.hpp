//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_VISIT_HPP
#define NUDB_VISIT_HPP

#include <nudb/common.hpp>
#include <nudb/file.hpp>
#include <nudb/detail/buffer.hpp>
#include <nudb/detail/bulkio.hpp>
#include <nudb/detail/format.hpp>
#include <algorithm>
#include <cstddef>
#include <string>

namespace nudb {

/** Visit each key/data pair in a database file.

    Function will be called with this signature:
        bool(void const* key, std::size_t key_size,
             void const* data, std::size_t size)

    If Function returns false, the visit is terminated.

    @return `true` if the visit completed
    This only requires the data file.
*/
template <class Codec, class Function>
bool
visit(
    path_type const& path,
    std::size_t read_size,
    Function&& f)
{
    using namespace detail;
    using File = native_file;
    File df;
    df.open (file_mode::scan, path);
    dat_file_header dh;
    read (df, dh);
    verify (dh);
    Codec codec;
    // Iterate Data File
    bulk_reader<File> r(
        df, dat_file_header::size,
            df.actual_size(), read_size);
    buffer buf;
    try
    {
        while (! r.eof())
        {
            // Data Record or Spill Record
            std::size_t size;
            auto is = r.prepare(
                field<uint48_t>::size); // Size
            read<uint48_t>(is, size);
            if (size > 0)
            {
                // Data Record
                is = r.prepare(
                    dh.key_size +           // Key
                    size);                  // Data
                std::uint8_t const* const key =
                    is.data(dh.key_size);
                auto const result = codec.decompress(
                    is.data(size), size, buf);
                if (! f(key, dh.key_size,
                        result.first, result.second))
                    return false;
            }
            else
            {
                // Spill Record
                is = r.prepare(
                    field<std::uint16_t>::size);
                read<std::uint16_t>(is, size);  // Size
                r.prepare(size); // skip bucket
            }
        }
    }
    catch (file_short_read_error const&)
    {
        throw store_corrupt_error(
            "nudb: data short read");
    }

    return true;
}

} // nudb

#endif
