#include<iostream>
#include<thread>
#include<time.h>
 
typedef struct {
    int first;
    int last ;
    int result;
}MY_ARGS;

int main(int argc, char const *argv[])
{
    /* code */
    int arr [5006];
    MY_ARGS args1{0,2500,0};
    MY_ARGS args2{2500,5000,0};
    srand((unsigned)time(NULL));
    for(int i=0;i<5000;++i){
        arr[i]=rand()%50;
    }
    auto mufunc = [&arr](MY_ARGS *args){
        int s=0;
        for(int i=args->first;i<args->last;++i){
            s+=arr[i];
            args->result=s;
        }
    };
    std::thread t1(mufunc,&args1);
    std::thread t2(mufunc,&args2);
    t1.join();
    t2.join();
    std::cout<<"s1 ="<<args1.result<<"\ns2="<<args2.result<<std::endl;
    return 0;
}
