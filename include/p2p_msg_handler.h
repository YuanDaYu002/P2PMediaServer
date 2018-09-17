#ifndef P2P_MSG_HANDLER_H
#define P2P_MSG_HANDLER_H

typedef int (*handler_t)();

typedef struct _p2p_msg_handler_t
{
	int cmd;
	handler_t hdl;
}p2p_msg_handler_t; 

enum _cmd_type
{
    CMD_TYPE_IPC_CLIENT=0,
    CMD_TYPE_IPC_TRANS,
};
enum cmd
{
	/* communicates with client(PC)*/
	PC_TRANS_LOGIN_SYN = 11000,
    	PC_TRANS_LOGIN_ACK = 11100,
	PC_TRANS_HEART_SYN = 11001,
    	PC_TRANS_HEART_ACK = 11101,
	/* communicates with device*/

	/* APP communicates with device */


}


#endif
