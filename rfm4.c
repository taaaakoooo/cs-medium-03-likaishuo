#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/mman.h>

#define MAX_INPUT 1024
#define SHARED_MEM_SIZE 4096

// 演示 Copy-On-Write 的函数
void demonstrate_cow() {
    printf("\n=== 开始演示 Copy-On-Write 机制 ===\n");
    
    // 在堆上分配内存
    int *shared_var = malloc(sizeof(int));//定义父进程对应的内存大小
    *shared_var = 100;
    
    printf("父进程: 分配内存，初始值 = %d, 地址 = %p\n", *shared_var, (void*)shared_var);
    printf("父进程: 变量值 = %d\n", *shared_var);
    
    pid_t pid = fork();//pid_t是一种数据类型，用来记录进程ID
    //先介绍fork这个特殊函数，fork调用一次，返回两次
    //fork的返回值为正数，0，负数三种，>0表示在父进程中，==0表示在子进程中，<0则创建失败
    //其中，整数就是父进程的ID，而负数为-1


    
    if (pid < 0) {//fork小于零，则print error
        perror("fork 失败");//perror函数可以将错误数字代码转化为错误描述
        free(shared_var);
        return;
    } else if (pid == 0) {
        // 子进程
        printf("子进程: fork() 后，变量值 = %d (与父进程相同)\n", *shared_var);
        printf("子进程: 修改变量值为 200\n");
        
        *shared_var = 200;  // 这里会触发 Copy-On-Write！
        
        printf("子进程: 修改后，变量值 = %d, 地址 = %p\n", *shared_var, (void*)shared_var);
        printf("子进程: 注意！地址相同，但物理内存页已被复制\n");
        
        free(shared_var);
        exit(0);
    } else {
        // 父进程
        sleep(1);  // 确保子进程先执行修改
        //由于父子进程的执行顺序由操作系统调度决定，并不确定，这里使用sleep确保子进程先执行
        
        printf("父进程: 子进程修改后，我的变量值 = %d (未被改变！)\n", *shared_var);
        printf("父进程: 这就是 Copy-On-Write: 写时才复制内存页\n");
        
        wait(NULL);  // 等待子进程结束
        free(shared_var);
    }
    
    printf("=== Copy-On-Write 演示结束 ===\n\n");
}

// 执行命令的函数
void execute_command(char *command) {
    pid_t pid = fork();
    
    if (pid < 0) {
        perror("minishell: fork 失败");
        return;
    } else if (pid == 0) {
        // 子进程：执行命令
        char *args[] = {command, NULL};
        
        if (execvp(command, args) == -1) {//通过字符串比较，确定子进程的内容是否修改
            printf("minishell: 命令未找到: %s\n", command);
            exit(1);
        }
    } else {
        // 父进程：等待子进程结束
        int status;
        waitpid(pid, &status, 0);
        
        if (WIFEXITED(status)) {
            printf("minishell: 命令执行完成，退出码: %d\n", WEXITSTATUS(status));
        }
    }
}

// 主函数：minishell 循环
int main() {
    char input[MAX_INPUT];
    
    printf("=== 欢迎使用 MiniShell ===\n");
    printf("支持的命令: ls, pwd, date, whoami, echo 等系统命令\n");
    printf("特殊命令: cow (演示Copy-On-Write), exit (退出)\n");
    printf("==========================\n");
    
    // 演示一次 Copy-On-Write
    demonstrate_cow();
    
    //执行minishell：
    while (1) {
        printf("minishell> ");
        fflush(stdout);//这个函数是刷新缓冲区，避免缓冲区被复制到子进程中，导致重复输出
        
        // 读取用户输入
        if (fgets(input, MAX_INPUT, stdin) == NULL) {//读取失败
            printf("\n");
            break;
        }
        
        // 去除换行符
        input[strcspn(input, "\n")] = 0;
        
        // 处理空输入
        if (strlen(input) == 0) {
            continue;
        }
        
        // 退出命令
        if (strcmp(input, "exit") == 0) {
            printf("再见！\n");
            break;
        }
        
        // 特殊命令：演示 Copy-On-Write
        if (strcmp(input, "cow") == 0) {
            demonstrate_cow();
            continue;
        }
        
        // 执行普通命令
        execute_command(input);
    }
    
    return 0;
}