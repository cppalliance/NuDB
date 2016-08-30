//
// Copyright (c) 2015-2016 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include<nudb/test/test_store.hpp>

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
#include <random>
#include <set>
#include <utility>

struct Timer
{
    using clock = std::chrono::steady_clock;
    using time_point = clock::time_point;
    time_point start_;

    Timer() : start_(clock::now())
    {
    }

    auto
    elapsed() const
    {
        return std::chrono::duration_cast<std::chrono::duration<double>>(
            clock::now() - start_);
    }
};

template <class Generator, class F>
std::chrono::duration<double>
time_block(std::uint64_t n, Generator&& g, F&& f)
{
    Timer timer;
    while(n--)
    {
        f(g());
    }
    return timer.elapsed();
}

class gen_key_value
{
    nudb::test::test_store& ts_;
    std::uint64_t cur_;

public:
    gen_key_value(nudb::test::test_store& ts,
        std::uint64_t cur)
        : ts_(ts),
        cur_(cur)
    {
    }
    auto
    operator()()
    {
        return ts_[cur_++];
    }
};

class rand_existing_key
{
    nudb::test::xor_shift_engine rng_;
    std::uniform_int_distribution<std::uint64_t> dist_;
    nudb::test::test_store& ts_;

public:
    rand_existing_key(nudb::test::test_store& ts,
        std::uint64_t max_index,
        std::uint64_t seed = 1337)
        : ts_(ts),
          dist_(0, max_index)
    {
        rng_.seed(seed);
    }
    auto
    operator()()
    {
        return ts_[dist_(rng_)];
    }
};

#if WITH_ROCKSDB
std::map<std::string, std::chrono::duration<double>>
do_timings_rocks(std::uint64_t num_inserts,
    std::uint64_t num_fetches,
    std::uint32_t key_size)
{
    std::map<std::string, std::chrono::duration<double>> result;
    nudb::test::temp_dir td;

    std::unique_ptr<rocksdb::DB> pdb = [path = td.path()]
    {
        rocksdb::DB* db = nullptr;
        rocksdb::Options options;
        options.create_if_missing = true;
        auto const status = rocksdb::DB::Open(options, path, &db);
        if (!status.ok())
            db = nullptr;
        return std::unique_ptr<rocksdb::DB>{db};
    }();

    if (!pdb)
    {
        std::cerr << "Failed to open rocks db.\n";
        return result;
    }

    auto inserter = [key_size, &db = *pdb](auto const& v)
    {
        db.Put(rocksdb::WriteOptions(),
            rocksdb::Slice(reinterpret_cast<char const*>(v.key), key_size),
            rocksdb::Slice(reinterpret_cast<char const*>(v.data), v.size));
    };

    auto fetcher = [&db = *pdb](auto const& v)
    {
        std::string value;
        auto const s = db.Get(rocksdb::ReadOptions(),
            rocksdb::Slice(
                reinterpret_cast<char const*>(&v.key), sizeof(v.key)),
            &value);
        (void)s;
        assert(s.ok());
    };

    nudb::test::test_store ts{key_size, 0, 0};
    result["insert"] = time_block(
        num_inserts, gen_key_value{ts, 0}, inserter);
    result["fetch"] = time_block(
        num_fetches, rand_existing_key{ts, num_inserts - 1}, fetcher);

    return result;
}
#endif

std::map<std::string, std::chrono::duration<double>>
do_timings(std::uint64_t num_inserts,
    std::uint64_t num_fetches,
    std::uint32_t key_size,
    std::size_t block_size,
    float load_factor)
{
    std::map<std::string, std::chrono::duration<double>> result;

    boost::system::error_code ec;

    try
    {
        nudb::test::test_store ts{key_size, block_size, load_factor};
        ts.create(ec);
        if (ec)
            goto fail;
        ts.open(ec);
        if (ec)
            goto fail;

        auto inserter = [&ts, &ec](auto const& v) {
            ts.db.insert(v.key, v.data, v.size, ec);
            if (ec)
                throw boost::system::system_error(ec);
        };

        auto fetcher = [&ts, &ec](auto const& v) {
            ts.db.fetch(v.key, [&](void const* data, std::size_t size) {}, ec);
            if (ec)
                throw boost::system::system_error(ec);
        };

        result["insert"] = time_block(
            num_inserts, gen_key_value{ts, 0}, inserter);
        result["fetch"] = time_block(
            num_fetches, rand_existing_key{ts, num_inserts - 1}, fetcher);
    }
    catch (boost::system::system_error const& e)
    {
        ec = e.code();
    }
    catch (std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
    }

fail:
    if (ec)
        std::cerr << "Error: " << ec.message() << '\n';

    return result;
}

