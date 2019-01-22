#ifndef SUITE_HPP
#define SUITE_HPP

#define DEFINE_TESTSUITE_INSERT(Library,Module,Class,manual) \
    static boost::beast::unit_test::detail::insert_suite <Class##_test>   \
        Library ## Module ## Class ## _test_instance(             \
            #Class, #Module, #Library, manual)
#define DEFINE_TESTSUITE(Library,Module,Class) \
        DEFINE_TESTSUITE_INSERT(Library,Module,Class,false)
#define DEFINE_TESTSUITE_MANUAL(Library,Module,Class) \
        DEFINE_TESTSUITE_INSERT(Library,Module,Class,true)

#endif
