#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

#include "mpi_sys.h"

#include "hi_comm_aio.h"
#include "mpi_ai.h"
#include "hi_comm_aenc.h"
#include "mpi_aenc.h"
#include "hi_comm_adec.h"
#include "mpi_adec.h"
#include "mpi_ao.h"
#include "acodec.h"

#include "hal_def.h"
//#include "hal.h"
#include "audio.h"
#include "fifo.h"
#include "encoder.h"
#include "shine/layer3.h"


#define TALK_BUF_SIZE (8*1024)


static int audio_init = 0;

typedef struct
{
    int getdata_enable;
    int putdata_enable;
    FIFO_HANDLE fifo;
    pthread_mutex_t lock;
    pthread_cond_t cond;
} TALKBACK_CONTEXT;

static TALKBACK_CONTEXT tb_ctx;

AIO_ATTR_S g_aio_attr[2] = {
    {AUDIO_SAMPLE_RATE_8000, AUDIO_BIT_WIDTH_16, AIO_MODE_I2S_MASTER, AUDIO_SOUND_MODE_MONO, 0, 20, AUDIO_PTNUMPERFRM, 1, 0},
    {AUDIO_SAMPLE_RATE_8000, AUDIO_BIT_WIDTH_16, AIO_MODE_I2S_MASTER, AUDIO_SOUND_MODE_MONO, 0, 20, AUDIO_PTNUMPERFRM, 1, 0}
};

static AENC_ATTR_ADPCM_S lAencAdpcm = {
	ADPCM_TYPE_DVI4
};

static AENC_CHN_ATTR_S lAencAttr = {
	PT_ADPCMA, AUDIO_PTNUMPERFRM, 20, &lAencAdpcm
};

static ADEC_ATTR_ADPCM_S lAdecAdpcm = {
	ADPCM_TYPE_DVI4
};

static ADEC_CHN_ATTR_S lAdecAttr = {
	PT_ADPCMA, 20, ADEC_MODE_STREAM, &lAdecAdpcm
};

static shine_config_t lShineConfig = {
    {PCM_MONO, 44100},
    {MONO, 32, NONE, 0, 1}
};

#if 0

void set_ai_samplerate(int samplerate, int perfrm)
{
    g_aio_attr[0].enSamplerate = samplerate;
    g_aio_attr[0].u32PtNumPerFrm = perfrm;

    /*

    int fd = open("/dev/acodec", O_RDWR);
    unsigned int adc_hpf; 
    adc_hpf = 0x1; 
    if (ioctl(fd, ACODEC_SET_ADC_HP_FILTER, &adc_hpf)) 
    { 
                    printf("ioctl err!\n"); 
    } 


    close(fd);

    int fd = open("/dev/acodec", O_RDWR);
    unsigned int gain_mic = 15;
    if (ioctl(fd, ACODEC_SET_GAIN_MICL, &gain_mic))
    {
                    printf("%s: set l acodec micin volume failed\n", __FUNCTION__);
                    close(fd);
                    return HI_FAILURE;
    }
	
    if (ioctl(fd, ACODEC_GET_GAIN_MICL, &gain_mic))
    {
                    printf("%s: set l acodec micin volume failed\n", __FUNCTION__);
                    close(fd);
                    return HI_FAILURE;
    }
    printf("gain_mic...................%d\n",gain_mic);
	
    gain_mic= 0;
    if (ioctl(fd, ACODEC_SET_GAIN_MICR, &gain_mic))
    {
                    printf("%s: set r acodec micin volume failed\n", __FUNCTION__);
                    close(fd);
                    return HI_FAILURE;
    }
	

    if (ioctl(fd, ACODEC_GET_GAIN_MICR, &gain_mic))
    {
                    printf("%s: set r acodec micin volume failed\n", __FUNCTION__);
                    close(fd);
                    return HI_FAILURE;
    }
    printf("gain_mic...................%d\n",gain_mic);

    close(fd);
     */
}
#endif

