add_compile_fail_test(callback_bad_field_name.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(field_insert_space.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(field_location.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(field_size.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(impossible_match_callback.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(impossible_match_field.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(owning_msg_incompatible_storage.cpp LIBRARIES warnings
                      cib_msg)
add_compile_fail_test(owning_msg_incompatible_view.cpp LIBRARIES warnings
                      cib_msg)
add_compile_fail_test(message_cmp_owner.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(message_cmp_view.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(message_const_field_write.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(message_dangling_view.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(message_dup_fieldnames.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(message_get_nonexistent_field.cpp LIBRARIES warnings
                      cib_msg)
add_compile_fail_test(message_incompatible_matcher.cpp LIBRARIES warnings
                      cib_msg)
add_compile_fail_test(message_set_nonexistent_field.cpp LIBRARIES warnings
                      cib_msg)
add_compile_fail_test(message_uninitialized_field.cpp LIBRARIES warnings
                      cib_msg)
add_compile_fail_test(rename_field_duplicate.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(rename_field_not_found.cpp LIBRARIES warnings cib_msg)
add_compile_fail_test(view_upsize.cpp LIBRARIES warnings cib_msg)
