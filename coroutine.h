#ifndef _COROUTINE_H_
#define _COROUTINE_H_

#include <ucontext.h>

#define STACKSZ (1024*64)
#define CORSZ 1024 //Э������

struct schedule;

enum State
{
    DEAD, READY, RUNNING, SUSPEND
};

//Э�̽ṹ��
typedef struct
{
    void *(*call_back)(struct schedule *s, void* args); //Э�̻ص�����
    void* args;  //�ص���������
    ucontext_t ctx; //Э��������
    char stack[STACKSZ]; //Э��ջ ��ÿ��Э�̶��У�
    enum State state;  // Э��״̬
}coroutine_t;

//Э�̵�����
typedef struct schedule
{
    coroutine_t **coroutines;  //����Э��
    int current_id; //��ǰ���е�Э�̣���һ�ж���ָ����±�
    int max_id; //����±�
    ucontext_t ctx_main; //������������
}schedule_t;

// ����������
schedule_t *schedule_create();

// ����Э�̣����ص�ǰЭ���ڵ��������±�
int coroutine_create(schedule_t *s, void *(*call_back)(schedule_t *, void *args), void *args);

// ����Э��
void coroutine_running(schedule_t *s, int id);

// �ó�CPU
void coroutine_yield(schedule_t *s);

// �ָ�CPU
void coroutine_resume(schedule_t *s, int id);

// �ͷŵ�����
void schedule_destroy(schedule_t *s);

// �ж��Ƿ�����Э�̶���������
int schedule_finished(schedule_t *s);

#endif