static int audio_config_acodec(AUDIO_SAMPLE_RATE_E sampleRate)
{
#define ACODEC_FILE  "/dev/acodec"
    int fd = -1;

    fd = open(ACODEC_FILE, O_RDWR);
    if (fd < 0) {
        ERROR_LOG("can't open acodec: %s\n", ACODEC_FILE);
        return HLE_RET_EIO;
    }

    if (ioctl(fd, ACODEC_SOFT_RESET_CTRL)) {
        ERROR_LOG("Reset audio codec error\n");
        close(fd);
        return HLE_RET_ERROR;
    }

    ACODEC_FS_E fs;
    if (AUDIO_SAMPLE_RATE_8000 == sampleRate) {
        fs = ACODEC_FS_8000;
    }
    else if (AUDIO_SAMPLE_RATE_16000) {
        fs = ACODEC_FS_16000;
    }
    else if (AUDIO_SAMPLE_RATE_44100 == sampleRate) {
        fs = ACODEC_FS_44100;
    }
    else {
        ERROR_LOG("not support enSample:%d\n", sampleRate);
        close(fd);
        return HLE_RET_ERROR;
    }

    if (ioctl(fd, ACODEC_SET_I2S1_FS, &fs)) {
        ERROR_LOG("set acodec sample rate failed\n");
        close(fd);
        return HLE_RET_ERROR;
    }

    ACODEC_MIXER_E im = ACODEC_MIXER_IN1;
    if (ioctl(fd, ACODEC_SET_MIXER_MIC, &im)) {
        ERROR_LOG("set acodec input mode failed\n");
        close(fd);
        return HLE_RET_ERROR;
    }

    int iv = 30;
    if (ioctl(fd, ACODEC_SET_INPUT_VOL, &iv)) {
        ERROR_LOG("set acodec micin volume failed\n");
        close(fd);
        return HLE_RET_ERROR;
    }

    int ov;
    ov = 6; // default -21,range -121~6
    if (ioctl(fd, ACODEC_SET_OUTPUT_VOL, &ov)) {
        ERROR_LOG("set acodec output volume failed\n");
        close(fd);
        return HLE_RET_ERROR;
    }

    ACODEC_VOL_CTRL vc;
    vc.vol_ctrl_mute = 0;
    vc.vol_ctrl = 0; // default 6 range 127~0
    if (ioctl(fd, ACODEC_SET_DACL_VOL, &vc)) {
        ERROR_LOG("set acodec dacl volume failed\n");
        close(fd);
        return HLE_RET_ERROR;
    }

    int eb = 1;
    if (ioctl(fd, ACODEC_ENABLE_BOOSTL, &eb)) {
        ERROR_LOG("set acodec boost failed\n");
        close(fd);
        return HLE_RET_ERROR;
    }

    close(fd);
    return HLE_RET_OK;
}

static int audio_start_aio(void)
{
    HLE_S32 ret;

    //配置AI设备并使能
    ret = audio_config_acodec(g_aio_attr[0].enSamplerate);
    if (HLE_RET_OK != ret) {
        return HLE_RET_ERROR;
    }
    ret = HI_MPI_AI_SetPubAttr(AI_DEVICE_ID, &g_aio_attr[0]);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AI_SetPubAttr fail: %#x\n", ret);
        return HLE_RET_ERROR;
    }
    ret = HI_MPI_AI_Enable(AI_DEVICE_ID);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AI_Enable(%d) fail: %#x\n", AI_DEVICE_ID, ret);
        return HLE_RET_ERROR;
    }

    //配置AO设备并使能
    ret = HI_MPI_AO_SetPubAttr(AO_DEVICE_ID, &g_aio_attr[1]);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AO_SetPubAttr(%d) fail: %#x!\n", AO_DEVICE_ID, ret);
        return HLE_RET_ERROR;
    }
    ret = HI_MPI_AO_Enable(AO_DEVICE_ID);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AO_Enable(%d) fail: %#x!\n", AO_DEVICE_ID, ret);
        return HLE_RET_ERROR;
    }

    //配置所有AI通道并使能
    ret = HI_MPI_AI_EnableChn(AI_DEVICE_ID, TALK_AI_CHN);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AI_EnableChn(%d,%d) fail: %#x\n", AI_DEVICE_ID, TALK_AI_CHN, ret);
        return HLE_RET_ERROR;
    }

    return HLE_RET_OK;
}

