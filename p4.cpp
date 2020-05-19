#include <iostream>
#include <thread>

#define MAX_SIZE 50000000
typedef struct
{
    int first;
    int last;
    int result;
} MY_ARGS;

int main(int argc, char const *argv[])
{
    int *arr = new int[MAX_SIZE];
    int results[2] = {0, 0};
    for (int i = 0; i < MAX_SIZE; ++i)
    {
        arr[i] = rand() % 50;
    }
    MY_ARGS args1 = {0, MAX_SIZE / 2, 0};
    MY_ARGS args2={MAX_SIZE/2,MAX_SIZE,1);
    for（int i＝0；i＜5000；i＋＋)arr［i］＝rand（)％5；
＃if 1／／example5
    auto myfunc＝［＆arr，＆results］（MY＿ARGS＊args)｛int s＝0；for（int i＝args－＞first；i＜args－＞last；i＋＋，results［args－＞id］＋＝arr［i］)；｝；
＃else／／example6
    auto myfunc＝［＆arr］（MY＿ARGS＊args)｛int s＝0；for（int i＝args－＞first；i＜args－＞last；i＋＋，s＋＝arr［i］)；args－＞id＝s；｝；
＃endif
    thread t1（myfunc，＆args1)；
    thread t2（myfunc，＆args2)；
    t1．join（)；t2．join（)；
＃if 1
    cout＜＜＂s1＝＂＜＜results［0］＜＜＂＼ｎs2＝＂＜＜results［1］＜＜＂＼ｎs1＋s2＝＂＜＜results［0］＋results［1］＜＜endl；
    ＃else
    cout＜＜＂s1＝＂＜＜args1．id＜＜＂＼ｎs2＝＂＜＜args2．id＜＜＂＼ｎs1＋s2＝＂＜＜args1．id＋args2．id＜＜endl；
＃endif
    delete［］arr；
    return 0;
}
