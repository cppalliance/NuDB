//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_IDENTITY_CODEC_HPP
#define NUDB_IDENTITY_CODEC_HPP

#include <utility>

namespace nudb {

/** Codec which maps input directly to output. */
class identity
{
public:
    template <class... Args>
    explicit
    identity(Args&&... args)
    {
    }

    char const*
    name() const
    {
        return "none";
    }

    template <class BufferFactory>
    std::pair<void const*, std::size_t>
    compress (void const* in,
        std::size_t in_size, BufferFactory&&) const
    {
        return std::make_pair(in, in_size);
    }

    template <class BufferFactory>
    std::pair<void const*, std::size_t>
    decompress (void const* in,
        std::size_t in_size, BufferFactory&&) const
    {
        return std::make_pair(in, in_size);
    }
};

} // nudb

#endif