static int audio_stop_ai(void)
{
    HLE_S32 ret;
    ret = HI_MPI_AI_DisableChn(AI_DEVICE_ID, TALK_AI_CHN);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AI_DisableChn(%d, %d) fail: %#x!\n", AI_DEVICE_ID, TALK_AI_CHN, ret);
        return HLE_RET_ERROR;
    }

    return HLE_RET_OK;
}

#if 0
#define MP3_ENC_BUF_LEN     (576 * 2 * 2)

static struct
{
    volatile int run;
    pthread_t tid;
    shine_t hdlShine;
    HLE_U8 inBuf[MP3_ENC_BUF_LEN];
    int inIdx;
} lMp3EncCtx = {0};

static void* mp3_enc_proc(void* para)
{
    while (lMp3EncCtx.run) {
		AUDIO_FRAME_S audFrm;
		AEC_FRAME_S aecFrm;
		int ret = HI_MPI_AI_GetFrame(AI_DEVICE_ID, TALK_AI_CHN, &audFrm, &aecFrm, 1000);
		if (HI_SUCCESS != ret) {
			ERROR_LOG("HI_MPI_AI_GetFrame fail: %#x\n", ret);
			continue;
		}
		if ((lMp3EncCtx.inIdx + audFrm.u32Len) <= MP3_ENC_BUF_LEN) {
			memcpy(&lMp3EncCtx.inBuf[lMp3EncCtx.inIdx], audFrm.pVirAddr[0], audFrm.u32Len);
			lMp3EncCtx.inIdx += audFrm.u32Len;
		} else {
			int len = MP3_ENC_BUF_LEN - lMp3EncCtx.inIdx;
			memcpy(&lMp3EncCtx.inBuf[lMp3EncCtx.inIdx], audFrm.pVirAddr[0], len);
			
			int mp3Len;
			HLE_U8* mp3Frm = shine_encode_buffer_interleaved(lMp3EncCtx.hdlShine, (HLE_S16*)lMp3EncCtx.inBuf, &mp3Len);
			if (mp3Frm) {
				// todo: put mp3Frm to fifo
			}

			lMp3EncCtx.inIdx = audFrm.u32Len - len;
			memcpy(lMp3EncCtx.inBuf, (HLE_U8*)(audFrm.pVirAddr[0]) + len, lMp3EncCtx.inIdx);
		}
		
		// todo: release
    }

}
#endif

#ifndef USE_AI_AENC_BIND
static volatile int lRunAiAenc = 0;
static FIFO_HANDLE* hdlAiFifo = NULL;

void request_ai(FIFO_HANDLE* fifo)
{
    hdlAiFifo = fifo;
}

void release_ai(void)
{
    hdlAiFifo = NULL;
}

static void* ai_aenc_proc(void* para)
{
	para = para;
	while (lRunAiAenc) {
		AUDIO_FRAME_S audFrm;
		AEC_FRAME_S aecFrm;
		int ret = HI_MPI_AI_GetFrame(AI_DEVICE_ID, TALK_AI_CHN, &audFrm, &aecFrm, 1000);
		if (HI_SUCCESS != ret) {
			ERROR_LOG("HI_MPI_AI_GetFrame fail: %#x\n", ret);
			continue;
		}

		ret = HI_MPI_AENC_SendFrame(TALK_AENC_CHN, &audFrm, &aecFrm);
		if (HI_SUCCESS != ret) {
			ERROR_LOG("HI_MPI_AENC_SendFrame fail: %#x\n", ret);
		}
		
		if (hdlAiFifo) {
			fifo_in(hdlAiFifo, audFrm.pVirAddr[0], audFrm.u32Len, 1);
		}
		
		ret = HI_MPI_AI_ReleaseFrame(AI_DEVICE_ID, TALK_AI_CHN, &audFrm, &aecFrm);
		if (HI_SUCCESS != ret) {
			ERROR_LOG("HI_MPI_AI_ReleaseFrame fail: %#x\n", ret);
		}
	}
	
	return NULL;
}
#endif

