#include "syscall.h"

int main(){
    int id,file; 
    char *str;
    id=Exec("yield");
    Join(id);
    file=Open("lsy.txt");
    Read(str,13,file);
    Exit(0);
}
