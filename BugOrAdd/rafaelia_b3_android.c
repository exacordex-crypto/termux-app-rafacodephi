#define _GNU_SOURCE
#include <stdint.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sched.h>
#include <stdlib.h>

#define N_WORKERS 4
#define WORK_SIZE 65536
#define TOTAL (N_WORKERS * WORK_SIZE)

static uint32_t crc32_sw(const uint8_t *p, int n){
    uint32_t crc = 0xFFFFFFFFu;
    for(int i=0;i<n;i++){
        crc ^= p[i];
        for(int k=0;k<8;k++)
            crc = (crc >> 1) ^ (0xEDB88320u & -(crc & 1));
    }
    return ~crc;
}

static void uhex(uint32_t x){
    char h[]="0123456789ABCDEF";
    char b[9];
    for(int i=0;i<8;i++) b[i]=h[(x >> (28-i*4)) & 15];
    b[8]='\n';
    write(1,b,9);
}

int main(){
    write(1,"B3_ANDROID: MULTICORE+CRC32SW\n",31);

    uint8_t *buf = mmap(0, TOTAL, PROT_READ|PROT_WRITE,
                        MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    uint32_t *crc = mmap(0, N_WORKERS*4, PROT_READ|PROT_WRITE,
                         MAP_SHARED|MAP_ANONYMOUS, -1, 0);

    if(buf == MAP_FAILED || crc == MAP_FAILED){
        write(1,"MMAP_FAIL\n",10);
        return 1;
    }

    uint16_t a=1,b=1;
    for(int i=0;i<TOTAL;i+=2){
        uint16_t c=a+b;
        buf[i]=c & 255;
        buf[i+1]=c >> 8;
        a=b; b=c;
    }

    struct timeval t0,t1;
    gettimeofday(&t0,0);

    for(int w=0; w<N_WORKERS; w++){
        pid_t p = fork();
        if(p == 0){
            crc[w] = crc32_sw(buf + w*WORK_SIZE, WORK_SIZE);
            write(1,"W:",2);
            char c = '0' + w;
            write(1,&c,1);
            write(1,"\n",1);
            _exit(0);
        }
    }

    for(int w=0; w<N_WORKERS; w++) wait(0);

    gettimeofday(&t1,0);
    long us = (t1.tv_sec-t0.tv_sec)*1000000L + (t1.tv_usec-t0.tv_usec);

    uint32_t x = crc[0]^crc[1]^crc[2]^crc[3];

    write(1,"US=",3);
    uhex((uint32_t)us);
    write(1,"CRC=",4);
    uhex(x);
    write(1,"B3_ANDROID:DONE\n",16);
    return 0;
}
