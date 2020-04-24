2.0.3

* Update docca submodule
* Add version of create() with uid parameter

---

2.0.2

* Add default success error_code to enum class error

---

2.0.1

* Fixed a bug introduced in 2.0 where flush time was incorrectly updated 
* Improved thread management in context class

---

2.0.0

* Add thread management context class

---

1.0.2

* Better CMakeLists.txt for finding Boost
* Fix typo in nudb tool
* Remove error::success (API Change)
* Update Travis to Boost 1.61.0

---

1.0.1

* Travis: Limit the number of jobs

---

1.0.0

* First Official Release!
* Fix badge markdown in README.md

---

1.0.0-b7

* Fix doc typos
* Improve file creation on POSIX

---

1.0.0-b6

* Fix incorrect file deletion in create()

---

1.0.0-b5

* fail_file also fails on reads
* Fix bug in rekey where an error code wasn't checked
* Increase coverage
* Add buffer unit test
* Add is_File concept and checks
* Update documentation
* Add example program
* Demote exceptions to asserts in gentex
* Improved commit process
* Dynamic block size in custom allocator

---

1.0.0-b4

* Improved test coverage
* Use master branch for codecov badge
* Throw on API calls when no database open
* Benchmarks vs. RocksDB

### API Changes:

* `insert` sets `error::key_exists` instead of returning `false`
* `fetch` sets `error::key_not_found` instead of returning `false`

---

1.0.0-b3

* Tune buffer sizes for performance
* Fix large POSIX and Win32 writes
* Adjust progress indicator for nudb tool
* Document link requirements
* Add visit test
* Improved coverage

---

1.0.0-b2

* Minor documentation and tidying
* Add CHANGELOG

---

1.0.0-b1

* Initial source tree


