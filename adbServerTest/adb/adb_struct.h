#ifndef _ADB_STRUCT_H_
#define _ADB_STRUCT_H_

//adb �õ��Ľṹ
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/lockfree/queue.hpp>	//��������
#include <boost/atomic.hpp>	//ԭ�Ӳ���
#include <string>	
#include <set>
#include <vector>
#include "sysdeps.h"

#define boost_second_get(t)			(boost::get_system_time() + boost::posix_time::seconds(t))
#define boost_millisecond_get(t)	(boost::get_system_time() + boost::posix_time::milliseconds(t))

void bip_apacke_del_all(void* p);	//apacket���� consume_all ����ɾ����

typedef std::vector<unsigned char>	bufferstream;
typedef	std::string					sstring;
typedef	boost::lockfree::queue<void*>	BipPtrList, *BipLpList;
typedef boost::atomic_int				_aInt;					//ԭ�����Ͳ��� ��������
typedef boost::atomic<char*>			_achar;
typedef boost::atomic<long>				_along;

typedef boost::mutex								adb_buff_mutex;
typedef boost::mutex::scoped_lock					adb_mutex_lock;	//��
typedef boost::system_time							adb_time;		//ʱ��

typedef adb_buff_mutex								adb_usb_mutex;
typedef adb_mutex_lock								adb_write_lock;	//��

#define ADB_MUTEX_DEFINE(x)		adb_buff_mutex x

extern adb_usb_mutex	log_lock;// ��־�����

#define delay(ms) boost::thread::sleep(boost::get_system_time()+boost::posix_time::milliseconds(ms)); //�ӳ�  ����

#define adb_sleep_ms(ms)	delay(ms)

//--------------------------�豸����״̬ID----------------------------------------
#define CS_ANY       -1
#define CS_OFFLINE    0
#define CS_BOOTLOADER 1
#define CS_DEVICE     2
#define CS_HOST       3
#define CS_RECOVERY   4
#define CS_NOPERM     5 /* Insufficient permissions to communicate with the device */
#define CS_SIDELOAD   6

//------------------------------���ݴ��ݵĽṹ ����-------------------------------

#define MAX_PAYLOAD 4096

#define A_SYNC 0x434e5953
#define A_CNXN 0x4e584e43
#define A_OPEN 0x4e45504f
#define A_OKAY 0x59414b4f
#define A_CLSE 0x45534c43
#define A_WRTE 0x45545257
#define A_AUTH 0x48545541

#define A_VERSION 0x01000000        // ADB protocol version

#define ADB_VERSION_MAJOR 1         // Used for help/version information
#define ADB_VERSION_MINOR 0         // Used for help/version information

#define ADB_SERVER_VERSION    31    // Increment this when we want to force users to start a new adb server

typedef struct amessage{
	unsigned command;       /* command identifier constant      */
	unsigned arg0;          /* first argument                   */
	unsigned arg1;          /* second argument                  */
	unsigned data_length;   /* length of payload (0 is allowed) */
	unsigned data_check;    /* checksum of data payload         */
	unsigned magic;         /* command ^ 0xffffffff             */
}amessage;

typedef struct apacket
{
	//apacket *next;

	unsigned len;
	unsigned char *ptr;

	amessage msg;
	unsigned char data[MAX_PAYLOAD];
}apacket, *apacketPtr;

typedef	boost::lockfree::queue<apacket*>	apacketlist, *apacketlistptr;	//���ݰ����� �û� ѭ������ʹ��

//-----------------------------------��д���� �ṹ ����-------------------------
#define  BIP_BUFFER_SIZE   4096
#define  BIPD(x)        do {} while (0)
#define  BIPDUMP(p,l)   BIPD(p)

typedef enum bipbuffer_type {	//���ݽ���ʱ ������������
	bipbuffer_close = -1,
	bipbuffer_shell_sync,
	bipbuffer_shell_async,		//����ǽ���ʽshell:
	bipbuffer_file_sync,		//�ļ�����
	bipbuffer_forward,			//ת��
} bipbuffer_type;

typedef struct BipBufferRec_
{
	int                fdin;
	int                fdout;
	_aInt			   closed;
	BipLpList		   ptrlist;	//��������ָ�� ���� ���� ����
} BipBufferRec, *BipBuffer;

typedef struct SocketPairRec_
{
	BipBufferRec  a2b_bip;
	BipBufferRec  b2a_bip;
	int           a_fd;		//��־���ݵ� ���� ���� ͨ��
	int           used;
} SocketPairRec, *SocketPair;

//----------------------------------------�豸��Ϣ�ṹ--------------------------------------------
#define TOKEN_SIZE 20

typedef enum transport_type {
	kTransportUsb,
	kTransportLocal,
	kTransportAny,
	kTransportHost,
} transport_type;

typedef struct usb_handle usb_handle;


#endif	//_ADB_STRUCT_H_
