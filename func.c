#include "allhead.h"

//递归读取目录
int mkdir_act(struct threadpool *mythreadpool, char *path, char *target, char *type)
{
    int j;
    int time_func = 0;
    DIR *mydir;
    mydir = opendir(path);
    if (mydir == NULL)
    {
        perror("打开目录失败了!\n");
        return -1;
    }
    char allpath[512] = {0};
    char dirpath[512] = {0};
    char result_now[1024] = {0};
    char result_new[1024] = {0};

    bzero(dirpath, sizeof(dirpath));
    sprintf(dirpath, "%s/%s", target, path);
    mkdir(dirpath, 0777);

    //打开目录
    struct dirent *p;
    if (type == NULL)
    {
        while ((p = readdir(mydir)) != NULL) //循环读取，不漏掉任何一个目录项
        {
            if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
                continue;

            if (p->d_type == DT_DIR) //说明这个目录项是个目录
            {
                bzero(allpath, sizeof(allpath));
                sprintf(allpath, "%s/%s", path, p->d_name);
                mkdir_act(mythreadpool, allpath, target, type);
            }
            else
            {
                struct twopath *pathlib = malloc(sizeof(struct twopath));
                bzero(result_now, sizeof(result_now));
                bzero(result_new, sizeof(result_new));
                sprintf(result_now, "%s/%s", path, p->d_name);
                sprintf(result_new, "%s/%s", dirpath, p->d_name);

                bzero(pathlib, sizeof(struct twopath));
                strcpy(pathlib->now_path, result_now);
                strcpy(pathlib->target_path, result_new);

                //往里面添加任务
                add_task(mythreadpool, thread_task, pathlib);
            }
        }
    }
    else
    {
        while ((p = readdir(mydir)) != NULL) //循环读取，不漏掉任何一个目录项
        {
            if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
                continue;

            if (p->d_type == DT_DIR) //说明这个目录项是个目录
            {
                bzero(allpath, sizeof(allpath));
                sprintf(allpath, "%s/%s", path, p->d_name);
                mkdir_act(mythreadpool, allpath, target, type);
            }
            else
            {
                if (strstr(p->d_name, type) != NULL)
                {
                    struct twopath *pathlib = malloc(sizeof(struct twopath));
                    bzero(result_now, sizeof(result_now));
                    bzero(result_new, sizeof(result_new));
                    sprintf(result_now, "%s/%s", path, p->d_name);
                    sprintf(result_new, "%s/%s", dirpath, p->d_name);

                    bzero(pathlib, sizeof(struct twopath));
                    strcpy(pathlib->now_path, result_now);
                    strcpy(pathlib->target_path, result_new);

                    //往里面添加任务
                    add_task(mythreadpool, thread_task, pathlib);
                }
            }
        }
    }

    //关闭目录
    closedir(mydir);
    return 0;
}

void *thread_task(void *arg)
{
    struct twopath *p = (struct twopath *)arg;
    char save_buf[1024 * 1024] = {0};
    int fd_now, fd_target;
    int ret;

    struct stat buf;
    stat(p->now_path, &buf);
    int temp = buf.st_size;

    fd_now = open(p->now_path, O_RDWR);
    if (fd_now == -1)
    {
        printf("打开源文件失败了!\n");
        return NULL;
    }
    fd_target = open(p->target_path, O_CREAT | O_TRUNC | O_RDWR);
    if (fd_target == -1)
    {
        // allfile_num--;
        // printf("原路径:%s相同文件名的文件已存在该路径!\n",p->now_path);
        remove(p->target_path);
        fd_target = open(p->target_path, O_CREAT | O_TRUNC | O_RDWR);
        // return NULL;
    }

    while (1)
    {
        bzero(save_buf, sizeof(save_buf));
        ret = read(fd_now, save_buf, sizeof(save_buf));
        if (ret == 0)
        {
            printf("路径:%s 总大小:%ldKB 复制完毕\n", p->now_path, buf.st_size / 1024);
            break;
        }
        write(fd_target, save_buf, ret);
        ready_size_MB = ready_size_MB + ret / 1024 / 1024;
        // temp = temp - ret;
        //  printf("%s剩余大小为:%dKB 占%.2f%%\n", p->now_path, temp / 1024, (temp * 1.0 / buf.st_size) * 100);
    }

    ready_num++;

    close(fd_now);
    close(fd_target);
    free((struct twopath *)arg);
    return NULL;
}

int count_file(char *path)
{
    int file_num = 0;
    DIR *mydir;
    mydir = opendir(path);
    if (mydir == NULL)
    {
        perror("打开目录失败了!\n");
        return -1;
    }
    char allpath[512] = {0};

    //打开目录
    struct dirent *p;
    while ((p = readdir(mydir)) != NULL) //循环读取，不漏掉任何一个目录项
    {
        if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
            continue;

        if (p->d_type != DT_DIR)
        {
            file_num++;
        }
        else if (p->d_type == DT_DIR) //说明这个目录项是个目录
        {
            file_num++;
            bzero(allpath, sizeof(allpath));
            sprintf(allpath, "%s/%s", path, p->d_name);
            count_file(allpath);
        }
    }
    // printf("该目录%s下文件个数为:%d\n", path, file_num);
    //关闭目录
    closedir(mydir);
    return file_num;
}

