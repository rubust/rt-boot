/*
 * Copyright (c) 2006-2018, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2017/10/5      Bernard      the first version
 */

#include <stdint.h>
//#include <string.h>

#include <kernel/rthw.h>
#include <kernel/rtthread.h>

#ifdef RT_USING_SIGNALS

#ifndef RT_SIG_INFO_MAX
#define RT_SIG_INFO_MAX 32
#endif

// #define DBG_ENABLE
#define DBG_SECTION_NAME    "SIGN"
#define DBG_COLOR
#define DBG_LEVEL           DBG_LOG
#include <kernel/rtdbg.h>

#define sig_mask(sig_no)    (1u << sig_no)
#define sig_valid(sig_no)   (sig_no >= 0 && sig_no < RT_SIG_MAX)

struct siginfo_node
{
    siginfo_t si;
    struct rt_slist_node list;
};

static struct rt_mempool *_rt_siginfo_pool;
static void _signal_deliver(rt_thread_t tid);
void rt_thread_handle_sig(rt_bool_t clean_state);

static void _signal_default_handler(int signo)
{
    LOG_I("handled signo[%d] with default action.", signo);
    return ;
}

static void _signal_entry(void *parameter)
{
    rt_thread_t tid = rt_thread_self();

    dbg_enter;

    /* handle signal */
    rt_thread_handle_sig(RT_FALSE);

    /* never come back... */
    rt_hw_interrupt_disable();
    /* return to thread */
    tid->sp = tid->sig_ret;
    tid->sig_ret = RT_NULL;

    LOG_D("switch back to: 0x%08x", tid->sp);
    tid->stat &= ~RT_THREAD_STAT_SIGNAL;

    rt_hw_context_switch_to((rt_uint32_t) & (tid->sp));
}

/*
 * To deliver a signal to thread, there are cases:
 * 1. When thread is suspended, function resumes thread and
 * set signal stat;
 * 2. When thread is ready:
 *   - If function delivers a signal to self thread, just handle
 *    it.
 *   - If function delivers a signal to another ready thread, OS
 *    should build a slice context to handle it.
 */
static void _signal_deliver(rt_thread_t tid)
{
    rt_ubase_t level;

    /* thread is not interested in pended signals */
    if (!(tid->sig_pending & tid->sig_mask)) return;

    level = rt_hw_interrupt_disable();
    if ((tid->stat & RT_THREAD_STAT_MASK) == RT_THREAD_SUSPEND)
    {
        /* resume thread to handle signal */
        rt_thread_resume(tid);
        /* add signal state */
        tid->stat |= RT_THREAD_STAT_SIGNAL;

        rt_hw_interrupt_enable(level);

        /* re-schedule */
        rt_schedule();
    }
    else
    {
        if (tid == rt_thread_self())
        {
            /* add signal state */
            tid->stat |= RT_THREAD_STAT_SIGNAL;

            rt_hw_interrupt_enable(level);

            /* do signal action in self thread context */
            rt_thread_handle_sig(RT_TRUE);
        }
        else if (!((tid->stat & RT_THREAD_STAT_MASK) & RT_THREAD_STAT_SIGNAL))
        {
            /* add signal state */
            tid->stat |= RT_THREAD_STAT_SIGNAL;

            /* point to the signal handle entry */
            tid->sig_ret = tid->sp;
            tid->sp = rt_hw_stack_init((void *)_signal_entry, RT_NULL,
                                       (void *)((char *)tid->sig_ret - 32), RT_NULL);

            rt_hw_interrupt_enable(level);
            LOG_D("signal stack pointer @ 0x%08x", tid->sp);

            /* re-schedule */
            rt_schedule();
        }
        else
        {
            rt_hw_interrupt_enable(level);
        }
    }
}

rt_sighandler_t rt_signal_install(int signo, rt_sighandler_t handler)
{
    rt_sighandler_t old = RT_NULL;
    rt_thread_t tid = rt_thread_self();

    if (!sig_valid(signo)) return SIG_ERR;

    rt_enter_critical();
    if (tid->sig_vectors == RT_NULL)
    {
        rt_thread_alloc_sig(tid);
    }

    if (tid->sig_vectors)
    {
        old = tid->sig_vectors[signo];

        if (handler == SIG_IGN) tid->sig_vectors[signo] = RT_NULL;
        else if (handler == SIG_DFL) tid->sig_vectors[signo] = _signal_default_handler;
        else tid->sig_vectors[signo] = handler;
    }
    rt_exit_critical();

    return old;
}

