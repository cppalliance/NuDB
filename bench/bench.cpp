//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <nudb/test/test_store.hpp>
#include <nudb/util.hpp>
#include <beast/unit_test/dstream.hpp>

#if WITH_ROCKSDB
#include "rocksdb/db.h"

char const* rocksdb_build_git_sha="Benchmark Dummy Sha";
char const* rocksdb_build_compile_date="Benchmark Dummy Compile Date";
#endif

#include <boost/program_options.hpp>
#include <boost/system/system_error.hpp>

#include <array>
#include <chrono>
#include <cmath>
#include <iomanip>
#include <map>
#include <memory>
#include <numeric>
#include <random>
#include <set>
#include <utility>

namespace nudb {
namespace test {

beast::unit_test::dstream dout{std::cout};
beast::unit_test::dstream derr{std::cerr};

struct Timer
{
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    time_point start_;

    Timer() : start_(clock::now())
    {
    }

    std::chrono::duration<double>
    elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(
            clock::now() - start_);
    }
};

class BenchProgress
{
    progress p_;
    std::uint64_t const total_=0;
    std::uint64_t batchStart_=0;

public:
    BenchProgress(std::ostream& os, std::uint64_t total)
        : p_(os), total_(total)
    {
        p_(0, total);
    }
    void
    update(std::uint64_t batchAmount)
    {
        p_(batchStart_ + batchAmount, total_);
    }
    void
    incBatchStart(std::uint64_t batchSize)
    {
        batchStart_ += batchSize;
    }
};

template <class Generator, class F>
std::chrono::duration<double>
time_block(std::uint64_t n, Generator&& g, F&& f, BenchProgress& progress)
{
    Timer timer;
    for (std::uint64_t i = 0; i < n; ++i)
    {
        f(g());
        if (!(i % 1000))
            progress.update(i);
    }
    return timer.elapsed();
}

class gen_key_value
{
    test_store& ts_;
    std::uint64_t cur_;

public:
    gen_key_value(test_store& ts,
        std::uint64_t cur)
        : ts_(ts),
        cur_(cur)
    {
    }
    item_type
    operator()()
    {
        return ts_[cur_++];
    }
};

class rand_existing_key
{
    xor_shift_engine rng_;
    std::uniform_int_distribution<std::uint64_t> dist_;
    test_store& ts_;

public:
    rand_existing_key(test_store& ts,
        std::uint64_t max_index,
        std::uint64_t seed = 1337)
        : dist_(0, max_index),
          ts_(ts)
    {
        rng_.seed(seed);
    }
    item_type
    operator()()
    {
        return ts_[dist_(rng_)];
    }
};

#if WITH_ROCKSDB
std::map<std::string, std::chrono::duration<double>>
do_timings_rocks(std::uint64_t num_inserts,
    std::uint64_t num_fetches,
    std::uint32_t key_size,
    BenchProgress& progress)
{
    std::map<std::string, std::chrono::duration<double>> result;
    temp_dir td;

    std::unique_ptr<rocksdb::DB> pdb = [&td]
    {
        rocksdb::DB* db = nullptr;
        rocksdb::Options options;
        options.create_if_missing = true;
        auto const status = rocksdb::DB::Open(options, td.path(), &db);
        if (!status.ok())
            db = nullptr;
        return std::unique_ptr<rocksdb::DB>{db};
    }();

    if (!pdb)
    {
        derr << "Failed to open rocks db.\n";
        return result;
    }

    auto inserter = [key_size, &pdb](item_type const& v)
    {
        pdb->Put(rocksdb::WriteOptions(),
            rocksdb::Slice(reinterpret_cast<char const*>(v.key), key_size),
            rocksdb::Slice(reinterpret_cast<char const*>(v.data), v.size));
    };

    auto fetcher = [key_size, &pdb](item_type const& v)
    {
        std::string value;
        auto const s = pdb->Get(rocksdb::ReadOptions(),
            rocksdb::Slice(reinterpret_cast<char const*>(v.key), key_size),
            &value);
        (void)s;
        assert(s.ok());
    };

    test_store ts{key_size, 0, 0};
    result["insert"] = time_block(
        num_inserts, gen_key_value{ts, 0}, inserter, progress);
    progress.incBatchStart(num_inserts);
    result["fetch"] = time_block(
        num_fetches, rand_existing_key{ts, num_inserts - 1}, fetcher, progress);
    progress.incBatchStart(num_fetches);

    return result;
}
#endif

std::map<std::string, std::chrono::duration<double>>
do_timings(std::uint64_t num_inserts,
    std::uint64_t num_fetches,
    std::uint32_t key_size,
    std::size_t block_size,
    float load_factor,
    BenchProgress& progress)
{
    std::map<std::string, std::chrono::duration<double>> result;

    boost::system::error_code ec;

    try
    {
        test_store ts{key_size, block_size, load_factor};
        ts.create(ec);
        if (ec)
            goto fail;
        ts.open(ec);
        if (ec)
            goto fail;

        auto inserter = [&ts, &ec](item_type const& v) {
            ts.db.insert(v.key, v.data, v.size, ec);
            if (ec)
                throw boost::system::system_error(ec);
        };

        auto fetcher = [&ts, &ec](item_type const& v) {
            ts.db.fetch(v.key, [&](void const* data, std::size_t size) {}, ec);
            if (ec)
                throw boost::system::system_error(ec);
        };

        result["insert"] = time_block(
            num_inserts, gen_key_value{ts, 0}, inserter, progress);
        progress.incBatchStart(num_inserts);
        result["fetch"] = time_block(
            num_fetches, rand_existing_key{ts, num_inserts - 1}, fetcher, progress);
        progress.incBatchStart(num_fetches);
    }
    catch (boost::system::system_error const& e)
    {
        ec = e.code();
    }
    catch (std::exception const& e)
    {
        derr << "Error: " << e.what() << '\n';
    }

fail:
    if (ec)
        derr << "Error: " << ec.message() << '\n';

    return result;
}

namespace po = boost::program_options;

void
print_help(std::string const& prog_name, const po::options_description& desc)
{
    derr << prog_name << ' ' << desc;
}

po::variables_map
parse_args(int argc, char** argv, po::options_description& desc)
{

#if WITH_ROCKSDB
    std::vector<std::string> const default_dbs = {"nudb", "rocksdb"};
#else
    std::vector<std::string> const default_dbs = {"nudb"};
#endif
    std::vector<std::uint64_t> const default_ops({100000,1000000});

    desc.add_options()
        ("help,h", "Display this message.")
        ("inserts",
          po::value<std::vector<std::uint64_t>>()->multitoken(),
          "Number of inserts Default: 100000 1000000)")
        ("fetches",
          po::value<std::uint64_t>(),
          "Number of fetches Default: 1000000)")
        ("dbs",
         po::value<std::vector<std::string>>()->multitoken(),
          "databases (Default: nudb rocksdb)")
        ("block_size", po::value<size_t>(),
         "nudb block size (default: 4096)")
        ("key_size", po::value<size_t>(),
         "key size (default: 64)")
        ("load_factor", po::value<float>(),
         "nudb load factor (default: 0.5)")
            ;

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);

