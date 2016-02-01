//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef NUDB_TEST_UTIL_HPP
#define NUDB_TEST_UTIL_HPP

#include <nudb/test/fail_file.hpp>
#include <nudb/test/temp_dir.hpp>
#include <nudb/test/xor_shift_engine.hpp>
#include <nudb/nudb.hpp>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <memory>

namespace nudb {
namespace test {

using key_type = std::uint64_t;

using fail_store =
    basic_store<xxhasher, fail_file<native_file>>;

static std::uint64_t constexpr saltValue = 42;

static std::uint64_t constexpr appnumValue = 1337;

static std::size_t constexpr arenaAllocSize = 16 * 1024 * 1024;


//------------------------------------------------------------------------------

// Can be used with basic_store::fetch
//
class Storage
{
    std::size_t size_ = 0;
    std::size_t capacity_ = 0;
    std::unique_ptr<std::uint8_t[]> buf_;

public:
    Storage() = default;
    Storage (Storage const&) = delete;
    Storage& operator= (Storage const&) = delete;

    std::size_t
    size() const
    {
        return size_;
    }

    std::uint8_t*
    get() const
    {
        return buf_.get();
    }

    std::uint8_t*
    reserve (std::size_t size)
    {
        if (capacity_ < size)
        {
            capacity_ = nudb::detail::ceil_pow2(size);
            buf_.reset (
                new std::uint8_t[capacity_]);
        }
        size_ = size;
        return buf_.get();
    }

    std::uint8_t*
    operator()(void const* data, std::size_t size) 
    {
        reserve(size);
        std::memcpy(buf_.get(), data, size);
        return buf_.get();
    }
};

struct value_type
{
    value_type() = default;
    value_type (value_type const&) = default;
    value_type& operator= (value_type const&) = default;

    key_type key;
    std::size_t size;
    uint8_t* data;
};

//------------------------------------------------------------------------------

class finisher
{
    std::function<void(void)> f_;

public:
    template<class F>
    explicit finisher(F&& f)
        : f_(std::forward<F>(f))
    {
    }

    ~finisher()
    {
        f_();
    }
};

template<class Generator>
static
void
rngcpy (void* buffer, std::size_t bytes,
    Generator& g)
{
    using result_type =
        typename Generator::result_type;
    while (bytes >= sizeof(result_type))
    {
        auto const v = g();
        memcpy(buffer, &v, sizeof(v));
        buffer = reinterpret_cast<
            std::uint8_t*>(buffer) + sizeof(v);
        bytes -= sizeof(v);
    }
    if (bytes > 0)
    {
        auto const v = g();
        memcpy(buffer, &v, bytes);
    }
}

//------------------------------------------------------------------------------

class Sequence
{
public:
    using key_type = test::key_type;

private:
    enum
    {
        minSize = 250,
        maxSize = 1250
    };

    Storage s_;
    xor_shift_engine gen_;
    std::uniform_int_distribution<std::uint32_t> d_size_;

public:
    Sequence()
        : d_size_ (minSize, maxSize)
    {
    }

    // Returns the n-th key
    key_type
    key (std::size_t n)
    {
        gen_.seed(n+1);
        key_type result;
        rngcpy (&result, sizeof(result), gen_);
        return result;
    }

    // Returns the n-th value
    value_type
    operator[] (std::size_t n)
    {
        gen_.seed(n+1);
        value_type v;
        rngcpy (&v.key, sizeof(v.key), gen_);
        v.size = d_size_(gen_);
        v.data = s_.reserve(v.size);
        rngcpy (v.data, v.size, gen_);
        return v;
    }
};

template<class T>
static
std::string
num (T t)
{
    std::string s = std::to_string(t);
    std::reverse(s.begin(), s.end());
    std::string s2;
    s2.reserve(s.size() + (s.size()+2)/3);
    int n = 0;
    for (auto c : s)
    {
        if (n == 3)
        {
            n = 0;
            s2.insert (s2.begin(), ',');
        }
        ++n;
        s2.insert(s2.begin(), c);
    }
    return s2;
}

template<class Stream>
void
print(Stream& os, verify_info const& info)
{
    auto const fhex =
        [](std::uint64_t v)
        {
            std::string s{"0x0000000000000000"};
            auto it = s.end();
            for(it = s.end(); v; v >>= 8)
                *--it = "0123456789abcdef"[v & 0xf];
            return s;
        };
    os <<
        "avg_fetch:       " << std::fixed << std::setprecision(3) << info.avg_fetch << "\n" <<
        "waste:           " << std::fixed << std::setprecision(3) << info.waste * 100 << "%" << "\n" <<
        "overhead:        " << std::fixed << std::setprecision(1) << info.overhead * 100 << "%" << "\n" <<
        "actual_load:     " << std::fixed << std::setprecision(0) << info.actual_load * 100 << "%" << "\n" <<
        "version:         " << num(info.version) << "\n" <<
        "uid:             " << fhex(info.uid) << "\n" <<
        "appnum:          " << info.appnum << "\n" <<
        "key_size:        " << num(info.key_size) << "\n" <<
        "salt:            " << fhex(info.salt) << "\n" <<
        "pepper:          " << fhex(info.pepper) << "\n" <<
        "block_size:      " << num(info.block_size) << "\n" <<
        "bucket_size:     " << num(info.bucket_size) << "\n" <<
        "load_factor:     " << std::fixed << std::setprecision(0) << info.load_factor * 100 << "%" << "\n" <<
        "capacity:        " << num(info.capacity) << "\n" <<
        "buckets:         " << num(info.buckets) << "\n" <<
        "key_count:       " << num(info.key_count) << "\n" <<
        "value_count:     " << num(info.value_count) << "\n" <<
        "value_bytes:     " << num(info.value_bytes) << "\n" <<
        "spill_count:     " << num(info.spill_count) << "\n" <<
        "spill_count_tot: " << num(info.spill_count_tot) << "\n" <<
        "spill_bytes:     " << num(info.spill_bytes) << "\n" <<
        "spill_bytes_tot: " << num(info.spill_bytes_tot) << "\n" <<
        "key_file_size:   " << num(info.key_file_size) << "\n" <<
        "dat_file_size:   " << num(info.dat_file_size) << std::endl;

    std::string s;
    for (size_t i = 0; i < info.hist.size(); ++i)
        s += (i==0) ?
            std::to_string(info.hist[i]) :
            (", " + std::to_string(info.hist[i]));
    os << "hist:            " << s << std::endl;
}

} // test
} // nudb

#endif
