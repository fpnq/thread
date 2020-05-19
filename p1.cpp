#include<iostream>
#include<thread>
int main(int argc, char const *argv[])
{
    std:: thread t1([]{for( int i=0;i<50;++i)cout<<i<<endl;});
    std:: thread t2([]{for( int i=0;i<50;++i)cout<<i<<endl;});
    t1.join();
    t2.join();
    return 0;
}
