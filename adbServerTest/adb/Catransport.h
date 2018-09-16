#ifndef _CATRANSPORT_H_
#define _CATRANSPORT_H_

#include "adb_func.h"
#include "Semaphore.h"
#include <boost/thread/shared_mutex.hpp>
#include <boost/enable_shared_from_this.hpp>

//����Ŀ���ǽ��� ����ָ�� �Զ������ڴ档����˳����Զ��ر�USBͨ�����������ڴ�
//ͨ������kicked �ж��Ƿ��Ѿ��Ͽ�

class Semaphore;	//Ԥ����
//ͨ����ȡʱʹ�õ� ��д��
typedef boost::shared_mutex								bip_shared_mutex;
typedef boost::shared_lock<bip_shared_mutex>			bip_read_lock;
typedef boost::unique_lock<bip_shared_mutex>			bip_write_lock;

typedef struct aclient{
	_aInt		user,			//�Ƿ����û���ʹ�ñ�ͨ��
				biptype,		//ִ������
				local_id,		//��ͨ��ID
				remote_id,		//�뱾��ͨѶ���豸ͨ��ID
				connected,		//����״̬
				closing;		//���ڹر�
	void*		last_apacket;	//ͬ�� �� forward ��ȡ������Ҫ
	BipPtrList	ptrlist;		//�������� ���� ���� ����

	aclient() :last_apacket(NULL), biptype(bipbuffer_close), connected(NULL), closing(NULL), local_id(NULL), remote_id(NULL), user(NULL), ptrlist(0){}
	~aclient(){ //�ͷŶ���
		bip_apacke_del_all(last_apacket);
		ptrlist.consume_all(bip_apacke_del_all);
	}
	inline void notify(size_t n = 1){ client_ctrol.Signal(n); }
	void wait(int w_sec = NULL){//�ȴ����ݽ��� ʱ������  /*, [&](){return (!ptrlist.empty() || (user == 0)); }*/  //[&](){return (!ptrlist.empty();}
		if (w_sec){
			client_ctrol.Wait_time(w_sec);
		}else{
			client_ctrol.Wait();
		}
	}
	inline void re_init(){ client_ctrol.reset(); }
private:
	Semaphore	client_ctrol;	//�ź���
}aclient, *aclientPtr;

typedef std::vector<aclientPtr>							client_list;	//�û�ͨ����
#define	client_list_max									32				//�û�ͨ�������ֵ
#define client_id_base									100				//�û�ͨ��ID��100��


//�������
enum atransportError
{
	A_ERROR_NONE,			//�޴���
	A_ERROR_OFFLINE,		//�豸����
	A_ERROR_UN_DEVICE,		//�豸��δͨ����֤
	A_ERROR_CLIENT_CLOSE,	//�û�ͨ���ر�
};

typedef struct copyinfo
{
	copyinfo *next;
	const char *src;
	const char *dst;
	unsigned int time;
	unsigned int mode;
	unsigned int size;
	int flag;
	//char data[0];
}copyinfo;

