#include "CommonDef.h"
#include <string>
#include <map>
#include "tx_msg.h"
#include "tx_base.h"
#include "comworker.h"
#include "listen_worker.h"

const IDtype INVALID_NODEID = 0;
const unsigned int RING_BIT_SIZE = 32;

struct NodeAddr
{
    NodeAddr()
    {
        id = INVALID_NODEID;
        address = "";
        port = "";
    }
    NodeAddr(IDtype pid, std::string paddr, std::string pport)
    {
        id = pid;
        address = paddr;
        port = pport;
    }
    IDtype id;
    std::string address;
    std::string port;
};

class StarNode : public comworker
{
private:
    /* data */
    NodeAddr _predecessor;
    NodeAddr _successor;
    NodeAddr _localaddr;
    IDtype _id;
    std::string _name;
    std::map<unsigned int, NodeAddr> _fingertable;

private:
    listen_worker _listener;

public:
    static StarNode &instance();

private:
    void handlemessage(tlinkptr plink, ptxmsg pMsg);

public:
    void Start();
    void Stop();

public:
    void Show();

public:
    StarNode(/* args */);
    ~StarNode();

public:
    virtual void handleSuccessorReq(tlinkptr plink, const txmsg *msg);
    virtual void handleSuccessorReq2(tlinkptr plink, const txmsg *msg);
    virtual void handleJoinReq(tlinkptr plink, const txmsg *msg);
    virtual void handleStabilizeReq(tlinkptr plink, const txmsg *msg);

private:
    void handleSuccessorRsp(const txmsg *msg);
    void handleSuccessorNty(const txmsg *msg);
    void Send2Node(NodeAddr addr, const void *buf, const txIdType mt, const size_t datalen);
    void Send2Node(NodeAddr addr, const txmsg *msg);

public:
    void StartJoin(const std::string &server, const std::string &port);
    void join(IDtype id);
    void exit();
    void stabilization();
    void RequestSuccessor(std::string addr, std::string port);

private:
    void check();
    NodeAddr findSuccessor(IDtype);
    void fixFingerTable();
    NodeAddr getClosestNodeInFingerTable(IDtype id);
};