static int audio_stop_aenc(void)
{
#if 1
    int ret;

#ifdef USE_AI_AENC_BIND
    MPP_CHN_S src, dst;
    src.enModId = HI_ID_AI;
    src.s32DevId = AI_DEVICE_ID;
    src.s32ChnId = TALK_AI_CHN;
    dst.enModId = HI_ID_AENC;
    dst.s32DevId = 0;
    dst.s32ChnId = TALK_AENC_CHN;
    ret = HI_MPI_SYS_UnBind(&src, &dst);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_SYS_UnBind((%d, %d, %d), (%d, %d, %d)) fail: %#x!\n",
                  src.enModId, src.s32DevId, src.s32ChnId,
                  dst.enModId, dst.s32DevId, dst.s32ChnId, ret);
        return HLE_RET_ERROR;
    }
#else
	lRunAiAenc = 0;
#endif

    ret = HI_MPI_AENC_DestroyChn(TALK_AENC_CHN);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AENC_DestroyChn(%d) fail: %#x!\n", TALK_AENC_CHN, ret);
        return HLE_RET_ERROR;
    }

    return HLE_RET_OK;
#else
    if (lMp3EncCtx.run) {
        lMp3EncCtx.run = 0;
        pthread_join(lMp3EncCtx.tid, NULL);
        shine_close(lMp3EncCtx.hdlShine);
    }
#endif
}

static int audio_start_aenc(void)
{
#if 1
    int ret = HI_MPI_AENC_CreateChn(TALK_AENC_CHN, &lAencAttr);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AENC_CreateChn(%d) fail: %#x!\n", TALK_AENC_CHN, ret);
        return HLE_RET_ERROR;
    }

#ifdef USE_AI_AENC_BIND
    MPP_CHN_S src, dst;
    src.enModId = HI_ID_AI;
    src.s32DevId = AI_DEVICE_ID;
    src.s32ChnId = TALK_AI_CHN;
    dst.enModId = HI_ID_AENC;
    dst.s32DevId = 0;
    dst.s32ChnId = TALK_AENC_CHN;
    ret = HI_MPI_SYS_Bind(&src, &dst);
    DEBUG_LOG("(%d, %d, %d)----->(%d, %d, %d)\n", src.enModId, src.s32DevId, src.s32ChnId, dst.enModId, dst.s32DevId, dst.s32ChnId);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_SYS_Bind((%d, %d, %d), (%d, %d, %d)) fail: %#x!\n",
                  src.enModId, src.s32DevId, src.s32ChnId,
                  dst.enModId, dst.s32DevId, dst.s32ChnId, ret);
        return HLE_RET_ERROR;
    }
#else
	pthread_t tid;
	lRunAiAenc = 1;
	ret = pthread_create(&tid, NULL, ai_aenc_proc, NULL);
	if (ret) {
		ERROR_LOG("pthread_create fail: %d\n", ret);
		return HLE_RET_ERROR;
	}
	pthread_detach(tid);
#endif

    return HLE_RET_OK;
#else
    if (0 == lMp3EncCtx.run) {
        lMp3EncCtx.hdlShine = shine_initialise(&lShineConfig);
        lMp3EncCtx.run = 1;
        int ret = pthread_create(&lMp3EncCtx.tid, NULL, mp3_enc_proc, NULL);
        if (ret) {
            ERROR_LOG("pthread_create fail: %#x\n", ret);
        }
    }
#endif
}

static int start_talkback_ao(void)
{

    int ret = HI_MPI_AO_EnableChn(AO_DEVICE_ID, TALK_AO_CHN);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AO_EnableChn(%d) fail: %#x!\n", TALK_AO_CHN, ret);
        return HLE_RET_ERROR;
    }

#if 1
    MPP_CHN_S src, dst;
    src.enModId = HI_ID_ADEC;
    src.s32DevId = 0;
    src.s32ChnId = TALK_ADEC_CHN;
    dst.enModId = HI_ID_AO;
    dst.s32DevId = AO_DEVICE_ID;
    dst.s32ChnId = TALK_AO_CHN;
    ret = HI_MPI_SYS_Bind(&src, &dst);
    DEBUG_LOG("(%d, %d, %d)----->(%d, %d, %d)\n", src.enModId, src.s32DevId, src.s32ChnId, dst.enModId, dst.s32DevId, dst.s32ChnId);
    if (ret != HI_SUCCESS) {
        HI_MPI_AO_DisableChn(AO_DEVICE_ID, TALK_AO_CHN);
        ERROR_LOG("HI_MPI_SYS_Bind((%d, %d, %d), (%d, %d, %d)) fail: %#x!\n",
                  src.enModId, src.s32DevId, src.s32ChnId,
                  dst.enModId, dst.s32DevId, dst.s32ChnId, ret);
        return HLE_RET_ERROR;
    }
