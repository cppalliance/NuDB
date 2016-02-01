//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_API_HPP
#define NUDB_API_HPP

#include <nudb/create.hpp>
#include <nudb/identity.hpp>
#include <nudb/store.hpp>
#include <nudb/recover.hpp>
#include <nudb/verify.hpp>
#include <nudb/visit.hpp>
#include <cstdint>

namespace nudb {

// Convenience for consolidating template arguments
//
template <
    class Hasher,
    class Codec = identity,
    class File = native_file,
    std::size_t BufferSize = 16 * 1024 * 1024
>
struct api
{
    using hash_type = Hasher;
    using codec_type = Codec;
    using file_type = File;
    using store = nudb::store<Hasher, Codec, File>;

    static std::size_t const buffer_size = BufferSize;

    template <class... Args>
    static
    bool
    create (
        path_type const& dat_path,
        path_type const& key_path,
        path_type const& log_path,
        std::uint64_t appnum,
        std::uint64_t salt,
        std::size_t key_size,
        std::size_t block_size,
        float load_factor,
        Args&&... args)
    {
        return nudb::create<Hasher, Codec, File>(
            dat_path, key_path, log_path,
                appnum, salt, key_size, block_size,
                    load_factor, args...);
    }

    template <class... Args>
    static
    bool
    recover (
        path_type const& dat_path,
        path_type const& key_path,
        path_type const& log_path,
        Args&&... args)
    {
        return nudb::recover<Hasher, Codec, File>(
            dat_path, key_path, log_path, BufferSize,
                args...);
    }

    static
    verify_info
    verify (
        path_type const& dat_path,
        path_type const& key_path)
    {
        return nudb::verify<Hasher>(
            dat_path, key_path, BufferSize);
    }

    template <class Function>
    static
    bool
    visit(
        path_type const& path,
        Function&& f)
    {
        return nudb::visit<Codec>(
            path, BufferSize, f);
    }
};

} // nudb

#endif
