#include "syscall.h"

int main(){
    int file=Open("sj.txt");
    Write("sj1600012800",12,file);
    Halt();
}