void rt_signal_mask(int signo)
{
    rt_base_t level;
    rt_thread_t tid = rt_thread_self();

    level = rt_hw_interrupt_disable();

    tid->sig_mask &= ~sig_mask(signo);

    rt_hw_interrupt_enable(level);
}

void rt_signal_unmask(int signo)
{
    rt_base_t level;
    rt_thread_t tid = rt_thread_self();

    level = rt_hw_interrupt_disable();

    tid->sig_mask |= sig_mask(signo);

    /* let thread handle pended signals */
    if (tid->sig_mask & tid->sig_pending)
    {
        rt_hw_interrupt_enable(level);
        _signal_deliver(tid);
    }
    else
    {
        rt_hw_interrupt_enable(level);
    }
}

int rt_signal_wait(const rt_sigset_t *set, rt_siginfo_t *si, rt_int32_t timeout)
{
    int ret = RT_EOK;
    rt_base_t   level;
    rt_thread_t tid = rt_thread_self();
    struct siginfo_node *si_node = RT_NULL, *si_prev = RT_NULL;

    /* current context checking */
    RT_DEBUG_IN_THREAD_CONTEXT;

    /* parameters check */
    if (set == RT_NULL || *set == 0 || si == RT_NULL )
    {
        ret = -RT_EINVAL;
        goto __done_return;
    }

    /* clear siginfo to avoid unknown value */
    rt_memset(si, 0x0, sizeof(rt_siginfo_t));

    level = rt_hw_interrupt_disable();

    /* already pending */
    if (tid->sig_pending & *set) goto __done;

    if (timeout == 0)
    {
        ret = -RT_ETIMEOUT;
        goto __done_int;
    }

    /* suspend self thread */
    rt_thread_suspend(tid);
    /* set thread stat as waiting for signal */
    tid->stat |= RT_THREAD_STAT_SIGNAL_WAIT;

    /* start timeout timer */
    if (timeout != RT_WAITING_FOREVER)
    {
        /* reset the timeout of thread timer and start it */
        rt_timer_control(&(tid->thread_timer),
                         RT_TIMER_CTRL_SET_TIME,
                         &timeout);
        rt_timer_start(&(tid->thread_timer));
    }
    rt_hw_interrupt_enable(level);

    /* do thread scheduling */
    rt_schedule();

    level = rt_hw_interrupt_disable();

    /* remove signal waiting flag */
    tid->stat &= ~RT_THREAD_STAT_SIGNAL_WAIT;

    /* check errno of thread */
    if (tid->error == -RT_ETIMEOUT)
    {
        tid->error = RT_EOK;
        rt_hw_interrupt_enable(level);

        /* timer timeout */
        ret = -RT_ETIMEOUT;
        goto __done_return;
    }

__done:
    /* to get the first matched pending signals */
    si_node = (struct siginfo_node *)tid->si_list;
    while (si_node)
    {
        int signo;

        signo = si_node->si.si_signo;
        if (sig_mask(signo) & *set)
        {
            *si  = si_node->si;

            LOG_D("sigwait: %d sig raised!", signo);
            if (si_prev) si_prev->list.next = si_node->list.next;
            else tid->si_list = si_node->list.next;

            /* clear pending */
            tid->sig_pending &= ~sig_mask(signo);
            rt_mp_free(si_node);
            break;
        }

        si_prev = si_node;
        si_node = (void *)rt_slist_entry(si_node->list.next, struct siginfo_node, list);
     }

__done_int:
    rt_hw_interrupt_enable(level);

__done_return:
    return ret;
}

void rt_thread_handle_sig(rt_bool_t clean_state)
{
    rt_base_t level;

    rt_thread_t tid = rt_thread_self();
    struct siginfo_node *si_node;

    level = rt_hw_interrupt_disable();
    if (tid->sig_pending & tid->sig_mask)
    {
        /* if thread is not waiting for signal */
        if (!(tid->stat & RT_THREAD_STAT_SIGNAL_WAIT))
        {
            while (tid->sig_pending & tid->sig_mask)
            {
                int signo, error;
                rt_sighandler_t handler;

                si_node = (struct siginfo_node *)tid->si_list;
                if (!si_node) break;

                /* remove this sig info node from list */
                if (si_node->list.next == RT_NULL)
                    tid->si_list = RT_NULL;
                else
                    tid->si_list = (void *)rt_slist_entry(si_node->list.next, struct siginfo_node, list);

                signo   = si_node->si.si_signo;
                handler = tid->sig_vectors[signo];
                rt_hw_interrupt_enable(level);

                LOG_D("handle signal: %d, handler 0x%08x", signo, handler);
                if (handler) handler(signo);

                level = rt_hw_interrupt_disable();
                tid->sig_pending &= ~sig_mask(signo);
                error = -RT_EINTR;

                rt_mp_free(si_node); /* release this siginfo node */
                /* set errno in thread tcb */
                tid->error = error;
            }

            /* whether clean signal status */
            if (clean_state == RT_TRUE) tid->stat &= ~RT_THREAD_STAT_SIGNAL;
        }
    }

    rt_hw_interrupt_enable(level);
}

