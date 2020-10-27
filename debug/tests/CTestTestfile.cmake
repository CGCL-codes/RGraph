# CMake generated Testfile for 
# Source directory: /home/jiangzhihao3/PGwithRDMA/tests
# Build directory: /home/jiangzhihao3/PGwithRDMA/debug/tests
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(random_test.cxx "random_test.cxxtest")
add_test(small_map_test.cxx "small_map_test.cxxtest")
add_test(small_set_test.cxx "small_set_test.cxxtest")
add_test(dense_bitset_test.cxx "dense_bitset_test.cxxtest")
add_test(serializetests.cxx "serializetests.cxxtest")
add_test(thread_tools.cxx "thread_tools.cxxtest")
add_test(test_lock_free_pool.cxx "test_lock_free_pool.cxxtest")
add_test(lock_free_pushback.cxx "lock_free_pushback.cxxtest")
add_test(union_find_test.cxx "union_find_test.cxxtest")
add_test(empty_test.cxx "empty_test.cxxtest")
add_test(csr_storage_test.cxx "csr_storage_test.cxxtest")
add_test(local_graph_test.cxx "local_graph_test.cxxtest")
add_test(synchronous_engine_test "synchronous_engine_test")
add_test(async_consistent_test "async_consistent_test")
add_test(test_vertex_set "test_vertex_set")
subdirs("data")
