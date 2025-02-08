add_executable(test_array_list test_array_list.c)
target_link_libraries(test_array_list PRIVATE cutils)
add_test(NAME test_array_list COMMAND test_array_list)

add_executable(test_linked_list test_linked_list.c)
target_link_libraries(test_linked_list PRIVATE cutils)
add_test(NAME test_linked_list COMMAND test_linked_list)
