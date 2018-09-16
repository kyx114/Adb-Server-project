#ifndef _ADB_FUNC_H_
#define _ADB_FUNC_H_

#include "Catransport.h"
#include "adb_struct.h"			//��������
#include "usb_vendors.h"
#include "adb_auth.h"
#include <boost/shared_ptr.hpp>	//����ָ��
#include <boost/weak_ptr.hpp>	//����ָ�������

//��־���
void show_log(const char* _Format, ...);
void show_apacket(const char* label, apacket* p);

//adb_mutex_lock _slock(log_lock);
#define D(fmt, ...)				show_log(fmt, ##__VA_ARGS__)
#define print_packet(label, p)	show_apacket(label, p)

class Catransport;
typedef boost::weak_ptr<Catransport>		atsweakPtr;		//�豸��������ָ������
typedef boost::shared_ptr<Catransport>		atransportPtr;	//�豸��������ָ��
typedef	std::map<sstring, atransportPtr>	devicesMap;

/**************************************************************************/
/*****    USB API				                                      *****/
/**************************************************************************/

extern	adb_buff_mutex					atransport_lock;
extern	devicesMap						devices_list;	//��ע����豸�б�

/// Checks if there is opened usb handle in handle_list for this device.
int known_device(const char* dev_name);

/// Checks if there is opened usb handle in handle_list for this device.
/// usb_lock mutex must be held before calling this routine.
int known_device_locked(const char* dev_name);

/// Registers opened usb handle (adds it to handle_list).
int register_new_device(usb_handle* handle);

/// Checks if interface (device) matches certain criteria
int recognized_device(usb_handle* handle);

///// Entry point for thread that polls (every second) for new usb interfaces.
///// This routine calls find_devices in infinite loop.
//void* device_poll_thread(void* unused);

///// Initializes this module
//void usb_init();

/// Cleans up this module
void usb_cleanup();

/// Opens usb interface (device) by interface (device) name.
usb_handle* do_usb_open(const wchar_t* interface_name);

/// Writes data to the opened usb handle
int usb_write(usb_handle* handle, const void* data, int len);

/// Reads data using the opened usb handle
int usb_read(usb_handle *handle, void* data, int len);

/// Cleans up opened usb handle
void usb_cleanup_handle(usb_handle* handle, int only_offline = 0);	//, int only_offline ָʾ�Ƿ���رյ�ͨ�� ������interface_name �����ӳ����»�ȡ�豸

/// Cleans up (but don't close) opened usb handle
void usb_kick(usb_handle* handle);

/// Closes opened usb handle
int usb_close(usb_handle* handle);

/// Gets interface (device) name for an opened usb handle
const char *usb_name(usb_handle* handle);

int is_adb_interface(int vid, int pid, int usb_class, int usb_subclass, int usb_protocol);

unsigned host_to_le32(unsigned n);

void register_usb_transport(usb_handle *usb, const char *serial, const char *devpath, unsigned writeable);

/// Enumerates present and available interfaces (devices), opens new ones and
/// registers usb transport for them.
void find_devices();

atransportPtr& get_device(const char* name_serial);
void	remove_device_All();
void	checked_devices();
void	get_device_name_list(sstring& retlist);

/**************************************************************************/
/*****    USB  ͨѶ API				                                  *****/
/**************************************************************************/
void	bip_buffer_init(BipBuffer  buffer);
void	bip_buffer_close(BipBuffer  bip);
void	bip_buffer_done(BipBuffer  bip);
int		bip_buffer_write(BipBuffer  bip, void* & _scr);
int		bip_buffer_read(BipBuffer  bip, void*& _dst);

SocketPair  adb_socketpair();
int	_fh_socketpair_close(int f, SocketPair  _pair);
int	_fh_socketpair_lseek(int  f, int pos, int  origin);
int	_fh_socketpair_read(int f, SocketPair  _pair, void*& _dst);
int	_fh_socketpair_write(int f, SocketPair  _pair, void*& _scr);
int  adb_read(int f, SocketPair  _pair, void*& _dst);
int  adb_write(int f, SocketPair  _pair, void*& _scr);
int  adb_lseek(int  fd, int  pos, int  where);
int  adb_close(int f, SocketPair  _pair);

int check_header(apacket *p);
int check_data(apacket *p);

size_t fill_connect_data(char *buf, size_t bufsize);

void qual_overwrite(char **dst, const char *src);

char *adb_strtok_r(char *s, const char *delim, char **last);

void *output_thread(void *_t);
void *input_thread(void *_t);
void *user_thread(void *_t);///�û��������


#endif	//_ADB_FUNC_H_