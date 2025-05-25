add_executable(hello_fortran hello.f)
add_coverage(hello_fortran)
add_test(test_fortran hello_fortran)
