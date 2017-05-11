#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define read_sysreg(r) ({                                       \
        unsigned long long __val;                               \
        asm volatile("mrs %0, " #r : "=r" (__val)); \
        __val;                                                  \
})

#define write_sysreg(v, r) do {                                 \
        unsigned long long __val = (unsigned long long)v;                                     \
        asm volatile("msr " #r ", %x0"              \
                     : : "rZ" (__val));                         \
} while (0)

static inline unsigned int read_daif() {
	unsigned int k;
	asm volatile("mrs %0, daif" : "=r"(k));

	return k;
}

int main(int argc, char *argv[]) {
	unsigned int daif = read_daif(); //read_sysreg(daif);
    printf("DAIF org(0x%X)\n", daif);

	write_sysreg(0x100, daif);
	daif = read_sysreg(daif);
	printf("DAIFchanged(0x%X)\n", daif);

    return 0;
}