namespace po = boost::program_options;

void
print_help(std::string const& prog_name, const po::options_description& desc)
{
    std::cerr << prog_name << ' ' << desc;
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

    try
    {
        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);
        po::notify(vm);

        return vm;
    }
    catch (std::exception const& e)
    {
        std::cerr << "Incorrect command line syntax.\n";
        std::cerr << "Exception: " << e.what() << '\n';
        return {};
    }
}

template<class T>
T
get_opt(po::variables_map const& vm, std::string const& key, T const& default_value)
{
    return vm.count(key) ? vm[key].as<T>() : default_value;
}

int
main(int argc, char** argv)
{
    po::options_description desc{"Benchmark Options"};
    auto vm = parse_args (argc, argv, desc);

    if (vm.count("help"))
    {
        auto prog_name = boost::filesystem::path(argv[0]).stem().string();
        print_help(prog_name, desc);
        return 0;
    }

    auto const block_size = get_opt<size_t>(vm, "block_size", 4096);
    auto const load_factor = get_opt<float>(vm, "load_factor", 0.5f);
    auto const key_size = get_opt<size_t>(vm, "key_size", 64);
    auto const inserts =
            get_opt<std::vector<std::uint64_t>>(vm, "inserts", {100000, 1'000'000});
    auto const fetches = get_opt<std::uint64_t>(vm, "fetches", 1'000'000);
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
            std::cerr << "Benchmark was not built with rocksdb support\n";
            exit(1);
#endif
            continue;
        }

        if (db != "nudb" && db != "rocksdb")
        {
            std::cerr << "Unsupported database: " << db << '\n';
            exit(1);
        }
    }

    bool const with_rocksdb = dbs.count("rocksdb");
    bool const with_nudb = dbs.count("nudb");

    std::map<std::pair<std::string,std::uint64_t>,
             std::map<std::string, std::chrono::duration<double>>> timings;

    for(auto n: inserts)
    {
        if (with_nudb)
            timings[{"nudb",n}]=
                do_timings(n, fetches, key_size, block_size, load_factor);

#if WITH_ROCKSDB
        if (with_rocksdb)
            timings[{"rocksdb",n}] = do_timings_rocks(n, fetches, key_size);
#endif
    }


    auto const col_w = 14;
    auto const db_w = 9;
    auto const iter_w = 15;

    auto tests = {"insert", "fetch"};

    for(auto const& t : tests)
    {
        std::cout << '\n' << t << '\n';
        if (t == "fetch")
        {
            std::cout << fetches << "\nitems\n";
            std::cout << std::setw(iter_w) << "# db keys";
        }
        else
        {
            std::cout << std::setw(iter_w) << "inserts";
        }
        if (with_nudb)
            std::cout << std::setw(col_w) << "nudb";
#if WITH_ROCKSDB
        if (with_rocksdb)
            std::cout << std::setw(col_w) << "rocksdb";
#endif
        std::cout << '\n';
        for (auto n : inserts)
        {
            std::cout << std::setw(iter_w) << n;
            if (with_nudb)
                std::cout << std::setw(col_w) << std::fixed
                    << std::setprecision(2)
                    << timings[{"nudb", n}][t].count();
#if WITH_ROCKSDB
            if (with_rocksdb)
                std::cout << std::setw(col_w) << std::fixed
                    << std::setprecision(2)
                    << timings[{"rocksdb", n}][t].count();
#endif
            std::cout << '\n';
        }
    }
}
