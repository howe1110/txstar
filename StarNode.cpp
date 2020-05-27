#include "StarNode.h"
#include "tcpclient.h"
#include <algorithm>
#include <math.h>
#include "tx_env.h"
#include "proto/starmessage.pb.h"

StarNode::StarNode(/* args */)
{
}

StarNode::~StarNode()
{
}

StarNode &StarNode::instance()
{
    static StarNode ins;
    return ins;
}

void StarNode::handlemessage(tlinkptr plink, ptxmsg pMsg)
{
    switch (pMsg->msgid)
    {
    case successor_req:
        handleSuccessorReq(plink, pMsg);
        break;
    case successor_rsp:
        handleSuccessorRsp(pMsg);
        break;
    default:
        break;
    }
}

void StarNode::Start()
{
    start(); //启动消息处理线程
    _listener.setcommwoker(this);
    _listener.start(); //启动侦听功能。
}

void StarNode::Stop()
{
    _listener.stop();
    stop();
}

void StarNode::StartJoin(const std::string &server, const std::string &port)
{
    RequestSuccessor(server, port);
}

void StarNode::handleSuccessorReq(tlinkptr plink, const txmsg *msg)
{
    txstar::successorReq req;
    req.ParseFromArray(msg->data, msg->msglen);
    IDtype id = req.node().nodeid();
    std::string addr = plink->PeerServer();
    std::string port = std::to_string(plink->PeerPort());
    NodeAddr naddr(id, addr, port);
    NodeAddr successor = findSuccessor(id);
    if (INVALID_NODEID == successor.id)
    {
        txstar::nodeinfo n;
        n.set_nodeid(_id);
        n.set_address(_localaddr.address);
        n.set_port(_localaddr.port);
        txstar::successorReq req;
        req.set_allocated_node(&n);

        size_t len = req.ByteSizeLong();
        char *buf = new char[len];

        if (!req.SerializeToArray(buf, len))
        {
            return;
        }
        NodeAddr addr = getClosestNodeInFingerTable(id); //获取最近的节点
        //向下个节点发起请求
        Send2Node(addr, buf, successor_req2, len);
    }
    else
    {
        txstar::nodeinfo n;

        n.set_nodeid(id);
        n.set_address(_localaddr.address);
        n.set_port(_localaddr.port);
        txstar::successorRsp rsp;
        rsp.set_allocated_node(&n);
        size_t len = rsp.ByteSizeLong();
        char *buf = new char[len];

        if (!rsp.SerializeToArray(buf, len))
        {
            return;
        }
        Send2Node(naddr, buf, successor_nty, len);
    }

    txstar::nodeinfo n;

    n.set_nodeid(id);
    n.set_address(_localaddr.address);
    n.set_port(_localaddr.port);
    txstar::successorRsp rsp;
    rsp.set_allocated_node(&n);
    size_t len = rsp.ByteSizeLong();
    char *buf = new char[len];

    if (!rsp.SerializeToArray(buf, len))
    {
        return;
    }
    plink->SendData(buf, successor_rsp, len);
}

void StarNode::handleSuccessorReq2(tlinkptr plink, const txmsg *msg)
{
    txstar::successorReq req;
    req.ParseFromArray(msg->data, msg->msglen);
    IDtype id = req.node().nodeid();
    std::string addr = req.node().address();
    std::string port = req.node().port();
    NodeAddr naddr(id, addr, port);
    NodeAddr successor = findSuccessor(id);
    if (INVALID_NODEID == successor.id)
    {
        NodeAddr addr = getClosestNodeInFingerTable(id); //获取最近的节点
        //向下个节点发起请求
        txmsg *msg2 = txmsg::Clone(msg);
        msg2->msgid = successor_req2;
        Send2Node(addr, msg2);
    }
    else
    {
        txstar::nodeinfo n;

        n.set_nodeid(id);
        n.set_address(_localaddr.address);
        n.set_port(_localaddr.port);
        txstar::successorRsp rsp;
        rsp.set_allocated_node(&n);
        size_t len = rsp.ByteSizeLong();
        char *buf = new char[len];

        if (!rsp.SerializeToArray(buf, len))
        {
            return;
        }
        Send2Node(naddr, buf, successor_nty, len);
    }

    txstar::nodeinfo n;

    n.set_nodeid(id);
    n.set_address(_localaddr.address);
    n.set_port(_localaddr.port);
    txstar::successorRsp rsp;
    rsp.set_allocated_node(&n);
    size_t len = rsp.ByteSizeLong();
    char *buf = new char[len];

    if (!rsp.SerializeToArray(buf, len))
    {
        return;
    }
    plink->SendData(buf, successor_rsp, len);
}

