
find_program(HAS_wayland-scanner wayland-scanner)
if (NOT HAS_wayland-scanner)
    message(FATAL_ERROR "wayland-scanner not found!")
endif()
execute_process(
    COMMAND wayland-scanner
            client-header
            /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/xdg-shell-client-protocol.h
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)
execute_process(
    COMMAND wayland-scanner
            client-header
            /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/xdg-decoration-protocol.h
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)
execute_process(
    COMMAND wayland-scanner
            client-header
            /usr/share/wayland-protocols/unstable/relative-pointer/relative-pointer-unstable-v1.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/relative-pointer-protocol.h
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)
execute_process(
    COMMAND wayland-scanner
            client-header
            /usr/share/wayland-protocols/stable/viewporter/viewporter.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/viewporter-protocol.h
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)
execute_process(
    COMMAND wayland-scanner
            client-header
            /usr/share/wayland-protocols/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/pointer-constraints-protocol.h
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)

execute_process(
    COMMAND wayland-scanner
            public-code
            /usr/share/wayland-protocols/stable/xdg-shell/xdg-shell.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/xdg-shell-protocol.c
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)
execute_process(
    COMMAND wayland-scanner
            public-code
            /usr/share/wayland-protocols/unstable/xdg-decoration/xdg-decoration-unstable-v1.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/xdg-decoration-protocol.c
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)
execute_process(
    COMMAND wayland-scanner
            public-code
            /usr/share/wayland-protocols/unstable/relative-pointer/relative-pointer-unstable-v1.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/relative-pointer-protocol.c
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)
execute_process(
    COMMAND wayland-scanner
            public-code
            /usr/share/wayland-protocols/stable/viewporter/viewporter.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/viewporter-protocol.c
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)
execute_process(
    COMMAND wayland-scanner
            public-code
            /usr/share/wayland-protocols/unstable/pointer-constraints/pointer-constraints-unstable-v1.xml
            ${PROJECT_SOURCE_DIR}/src/protocols/pointer-constraints-protocol.c
    RESULT_VARIABLE result
    OUTPUT_VARIABLE output
    ERROR_VARIABLE error_output
)

