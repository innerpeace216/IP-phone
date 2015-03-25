CPointSocket.cpp
// 用于单呼的套接字类的实现
//////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CPointSocket.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
int CPointSocket::Initialize()
{
	WSADATA wsd;

	return WSAStartup(MAKEWORD(2,2), &wsd);//////加载winsock库
}

int CPointSocket::CleanUp()
{
	return WSACleanup();//释放winsock分配的资源
}

CPointSocket::CPointSocket(SOCKET s)
{
	m_socket = s;
}

CPointSocket::CPointSocket()
{
	m_socket = INVALID_SOCKET;
}

CPointSocket::~CPointSocket()
{
}

int CPointSocket::Create()
{
	m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);////创建套接字

	return m_socket;
}

int CPointSocket::Bind(u_short port /* = DEFAULT_PORT */)/////绑定套接字
{
	m_local.sin_addr.s_addr = htonl(INADDR_ANY);
	m_local.sin_family = AF_INET;
	m_local.sin_port = port;

	return bind(m_socket, (struct sockaddr*)&m_local, sizeof(m_local));
}

int CPointSocket::Listen(int backlog)///将套接字设为监听模式
{
	return listen(m_socket, backlog);
}

SOCKET CPointSocket::Accept(sockaddr* addr , int* addrlen )/////接收连接请求
{
	return accept(m_socket, addr, addrlen);
}

int CPointSocket::Connect(DWORD ip, u_short port /* = DEFAULT_PORT */)///发起连接请求
{
	m_foreign.sin_addr.s_addr = ip;
	m_foreign.sin_family = AF_INET;
	m_foreign.sin_port = port;
	return connect(m_socket, (struct sockaddr*)&m_foreign, sizeof(m_foreign));
}

int CPointSocket::Send(const char *pBuf, int len)///发送数据
{
	return send(m_socket, pBuf, len, 0);
}

int CPointSocket::Recv(char *pbuf, int len)///接收数据
{
	return recv(m_socket, pbuf, len, 0);
}

int	 CPointSocket::Close()/////关闭套接字
{
	int ret;
	ret = shutdown(m_socket,2);
	return closesocket(m_socket);
}