int count_dirfile(char *path)
{
    int file_num = 0;
    DIR *mydir;
    mydir = opendir(path);
    if (mydir == NULL)
    {
        perror("打开目录失败了!\n");
        return -1;
    }
    char allpath[512] = {0};

    //打开目录
    struct dirent *p;
    while ((p = readdir(mydir)) != NULL) //循环读取，不漏掉任何一个目录项
    {
        if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
            continue;

        if (p->d_type == DT_DIR) //说明这个目录项是个目录
        {
            file_num++;
            bzero(allpath, sizeof(allpath));
            sprintf(allpath, "%s/%s", path, p->d_name);
            count_file(allpath);
        }
    }
    // printf("该目录%s下文件个数为:%d\n", path, file_num);
    //关闭目录
    closedir(mydir);
    return file_num;
}

int count_allfile(char *path, char *type)
{
    DIR *mydir;
    mydir = opendir(path);
    if (mydir == NULL)
    {
        perror("打开目录失败了!\n");
        return -1;
    }
    char allpath[512] = {0};

    //打开目录
    struct dirent *p;
    if (type == NULL)
    {
        while ((p = readdir(mydir)) != NULL) //循环读取，不漏掉任何一个目录项
        {
            if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
                continue;

            if (p->d_type != DT_DIR)
            {
                allfile_num++;
                sprintf(allpath, "%s/%s", path, p->d_name);
                //printf("%s\n", allpath);

                struct stat buf;
                stat(allpath, &buf);
                allfile_size_MB = allfile_size_MB + buf.st_size / 1024 / 1024;
            }
            else if (p->d_type == DT_DIR) //说明这个目录项是个目录
            {
                // allfile_num++;
                bzero(allpath, sizeof(allpath));
                sprintf(allpath, "%s/%s", path, p->d_name);
                //printf("%s\n", allpath);
                count_allfile(allpath, type);
            }
        }
    }
    else
    {
        while ((p = readdir(mydir)) != NULL) //循环读取，不漏掉任何一个目录项
        {
            if (strcmp(p->d_name, ".") == 0 || strcmp(p->d_name, "..") == 0)
                continue;

            if (p->d_type != DT_DIR)
            {
                if (strstr(p->d_name, type) != NULL)
                {
                    allfile_num++;
                    sprintf(allpath, "%s/%s", path, p->d_name);
                    // printf("%s\n", allpath);

                    struct stat buf;
                    stat(allpath, &buf);
                    allfile_size_MB = allfile_size_MB + buf.st_size / 1024 / 1024;
                }
            }
            else if (p->d_type == DT_DIR) //说明这个目录项是个目录
            {
                // allfile_num++;
                bzero(allpath, sizeof(allpath));
                sprintf(allpath, "%s/%s", path, p->d_name);
                // printf("%s\n", allpath);
                count_allfile(allpath, type);
            }
        }
    }
    //关闭目录
    closedir(mydir);
    return allfile_num;
}


//int file_num=0;
//int dir_num=0;
int count_num(char *path,int *filenum,int *docnum)
{
	int file_num=0;
	int dir_num=0;
	DIR *mydir;
	mydir=opendir(path);
	if(mydir==NULL)
	{
		perror("打开目录失败了!\n");
		return -1;
	}
	struct dirent *p;
	while((p=readdir(mydir))!=NULL)  //循环读取，不漏掉任何一个目录项
	{
		//排除掉.和..
		if(strcmp(p->d_name,".")==0 || strcmp(p->d_name,"..")==0)
			continue;
		
		if(p->d_type==DT_DIR)  //说明是个目录
		{
			file_num++;
		}
		if(p->d_type==DT_REG)  //说明是个普通文件
		{		
			dir_num++; 	
		}
	}
	//关闭目录
	closedir(mydir);
	*docnum=file_num;
	*filenum=dir_num;
	return file_num+dir_num;
}

int dirnum[50];
int filenum[50];
int tree_directory(char *destpath,int k)
{
	//统计多少个目录和文件
	//int sum=count_num(destpath,&filenum[k],&docnum[k]);
    count_num(destpath,&filenum[k],&dirnum[k]);
	//定义数组存放子目录/文件的路径
	char destfilepath[512]={0};
	DIR *mydir;
	mydir=opendir(destpath);
	if(mydir==NULL)
	{
		perror("打开目录失败了!\n");
		return -1;
	}
	struct dirent *p;
	while((p=readdir(mydir))!=NULL)  //循环读取，不漏掉任何一个目录项
	{
	
		if(strcmp(p->d_name,".")==0 || strcmp(p->d_name,"..")==0)
			continue;
		for(int i=0;i<=k - 1;i++)
		{
			if(filenum[i]==0&&dirnum[i]==0)
			{
				printf("    ");
			}
			else 
				printf("│   ");
		}
		//说明是个目录
		if(p->d_type==DT_DIR)  
		{	
			dirnum[k]--;
			if(dirnum[k]==0&&filenum[k]==0)
			{
				printf("└── ""\033[47;34m""%s""\033[0m""\n",p->d_name);
			}	
			else
			{
				printf("├── ""\033[47;34m""%s""\033[0m""\n",p->d_name);
			}
			bzero(destfilepath,512);
			//拼接得到子目录的完整路径 
			sprintf(destfilepath,"%s/%s",destpath,p->d_name);
			
			tree_directory(destfilepath,k+1);
		}
		//说明是个普通文件		
		if(p->d_type==DT_REG) 
		{	
			filenum[k]--;
			
			if(dirnum[k]==0&&filenum[k]==0)
			{
				printf("└── ""\033[34m""%s""\033[0m""\n",p->d_name);
			}	
			else
			{
				printf("├── ""\033[34m""%s""\033[0m""\n",p->d_name); 
			}
		}
	}
	//关闭目录
	closedir(mydir);
	return 0;
}