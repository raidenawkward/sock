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

// header trans before file content
// which contents:
// size: 32 bytes
// name: 256 bytes
// then be filled with '0'
#define FTRANS_TRANS_HEADER_SIZE 512
#define FTRANS_TRANS_HEADER_FILESIZE 32
#define FTRANS_TRANS_HEADER_FILENAME 256

#endif //_FTRANS_H
