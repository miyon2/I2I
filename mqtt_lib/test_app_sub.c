#include "mosq_sub_lib.c"

int main(void){
    mosq_sub_init();
    mosq_sub();
    mosq_del();
    
    return 0;
}