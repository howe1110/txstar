#include "tx_worker.h"
class tx_monitor : public tx_worker
{
private:
    /* data */
private:
    tx_monitor(/* args */);

public:
    ~tx_monitor();

public:
    static tx_monitor &instance();

public:
    void stop();

public:
    void handleMessage(txmsgptr pMsg);
    void handleTimer(txmsgptr pMsg);
};
