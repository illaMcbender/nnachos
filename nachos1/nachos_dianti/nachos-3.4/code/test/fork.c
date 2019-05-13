#include "syscall.h"
void f()
{
 int file=Open("sj.txt");
    char *str;
    Read(str,12,file);
}
int main(){
    Fork(f);
    Exit(0);
}
