#ifndef HLE_MSG_H
#define HLE_MSG_H


#define HLE_MAGIC 0xB1A75   //HLE ASCII : 76 72 69-->767269-->0xB1A75

enum p2p_device_type_e
{
    DEVICE_TYPE_IPC = 0,
};


struct trans_msg_s
{
    unsigned int 	magic;		 /* 特殊头 */
    unsigned int	cmd;		 /* 指令字*/
    unsigned int	length;		 /* 包体长度 */
    //char			*buf;		 /* 包体内容 */
};

#endif


