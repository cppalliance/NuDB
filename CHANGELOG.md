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


