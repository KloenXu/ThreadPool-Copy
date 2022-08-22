#include "allhead.h"

struct tasklist *myhead; //主线程跟线程池里面的子线程共享的一个全局变量
int allfile_num = 0;
int allfile_size_MB = 0;
int ready_num = 0;
int ready_size_MB = 0;
int time_all = 0; //用于打印文件树时文件名前的空位
int main(int argc, char **argv)
{
    int percent = 0, star, nothing;
    int choose, timeuse;
    struct timeval start, end;
    double Total_time;
    // system("clear");
    printf("请输入所要实现的功能编号:\n");
    printf("\033[31m"
           "0为复制:"
           "\033[0m"
           "\n");
    printf("\033[33m"
           "1为打印源目录的目录树:"
           "\033[0m"
           "\n");
    scanf("%d", &choose);
    if (choose == 0)
    //初始化任务链表的头
    {
        printf("你选择了0复制功能\n");
        count_allfile(argv[1], argv[3]);
        myhead = task_init();

        //初始化线程池
        struct threadpool *mythreadpool = pool_init(5);

        //执行任务
        mkdir_act(mythreadpool, argv[1], argv[2], argv[3]);
        gettimeofday(&start, NULL);
        if (allfile_size_MB > 0)
        {
            while (ready_num <= allfile_num)
            {
                printf("已完成:%d/%d  %dMB/%dMB\n", ready_num, allfile_num, ready_size_MB, allfile_size_MB);
                for (percent = 0; percent <= ready_size_MB * 1.00 / allfile_size_MB / 0.02; percent++)
                    printf("*");
                for (percent = ready_size_MB * 1.00 / allfile_size_MB / 0.02 + 1; percent <= 49; percent++)
                    printf(" ");
                printf("|%.1f%%\n", (ready_size_MB * 1.0 / allfile_size_MB) * 100);
                sleep(1);
                if (ready_num == allfile_num && allfile_num != 0)
                {
                    printf("复制已完成:%d/%d  %dMB/%dMB\n", ready_num, allfile_num, allfile_size_MB, allfile_size_MB);
                    printf("**************************************************|100%%\n");
                    break;
                }
            }
        }
        gettimeofday(&end, NULL);
        timeuse = 1000000 * (end.tv_sec - start.tv_sec) + end.tv_usec - start.tv_usec;
        printf("操作所用时间%.2lf秒\n", (double)timeuse / 1000000);
        //线程池的销毁
        pool_destroy(mythreadpool);
    }

    else if (choose == 1)
    {
        int num;
        printf("你选择了1打印文件目录树功能\n");
        //count_allfile(argv[1], argv[3]);
        printf("\033[47;34m""%s""\033[0m""\n",argv[1]);
        tree_directory(argv[1], 0);
    }

    else
    {
        printf("输入错误,函数结束\n");
        return -1;
    }

    return 0;
}