class Catransport
	:public boost::enable_shared_from_this<Catransport>
{
//======================�豸ͨѶ����========================
public:
	SocketPair	pair;	//�������õ��ڴ�ָ��

	int fd;
	int transport_socket;
	_aInt connectCount;	//�ۼƱ��豸���ӵ��Ķ�������
	_aInt ref_count;
	unsigned sync_token;
	_aInt connection_state;
	int online;
	transport_type type;

	/* usb handle or socket fd as needed */
	usb_handle *usb;
	int sfd;

	/* used to identify transports for clients */
	_achar serial;
	char *product;
	char *model;
	char *device;
	char *devpath;
	int adb_port; // Use for emulators (local transport)

	/* a list of adisconnect callbacks called when the transport is kicked */
	_aInt          kicked;

	void *key;
	unsigned char token[TOKEN_SIZE];
	//fdevent auth_fde;
	unsigned failed_auth_attempts;
//======================================================================================
public:	//�û��������ݶ���
	bip_shared_mutex	bip_lock;	//�û��б������
	client_list			userlist;	//�û�ͨѶͨ����
	apacketlist			loop_to_use_apacket;	//���ݰ�ѭ��ʹ��
private://����������
	adb_usb_mutex	start_mutex;
	void*			atp;	//��ʱ��������ָ��
private://ʵ���߳�ͬ�� �ź���
	Semaphore		d_write_ctrol, d_read_ctrol;	//d_write_ctrol д��usb���� d_read_ctrol ��usb��ȡ������
public://�������
	_aInt	t_error;
public:
	Catransport(usb_handle *h, const char *_serial, const char *_devpath, int state);
	~Catransport();	//���� �ͷ�������Դ ���� usb_handle �������޳�
	bool Start();	//��������ͨѶ
	char *connection_state_name();
	void Clear();
	void free_ctrol();//�����ͷ��ͷ�ͬ���� ����ʹ��
private://�ͷ���Դ
	void	remote_usb_clear();//usb�豸��Դ����
public:	//�̵߳���
	int read_from_remote(apacket *p);
	int write_to_remote(apacket *p);
	//void(*close)(atransport *t);	//������Ҫ�ֶ�����
	inline void kick(){kicked = 1; }
	inline void handle_online(){ online = 1; }
	inline void handle_offline(){ online = 0; }
public://���ݰ�����
	apacketPtr	get_apacket();
	void		put_apacket(apacketPtr& p);
public://���ݴ���
	void send_auth_response(uint8_t *token, size_t token_size);
	void send_auth_publickey();
	void send_connect();
	void send_ready(unsigned local, unsigned remote);
	void send_close(unsigned local, unsigned remote);
	void send_packet(apacketPtr& p);
	void parse_banner(char *banner);
	void handle_packet(apacketPtr& p);
public://���ݴ��䱾�����û��˽���
	int local_client_enqueue(int remote_arg0, int local_arg1, apacketPtr& p);//���ݷ������û�ͨ��
	int smart_local_remote_enqueue(int local_id, const char *destination);	//���豸ͨ��ͨ�� �ɹ�0
	void connect_to_remote(int local_id, const char *destination);	//�Խӱ������豸ͨ��	A_OPEN
	int local_remote_enqueue(int local_id, apacketPtr& p);		//���ط��͵��豸���� A_WRAE
	int	local_client_type_set(int _type, int local_id);	//��־ͨѶ����
public://�û�����ʱ����
	//shell
	int shell_send(const char *destination, int local_id);
	int shell_recv(sstring &ret, int local_id, int wait_times = 0);
	//forward sync ͨ��
	long client_read(unsigned char* retData, bufferstream* buffer, long len, aclientPtr local, int wait_times = 0);//��ͨ���ж�ȡָ�����ȵ����� retData buffer ���ݿ��Դ��䵽�����е�һ��
	int client_write(void* sendData, long len, int local_id);	//���豸����ָ����������
	//forward
	int forward_connect(const char* remote_connect, int local_id);
	int forward_write(void* sendData, long len, int local_id);	//��������
	int forward_read(unsigned char* retData, long len, int local_id, int wait_times = 0);	//��ȴ�ʱ��(��)
	inline int forward_disconnect(int local_id){ return local_client_type_set(bipbuffer_close, local_id); }
	//sync �ļ�����
	inline int sync_push(const char *lpath, const char *rpath, int verifyApk, int local_id){ 
		return do_sync_push(lpath, rpath, verifyApk, local_id); }
	inline int sync_push_buffer(unsigned char* buffer, size_t len, const char *rpath, int local_id) { 
		return do_sync_push_buffer(buffer, len, rpath, local_id); }
	inline int sync_pull(const char *rpath, const char *lpath, int local_id){ 
		return do_sync_pull(rpath, lpath, local_id); }
	inline int sync_pull_buffer(bufferstream &buffer, size_t &len, const char *rpath, int local_id){ 
		return do_sync_pull_buffer(buffer, len, rpath, local_id); }
	inline int sync_sync(const char *lpath, const char *rpath, int listonly, int local_id){ 
		return do_sync_sync(lpath, rpath, listonly, local_id); }
public://ͨ������
	int	read_packet(int f, apacketPtr& packet);
	int	write_packet(int f, apacketPtr& packet);
	int client_get_id();	//ͨѶͨ������
	void client_free_id(int _id);	//ͨ���ر�
	aclientPtr	client_get_ptr(int _id);	//��ȡͨ������ָ��
	void	client_init(aclientPtr a);		//�������ʼ��
	void	client_clear(aclientPtr a);		//�������
	void	client_removeAll();				//�ͷ����ж���
private://usb�豸���ݴ���
	void wait_for_usb_read_packet(int f, BipBuffer bip);
	int	usb_read_packet(int f, apacketPtr& packet);
	int	usb_write_packet(int f, apacketPtr& packet);
	int remote_usb_read(apacket* p);
	int remote_usb_write(apacket* p);
private://sync ͨѶ����
	//sync ͨѶ
	int sync_connect(int local_id);
	int sync_write(void* sendData, long len, int local_id);
	int sync_read(unsigned char* retData, long len, int local_id, int wait_times = 0);	//����һ�����ȣ� β����η������һ������ָ�뱣��δ��������� ��ȴ�ʱ��(��)
	int sync_read_to_buffer(bufferstream &buffer, long len, int local_id, int wait_times = 0);	//����һ�����ȣ� β����η������һ������ָ�뱣��δ��������� ��ȴ�ʱ��(��)
	inline int sync_disconnect(int local_id){ return local_client_type_set(bipbuffer_close, local_id); }
	//sync ���� file_class_sync_client.cpp
	void sync_quit(int local_id);
	int sync_ls(const char *path, void *cookie, int local_id);
	int sync_readtime(const char *path, unsigned *timestamp, int local_id);
	int sync_start_readtime(const char *path, int local_id);
	int sync_finish_readtime(unsigned int *timestamp, unsigned int *mode, unsigned int *size, int local_id);
	int sync_readmode(const char *path, unsigned *mode, int local_id);
	int write_data_file(const char *path, int local_id);
	int write_data_buffer(char* file_buffer, int size, int local_id);
	int sync_send(const char *lpath, const char *rpath, unsigned mtime, _mode_t mode, int verifyApk, int local_id);
	int sync_send_buffer(unsigned char* buffer, size_t _len, const char *rpath, unsigned mtime, _mode_t mode, int local_id);
	int sync_recv(const char *rpath, const char *lpath, int local_id);
	int sync_recv_buffer(bufferstream &_buffer, size_t &_len, const char *rpath, int local_id);
	int copy_local_dir_remote(const char *lpath, const char *rpath, int checktimestamps, int listonly, int local_id);
	int do_sync_push(const char *lpath, const char *rpath, int verifyApk, int local_id);
	int remote_build_list(copyinfo **filelist, const char *rpath, const char *lpath, int local_id);
	int copy_remote_dir_local(const char *rpath, const char *lpath, int checktimestamps, int local_id);
	int do_sync_pull(const char *rpath, const char *lpath, int local_id);
	int do_sync_sync(const char *lpath, const char *rpath, int listonly, int local_id);
	int do_sync_push_buffer(unsigned char* buffer, size_t len, const char *rpath, int local_id);
	int do_sync_pull_buffer(bufferstream &buffer, size_t &len, const char *rpath, int local_id);
};

#endif	//_CATRANSPORT_H_