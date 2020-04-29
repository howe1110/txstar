#include "tx_monitor.h"
#include "tx_env.h"
#include <csignal>


tx_monitor &tx_monitor::instance()
{
    static tx_monitor insminitor;
    return insminitor;
}

tx_monitor::tx_monitor(/* args */) : tx_worker("tx_monitor")
{
}

tx_monitor::~tx_monitor()
{
}

void tx_monitor::handleMessage(txmsgptr pMsg)
{
    switch (pMsg->msgid)
    {
    case msgid_timerevent /* constant-expression */:
        /* code */
        handleTimer(pMsg);
        break;

    default:
        break;
    }
}

void tx_monitor::handleTimer(txmsgptr pMsg)
{
    for (auto pos = _worker_table.begin(); pos != _worker_table.end(); ++pos)
    {
        if (pos->second == nullptr)
        {
            continue;
        }
        tx_worker *tw = dynamic_cast<tx_worker *>(pos->second);
        if (tw == nullptr)
        {
            continue;
        }
        if (tw->isdead()) //喂狗超时，需要退出进程。
        {
            logcritical("Worker {%s} is timeout, so exit application.");
            raise(SIGTERM);
        }
    }
}

void tx_monitor::stop()
{
    for (auto pos = _worker_table.begin(); pos != _worker_table.end(); ++pos)
    {
        if (pos->second == nullptr)
        {
            continue;
        }
        if (pos->second == this)
        {
            continue;
        }
        pos->second->stop();
    }
    tx_worker_base::stop();
}
