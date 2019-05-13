#include "syscall.h"

int main(){
    int file=Open("sj.txt");
    char *str;
    Read(str,12,file);
    Halt();
}