        return vm;
}

template<class T>
T
get_opt(po::variables_map const& vm, std::string const& key, T const& default_value)
{
    return vm.count(key) ? vm[key].as<T>() : default_value;
}

} // test
} // nudb

int
main(int argc, char** argv)
{
    using namespace nudb::test;

    po::variables_map vm;

    {
        po::options_description desc{"Benchmark Options"};
        bool parse_error = false;
        try
        {
            vm = parse_args(argc, argv, desc);
        }
        catch (std::exception const& e)
        {
            derr << "Incorrect command line syntax.\n";
            derr << "Exception: " << e.what() << '\n';
            parse_error = true;
        }

        if (vm.count("help") || parse_error)
        {
            auto prog_name = boost::filesystem::path(argv[0]).stem().string();
            print_help(prog_name, desc);
            return 0;
        }
    }

    auto const block_size = get_opt<size_t>(vm, "block_size", 4096);
    auto const load_factor = get_opt<float>(vm, "load_factor", 0.5f);
    auto const key_size = get_opt<size_t>(vm, "key_size", 64);
    auto const inserts =
            get_opt<std::vector<std::uint64_t>>(vm, "inserts", {100000, 1000000});
    auto const fetches = get_opt<std::uint64_t>(vm, "fetches", 1000000);
#if WITH_ROCKSDB
    std::vector<std::string> const default_dbs({"nudb", "rocksdb"});
#else
    std::vector<std::string> const default_dbs({"nudb"});
#endif
    auto to_set = [](std::vector<std::string> const& v) {
        return std::set<std::string>(v.begin(), v.end());
    };
    auto const dbs = to_set(get_opt<std::vector<std::string>>(vm, "dbs", default_dbs));

    for (auto const& db : dbs)
    {
        if (db == "rocksdb")
        {
#if !WITH_ROCKSDB
            derr << "Benchmark was not built with rocksdb support\n";
            exit(1);
#endif
            continue;
        }

        if (db != "nudb" && db != "rocksdb")
        {
            derr << "Unsupported database: " << db << '\n';
            exit(1);
        }
    }

    bool const with_rocksdb = dbs.count("rocksdb") != 0;
    (void) with_rocksdb;
    bool const with_nudb = dbs.count("nudb") != 0;

    std::map<std::pair<std::string,std::uint64_t>,
             std::map<std::string, std::chrono::duration<double>>> timings;

    std::uint64_t const numDB = int(with_nudb) + int(with_rocksdb);
    std::uint64_t const totalOps =
        (std::accumulate(inserts.begin(), inserts.end(), 0ull) +
            inserts.size() * fetches) *
        numDB;
    BenchProgress progress(derr, totalOps);
    for(auto n: inserts)
    {
        derr << "# Running inserts: " << n << '\n';
        if (with_nudb)
            timings[{"nudb",n}]=
                do_timings(n, fetches, key_size, block_size, load_factor, progress);

#if WITH_ROCKSDB
        if (with_rocksdb)
            timings[{"rocksdb",n}] = do_timings_rocks(n, fetches, key_size, progress);
#endif
    }


    auto const col_w = 14;
    auto const iter_w = 15;

    auto tests = {"insert", "fetch"};

    for(auto const& t : tests)
    {
        dout << '\n' << t << " (per second)\n";
        if (!strcmp(t, "fetch"))
        {
            dout << std::setw(iter_w) << "# db keys";
        }
        else
        {
            dout << std::setw(iter_w) << "inserts";
        }
        if (with_nudb)
            dout << std::setw(col_w) << "nudb";
#if WITH_ROCKSDB
        if (with_rocksdb)
            dout << std::setw(col_w) << "rocksdb";
#endif
        dout << '\n';
        for (auto n : inserts)
        {
            auto const num_ops = !strcmp(t, "fetch") ? fetches : n;
            dout << std::setw(iter_w) << n;
            if (with_nudb)
                dout << std::setw(col_w) << std::fixed
                    << std::setprecision(2)
                    << num_ops/timings[{"nudb", n}][t].count();
#if WITH_ROCKSDB
            if (with_rocksdb)
                dout << std::setw(col_w) << std::fixed
                    << std::setprecision(2)
                    << num_ops/timings[{"rocksdb", n}][t].count();
#endif
            dout << '\n';
        }
    }
}