#else
#endif

    return HLE_RET_OK;
}

static int start_talkback_adec(void)
{
#if 1
    HLE_S32 ret = HI_MPI_ADEC_CreateChn(TALK_ADEC_CHN, &lAdecAttr);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_ADEC_CreateChn(%d) fail: %#x!\n", TALK_ADEC_CHN, ret);
        return HLE_RET_ERROR;
    }

    return HLE_RET_OK;
#else
#endif
}

static void stop_talkback_adec(void)
{
#if 1
    MPP_CHN_S src, dst;
    src.enModId = HI_ID_ADEC;
    src.s32DevId = 0;
    src.s32ChnId = TALK_ADEC_CHN;
    dst.enModId = HI_ID_AO;
    dst.s32DevId = AO_DEVICE_ID;
    dst.s32ChnId = TALK_AO_CHN;
    int ret = HI_MPI_SYS_UnBind(&src, &dst);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_SYS_UnBind((%d, %d, %d), (%d, %d, %d)) fail: %#x!\n",
                  src.enModId, src.s32DevId, src.s32ChnId,
                  dst.enModId, dst.s32DevId, dst.s32ChnId, ret);
    }

    ret = HI_MPI_ADEC_DestroyChn(TALK_ADEC_CHN);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_ADEC_DestroyChn(%d) fail: %#x!\n", TALK_ADEC_CHN, ret);
    }
#else
#endif
}

static void stop_talkback_ao(void)
{
    int ret = HI_MPI_AO_DisableChn(AO_DEVICE_ID, TALK_AO_CHN);
    if (ret != HI_SUCCESS) {
        ERROR_LOG("HI_MPI_AO_DisableChn(%d, %d) fail: %#x!\n",
                  AO_DEVICE_ID, TALK_AO_CHN, ret);
    }
}

int add_to_talkback_fifo(void *data, int size)
{
    if (audio_init == 0)
        return HLE_RET_ENOTINIT;

    //DEBUG_LOG("add_to_talkback_fifo size %d\n", size);

    pthread_mutex_lock(&tb_ctx.lock);
    if (tb_ctx.getdata_enable == 0) {
        pthread_mutex_unlock(&tb_ctx.lock);
        return HLE_RET_ENOTINIT;
    }

    int avail = fifo_avail(tb_ctx.fifo, 0);
    if (avail < size) {
        DEBUG_LOG("not enough space to hold\n");
        pthread_mutex_unlock(&tb_ctx.lock);
        return -1;
    }

    int ret = fifo_in(tb_ctx.fifo, data, size, 0);
    if (avail == TALK_BUF_SIZE) {
        pthread_cond_signal(&tb_ctx.cond);
    }

    pthread_mutex_unlock(&tb_ctx.lock);
    return ret;
}

int audioin_get_chns(void)
{
    return 1;
}

int talkback_get_audio_fmt(TLK_AUDIO_FMT *aenc_fmt, TLK_AUDIO_FMT *adec_fmt)
{
    aenc_fmt->enc_type = AENC_STD_ADPCM;
    aenc_fmt->sample_rate = AUDIO_SR_8000;
    aenc_fmt->bit_width = AUDIO_BW_16;
    aenc_fmt->sound_mode = AUDIO_SM_MONO;

    adec_fmt->enc_type = AENC_STD_ADPCM;
    adec_fmt->sample_rate = AUDIO_SR_8000;
    adec_fmt->bit_width = AUDIO_BW_16;
    adec_fmt->sound_mode = AUDIO_SM_MONO;

    return 0;
}

