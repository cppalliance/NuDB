# Benchmarks for NuDB

These benchmarks time two operations:

1. The time to insert N values into a database. The inserted keys and values are
   pseudo-randomly generated. The random number generator is always seeded with
   the same value for each run, so the same values are always inserted.
2. The time to fetch M existing values from a database with N values. The order
   that the keys are fetched are pseudo-randomly generated. The random number
   generator is always seeded with the same value on each fun, so the keys are
   always looked up in the same order.

At the end of a run, the program outputs a table of operations per second. The
tables have a row for each database size, and a column for each database (in
cases where NuDB is compared against other databases). A cell in the table is
the number of operations per second for that trial. For example, in the table
below NuDB had 340397 Ops/Sec when fetching from an existing database with
10,000,000 values.

A sample output:

insert (per second)
        inserts          nudb       rocksdb
        1000000     387894.04     148233.29
        5000000     348982.15      93376.19
       10000000     279767.88      62597.36

fetch (per second)
      # db keys          nudb       rocksdb
        1000000     455249.16     164997.45
        5000000     291651.66      40969.44
       10000000     340397.87      21596.47

# Building

## Building with CMake

Note: Building with RocksDB is currently not supported on Windows.

1. The benchmark requires boost. If building with rocksdb, it also requires zlib
   and snappy. These are popular libraries and should be available through the
   package manager.
1. The benchmark and test programs require some submodules that are not
   installed by default. Get these submodules by running:
   `git submodule update --init`
2. From the main nudb directory, create a directory for the build and change to
   that directory: `mkdir bench_build;cd bench_build`
3. Generate a project file or makefile.
   * If building on Linux, generate a makefile. If building with rocksdb
   support, use: `cmake -DCMAKE_BUILD_TYPE=Release ../bench` If building
   without rocksdb support, use: `cmake -DCMAKE_BUILD_TYPE=Release ../bench
   -DWITH_ROCKSDB=false` Replace `../bench` with the path to the `bench`
   directory if the build directory is not in the suggested location.
   * If building on windows, generate a project file. The CMake gui program is
   useful for this. Use the `bench` directory as the `source` directory and
   the `bench_build` directory as the `binaries` directory. Press the `Add
   Entry` button and add a `BOOST_ROOT` variable that points to the `boost`
   directory. Hit `configure`. A dialog box will pop up. Select the generator
   for Win64. Select `generate` to generate the visual studio project.
4. Compile the program.
   * If building on Linux, run: `make`
   * If building on Windows, open the project file generated above in Visual
   Studio.

## Test the build

Try running the benchmark with a small database: `./bench --inserts=1000
10000`. A report similar to sample should appear after a few seconds.

# Command Line Options

*  `--inserts arg` : Number of values to insert. When timing fetches, the data
   base will have this many values in it. The argument may be a list, so several
   timing may be collected. For example, the sample output above was run with
   `--inserts=1000000 5000000 10000000`. If `inserts` is not specified, it
   defaults to `100000 1000000`
*  `--fetches arg` : Number of values to fetch from the database. If `fetches`
   is not specified, it defaults to `1000000`. Unlike `inserts`, `fetches` is
   not a list. It takes a single value only.
*  `--dbs arg` : Databases to run the benchmark on. Currently, only `nudb` and
   `rocksdb` are supported. Building with `rocksdb` is optional on Linux, and
   only `nudb` is supported on windows. The argument may be a list. If `dbs` is
   not specified, it defaults to all the database the build supports (either
   `nudb` or `nudb rocksdb`).
*  `--key_size arg` : nudb key size. If not specified the default is 64.
*  `--block_size arg` : nudb block size. This is an advanced argument. If not
   specified the default is 4096.
*  `--load_factor arg` : nudb load factor. This is an advanced argument. If not
   specified the default is 0.5.

