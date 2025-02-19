set(CMAKE_LINKER ${LLVM_PATH}/ld.lld)
set(LINKER_SCRIPT "linker.ld")

set(CMAKE_C_LINK_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "-nostdlib")
set(CMAKE_SHARED_LINKER_FLAGS "-nostdlib")
# set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNDEBUG")

file(GLOB KERNEL_SOURCES "*.c" "*.S")

add_executable(kernel.elf ${KERNEL_SOURCES})

target_link_libraries(kernel.elf mylib)
target_include_directories(kernel.elf PRIVATE ./)

set_target_properties(kernel.elf PROPERTIES LINK_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}")

add_custom_target(kernel8.img ALL
    COMMAND llvm-objcopy --output-target=aarch64-rpi3-elf
    		${CMAKE_BINARY_DIR}/kernel.elf -O binary ${CMAKE_BINARY_DIR}/kernel8.img
    DEPENDS kernel.elf
    COMMENT "Generating kernel8.img"
)
