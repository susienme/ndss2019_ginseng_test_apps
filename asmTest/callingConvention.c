#include <stdio.h>

long long g = 123;

long long foo(long long arg1, long long arg2) {
    printf("foo#1 g(%lld) arg1(0x%llx) arg2(0x%llx)\n", g, arg1, arg2);
    printf("foo#2 g(%lld) arg1(0x%llx) arg2(0x%llx)\n", g, arg1, arg2);

    return 99;
}

void bar(void) {
    printf("bar g(%lld)\n", g);
}

void printStack(long long *pSP, int items) {

}

int main(int argc, char *argv[]) {
	foo(100, 200);
    bar();
    printf("ARGC (%d)\n", argc);
    return 0;
}