int talkback_start(int getmode, int putmode)
{
    if ((audio_init == 0) || ((getmode == 0) && (putmode == 0)))
        return HLE_RET_ENOTINIT;

    pthread_mutex_lock(&tb_ctx.lock);
    if (putmode) {
        if (tb_ctx.putdata_enable) {
            pthread_mutex_unlock(&tb_ctx.lock);
            return HLE_RET_EBUSY;
        }

        if (HLE_RET_OK != start_talkback_adec()) {
            pthread_mutex_unlock(&tb_ctx.lock);
            return HLE_RET_ERROR;
        }

        if (HLE_RET_OK != start_talkback_ao()) {
            stop_talkback_adec();
            pthread_mutex_unlock(&tb_ctx.lock);
            return HLE_RET_ERROR;
        }

        tb_ctx.putdata_enable = 1;
    }

    if (getmode) {
        if (tb_ctx.getdata_enable) {
            pthread_mutex_unlock(&tb_ctx.lock);
            return HLE_RET_EBUSY;
        }

        fifo_reset(tb_ctx.fifo, 0);
        tb_ctx.getdata_enable = 1;
    }

    pthread_mutex_unlock(&tb_ctx.lock);
    DEBUG_LOG("talkback_start success\n");
    return HLE_RET_OK;
}

int talkback_stop(void)
{
    if (audio_init == 0)
        return HLE_RET_ENOTINIT;

    pthread_mutex_lock(&tb_ctx.lock);
    if (tb_ctx.putdata_enable != 0) {
        stop_talkback_ao();
        stop_talkback_adec();
        tb_ctx.putdata_enable = 0;
    }

    tb_ctx.getdata_enable = 0;
    pthread_mutex_unlock(&tb_ctx.lock);

    return HLE_RET_OK;
}

int talkback_get_data(void *data, int *size)
{
    if (audio_init == 0)
        return HLE_RET_ENOTINIT;

    pthread_mutex_lock(&tb_ctx.lock);
    if (tb_ctx.getdata_enable == 0) {
        pthread_mutex_unlock(&tb_ctx.lock);
        return HLE_RET_ENOTINIT;
    }

    while (fifo_len(tb_ctx.fifo, 0) == 0) {
        pthread_cond_wait(&tb_ctx.cond, &tb_ctx.lock);
    }

    *size = fifo_out(tb_ctx.fifo, data, *size, 0);

    pthread_mutex_unlock(&tb_ctx.lock);
    return HLE_RET_OK;
}

int talkback_put_data(void *data, int size)
{
    if (audio_init == 0)
        return HLE_RET_ENOTINIT;

    pthread_mutex_lock(&tb_ctx.lock);
    if (tb_ctx.putdata_enable == 0) {
        pthread_mutex_unlock(&tb_ctx.lock);
        return HLE_RET_ENOTINIT;
    }
    pthread_mutex_unlock(&tb_ctx.lock);

#if 1
    AUDIO_STREAM_S stAudioStream;
    memset(&stAudioStream, 0, sizeof(stAudioStream));
    stAudioStream.u32Len = size;
    stAudioStream.pStream = (HLE_U8 *) data;
    HLE_S32 ret = HI_MPI_ADEC_SendStream(TALK_ADEC_CHN, &stAudioStream, HI_TRUE);
    if (ret) {
        ERROR_LOG("HI_MPI_ADEC_SendStream fail: %#x!\n", ret);
    }
#else
    // todo
#endif

    return HLE_RET_OK;
}

//aio init

int hal_audio_init(void)
{
    int ret = audio_start_aio();
    if (HLE_RET_OK != ret) {
        ERROR_LOG("audio_start_aio fail: %d\n", ret);
        return HLE_RET_ERROR;
    }

    ret = audio_start_aenc();
    if (HLE_RET_OK != ret) {
        ERROR_LOG("audio_start_aenc fail: %d\n", ret);
        return HLE_RET_ERROR;
    }

    tb_ctx.putdata_enable = 0;
    tb_ctx.getdata_enable = 0;
    tb_ctx.fifo = fifo_malloc(TALK_BUF_SIZE);
    if (tb_ctx.fifo == NULL)
        return HLE_RET_ERROR;

    pthread_mutex_init(&tb_ctx.lock, NULL);
    pthread_cond_init(&tb_ctx.cond, NULL);
    audio_init = 1;

    talkback_start(0, 1);
    DEBUG_LOG("success\n");
    return HLE_RET_OK;
}

//aio exit

