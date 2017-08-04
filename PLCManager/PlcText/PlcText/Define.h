#pragma once

typedef enum EMSG_ID
{
	EMSG_PLAT_REQ_MSG = 1,
	EMSG_PLAT_RES_MSG,
	EMSG_PLAT_MSG,
	EMSG_PROTOCOL_ERROR,
	EMSG_RTSP_PDU,
	EMSG_MEDIA_DATA,
	EMSG_ON_COMPELETE,
	EMSG_SET_MEDIA_HANDLE,

	EMSG_HEARDBEAT,
	EMSG_ON_TIMEOUT,
	EMSG_ON_OPT_TIMEOUT,
	EMSG_CMS_LOGIN,
	EMSG_CMS_LOGOUT,
	EMSG_CMS_SET_STATE,
	EMSG_DMS_LOGIN,
	EMSG_ON_DMS_LOGIN,
	EMSG_DMS_LOGOUT,
	EMSG_GET_DEV_STATUS,
	EMSG_CONNECT,
	EMSG_SEND,
	EMSG_RECV,
	EMSG_DISCONNECT,
	EMSG_ON_CONNECT,
	EMSG_SEND_DATA_MSG,
	EMSG_ON_SEND_DATA_MSG,
	EMSG_MEDIA_GET_URL,
	EMSG_PTZ_CONTROL,
	EMSG_SEND_CFL_MSG,
	
	EMSG_MEDIA_DEC_OPEN,
	EMSG_MEDIA_DEC_OPEN_SOUND,
	EMSG_ON_MEDIA_DEC_OPEN,
	EMSG_MEDIA_PLAY,
	EMSG_MEDIA_PAUSE,
	EMSG_ON_MEDIA_PAUSE,
	EMSG_MEDIA_STOP,
	EMSG_ON_MEDIA_STOP,
	EMSG_MEDIA_CLOSE,
	EMSG_ON_MEDIA_CLOSE,
	EMSG_MEDIA_SEEK_TO_TIME,
	EMSG_MEDIA_SEEK_TO_POS,
	EMSG_ON_MEDIA_SEEK,
	EMSG_MEDIA_SET_SPEED,
	EMSG_ON_MEDIA_SET_SEED,
	EMSG_MEDIA_SEND_DATA,
	EMSG_MEDIA_CLEAR_BUF,
	EMSG_SAVE_FILE,
	EMSG_STOP_SAVE_FILE,
	EMSG_CAPTUREPICTURE,
	EMSG_SET_DEC_PARAM,
    EMSG_SET_DEC_PARAM2,             
	EMSG_SET_DEC_SOUND,
	EMSG_SET_DEC_VOLUME,
	EMSG_SET_DEC_COLOR,
	EMSG_SET_DEC_FLUENCY,
	EMSG_MEDIA_RETRY_PUSH_DATA,
	EMSG_BUFFER_SATE_CHANNGE,
	EMSG_ADD_DEV_TO_DMS,
	EMSG_ON_SELECT_CHANNGE,
	EMSG_GETFILE,				// �����ļ�
	EMSG_PUTFILE,				// �����ļ�
	EMSG_UPGRADE,				// ������Ϣ�ص�
	EMSG_GET_RECORD_FILE,		// ��ѯAS300 ¼���ļ�
	EMSG_SET_TIME,				//����rtsp�Ŀ�ʼ���źͽ���ʱ��.
    EMSG_LBUTTORN_DblClk,             //������˫���¼�

	EMSG_EVENT_CALL_BACK		= 1000,
	EMSG_ON_LOGIN				= 1001,		// ��¼����
	EMSG_ON_MEDIA_PLAY			= 1002,		// ����Ƶ�������
	EMSG_ON_MEDIA_DISCONNECT	= 1003,		// ý���쳣�Ͽ�
	EMSG_ON_SAVE_FILE			= 1004,		// �����ļ���id,���,0,0,�ļ���
	EMSG_ON_CAPTUREPICTURE		= 1005,		// ץͼ�ɹ��󷵻أ�id,���,0,0,�ļ���
	EMSG_ON_MEDIA_END			= 1006,		// ý���ļ����Ž���֪ͨ��id,���ں�,���
	EMSG_ON_PLAY_POS_BITS		= 1007,		// �����ļ�ʱ--��ʱ����
	EMSG_ON_PLAY_TIME_BITS		= 1008,		// ��ǰ�����ʱ�䣺id,lView,����,0,ʱ��
	EMSG_ON_MEDIA_DOWNLOAD		= 1009,		// �����ļ���id������ID�����
	EMSG_ON_DOWNLOAD_SIZE		= 1010,		// �����ļ�ʱ���طŽ��Ȼص���id,lView,������������λ�ֽڣ�
	EMSG_ON_DOWNLOAD_COMPELETE	= 1011,		// �ļ����ؽ���
	EMSG_ON_ALARM				= 1012,		// �澯��Ϣ�ص���id,ͨ����,����,״̬,��Ϣ�� ��Ϣ��ʽ���豸ID, ͨ����, ����, �澯ʱ��, �豸����, ����
	EMSG_ON_DEVICE_STATUS		= 1013,		// �豸״̬��id,״ֵ̬,0,0,�豸ID��״ֵ̬��1���ߣ�2���ߣ�3����4�Ƴ�
	EMSG_ON_DISCONNECT			= 1014,		// ϵͳ�쳣����
	EMSG_ON_SEARCH_DEVS			= 1015,		// �����豸�������
	EMSG_ON_RELOGIN				= 1016,		// ϵͳ���µ�¼�ɹ�
	EMSG_ON_LOGOUT				= 1017,		// ϵͳ�ǳ�
	EMSG_ON_DRAW_WND			= 1018,		// ���ڻ���
	EMSG_ON_GETFILE				= 1019,		// �����ļ�
	EMSG_ON_UPGRADE				= 1020,		// ������Ϣ�ص�
	EMSG_ON_PUTFILE				= 1021,		// �ϴ��ļ�
	EMSG_ON_GET_RECORD_FILE		= 1022,		// ����ƽ̨���豸��¼���ͼƬ�ļ�
	EMSG_ON_GET_DEVICE_LIST		= 1023,		// ��¼ƽ̨�����ص��豸�б�
    EMSG_ON_OUT_FULL_SCREEN		= 1024,		// �ط��˳�ȫ������Ҫ�������¼��ط�״̬��Ϣ
    EMSG_ON_INIT_OK		= 1025,		// ��ʼ�����
}EMSG_ID;