void rt_thread_alloc_sig(rt_thread_t tid)
{
    int index;
    rt_base_t level;
    rt_sighandler_t *vectors;

    vectors = (rt_sighandler_t *)RT_KERNEL_MALLOC(sizeof(rt_sighandler_t) * RT_SIG_MAX);
    RT_ASSERT(vectors != RT_NULL);

    for (index = 0; index < RT_SIG_MAX; index ++)
    {
        vectors[index] = _signal_default_handler;
    }

    level = rt_hw_interrupt_disable();
    tid->sig_vectors = vectors;
    rt_hw_interrupt_enable(level);
}

void rt_thread_free_sig(rt_thread_t tid)
{
    rt_base_t level;
    struct siginfo_node *si_list;
    rt_sighandler_t *sig_vectors;

    level = rt_hw_interrupt_disable();
    si_list = (struct siginfo_node *)tid->si_list;
    tid->si_list = RT_NULL;

    sig_vectors = tid->sig_vectors;
    tid->sig_vectors = RT_NULL;
    rt_hw_interrupt_enable(level);

    if (si_list)
    {
        struct rt_slist_node *node;
        struct siginfo_node  *si_node;

        LOG_D("free signal info list");
        node = &(si_list->list);
        do
        {
            si_node = rt_slist_entry(node, struct siginfo_node, list);
            rt_mp_free(si_node);

            node = node->next;
        } while (node);
    }

    if (sig_vectors)
    {
        RT_KERNEL_FREE(sig_vectors);
    }
}

int rt_thread_kill(rt_thread_t tid, int sig)
{
    siginfo_t si;
    rt_base_t level;
    struct siginfo_node *si_node;

    RT_ASSERT(tid != RT_NULL);
    if (!sig_valid(sig)) return -RT_EINVAL;

    LOG_I("send signal: %d", sig);
    si.si_signo = sig;
    si.si_code  = SI_USER;
    si.si_value.sival_ptr = RT_NULL;

    level = rt_hw_interrupt_disable();
    if (tid->sig_pending & sig_mask(sig))
    {
        /* whether already emits this signal? */
        struct rt_slist_node *node;
        struct siginfo_node  *entry;

        node = (struct rt_slist_node *)tid->si_list;
        rt_hw_interrupt_enable(level);

        /* update sig info */
        rt_enter_critical();
        for (; (node) != RT_NULL; node = node->next)
        {
            entry = rt_slist_entry(node, struct siginfo_node, list);
            if (entry->si.si_signo == sig)
            {
                rt_memcpy(&(entry->si), &si, sizeof(siginfo_t));
                rt_exit_critical();
                return 0;
            }
        }
        rt_exit_critical();

        /* disable interrupt to protect tcb */
        level = rt_hw_interrupt_disable();
    }
    else
    {
        /* a new signal */
        tid->sig_pending |= sig_mask(sig);
    }
    rt_hw_interrupt_enable(level);

    si_node = (struct siginfo_node *) rt_mp_alloc(_rt_siginfo_pool, 0);
    if (si_node)
    {
        rt_slist_init(&(si_node->list));
        rt_memcpy(&(si_node->si), &si, sizeof(siginfo_t));

        level = rt_hw_interrupt_disable();
        if (!tid->si_list) tid->si_list = si_node;
        else
        {
            struct siginfo_node *si_list;

            si_list = (struct siginfo_node *)tid->si_list;
            rt_slist_append(&(si_list->list), &(si_node->list));
        }
        rt_hw_interrupt_enable(level);
    }
    else
    {
        LOG_E("The allocation of signal info node failed.");
    }

    /* deliver signal to this thread */
    _signal_deliver(tid);

    return RT_EOK;
}

int rt_system_signal_init(void)
{
    _rt_siginfo_pool = rt_mp_create("signal", RT_SIG_INFO_MAX, sizeof(struct siginfo_node));
    if (_rt_siginfo_pool == RT_NULL)
    {
        LOG_E("create memory pool for signal info failed.");
        RT_ASSERT(0);
    }

    return 0;
}

#endif
