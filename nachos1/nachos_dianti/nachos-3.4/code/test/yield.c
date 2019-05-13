#include "syscall.h"

int main(){
    int file;
    Create("lsy.txt");
    Yield();
    file=Open("lsy.txt");
    Write("lsy1600012800",13,file);
    Exit(0);
}
