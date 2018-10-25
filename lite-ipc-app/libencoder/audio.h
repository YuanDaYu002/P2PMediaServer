#ifndef HAL_AUDIO_H
#define HAL_AUDIO_H

#include "typeport.h"

#ifdef __cplusplus
extern "C"
{
#endif


//对讲音频编解码格式信息

typedef struct
{
    HLE_U8 enc_type; //音频编码类型，具体见E_AENC_STANDARD
    HLE_U8 sample_rate; //采样频率，具体见E_AUDIO_SAMPLE_RATE
    HLE_U8 bit_width; //采样位宽，具体见E_AUDIO_BIT_WIDTH
    HLE_U8 sound_mode; //单声道还是立体声，具体见E_AUDIO_SOUND_MODE
} TLK_AUDIO_FMT;


/*
    function:  audioin_get_chns
    description:  获取视频伴音输入通道数
    args:
        无
    return:
        >=0, 成功，返回伴音通道数
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int audioin_get_chns(void);


/*
    function:  talkback_get_audio_fmt
    description:  获取对讲音频编码格式以及支持的解码格式
    args:
        TLK_AUDIO_FMT *aenc_fmt[out]    编码格式
        TLK_AUDIO_FMT *adec_fmt[out]    解码格式
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_get_audio_fmt(TLK_AUDIO_FMT *aenc_fmt, TLK_AUDIO_FMT *adec_fmt);

/*
    function:  talkback_start
    description:  开始对讲接口
    args:
        int getmode[in] 1:捕获数据 0:不捕获
        int putmode[in] 1:开启音频解码 0:不开启
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_start(int getmode, int putmode);

/*
    function:  talkback_stop
    description:  结束对讲接口
    args:
        无
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_stop(void);

/*
    function:  talkback_get_data
    description:  对讲获取本地数据接口
    args:
        void *data[in]，数据缓冲地址
        int *size[in,out]，传入的是缓冲区的长度，传出的是实际采集到的数据长度
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_get_data(void *data, int *size);

/*
    function:  talkback_put_data
    description:  对讲数据播放接口
    args:
        void *data[in]，数据缓冲地址
        int size[in]，数据长度
    return:
        0, 成功
        <0, 失败，返回值为错误码，具体见错误码定义
 */
int talkback_put_data(void *data, int size);

typedef void (*PLAY_END_CALLBACK)(void* para);
int play_tip(char* fn, PLAY_END_CALLBACK cbFunc, void* cbfPara);
int hal_audio_init(void);
int hal_audio_exit(void);
//int hal_sonic_wave_init(void);
//int hal_sonic_wave_exit(void);
//int sonic_aio_disable(void);


#ifdef __cplusplus
}
#endif

#endif

