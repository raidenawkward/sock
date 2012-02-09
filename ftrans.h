#ifndef _FTRANS_H
#define _FTRANS_H

#define FTRANS_PORT 7801
#define FTRANS_SEND_BUF_SIZE 512
#define FTRANS_RECV_BUF_SIZE 512

#define FTRANS_UPLOAD_PATH "./uploads"
#define FTRANS_UPLOAD_MOD (0770)

#define FTRANS_TCP_LISTENBACKLOG 5

#define FTRANS_MAX_FORKS 10

// use ether sem or mutex for sync
#define FTRANS_USE_SEM

#ifndef FTRANS_USE_SEM
	#define FTRANS_USE_MUTEX
#endif

#endif //_FTRANS_H
