add_executable(test_array_list test_array_list.c)
target_link_libraries(test_array_list PRIVATE cutils)
add_test(NAME test_array_list COMMAND test_array_list)

add_executable(test_linked_list test_linked_list.c)
target_link_libraries(test_linked_list PRIVATE cutils)
add_test(NAME test_linked_list COMMAND test_linked_list)

add_executable(test_hashmap test_hashmap.c)
target_link_libraries(test_hashmap PRIVATE cutils)
add_test(NAME test_hashmap COMMAND test_hashmap)
