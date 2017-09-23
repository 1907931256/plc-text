/*
* Copyright (c) 2007, �㽭�󻪼����ɷ����޹�˾
* All rights reserved.
*
* �ļ����ƣ�ITPListener.h
* �ļ���ʶ���μ����ù���ƻ���
* ժ����Ҫ�������������ӿ�
*
* ��ǰ�汾��1.0
* �������ߣ�������
* ������ڣ�2007��4��28��

*
* ȡ���汾��1.0
* ԭ���ߡ���
* ������ڣ�
* �޶���¼��
*/
#ifndef _ITPLISTENER_H_
#define _ITPLISTENER_H_

typedef enum 
{
	ss_sendQueueThreshold = 0,	//���г��ȵ��﷧ֵ
}internalAnounceType;

class ITPListener
{
public:    
	virtual int onData(int engineId, int connId, const char* data, int len) = 0;
	virtual int onClose(int engineId, int connId) = 0;

    //����ֵΪ0��ʾ���ܴ����ӣ�����ֵΪ1��ʾ�ܾ�����
	virtual int onConnect(int engineId, int connId, const char* ip, int port) = 0;

	//����ʱ���ݷ��ڶ�����,������heartbeatʱ��ʵ�ʷ���,���ͳɹ�ʱ��֮ǰ
	//��IDȷ�ϸ�������,�������������ָ��.����ֱ��ɾ��ָ����������.
	//* ע�⣺onSendDataAck�в������ٵ����κδ����ķ���������SendData()�ȣ�
	//* ��������������ʹ������д���
	//sendLen == 0��ʾ���ͳɹ��� !=0��ʾ���ַ��ͳɹ�����ʱӦ�ò�Ӧ������״�����������
	virtual int onSendDataAck(int engineId, int id, int nSeq, int sendLen) = 0;

	//����״̬�ص�
	//statusType == ss_sendQueueThreshold ʱ�� connId�������ã� param ��ʾ�ڲ����Ͷ��г���
	virtual int onSendStatus(int engineId, int connId, int statusType, int param) = 0;

	//��ʱ���ص�
	//id			��ʱ������
	//context		������
	//return	0 ������ 1 ȡ��ʱ�ӣ���ѭ��ʱ����Ч��
	virtual int onTimeout(int id, int context) = 0;

};

#endif	//_ITPLISTENER_H_

