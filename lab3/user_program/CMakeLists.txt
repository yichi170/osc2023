set(CMAKE_LINKER ${LLVM_PATH}/ld.lld)

set(LINKER_SCRIPT "linker.ld")

set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")
set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib")

file(GLOB USER_SOURCES "*.c" "*.S")

add_executable(user.elf ${USER_SOURCES})

target_link_libraries(user.elf mylib)
target_include_directories(user.elf PRIVATE ./)

set_target_properties(user.elf PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}")

add_custom_target(user.bin ALL
    COMMAND llvm-objcopy --output-target=aarch64-rpi3-elf
    		${CMAKE_BINARY_DIR}/user.elf -O binary ${CMAKE_BINARY_DIR}/user.bin
    DEPENDS kernel.elf
    COMMENT "Generating user.bin"
)
