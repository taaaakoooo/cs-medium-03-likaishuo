#include <stdio.h>
#include <stdlib.h>

#define N 64  // 矩阵大小
#define BLOCK_SIZE 8  // 块大小，通常选择使块能放入L1缓存：这里int占四字节，block的大小约为4kb
//整体的大小适合放入缓存中

void matrix_multiply(int A[N][N], int B[N][N], int C[N][N]) {
    //外层块循环
    for (int i = 0; i < N; i += BLOCK_SIZE) {
        //中层循环，获得B的列
        for (int j = 0; j < N; j += BLOCK_SIZE) {
            //内层循环，获得A的列和B的行
            for (int k = 0; k < N; k += BLOCK_SIZE) {
                // 处理一个块，这里使用逐个循环：
                for (int ii = i; ii < i + BLOCK_SIZE && ii < N; ii++) {
                    //从这里开始，ii为定值，A的行不变
                    for (int jj = j; jj < j + BLOCK_SIZE && jj < N; jj++) {
                        int sum = 0;//从这里开始，B的列不在改变
                        for (int kk = k; kk < k + BLOCK_SIZE && kk < N; kk++) {
                            sum += A[ii][kk] * B[kk][jj];
                        }//统一A和B共有的K变量，A的列改变，B的行改变，实现矩阵乘法的行x列
                        C[ii][jj] += sum;
                    }
                }//截止到这里
            }
        }
    }
}//这个函数复杂的原因是因为在c语言中一个循环只能定义一个自增量，不能像数学中一样使用累加符号
//但是这个函数整体的思路很简单，就是实现行x列再累加

int main() 
{
    // 初始化矩阵A和B
    //由于矩阵为1024的方阵，一个一个填不现实，因此这里采用了伪随机数来一键填入
    int A[N][N], B[N][N], C[N][N];
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            A[i][j] = rand() % 100;
            B[i][j] = rand() % 100;
            C[i][j] = 0;
        }
    }
    matrix_multiply(A, B, C);
    return 0;
    //由于矩阵太大因此这里不进行打印
}