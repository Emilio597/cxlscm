# CMakeLists.txt for CXL SCM Firmware (Final Build Version)
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

project(cxlscm_firmware
    VERSION 1.0.0
    LANGUAGES C ASM
)

set(TARGET_CPU "cortex_m7" CACHE STRING "Target ARM Cortex-M core")
message(STATUS "Targeting ARM Core: ${TARGET_CPU}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- 收集源文件 ---
file(GLOB_RECURSE FE_SOURCES "fe/src/*.c")
file(GLOB_RECURSE BE_SOURCES "be/src/*.c")
file(GLOB TX_COMMON_SOURCES "threadx/common/src/*.c")

# 关键：确保包含了 example_build 目录下的所有启动文件 (.S)
file(GLOB TX_PORT_SOURCES
    "threadx/ports/${TARGET_CPU}/gnu/src/*.c"
    "threadx/ports/${TARGET_CPU}/gnu/src/*.S"
    "threadx/ports/${TARGET_CPU}/gnu/example_build/*.S"
)

# --- 创建可执行目标 ---
add_executable(cxlscm_firmware "")
target_sources(cxlscm_firmware PRIVATE
    cxlscm/start_threadx.c
    cxlscm/syscalls.c
    ${FE_SOURCES}
    ${BE_SOURCES}
    ${TX_COMMON_SOURCES}
    ${TX_PORT_SOURCES}
)

# --- 设置包含目录 ---
target_include_directories(cxlscm_firmware PRIVATE
    fe/inc
    be/inc
    cxlscm
    threadx/common/inc
    "threadx/ports/${TARGET_CPU}/gnu/inc"
)

# --- 设置编译定义 ---
target_compile_definitions(cxlscm_firmware PRIVATE TX_INCLUDE_USER_DEFINE_FILE CXL_SCM_FIRMWARE)

# --- 包含特定于CPU的CMake设置文件 ---
if(EXISTS "threadx/cmake/${TARGET_CPU}.cmake")
    include("threadx/cmake/${TARGET_CPU}.cmake")
    message(STATUS "Included CPU-specific settings from threadx/cmake/${TARGET_CPU}.cmake")
endif()

# ==================== 关键链接器修复 ====================
# 使用 target_link_options 来添加链接器选项
target_link_options(cxlscm_firmware PRIVATE
    # 1. 禁用标准C库的启动文件，解决 _start 重定义问题
    -nostartfiles

    # 2. 指定我们自己的链接描述文件
    #    注意：-T 后面没有空格
    -T${CMAKE_CURRENT_SOURCE_DIR}/sample_threadx.ld

    # 3. 其他链接选项
    -Wl,--gc-sections
    -Wl,-Map=cxlscm_firmware.map
)
# =========================================================

# --- 构建后操作 (保持不变) ---
if(CMAKE_CROSSCOMPILING)
    add_custom_command(TARGET cxlscm_firmware POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:cxlscm_firmware> cxlscm_firmware.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:cxlscm_firmware> cxlscm_firmware.hex
        COMMENT "Creating binary and hex files"
    )
#    add_custom_command(TARGET cxlscm_firmware POST_BUILD
#        COMMAND ${CMAKE_SIZE} $<TARGET_FILE:cxlscm_firmware>
#        COMMENT "Firmware size:"
#    )
endif()

message(STATUS "CXL SCM Firmware project configured.")
