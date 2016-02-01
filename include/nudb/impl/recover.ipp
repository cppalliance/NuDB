//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_IMPL_RECOVER_IPP
#define NUDB_IMPL_RECOVER_IPP

#include <nudb/file.hpp>
#include <nudb/type_traits.hpp>
#include <nudb/detail/bucket.hpp>
#include <nudb/detail/bulkio.hpp>
#include <nudb/detail/format.hpp>
#include <algorithm>
#include <cstddef>
#include <string>

namespace nudb {

template<
    class Hasher,
    class File,
    class... Args>
void
recover(
    path_type const& dat_path,
    path_type const& key_path,
    path_type const& log_path,
    error_code& ec,
    Args&&... args)
{
    static_assert(is_Hasher<Hasher>::value,
        "Hasher requirements not met");
    using namespace detail;
    File df(args...);
    File lf(args...);
    File kf(args...);
    df.open(file_mode::append, dat_path, ec);
    if(ec)
        return;
    kf.open(file_mode::write, key_path, ec);
    if(ec)
        return;
    lf.open(file_mode::append, log_path, ec);
    if(ec == errc::no_such_file_or_directory)
    {
        ec = {};
        return;
    }
    if(ec)
        return;
    key_file_header kh;
    read(kf, kh, ec);
    if(ec)
        return;
    auto const readSize = 32 * kh.block_size;
    // VFALCO should the number of buckets be based on the
    //        file size in the log record instead?
    verify<Hasher>(kh, ec);
    if(ec)
        return;
    dat_file_header dh;
    read(df, dh, ec);
    if(ec)
        return;
    verify<Hasher>(dh, kh, ec);
    if(ec)
        return;
    auto const lf_size = lf.size(ec);
    if(ec)
        return;
    if(lf_size == 0)
    {
        // Nothing to recover
        lf.close();
        File::erase(log_path, ec);
        return;
    }
    auto const bucketSize = bucket_size(kh.capacity);
    log_file_header lh;
    read(lf, lh, ec);
    if(ec != error::short_read)
    {
        if(ec)
            return;
        verify<Hasher>(kh, lh, ec);
        if(ec)
            return;
        auto const dataFileSize = df.size(ec);
        if(ec)
            return;
        buffer buf{kh.block_size};
        bucket b{kh.block_size, buf.get()};
        bulk_reader<File> r{lf,
            log_file_header::size, lf_size, readSize};
        while(! r.eof())
        {
            nbuck_t n;
            // Log Record
            auto is = r.prepare(field<std::uint64_t>::size, ec);
            // Log file is incomplete, so roll back.
            if(ec == error::short_read)
            {
                ec = {};
                break;
            }
            if(ec)
                return;
            read<std::uint64_t>(is, n); // Index
            b.read(r, ec);              // Bucket
            if(ec == error::short_read)
            {
                ec = {};
                break;
            }
            if(b.spill() && b.spill() + bucketSize > dataFileSize)
            {
                ec = error::invalid_log_spill;
                return;
            }
            if(n > kh.buckets)
            {
                ec = error::invalid_log_index;
                return;
            }
            b.write(kf, static_cast<noff_t>(n + 1) * kh.block_size, ec);
            if(ec)
                return;
        }
        df.trunc(lh.dat_file_size, ec);
        if(ec)
            return;
        df.sync(ec);
        if(ec)
            return;
        kf.trunc(lh.key_file_size, ec);
        if(ec)
            return;
        kf.sync(ec);
        if(ec)
            return;
    }
    else
    {
        // Log file is incomplete, so roll back.
        ec = {};
    }
    lf.trunc(0, ec);
    if(ec)
        return;
    lf.sync(ec);
    if(ec)
        return;
    lf.close();
    File::erase(log_path, ec);
    if(ec)
        return;
}

} // nudb

#endif
