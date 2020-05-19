#include<iostream>
#include<thread>
#include<atomic>
#include<mutex>

void solve1(){
    std:: atomic_int s(0);
    auto myfunc=[&s]{for(int i=0;i<10000000;++i)s++;};
    std:: thread t1(myfunc);
    std:: thread t2(myfunc);
    t1.join();
    t2.join();
    std:: cout<<s<<std:: endl;
}
void solve2(){
    std::mutex _lock;
    int s=0;
    auto myfunc=[&s,&_lock]{
        _lock.lock();
        for(int i=0;i<1000000;++i){
            ++s;
        }
        _lock.unlock();
    };
    std:: thread t1(myfunc);
    std:: thread t2(myfunc);
    t1.join();
    t2.join();
    std:: cout<<s<<"  "<<std::endl;
    return ;
}

void solve3(){
    int s=0;
    auto myfunc=[&s]{
        for(int i=0;i<10000000;++i){
            s++;
        }
    };
    std:: thread t1(myfunc);
    std:: thread t2(myfunc);
    t1.join();
    t2.join();
    std:: cout<<s<<std:: endl;
    return ;
}
int main(int argc, char const *argv[])
{
   solve1();
   solve2();
   solve3();
    return 0;
}
