package core;

import com.google.protobuf.Message;

/**
 *
 * @Created on: 2015年1月8日 下午7:06:21
 * @Author: xuzewen
 * 
 */
public class Gend
{
	/** 返回值. */
	public short ret = 0x00;
	/** pb消息. */
	public Message end = null;
	/** 扩展数据. */
	public byte[] extend = null;
}
