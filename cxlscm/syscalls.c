#include <sys/stat.h>
#include <sys/types.h>
#include <sys/errno.h>
#include <sys/times.h>
#include <sys/time.h>
#include <unistd.h>

// 忽略未使用的参数警告
#pragma GCC diagnostic ignored "-Wunused-parameter"

// 定义一个全局变量来跟踪错误号
#undef errno
extern int errno;

// 程序的末尾地址，由链接器脚本提供
extern char end;
static char *heap_end;

// 退出程序（在裸机中通常是死循环）
void _exit(int status) {
    while (1) {
        // Halt
    }
}

// 关闭文件
int _close(int file) {
    errno = EBADF;
    return -1;
}

// 将执行环境从一个程序传递到另一个程序（不支持）
int _execve(char *name, char **argv, char **env) {
    errno = ENOMEM;
    return -1;
}

// 创建子进程（不支持）
int _fork(void) {
    errno = EAGAIN;
    return -1;
}

// 获取文件状态
int _fstat(int file, struct stat *st) {
    st->st_mode = S_IFCHR;
    return 0;
}

// 获取进程ID（返回1即可）
int _getpid(void) {
    return 1;
}

// 检查文件描述符是否连接到终端
int _isatty(int file) {
    return 1;
}

// 发送信号（不支持）
int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

// 创建文件链接（不支持）
int _link(char *old, char *new) {
    errno = EMLINK;
    return -1;
}

// 设置文件指针位置
int _lseek(int file, int ptr, int dir) {
    return 0;
}

// 读取文件
int _read(int file, char *ptr, int len) {
    return 0;
}

// 动态内存分配 (sbrk)
// 这个实现非常重要，它使用了链接器脚本中的 'end' 符号
void *_sbrk(int incr) {
    char *prev_heap_end;

    if (heap_end == 0) {
        heap_end = &end;
    }
    prev_heap_end = heap_end;

    // 这里可以添加检查，确保 heap_end 不会与栈指针碰撞
    // char *stack_ptr = (char*) __get_MSP(); // 示例，需要CMSIS支持
    // if (heap_end + incr > stack_ptr) {
    //     errno = ENOMEM;
    //     return (void *)-1;
    // }

    heap_end += incr;
    return (void *)prev_heap_end;
}

// 获取时钟（可以返回0）
int _times(struct tms *buf) {
    return -1;
}

// 删除文件（不支持）
int _unlink(char *name) {
    errno = ENOENT;
    return -1;
}

// 等待子进程（不支持）
int _wait(int *status) {
    errno = ECHILD;
    return -1;
}

// 写入文件
// 您可以修改这里，让它通过UART等方式输出字符，以实现 printf
int _write(int file, char *ptr, int len) {
    // 例如，通过UART发送 'len' 个字节
    // for (int i = 0; i < len; i++) {
    //     uart_put_char(ptr[i]);
    // }
    return len;
}