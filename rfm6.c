#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

// --- 预设的数据结构和模拟环境 ---

#define NUM_PAGES 16    // 为了简化输出，我们假设一个进程最多16个虚拟页
#define NUM_FRAMES 64   // 系统总共有64个物理帧

// 模拟物理内存帧
typedef struct {
    bool in_use;        // 此物理帧是否已被分配
    int share_count;    // 有多少个PTE指向此帧（用于COW）
} PhysicalFrame;

// 模拟页表项 (PTE)
typedef struct {
    bool present;       // 存在位
    bool writable;      // 可写位
    int frame_index;    // 指向的物理帧的索引
} PTE;

// 模拟进程控制块 (PCB)
typedef struct {
    int pid;
    PTE page_table[NUM_PAGES]; // 每个进程有一个页表
} Process;

// 全局模拟环境
PhysicalFrame G_physical_memory[NUM_FRAMES]; // G_ 前缀表示全局变量
Process* G_process_list[10];                 // 最多10个进程
int G_next_pid = 100;                        // 下一个要分配的PID
int G_process_count = 0;

// --- 框架提供的辅助函数 (无需修改) ---

// 初始化模拟环境
void init_simulation() {
    for (int i = 0; i < NUM_FRAMES; ++i) {
        G_physical_memory[i].in_use = false;
        G_physical_memory[i].share_count = 0;
    }
    printf("Simulation environment initialized.\n");
}

// 创建初始的父进程以供测试
Process* create_initial_process() {
    Process* parent = (Process*)malloc(sizeof(Process));
    parent->pid = G_next_pid++;
    G_process_list[G_process_count++] = parent;

    // 初始化页表
    for (int i = 0; i < NUM_PAGES; ++i) parent->page_table[i].present = false;

    // 分配几个页面
    // 页面0: 代码页 (只读)
    parent->page_table[0] = (PTE){.present=true, .writable=false, .frame_index=10};
    G_physical_memory[10] = (PhysicalFrame){.in_use=true, .share_count=1};
    // 页面1: 数据页 (可写)
    parent->page_table[1] = (PTE){.present=true, .writable=true, .frame_index=25};
    G_physical_memory[25] = (PhysicalFrame){.in_use=true, .share_count=1};
    // 页面2: 堆页面 (可写)
    parent->page_table[2] = (PTE){.present=true, .writable=true, .frame_index=30};
    G_physical_memory[30] = (PhysicalFrame){.in_use=true, .share_count=1};

    printf("Initial parent process (PID %d) created.\n", parent->pid);
    return parent;
}

// 打印一个进程的页表状态，用于验证
void print_process_pagetable(Process* proc) {
    if (!proc) return;
    printf("\n--- Page Table for PID: %d ---\n", proc->pid);
    printf("V.Page | Present | Writable | P.Frame | Frame Share Count\n");
    printf("----------------------------------------------------------\n");
    for (int i = 0; i < NUM_PAGES; ++i) {
        if (proc->page_table[i].present) {
            int frame_idx = proc->page_table[i].frame_index;
            printf("  %-4d |    %-3s  |   %-5s  |   %-5d | %-d\n",
                   i,
                   proc->page_table[i].present ? "Yes" : "No",
                   proc->page_table[i].writable ? "Yes" : "No",
                   frame_idx,
                   G_physical_memory[frame_idx].share_count);
        }
    }
    printf("----------------------------------------------------------\n");
}

/**
 * @brief 模拟fork()系统调用，创建一个子进程，并实现写时复制(COW)。
 * @param parent 指向父进程的指针。
 * @return 指向新创建的子进程的指针。
 */
Process* my_fork(Process* parent) {

    printf("\n>>> Calling my_fork() on parent PID %d...\n", parent->pid);
    
    // TODO: 步骤 1: 创建一个新的子进程结构体(Process)，并为其分配一个新的PID。
    Process* child = (Process*)malloc(sizeof(Process));
    if (child == NULL) {
        printf("Error: Failed to allocate memory for child process\n");
        return NULL;
    }
     child->pid = G_next_pid++;
    
    // 初始化子进程页表为全无效
    for (int i = 0; i < NUM_PAGES; ++i) {
        child->page_table[i].present = false;
    }


    // TODO: 步骤 2: 遍历父进程的页表 (从 i=0 到 NUM_PAGES-1)。
    for (int i = 0; i < NUM_PAGES; ++i) {//这里写一个大循环，内层进行条件判断，实现遍历



    // TODO: 步骤 3: 对于父进程中每一个有效的页表项 (即 present_bit 为 true 的PTE):
    if (parent->page_table[i].present) {


    //     a. 将父进程的PTE完整地复制给子进程的对应PTE。
        child->page_table[i] = parent->page_table[i];
    
    //     b. 检查这个页面是否是可写的 (writable)。如果是，则需要触发COW机制。
        if (parent->page_table[i].writable) {
    //     c. COW处理:
    //        i.  将父进程中该页面的PTE权限设置为只读 (writable = false)。
    //        ii. 将子进程中该页面的PTE权限也设置为只读 (writable = false)。
            parent->page_table[i].writable = false;
            child->page_table[i].writable = false;
                
                printf("  Page %d: COW enabled (both processes set to read-only)\n", i);
            } else {
                printf("  Page %d: Read-only page, directly shared\n", i);
            }
    //     d. 无论是只读页还是被设置为只读的可写页，现在它们都被共享了。
    //        因此，需要增加其对应的物理帧的共享计数 (share_count)。
    //        提示: 物理帧的索引是 pte.frame_index。
            int frame_idx = parent->page_table[i].frame_index;
            G_physical_memory[frame_idx].share_count++;
            
            printf("  Frame %d share count increased to %d\n", 
                   frame_idx, G_physical_memory[frame_idx].share_count);
        }
    }



    // TODO: 步骤 4: 将新创建的子进程添加到全局进程列表 G_process_list 中，并更新 G_process_count。
if (G_process_count < 10) {
        G_process_list[G_process_count] = child;
        G_process_count++;
        printf("Child process (PID %d) created successfully.\n", child->pid);
    } else {
        printf("Error: Process list is full!\n");
        free(child);
        return NULL;
    }

    // TODO: 步骤 5: 返回指向子进程的指针。
  return child;
}

   


// --- 用于测试你的实现的 main 函数 (无需修改) ---
int main() {
    init_simulation();

    Process* parent = create_initial_process();
    printf("\n--- State BEFORE fork ---\n");
    print_process_pagetable(parent);

    Process* child = my_fork(parent);

    printf("\n--- State AFTER fork ---\n");
    printf("Parent process state after fork:\n");
    print_process_pagetable(parent);

    printf("Child process state after fork:\n");
    print_process_pagetable(child);

    // 释放内存 (在真实OS中，这是由进程退出时完成的)
    free(parent);
    free(child);

    return 0;
}