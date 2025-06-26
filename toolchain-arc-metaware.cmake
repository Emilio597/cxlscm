# 用于MetaWare工具链的CMake工具链文件

# 1. 设置目标系统信息
set(CMAKE_SYSTEM_NAME Generic) # 表示这是一个嵌入式系统
set(CMAKE_SYSTEM_PROCESSOR arc_hs)

# 2. 指定交叉编译器
# CMake会使用这些变量来找到编译器、汇编器等工具
# 确保这些工具链程序在你的系统PATH中
set(CMAKE_C_COMPILER   ccac)
set(CMAKE_CXX_COMPILER ccac) # 如果有C++代码
set(CMAKE_ASM_COMPILER ccac) # MetaWare使用ccac处理.s文件
set(CMAKE_AR           arac)
# objcopy 和 size 可能需要加上工具链前缀，例如 "arc-elf32-objcopy"
# 具体名称请根据你的工具链确定
set(CMAKE_OBJCOPY      objcopyac) # 通常MetaWare工具链会提供带ac后缀的GNU工具
set(CMAKE_SIZE         sizeac)

# 3. 指定编译器不需要链接测试
# 在交叉编译时，CMake无法运行测试程序来检查编译器是否工作
set(CMAKE_C_COMPILER_WORKS 1)
set(CMAKE_CXX_COMPILER_WORKS 1)

# 4. 设置查找库和头文件的根路径
# 这可以防止CMake在你的宿主系统(Linux/Windows)中查找库
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)