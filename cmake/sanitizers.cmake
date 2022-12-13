add_library(sanitizers INTERFACE)

if(SANITIZERS)
    target_compile_options(
        sanitizers
        INTERFACE -g -fno-omit-frame-pointer -fno-optimize-sibling-calls
                  -fsanitize=${SANITIZERS} -fno-sanitize-recover=${SANITIZERS})

    string(REGEX MATCH "memory" SANITIZER_MEMORY "${SANITIZERS}")
    if(SANITIZER_MEMORY)
        target_compile_options(sanitizers
                               INTERFACE -fsanitize-memory-track-origins)
    endif()

    string(REGEX MATCH "address|memory" SANITIZER_NEW_DEL "${SANITIZERS}")
    if(SANITIZER_NEW_DEL)
        target_compile_definitions(sanitizers INTERFACE SANITIZER_NEW_DEL)
    endif()

    target_link_options(sanitizers INTERFACE -fsanitize=${SANITIZERS})
endif()
