/*
* Copyright (c) 2007, 浙江大华技术股份有限公司
* All rights reserved.
*
* 文件名称：ITPListener.h
* 文件标识：参见配置管理计划书
* 摘　　要：传输层监听器接口
*
* 当前版本：1.0
* 作　　者：李明江
* 完成日期：2007年4月28日

*
* 取代版本：1.0
* 原作者　：
* 完成日期：
* 修订记录：
*/
#ifndef _ITPLISTENER_H_
#define _ITPLISTENER_H_

typedef enum 
{
	ss_sendQueueThreshold = 0,	//队列长度到达阀值
}internalAnounceType;

class ITPListener
{
public:    
	virtual int onData(int engineId, int connId, const char* data, int len) = 0;
	virtual int onClose(int engineId, int connId) = 0;

    //返回值为0表示接受此连接，返回值为1表示拒绝接受
	virtual int onConnect(int engineId, int connId, const char* ip, int port) = 0;

	//发送时数据放在队列中,当心跳heartbeat时才实际发送,发送成功时将之前
	//的ID确认给调用者,用于清除数据区指针.发送直接删除指针会引起错误.
	//* 注意：onSendDataAck中不允许再调用任何传输层的方法，比如SendData()等，
	//* 否则会引起死锁和传输层队列错误。
	//sendLen == 0表示发送成功， !=0表示部分发送成功（此时应用层应对网络状况有所察觉）
	virtual int onSendDataAck(int engineId, int id, int nSeq, int sendLen) = 0;

	//发送状态回调
	//statusType == ss_sendQueueThreshold 时， connId不起作用， param 表示内部发送队列长度
	virtual int onSendStatus(int engineId, int connId, int statusType, int param) = 0;

	//定时器回调
	//id			计时器索引
	//context		上下文
	//return	0 正常， 1 取消时钟（对循环时钟有效）
	virtual int onTimeout(int id, int context) = 0;

};

#endif	//_ITPLISTENER_H_