int hal_audio_exit(void)
{
    int ret;

    ret = audio_stop_aenc();
    if (HLE_RET_OK != ret) {
        ERROR_LOG("audio_stop_aenc fail: %d\n", ret);
        return HLE_RET_ERROR;
    }

    ret = audio_stop_ai();
    if (HLE_RET_OK != ret) {
        ERROR_LOG("audio_stop_ai fail: %d\n", ret);
        return HLE_RET_ERROR;
    }

    tb_ctx.putdata_enable = 0;
    tb_ctx.getdata_enable = 0;
    fifo_free(tb_ctx.fifo, 0);
    tb_ctx.fifo = NULL;

    pthread_mutex_destroy(&tb_ctx.lock);
    pthread_cond_destroy(&tb_ctx.cond);

    stop_talkback_adec();
    stop_talkback_ao();

    DEBUG_LOG("success\n");
    return HLE_RET_OK;
}

#if 0

int hal_sonic_wave_init(void)
{
    int ret = audio_start_aio();
    if (HLE_RET_OK != ret) {
        ERROR_LOG("audio_start_aio fail: %d\n", ret);
        return HLE_RET_ERROR;
    }

    tb_ctx.putdata_enable = 0;
    tb_ctx.getdata_enable = 0;
    tb_ctx.fifo = fifo_malloc(TALK_BUF_SIZE);
    if (tb_ctx.fifo == NULL)
        return HLE_RET_ERROR;

    pthread_mutex_init(&tb_ctx.lock, NULL);
    pthread_cond_init(&tb_ctx.cond, NULL);
    audio_init = 1;

    talkback_start(0, 1);
    DEBUG_LOG("success\n");
    return HLE_RET_OK;
}

int hal_sonic_wave_exit(void)
{
    int ret;

    ret = audio_stop_ai();
    if (HLE_RET_OK != ret) {
        ERROR_LOG("audio_stop_ai fail: %d\n", ret);
        return HLE_RET_ERROR;
    }

    stop_talkback_adec();
    stop_talkback_ao();

    tb_ctx.putdata_enable = 0;
    tb_ctx.getdata_enable = 0;
    fifo_free(tb_ctx.fifo);
    tb_ctx.fifo = NULL;

    pthread_mutex_destroy(&tb_ctx.lock);
    pthread_cond_destroy(&tb_ctx.cond);

    DEBUG_LOG("success\n");
    return HLE_RET_OK;
}

int sonic_aio_disable(void)
{
    HI_MPI_AI_Disable(AI_DEVICE_ID);
    HI_MPI_AO_Disable(AO_DEVICE_ID);
}
#endif

typedef struct
{
    int fd;
    PLAY_END_CALLBACK cbFunc;
    void* cbfPara;
} PLAY_TIP_CONTEX;

PLAY_TIP_CONTEX lTipCtx = {-1, NULL, NULL};

static void* play_tip_proc(void* para)
{
    PLAY_TIP_CONTEX* ctx = (PLAY_TIP_CONTEX*) para;

    char buf[160 + 4];
    buf[0] = 0;
    buf[1] = 1;
    buf[2] = 80;
    buf[3] = 0;

    int len;
    while ((len = read(ctx->fd, &buf[4], 160)) == 160) {
        talkback_put_data(buf, sizeof (buf));
        //buf[3]++;
    }
    if (len < 0) {
        ERROR_LOG("read fail: %d\n", errno);
    }

    close(ctx->fd);
    ctx->fd = -1;
    if (ctx->cbFunc) ctx->cbFunc(ctx->cbfPara);

    return NULL;
}

int play_tip(char* fn, PLAY_END_CALLBACK cbFunc, void* cbfPara)
{
    if (lTipCtx.fd >= 0) return HLE_RET_EBUSY;

    lTipCtx.cbFunc = cbFunc;
    lTipCtx.cbfPara = cbfPara;
    int fd = open(fn, O_RDONLY);
    if (fd < 0) {
        //ERROR_LOG("open %s fail: %d\n", fn, errno);
        return HLE_RET_EIO;
    }
    lTipCtx.fd = fd;

    pthread_t tid;
    int ret = pthread_create(&tid, NULL, play_tip_proc, &lTipCtx);
    if (ret) {
        ERROR_LOG("pthread_create fail: %d\n", ret);
        close(lTipCtx.fd);
        lTipCtx.fd = -1;
        return HLE_RET_ERROR;
    }
    pthread_detach(tid);

    return HLE_RET_OK;
}