typedef enum EERROR
{
	ERR_NO_ERROR = 0,
    ERR_ERROR_EX = -20000,//����Ϣ�ص���ʱ��param1�����ERR_ERROR_EX,��param2��ʾ�����ǰ�˷��ص���Ϣ�������
	ERR_ERROR = -10000,
	ERR_TIMEOUT,			// ��ʱ
	ERR_NOACTIVE_TIME_OUT,	// ��ʱ��û��ЧӦ
	ERR_NET_CONNECT,		// �������ӳ���
	ERR_NOT_LOGIN,			// û�е�¼
	ERR_RTSP_SETUP,			// RTSP���ӣ�Setup����
	ERR_RTSP_PLAY,			// RTSP���ӣ�Play����
	ERR_NO_SUPPORT,			// ��֧��
	ERR_CREATE_FILE_ERROR,	// �����ļ�ʧ��
	ERR_USER_LOGGED,		// �Ѿ���¼
	ERR_IN_TALKING = -9990,			// �豸�Ѿ��ڶԽ�
	ERR_NET_ERROR,			// �����쳣
	ERR_USER_ERROR,			// �û������벻ƥ��
	ERR_OPEN_FILE,			// ���ļ�ʧ��
	ERR_USER_CANCEL,		// �û�ȡ��
	ERR_REPEAT_OPT,			// �ظ��������Ƚ�����������û�н������ֿ�ʼ��һ��
    ERR_FILE_LEN_ZERO,	    // �ļ�Ϊ���ļ�
}EERROR;

typedef struct SInfoEx
{
	int nPlayPort;
	int nDrawMark;
}SInfoEx;