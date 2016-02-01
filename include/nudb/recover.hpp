//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_RECOVER_HPP
#define NUDB_RECOVER_HPP

#include <nudb/common.hpp>
#include <nudb/file.hpp>
#include <nudb/detail/bucket.hpp>
#include <nudb/detail/bulkio.hpp>
#include <nudb/detail/format.hpp>
#include <algorithm>
#include <cstddef>
#include <string>

namespace nudb {

/** Perform recovery on a database.
    This implements the recovery algorithm by rolling back
    any partially committed data.
*/
template <
    class Hasher,
    class Codec,
    class File = native_file,
    class... Args>
bool
recover (
    path_type const& dat_path,
    path_type const& key_path,
    path_type const& log_path,
    std::size_t read_size,
    Args&&... args)
{
    using namespace detail;
    File df(args...);
    File lf(args...);
    File kf(args...);
    if (! df.open (file_mode::append, dat_path))
        return false;
    if (! kf.open (file_mode::write, key_path))
        return false;
    if (! lf.open (file_mode::append, log_path))
        return true;
    dat_file_header dh;
    key_file_header kh;
    log_file_header lh;
    try
    {
        read (kf, kh);
    }
    catch (file_short_read_error const&)
    {
        throw store_corrupt_error(
            "short key file header");
    }
    // VFALCO should the number of buckets be based on the
    //        file size in the log record instead?
    verify<Hasher>(kh);
    try
    {
        read (df, dh);
    }
    catch (file_short_read_error const&)
    {
        throw store_corrupt_error(
            "short data file header");
    }
    verify<Hasher>(dh, kh);
    auto const lf_size = lf.actual_size();
    if (lf_size == 0)
    {
        lf.close();
        File::erase (log_path);
        return true;
    }
    try
    {
        read (lf, lh);
        verify<Hasher>(kh, lh);
        auto const df_size = df.actual_size();
        buffer buf(kh.block_size);
        bucket b (kh.block_size, buf.get());
        bulk_reader<File> r(lf, log_file_header::size,
            lf_size, read_size);
        while(! r.eof())
        {
            std::size_t n;
            try
            {
                // Log Record
                auto is = r.prepare(field<
                    std::uint64_t>::size);
                read<std::uint64_t>(is, n); // Index
                b.read(r);                  // Bucket
            }
            catch (store_corrupt_error const&)
            {
                throw store_corrupt_error(
                    "corrupt log record");
            }
            catch (file_short_read_error const&)
            {
                // This means that the log file never
                // got fully synced. In which case, there
                // were no changes made to the key file.
                // So we can recover by just truncating.
                break;
            }
            if (b.spill() &&
                    b.spill() + kh.bucket_size > df_size)
                throw store_corrupt_error(
                    "bad spill in log record");
            // VFALCO is this the right condition?
            if (n > kh.buckets)
                throw store_corrupt_error(
                    "bad index in log record");
            b.write (kf, (n + 1) * kh.block_size);
        }
        kf.trunc(lh.key_file_size);
        df.trunc(lh.dat_file_size);
        kf.sync();
        df.sync();
    }
    catch (file_short_read_error const&)
    {
        // key and data files should be consistent here
    }

    lf.trunc(0);
    lf.sync();
    lf.close();
    File::erase (log_path);
    return true;
}

} // nudb

#endif
