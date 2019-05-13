#include "syscall.h"

int main(){
    int file=Open("sj.txt");
    Close(file);
    Halt();
}
