# CMakeLists.txt for CXL SCM Firmware (ARC HS Target)
cmake_minimum_required(VERSION 3.13 FATAL_ERROR)

# 设置项目
project(cxlscm_firmware
    VERSION 1.0.0
    LANGUAGES C ASM
)

# ------------------------------------------------------------------
# 项目设置
# ------------------------------------------------------------------

# 生成编译数据库（用于IDE如VS Code的IntelliSense）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ------------------------------------------------------------------
# 收集源文件
# ------------------------------------------------------------------

# 1. 您的应用程序源文件
file(GLOB_RECURSE FE_SOURCES "fe/src/*.c")
file(GLOB_RECURSE BE_SOURCES "be/src/*.c")

# 2. ThreadX 通用C代码
file(GLOB TX_COMMON_SOURCES "common/src/*.c")

# 3. ThreadX ARC HS 移植层汇编代码
file(GLOB TX_PORT_SOURCES "ports/arc_hs/metaware/src/*.s")

# 4. 特定于示例的启动和向量汇编代码 (至关重要!)
#    这些文件定义了ThreadX的低级初始化和中断向量
file(GLOB TX_STARTUP_SOURCES "ports/arc_hs/metaware/example_build/sample_threadx/*.s")

# ------------------------------------------------------------------
# 创建可执行目标
# ------------------------------------------------------------------

add_executable(cxlscm_firmware "") # 稍后通过target_sources添加源文件

target_sources(cxlscm_firmware PRIVATE
    # 主启动文件
    cxlscm/start_threadx.c

    # 您的应用代码
    ${FE_SOURCES}
    ${BE_SOURCES}

    # ThreadX 通用代码
    ${TX_COMMON_SOURCES}

    # ThreadX 移植层代码
    ${TX_PORT_SOURCES}
    ${TX_STARTUP_SOURCES}
)

# ------------------------------------------------------------------
# 包含目录
# ------------------------------------------------------------------

target_include_directories(cxlscm_firmware PRIVATE
    # 您项目的包含目录
    fe/inc
    be/inc
    cxlscm

    # ThreadX 的包含目录
    common/inc
    ports/arc_hs/metaware/inc
)

# ------------------------------------------------------------------
# 编译定义和选项
# ------------------------------------------------------------------

# 添加必要的编译定义
target_compile_definitions(cxlscm_firmware PRIVATE
    TX_INCLUDE_USER_DEFINE_FILE # 启用 tx_user.h (如果需要自定义)
    CXL_SCM_FIRMWARE
    CXLSCM_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
    CXLSCM_VERSION_MINOR=${PROJECT_VERSION_MINOR}
    CXLSCM_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

# 通用编译选项
target_compile_options(cxlscm_firmware PRIVATE
    -Wall
    -Wextra
    -g # 保留调试信息
)

# 特定于ARC HS架构的编译和链接选项
target_compile_options(cxlscm_firmware PRIVATE
    -Harc        # 指定ARC工具链模式
    -arcv2hs     # 指定处理器版本
    -core2       # 指定核心配置
)

target_link_options(cxlscm_firmware PRIVATE
    -Harc
    -arcv2hs
    -core2
    -Wl,-Map=cxlscm_firmware.map # 生成map文件
)

# 调试构建的特定设置
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(cxlscm_firmware PRIVATE DEBUG=1)
    target_compile_options(cxlscm_firmware PRIVATE -DDEBUG_ENABLED)
    message(STATUS "Debug build enabled")
else()
    # Release构建时启用优化
    target_compile_options(cxlscm_firmware PRIVATE -O2)
    message(STATUS "Release build enabled with -O2 optimization")
endif()

# ------------------------------------------------------------------
# 构建后操作
# ------------------------------------------------------------------

# 创建二进制和十六进制文件
if(CMAKE_CROSSCOMPILING)
    add_custom_command(TARGET cxlscm_firmware POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:cxlscm_firmware> cxlscm_firmware.bin
        COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:cxlscm_firmware> cxlscm_firmware.hex
        COMMENT "Creating binary and hex files"
    )
endif()

# 显示固件大小
add_custom_command(TARGET cxlscm_firmware POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:cxlscm_firmware>
    COMMENT "Firmware size:"
)

# ------------------------------------------------------------------
# 安装和打包 (可选)
# ------------------------------------------------------------------
install(TARGETS cxlscm_firmware
    RUNTIME DESTINATION bin
)
install(DIRECTORY fe/inc/ DESTINATION include/fe FILES_MATCHING PATTERN "*.h")
install(DIRECTORY be/inc/ DESTINATION include/be FILES_MATCHING PATTERN "*.h")

set(CPACK_SOURCE_GENERATOR "ZIP")
# ... (CPack设置与您之前的版本相同)
include(CPack)

message(STATUS "CXL SCM Firmware configured for ARC HS target.")