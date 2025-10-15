#include <stdio.h> 
#include <unistd.h>     // 提供了对POSIX操作系统API的访问，如 getpid(), execlp()
#include <stdlib.h> 

int main() {
    
    // getpid() 是一个系统调用，会返回当前进程的ID (Process ID)
    printf("--> Hello from exec_demo! My PID is %d.\n", getpid());
    printf("--> I am about to call execlp() to transform into 'ls -l'...\n");
    fflush(stdout); // 刷新标准输出
    //这里这行代码是为了防止exexlp将原进程覆盖掉，导致printf无法执行
    //这里由于程序简单，即使注释掉也无影响


    execlp("ls", "ls", "-l", NULL);
    // execlp() 会在系统的PATH环境变量所指定的目录中，查找名为"ls"的可执行文件。
    // 参数列表:
    // 第一个 "ls": 要执行的程序名。
    // 第二个 "ls": 作为新程序argv[0]的值（通常是程序名本身）。
    // 第三个 "-l": 作为新程序argv[1]的值（一个参数）。
    // NULL: 参数列表必须以NULL结尾。

    printf("This message will be printed here.\n");

    return 0;
}