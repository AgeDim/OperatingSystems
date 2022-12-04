#include <sys/ioctl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <malloc.h>
#include <unistd.h>
#include <linux/ptrace.h>

#define WR_AVALUE _IOW('a','a',struct message*)
#define WR_SVALUE _IOW('a','c',struct message*)
#define RD_VALUE _IOR('a','b',struct message*)

struct task_cputime{
	uint64_t stime;
	uint64_t utime;
 	unsigned long long sum_exec_runtime;
}; 

struct seccomp_data {
    int nr;
    __u32 arch;
    __u64 instruction_pointer;
    __u64 args[6];
};

struct syscall_info {
    __u64 sp;
    struct seccomp_data data;
};

struct message {
    struct task_cputime ctime;
    struct syscall_info  si;
};

int main(int argc, char *argv[]) {
    int fd;
    if(argc < 3){
	printf("Program need 2 arguments!\n");
	return -1;
    }
    int32_t value = atoi(argv[1]);
    char* pth = argv[2];
    struct message msg;
    printf("\nOpening Driver\n");
    fd = open("/dev/etc_device", O_WRONLY);
    if(fd < 0) {
    	printf("Cannot open device file...\n");
        return 0;
    }
    printf("Writing Pid to Driver\n");
    ioctl(fd, WR_SVALUE, (int32_t*) &value);
    printf("Writing Path to Driver\n");
    ioctl(fd, WR_AVALUE, pth);
    printf("Reading Value from Driver\n");
    ioctl(fd, RD_VALUE, (struct message*) &msg);
    printf("Task_cputime:\n");
    printf("Stime: %ld\n", msg.ctime.stime);
    printf("Utime: %ld\n", msg.ctime.utime);
    printf("Sum_exec_runtime: %llu\n", msg.ctime.sum_exec_runtime);
    printf("________________________\n");
    int i;
    printf("syscall_info:\n");
    printf("sp %lld\n", msg.si.sp);
    printf("nr %d\n", msg.si.data.nr);
    printf("instruction_pointer %lld\n", msg.si.data.instruction_pointer);
    for (i = 0; i<6; i++) printf("arg %d  0x%08X\n", i, (unsigned int) msg.si.data.args[i]);
     printf("_________________________\n");
        printf("Closing Driver\n");
        close(fd);
}
