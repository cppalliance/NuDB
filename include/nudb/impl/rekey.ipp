//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_IMPL_REKEY_IPP
#define NUDB_IMPL_REKEY_IPP

#include <nudb/detail/bulkio.hpp>
#include <nudb/detail/format.hpp>
#include <cmath>

namespace nudb {

// VFALCO Should this delete the key file on an error?
template<class Hasher, class Progress>
void
rekey(
    path_type const& dat_path,
    path_type const& key_path,
    std::uint64_t itemCount,
    std::size_t bufferSize,
    Progress& progress,
    error_code& ec)
{
    static_assert(is_Hasher<Hasher>::value,
        "Hasher requirements not met");
    static_assert(is_Progress<Progress>::value,
        "Progress requirements not met");
    auto const bulk_size = 64 * 1024 * 1024UL;
    float const load_factor = 0.5;
    auto const& dp = dat_path;
    auto const& kp = key_path;

    // Open data file for reading and appending
    native_file df;
    df.open(file_mode::append, dp, ec);
    if(ec)
        return;
    detail::dat_file_header dh;
    read(df, dh, ec);
    if(ec)
        return;
    auto const df_size = df.size(ec);
    if(ec)
        return;
    detail::bulk_writer<native_file> dw{df, df_size, bulk_size};

    // Create key file
    detail::key_file_header kh;
    kh.version = detail::currentVersion;
    kh.uid = dh.uid;
    kh.appnum = dh.appnum;
    kh.key_size = dh.key_size;
    kh.salt = make_salt();
    kh.pepper = detail::pepper<Hasher>(kh.salt);
    kh.block_size = block_size(kp);
    kh.load_factor = std::min<std::size_t>(
        static_cast<std::size_t>(65536.0 * load_factor), 65535);
    kh.buckets = static_cast<std::size_t>(
        std::ceil(itemCount /(
            detail::bucket_capacity(kh.block_size) * load_factor)));
    kh.modulus = detail::ceil_pow2(kh.buckets);
    native_file kf;
    kf.create(file_mode::append, kp, ec);
    if(ec)
        return;
    detail::buffer buf(kh.block_size);
    {
        std::memset(buf.get(), 0, kh.block_size);
        detail::ostream os(buf.get(), kh.block_size);
        write(os, kh);
        kf.write(0, buf.get(), kh.block_size, ec);
        if(ec)
            return;
    }
    // Build contiguous sequential sections of the
    // key file using multiple passes over the data.
    //
    auto const chunkSize = std::max<std::size_t>(1,
        bufferSize / kh.block_size);
    // Calculate work required
    auto const passes =
       (kh.buckets + chunkSize - 1) / chunkSize;
    auto const nwork = passes * df_size;
    progress(0, nwork);

    buf.reserve(chunkSize * kh.block_size);
    for(nbuck_t b0 = 0; b0 < kh.buckets; b0 += chunkSize)
    {
        auto const b1 = std::min<std::size_t>(b0 + chunkSize, kh.buckets);
        // Buffered range is [b0, b1)
        auto const bn = b1 - b0;
        // Create empty buckets
        for(std::size_t i = 0; i < bn; ++i)
        {
            detail::bucket b(kh.block_size,
                buf.get() + i * kh.block_size, detail::empty);
        }
        // Insert all keys into buckets
        // Iterate Data File
        detail::bulk_reader<native_file> r{df,
            detail::dat_file_header::size, df_size, bulk_size};
        while(! r.eof())
        {
            auto const offset = r.offset();
            // Data Record or Spill Record
            std::size_t size;
            auto is = r.prepare(
                detail::field<detail::uint48_t>::size, ec); // Size
            if(ec)
                return;
            progress((b0 / chunkSize) * df_size + r.offset(), nwork);
            detail::read<detail::uint48_t>(is, size);
            if(size > 0)
            {
                // Data Record
                is = r.prepare(
                    dh.key_size +           // Key
                    size, ec);              // Data
                std::uint8_t const* const key =
                    is.data(dh.key_size);
                auto const h = detail::hash<Hasher>(
                    key, dh.key_size, kh.salt);
                auto const n = detail::bucket_index(
                    h, kh.buckets, kh.modulus);
                if(n < b0 || n >= b1)
                    continue;
                detail::bucket b{kh.block_size, buf.get() +
                   (n - b0) * kh.block_size};
                detail::maybe_spill(b, dw, ec);
                if(ec)
                    return;
                b.insert(offset, size, h);
            }
            else
            {
                // VFALCO Should never get here
                // Spill Record
                is = r.prepare(
                    detail::field<std::uint16_t>::size, ec);
                if(ec)
                    return;
                detail::read<std::uint16_t>(is, size);  // Size
                r.prepare(size, ec); // skip
                if(ec)
                    return;
            }
        }
        kf.write((b0 + 1) * kh.block_size, buf.get(),
            static_cast<std::size_t>(bn * kh.block_size), ec);
        if(ec)
            return;
    }
    dw.flush(ec);
    if(ec)
        return;
}

} // nudb

#endif
