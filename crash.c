#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

#define ITERS 2000

#define ACCESS(p)     (*(volatile char *)(p))
#define BARRIER()     asm volatile("isb" ::: "memory")
#define INVALIDATE(p) asm volatile("dc civac, %0" :: "r"(p) : "memory")
#define WAIT_STORES() asm volatile("dsb sy; isb" ::: "memory")
#define ZVA(p)        asm volatile("dc zva, %0" :: "r"(p) : "memory")

int main(void) 
{
    char *arena = mmap(NULL, 2 * 1024 * 1024, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    
    int fd = open("addr.bin", O_RDONLY);
    
    unsigned *addrs = calloc(ITERS, 8);
    if (read(fd, addrs, ITERS * 8) != ITERS * 8) {
        return 1;
    }

    for (int go = 0; go < 10000; ++go) {
        printf("%d\n", go);

        for (int a = 0; a < ITERS; ++a) {
            if (((addrs[a*2] + 128) | (addrs[a*2 + 1] + 128)) >= 2 * 1024 * 1024) {
                return 1;
            }

            char **ref = (char **)(arena + 0xdfb90);
            char *alloc = arena + addrs[a * 2];
            char *str = arena + 0xdfb5f;
        
            *ref = str;
            for (int i = 0; i < 40; ++i) {
                str[i] = i;
            }
            ACCESS(alloc);
            INVALIDATE(alloc);
            INVALIDATE(ref);

            WAIT_STORES();

            ACCESS(alloc - 64);
        
            WAIT_STORES();
            // And we begin!

            *alloc = 0;
            BARRIER(); // should be optional
            ZVA(alloc);
            BARRIER(); // should be optional
        
            const char *s = *ref;
        
            for (int i = 0; i < 40; ++i) {
                ACCESS(s + i);
            }

            // And that is it!
            WAIT_STORES();

            if ((a % 60) == 0) {
                usleep(10000);
            }
        }
    }
}
