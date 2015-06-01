package core;

import java.lang.reflect.InvocationTargetException;
import java.lang.reflect.Method;

import misc.Log;

import com.google.protobuf.AbstractMessage;
import com.google.protobuf.InvalidProtocolBufferException;
import com.google.protobuf.Message;
import com.google.protobuf.Message.Builder;
import com.google.protobuf.ProtocolMessageEnum;

/**
 * 
 * 与网络相关的消息回调.
 *
 * @Created on: 2015年1月8日 下午7:32:06
 * @Author: xuzewen
 * 
 */
public class Cb
{
	/** 命令字. */
	public ProtocolMessageEnum cmd;
	/** 鉴权前/后. */
	public boolean gusr;
	/** 回调地址. */
	public Method cb;
	/** STMP-BEGIN pb消息的newBulider. */
	public Method beginBuilder;
	public String beginName;
	/** STMP-END pb消息的newBulider. */
	public Method endBuilder;
	public String endName;
	/** STMP-UNI pb消息的newBulider. */
	public Method uniBuilder;
	public String uniName;

	public Cb(ProtocolMessageEnum cmd, Method cb, Class<?> begin, Class<?> end, Class<?> uni, boolean gusr)
	{
		this.gusr = gusr;
		this.cmd = cmd;
		this.cb = cb;
		try
		{
			if (begin != null)
			{
				this.beginBuilder = begin.getMethod("newBuilder");
				this.beginName = begin.getSimpleName();
			}
			if (end != null)
			{
				this.endBuilder = end.getMethod("newBuilder");
				this.endName = end.getSimpleName();
			}
			if (uni != null)
			{
				this.uniBuilder = uni.getMethod("newBuilder");
				this.uniName = uni.getSimpleName();
			}
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
		}
	}

	/** 反射回一个STMP-BEGIN中的pb对象. */
	public final Message newBeginMsg(byte by[]) throws InvalidProtocolBufferException, IllegalAccessException, IllegalArgumentException, InvocationTargetException
	{
		return ((AbstractMessage.Builder<?>) this.beginBuilder.invoke(null)).mergeFrom(by).build();
	}

	/** 反射回一个STMP-END中的pb-builder. */
	public final Builder newEndBulider() throws IllegalAccessException, IllegalArgumentException, InvocationTargetException
	{
		return ((AbstractMessage.Builder<?>) this.endBuilder.invoke(null));
	}

	/** 反射回一个STMP-UNI中的pb-builder. */
	public final Builder newUniBulider() throws IllegalAccessException, IllegalArgumentException, InvocationTargetException
	{
		return ((AbstractMessage.Builder<?>) this.uniBuilder.invoke(null));
	}
}
