#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include <ucontext.h>

#define STACKSZ (1024*64)
#define CORSZ 1024 //协程数量

struct schedule;

enum State
{
    DEAD, READY, RUNNING, SUSPEND
};

//协程结构体
typedef struct
{
    void *(*call_back)(struct schedule *s, void* args); //协程回调函数
    void* args;  //回调函数参数
    ucontext_t ctx; //协程上下文
    char stack[STACKSZ]; //协程栈 （每个协程独有）
    enum State state;  // 协程状态
}coroutine_t;

//协程调度器
typedef struct schedule
{
    coroutine_t **coroutines;  //所有协程
    int current_id; //当前运行的协程（上一行二级指针的下表）
    int max_id; //最大下标
    ucontext_t ctx_main; //主流程上下文
}schedule_t;

// 创建调度器
schedule_t *schedule_create();

// 创建协程，返回当前协程在调度器的下标
int coroutine_create(schedule_t *s, void *(*call_back)(schedule_t *, void *args), void *args);

// 启动协程
void coroutine_running(schedule_t *s, int id);

// 让出CPU
void coroutine_yield(schedule_t *s);

// 恢复CPU
void coroutine_resume(schedule_t *s, int id);

// 释放调度器
void schedule_destroy(schedule_t *s);

// 判断是否所有协程都运行完了
int schedule_finished(schedule_t *s);

#endif
