#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <thread>

#define private public // hack complier
#define protected public
#include "Hash.cpp"
#include "tx_ref.h"
#include "tx_base.cpp"
#include "devinf.cpp"
#include "tcpcomm.cpp"
#include "tx_worker.cpp"
#include "tx_link.cpp"
#include "comworker.cpp"
#include "listen_worker.cpp"
#include "tcpclient.cpp"
#include "tx_timer.cpp"

using ::testing::_;
using ::testing::AtLeast; // #1
using testing::ByRef;
using ::testing::DoAll;
using ::testing::Return;
using testing::SaveArg;
using testing::SaveArgPointee;
using testing::SetArgPointee;
using testing::SetArgReferee;

const std::string testPort = "66666";
const int testSocketID = 12345;
const int testDataLen = 4096;

class Mocknetwork : public devinf
{
public:
  MOCK_METHOD(int, connectI, (int sockfd, const struct sockaddr *addr, socklen_t addrlen), (override));
  MOCK_METHOD(int, recvI, (int sockfd, void *buf, size_t len, int flags), (override));
  MOCK_METHOD(int, sendI, (int sockfd, const void *buf, size_t len, int flags), (override));
  //
  MOCK_METHOD(int, getaddrinfoI, (const char *node, const char *service, const struct addrinfo *hints, struct addrinfo **res), (override));
  MOCK_METHOD(void, freeaddrinfoI, (struct addrinfo * res), (override));
  MOCK_METHOD(int, socketI, (int domain, int type, int protocol), (override));
  MOCK_METHOD(int, setsocketblock, (int s, bool block), (override));
  MOCK_METHOD(int, bindI, (int sockfd, const struct sockaddr *addr, socklen_t addrlen), (override));
  MOCK_METHOD(int, listenI, (int sockfd, int backlog), (override));
  MOCK_METHOD(int, selectI, (int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout), (override));
  MOCK_METHOD(int, acceptI, (int sockfd, struct sockaddr *addr, socklen_t *addrlen), (override));
  MOCK_METHOD(int, shutdownI, (int sockfd, int how), (override));
  MOCK_METHOD(int, closesocketI, (int fd), (override));
  MOCK_METHOD(int, setsocketreuseaddr, (int fd), (override));
};

class tx_worker_test : public tx_worker
{
private:
public:
  tx_worker_test(std::string n) : tx_worker(n) {}
  ~tx_worker_test() {}

public:
  MOCK_METHOD(void, handleTest, (txmsgptr pMsg), ());
  MOCK_METHOD(void, handleTimer, (txmsgptr pMsg), ());
  void handleMessage(txmsgptr pMsg)
  {
    switch (pMsg->msgid)
    {
    case msgid_test_event /* constant-expression */:
      /* code */
      handleTest(pMsg);
      break;
    case msgid_timerevent /* constant-expression */:
      /* code */
      handleTimer(pMsg);
      break;

    default:
      break;
    }
  }
};

TEST(worker_test, testhandlemessage)
{
  int *pval = new int(1024);
  txmsg *msg = txmsg::Create(pval, msgid_test_event, sizeof(int));
  txmsgptr msgptr(msg);

  tx_worker_test tt("tt");
  EXPECT_CALL(tt, handleTest)
      .Times(1);

  tt.start();
  tt.postmessage(msgptr);
  std::this_thread::sleep_for(std::chrono::seconds(1));
  tt.stop();
}

TEST(worker_test, testsendmessage)
{
  int *pval = new int(1024);
  txmsg *msg = txmsg::Create(pval, msgid_test_event, sizeof(int));
  txmsgptr msgptr(msg);
  tx_worker_test t1("t1"), t2("t2");

  EXPECT_CALL(t2, handleTest)
      .Times(1);

  t1.start();
  t2.start();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  t1.SendMessage(t2.getid(), msgptr);
  std::this_thread::sleep_for(std::chrono::seconds(1));

  t1.stop();
  t2.stop();
}

class listen_worker_mock : public listen_worker
{
private:
public:
  listen_worker_mock() {}
  ~listen_worker_mock() {}

public:
};

class com_worker_mock : public comworker
{
private:
public:
  com_worker_mock() {}
  ~com_worker_mock() {}

public:
  void handlemessage(tlinkptr plink, ptxmsg pMsg)
  {
    Trace("handle message.");
    int result = 234;
    plink->SendData(&result, msgtype_shakehand, sizeof(result));
    return;
  }
};

bool InitNetwork()
{
  tcpcomm *pcomm = new tcpcomm();
  if (pcomm == nullptr)
  {
    return false;
  }
  SetincInstance(pcomm);
  return true;
}

void RestNetwork()
{
  ResetincInstance();
}

class com_test : public ::testing::Test
{
public:
  void SetUp() override
  {
    InitNetwork();
  }

  void TearDown() override
  {
    RestNetwork();
  }
  listen_worker_mock listener;
  com_worker_mock cwk;
};

TEST_F(com_test, test_comworker)
{
  cwk.start();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  cwk.stop();
}

TEST_F(com_test, test_listen)
{
  listener.start();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  listener.stop();
}

TEST_F(com_test, test_accept)
{
  com_worker_mock cwk;
  listener.setcommwoker(&cwk);

  cwk.start();
  listener.start();

  std::this_thread::sleep_for(std::chrono::seconds(1));
  txcomclient client;

  bool bRet = client.Connect("192.168.126.141", "55555");
  ASSERT_EQ(bRet, true);

  int request = 123;
  txcomresponse rst = client.Request(&request, msgtype_shakehand, sizeof(request));

  std::this_thread::sleep_for(std::chrono::seconds(1));

  ASSERT_EQ(rst.isNullPtr(), false);

  int val = *(int *)&rst->data[0];
  EXPECT_EQ(val, 234);

  listener.stop();
  cwk.stop();
}

class tx_timer_test : public ::testing::Test
{
public:
  void SetUp() override
  {
    _timer.start();
  }

  void TearDown() override
  {
    _timer.stop();
  }
  tx_timer _timer;
};

TEST_F(tx_timer_test, timer_event_test)
{
  tx_worker_test _wk("timerhandler");

  EXPECT_CALL(_wk, handleTimer)
      .Times(10);

  int para = 1024;
  _wk.regtimer(100, &para);
  _wk.start();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  _wk.stop();
}

#undef private
#undef protected
