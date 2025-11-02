set(CMAKE_SYSTEM_NAME               Generic)
set(CMAKE_SYSTEM_PROCESSOR          arm)

set(CMAKE_C_COMPILER_ID GNU)
set(CMAKE_CXX_COMPILER_ID GNU)

# Some default GCC settings
# arm-none-eabi- must be part of path environment
set(_ULYSSES_TOOLCHAIN_CANDIDATES
    $ENV{ARM_GNU_TOOLCHAIN}
    $ENV{ARM_GNU_TOOLCHAIN_ROOT}
    "/Applications/ArmGNUToolchain/14.3.rel1/arm-none-eabi"
    "/Applications/ArmGNUToolchain/14.3.rel1"
)

unset(_ULYSSES_TOOLCHAIN_BIN_DIR CACHE)
foreach(_candidate IN LISTS _ULYSSES_TOOLCHAIN_CANDIDATES)
    if(NOT _candidate)
        continue()
    endif()
    if(EXISTS "${_candidate}/bin/arm-none-eabi-gcc")
        set(_ULYSSES_TOOLCHAIN_BIN_DIR "${_candidate}/bin")
        break()
    elseif(EXISTS "${_candidate}/arm-none-eabi/bin/arm-none-eabi-gcc")
        set(_ULYSSES_TOOLCHAIN_BIN_DIR "${_candidate}/arm-none-eabi/bin")
        break()
    endif()
endforeach()

if(_ULYSSES_TOOLCHAIN_BIN_DIR)
    set(TOOLCHAIN_PREFIX            "${_ULYSSES_TOOLCHAIN_BIN_DIR}/arm-none-eabi-")
else()
    set(TOOLCHAIN_PREFIX            arm-none-eabi-)
endif()

set(CMAKE_C_COMPILER                ${TOOLCHAIN_PREFIX}gcc)
set(CMAKE_ASM_COMPILER              ${CMAKE_C_COMPILER})
set(CMAKE_CXX_COMPILER              ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_LINKER                    ${TOOLCHAIN_PREFIX}g++)
set(CMAKE_OBJCOPY                   ${TOOLCHAIN_PREFIX}objcopy)
set(CMAKE_SIZE                      ${TOOLCHAIN_PREFIX}size)

set(CMAKE_EXECUTABLE_SUFFIX_ASM     ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_C       ".elf")
set(CMAKE_EXECUTABLE_SUFFIX_CXX     ".elf")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# MCU specific flags
set(TARGET_FLAGS "-mcpu=cortex-m33 -mfpu=fpv4-sp-d16 -mfloat-abi=hard ")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_ASM_FLAGS "${CMAKE_C_FLAGS} -x assembler-with-cpp -MMD -MP")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fdata-sections -ffunction-sections")

set(CMAKE_C_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_C_FLAGS_RELEASE "-Os -g0")
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g3")
set(CMAKE_CXX_FLAGS_RELEASE "-Os -g0")

set(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -fno-rtti -fno-exceptions -fno-threadsafe-statics")

set(CMAKE_EXE_LINKER_FLAGS "${TARGET_FLAGS}")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -T \"${CMAKE_SOURCE_DIR}/STM32U575xx_FLASH.ld\"")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --specs=nano.specs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${CMAKE_PROJECT_NAME}.map -Wl,--gc-sections")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--print-memory-usage")
set(TOOLCHAIN_LINK_LIBRARIES "m")
