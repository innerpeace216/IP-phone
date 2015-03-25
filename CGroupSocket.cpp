CGroupSocket.cpp
// 用于组呼的套接字类的实现
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CGroupSocket.h"
#include <WS2tcpip.h>
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
struct netunit/////netunit结构体用于在读配置文件时将本机IP地址与本机电话号码以及本机所在组号对应起来
{
	char ip[18];
	char telenum[6];
	char groupnum[4];
};
int CGroupSocket::Initialize()////加载winsock库
{
	WSADATA wsd;

	return WSAStartup(MAKEWORD(2,2), &wsd);
}

int CGroupSocket::CleanUp()/////释放winsock分配的资源
{
	return WSACleanup();
}

CGroupSocket::CGroupSocket(SOCKET s)///创建套接字
{
	m_socket = s;
}

CGroupSocket::CGroupSocket()
{
	m_socket = INVALID_SOCKET;
}

CGroupSocket::~CGroupSocket()
{
}

int CGroupSocket::Create()///创建套接字
{

	m_socket=WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_MULTIPOINT_C_LEAF|WSA_FLAG_MULTIPOINT_D_LEAF|WSA_FLAG_OVERLAPPED);
	return m_socket;
}

int CGroupSocket::Bind(u_short port )/////绑定在8000这个端口
{
	m_local.sin_addr.s_addr =inet_addr(INADDR_ANY);
	m_local.sin_family = AF_INET;
	m_local.sin_port = ::ntohs(8000);
	int b=::bind(m_socket, (struct sockaddr*)&m_local, sizeof(m_local));////将套接字绑定在本地8000这个端口
	BOOL bFlag=TRUE;
	::setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag));
	SOCKADDR_IN remote;
	remote.sin_family=AF_INET;
	CString groupaddr="224.0.0.";////设置多播组地址的前几位
	int intnum=getgroupnum();///获得所在的组号
	CString num;
	num.Format("%d", intnum);
	groupaddr+=num;////得到完整的多播组地址
	remote.sin_addr.s_addr=inet_addr(groupaddr);
	remote.sin_port=::htons(8000);
	m_socketM=WSAJoinLeaf(m_socket,(SOCKADDR*)&remote,sizeof(remote),NULL,NULL,NULL,NULL,JL_BOTH);////加入该多播组 
	return b;

}
int CGroupSocket::Send(const char *pBuf, int len)/////用于将数据发到指定多播组
{
	//m_local.sin_addr.s_addr = inet_addr("127.0.0.1");;
	//m_local.sin_family = AF_INET;
	//m_local.sin_port = ::htons(8888);
	//bind(m_socket, (struct sockaddr*)&m_local, sizeof(m_local));
	BOOL bFlag=TRUE;
	::setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag));
	SOCKADDR_IN remote;
	remote.sin_family=AF_INET;
	CString groupaddr="224.0.0.";////设置多播组地址的前几位
	int intnum=getgroupnum();///获得所在的组号
	CString num;
	num.Format("%d", intnum);
	groupaddr+=num;////得到完整的多播组地址
	remote.sin_addr.s_addr=inet_addr(groupaddr);
	remote.sin_port=::htons(8000);
	m_socketM=WSAJoinLeaf(m_socket,(SOCKADDR*)&remote,sizeof(remote),NULL,NULL,NULL,NULL,JL_BOTH)==INVALID_SOCKET;////加入该多播组
	int b= ::sendto(m_socket, pBuf, len, 0, (sockaddr*)&remote, sizeof(remote));///向指定多播组发送数据
	return b;
}

int CGroupSocket::Recv(char *pbuf, int len)////接收所在多播组的数据
{  
	BOOL bFlag=TRUE;
	::setsockopt(m_socket,SOL_SOCKET,SO_REUSEADDR,(char*)&bFlag,sizeof(bFlag));
	SOCKADDR_IN remote;
	remote.sin_family=AF_INET;
	CString groupaddr="224.0.0.";////设置多播组地址的前几位
	int intnum=getgroupnum();///获得所在的组号
	CString num;
	num.Format("%d", intnum);
	groupaddr+=num;////得到完整的多播组地址
	remote.sin_addr.s_addr=inet_addr(groupaddr);
	remote.sin_port=::htons(8000);
	m_socketM=WSAJoinLeaf(m_socket,(SOCKADDR*)&remote,sizeof(remote),NULL,NULL,NULL,NULL,JL_BOTH); ////加入多播组
	SOCKADDR_IN from;
	int ret;
	int lena=sizeof(from);
	ret=::recvfrom(m_socket,pbuf,len,0,(SOCKADDR*)&from,&lena);////接收数据
	char *k=inet_ntoa(from.sin_addr);
	m_address=from.sin_addr.S_un.S_addr;////得到数据源的地址
	return ret;
}

int	 CGroupSocket::Close()/////关闭相应套接字
{
	int ret;
	ret = shutdown(m_socket,2);
	return closesocket(m_socket);
	return closesocket(m_socketM);

}
DWORD CGroupSocket::getmyaddress()////得到本地IP地址
{
	char name[50];
	ZeroMemory(name,sizeof(name));
	gethostname(name,50);
	struct addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	struct addrinfo *res;
	getaddrinfo(name, NULL, &hints, &res);
	struct sockaddr_in* pSockaddr=(sockaddr_in*)res->ai_addr;
	char *pIP=inet_ntoa(pSockaddr->sin_addr);
	DWORD Localhost_IP=inet_addr(pIP);
	return Localhost_IP;
}
int CGroupSocket::getgroupnum()////得到本机所在组的组号
{
	netunit ipnum;
	FILE *fp;
	fp=fopen("config.txt","r"); 
	DWORD mynum=getmyaddress();////得到本地IP地址
	while (!feof(fp))
	{
		fscanf(fp,"%s %s %s",&ipnum.ip,&ipnum.telenum,&ipnum.groupnum);
		if (inet_addr(ipnum.ip)==mynum)////如果读配置文件读到的地址与本地IP地址相等，则跳出循环
			break;
	}
	int num=atoi(ipnum.groupnum);
	fclose(fp);///关闭文件
	return num; 
}