void StarNode::handleJoinReq(tlinkptr plink, const txmsg *msg)
{
}

void StarNode::handleStabilizeReq(tlinkptr plink, const txmsg *msg)
{
}

void StarNode::handleSuccessorRsp(const txmsg *msg)
{
    txstar::successorRsp rsp;
    rsp.ParseFromArray(msg->data, msg->msglen);
}

void StarNode::handleSuccessorNty(const txmsg *msg)
{
    txstar::successorRsp rsp;
    rsp.ParseFromArray(msg->data, msg->msglen);
}

void StarNode::Send2Node(NodeAddr addr, const void *buf, const txIdType mt, const size_t datalen)
{
    txcomclient client;
    if (client.Connect(addr.address, addr.port))
    {
        logerr("Connect to node {%s:%s} failed.", addr.address, addr.port);
        return;
    }
    if (client.Send(buf, mt, datalen))
    {
        logerr("Send to node {%s:%s} failed.", addr.address, addr.port);
    }
}

void StarNode::Send2Node(NodeAddr addr, const txmsg *msg)
{
    Send2Node(addr, msg->data, msg->msgid, msg->msglen);
}

void StarNode::join(IDtype id)
{
}

void StarNode::RequestSuccessor(std::string addr, std::string port)
{
    txstar::nodeinfo n;
    n.set_nodeid(_id);
    n.set_address(_localaddr.address);
    n.set_port(_localaddr.port);
    txstar::successorReq req;
    req.set_allocated_node(&n);

    size_t len = req.ByteSizeLong();
    char *buf = new char[len];

    if (!req.SerializeToArray(buf, len))
    {
        return;
    }
    txcomclient client;
    client.Connect(addr, port);
    txcomresponse psp = client.Request(buf, successor_req, len);
}

void StarNode::exit()
{
}

void StarNode::stabilization()
{
}

void StarNode::Show()
{
    Trace("LocalNode:{%u}\n", _id);
    Trace("Predecessor: {%u}, Successor: {%s}", _predecessor, _successor);
}

NodeAddr StarNode::findSuccessor(IDtype id)
{
    if (_id < id && id < _successor.id)
    {
        return _successor;
    }
    else if (_id > _successor.id) //ring is end. response this node id.
    {
        return _successor;
    }
    else if (_id == _successor.id) //p2p network only have one node.
    {
        return _successor;
    }
    else //not found.
    {
        return NodeAddr();
    }
}

//_fingertable[i] = findSuccessor((_id + Power(2, i))% RING_BIT_SIZE);
void StarNode::fixFingerTable()
{
    for (int i = 0; i < RING_BIT_SIZE; ++i)
    {
        IDtype rec = pow(2, i);
        IDtype next = INVALID_NODEID;
        if (std::numeric_limits<IDtype>::max() - rec > _id)
        {
            next = pow(2, i) - (std::numeric_limits<IDtype>::max() - _id);
        }
        else
        {
            next = _id + rec;
        }

        _fingertable[i] = findSuccessor(next);
        if (_fingertable[i].id == _id) //第一个后继节点是自己的
        {
            break;
        }
    }
    for (int i = i + 1; i < RING_BIT_SIZE; i++) //后续每一个后继节点都是此节点
    {
        _fingertable[i] = _localaddr;
    }
}

NodeAddr StarNode::getClosestNodeInFingerTable(IDtype id)
{
    for (int i = RING_BIT_SIZE - 1; i >= 0; i--) //倒序查找，先大范围查找。
    {
        if (_fingertable[i].id == INVALID_NODEID)
        {
            continue;
        }
        if (_fingertable[i].id == _id)
        {
            /* code */
            continue;
        }
        if (id < _fingertable[i].id && _fingertable[i].id < _id)
        {
            return _fingertable[i];
        }
    }
    return _successor;
}
