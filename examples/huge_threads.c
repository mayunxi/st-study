//gcc -I../obj -g huge_threads.c ../obj/libst.a -o huge_threads
//./huge_threads 10000
//./huge_threads 30000
//
 
#include <stdio.h>
#include "st.h"
 
#define SLEEP_INTERVAL 30  //in ms
 
void* do_calc(void* arg){
    long pidx = *(long*)arg;
    for(;;){
        printf("sthread [#%ld] usleep\n", pidx);
        st_usleep(SLEEP_INTERVAL * 1000);
    }
    return NULL;
}
 
int main(int argc, char** argv){
    if(argc <= 1){
        printf("Test the concurrence of state-threads!\n"
            "Usage: %s <sthread_count>\n"
            "eg. %s 10000\n", argv[0], argv[0]);
        return -1;
    }

    int ret = 0;
    if (st_set_eventsys(ST_EVENTSYS_ALT) == -1) {
        ret = 1013;
        printf("st_set_eventsys use %s failed. ret=%d", st_get_eventsys_name(), ret);
        return ret;
    }
    printf("st_set_eventsys to %s", st_get_eventsys_name());
    
    if(st_init() < 0){
        printf("state threads lib runtime init error!");
        return -1;
    }
    if(st_init() < 0){
        printf("state threads lib runtime init error!");
        return -1;
    }
 
    int i, count = atoi(argv[1]);
    for(i = 1; i <= count; i++){
        if(st_thread_create(do_calc, (void*)&i, 0, 0) == NULL){
            printf("create state thread %d failed\n", i);
            return -1;
        }
    }
 
    st_thread_exit(NULL);
    return 0;
}
