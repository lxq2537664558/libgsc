package stmp;

import java.util.HashMap;

import com.google.protobuf.Descriptors.EnumDescriptor;
import com.google.protobuf.Descriptors.EnumValueDescriptor;

/**
 * 
 * @author xuzewen
 * 
 */
public final class Stmp
{
	public static final byte __STMP_LEN_0xFE__ = (byte) 0xFE;
	public static final byte __STMP_LEN_0xFF__ = (byte) 0xFF;
	private static final String __IEI_UNKNOWN__ = "IEI_UNKNOWN";
	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/** 事务部分. */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 事务开始. */
	public static final byte STMP_TAG_TRANS_BEGIN = 0x60;
	/** 事务结束. */
	public static final byte STMP_TAG_TRANS_END = 0x61;
	/** 事务继续. */
	public static final byte STMP_TAG_TRANS_CONTINUE = 0x62;
	/** 消息前转. */
	public static final byte STMP_TAG_TRANS_SWITCH = 0x63;
	/** 事务中断. */
	public static final byte STMP_TAG_TRANS_ABORT = 0x64;
	/** 事务取消. */
	public static final byte STMP_TAG_TRANS_CANCEL = 0x65;
	/** 单向消息. */
	public static final byte STMP_TAG_TRANS_UNI = 0x66;
	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/** 一般信元. */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	/** 加密块. */
	public static final byte STMP_TAG_SEC = 0x67;
	/** 扩展. */
	public static final byte STMP_TAG_EXT = 0x68;
	//
	/** 源事务ID. */
	public static final byte STMP_TAG_STID = 0x00;
	/** 目的事务ID. */
	public static final byte STMP_TAG_DTID = 0x01;
	/** 用户标识. */
	public static final byte STMP_TAG_UID = 0x02;
	/** 会话标识. */
	public static final byte STMP_TAG_SID = 0x03;
	/** 命令字. */
	public static final byte STMP_TAG_CMD = 0x04;
	/** 返回值. */
	public static final byte STMP_TAG_RET = 0x05;
	/** 数据. */
	public static final byte STMP_TAG_DAT = 0x06;
	/** 附件(attachment). */
	public static final byte STMP_TAG_ATT = 0x07;

	/** ---------------------------------------------------------------- */
	/**                                                                  */
	/** 返回值. */
	/**                                                                  */
	/** ---------------------------------------------------------------- */
	public enum Ret implements com.google.protobuf.ProtocolMessageEnum
	{
		RET_SUCCESS, /** 成功. */
		RET_FAILURE, /** 失败. */
		RET_INVALID, /** 无效. */
		RET_PRESENT, /** 有. */
		RET_NOT_PRESENT, /** 没有. */
		RET_EXCEPTION, /** 异常. */
		RET_NOT_FOUND, /** 找不到. */
		RET_TIME_OUT, /** 超时. */
		RET_SID_ERROR, /** 会话id错误, 适用于短连接. */
		/** 应用自定义起始0x10. */
		RET_USR_DEF;

		public EnumDescriptor getDescriptorForType()
		{
			return null;
		}

		public int getNumber()
		{
			return this.ordinal();
		}

		public EnumValueDescriptor getValueDescriptor()
		{
			return null;
		}
	}

	/** ----------------------------------------------------- */
	/**                                                       */
	/**  */
	/**                                                       */
	/** ----------------------------------------------------- */
	private static final HashMap<Short, String> tags = new HashMap<Short, String>();

	static
	{
		tags.put((short) Stmp.STMP_TAG_TRANS_BEGIN, "TRANS_BEGIN");
		tags.put((short) Stmp.STMP_TAG_TRANS_END, "TRANS_END");
		tags.put((short) Stmp.STMP_TAG_TRANS_CONTINUE, "TRANS_CONTINUE");
		tags.put((short) Stmp.STMP_TAG_TRANS_SWITCH, "TRANS_SWITCH");
		tags.put((short) Stmp.STMP_TAG_TRANS_ABORT, "TRANS_ABORT");
		tags.put((short) Stmp.STMP_TAG_TRANS_CANCEL, "TRANS_CANCEL");
		tags.put((short) Stmp.STMP_TAG_TRANS_UNI, "TRANS_UNI");
		tags.put((short) Stmp.STMP_TAG_SEC, "SEC");
		tags.put((short) Stmp.STMP_TAG_EXT, "EXT");
		//
		tags.put((short) Stmp.STMP_TAG_STID, "STID");
		tags.put((short) Stmp.STMP_TAG_DTID, "DTID");
		tags.put((short) Stmp.STMP_TAG_UID, "UID");
		tags.put((short) Stmp.STMP_TAG_SID, "SID");
		tags.put((short) Stmp.STMP_TAG_CMD, "CMD");
		tags.put((short) Stmp.STMP_TAG_RET, "RET");
		tags.put((short) Stmp.STMP_TAG_DAT, "DAT");
		tags.put((short) Stmp.STMP_TAG_ATT, "ATT");
	}

	/** 返回tag的字符串表现形式, 用于日志. */
	public static final String tagDesc(short t)
	{
		String desc = Stmp.tags.get(t);
		return desc == null ? Stmp.__IEI_UNKNOWN__ : desc;
	}

	/** 获得len影响的l字段的长度, 可能的返回值是1, 表示l只需一个字表示, 3, 表示需两个字节表示(ushort), 5, 表示需4个字节表示(uint). */
	public static final int tlvLen(int len)
	{
		return len < 0x000000FE ? 1 : (len <= 0x0000FFFF ? 3 : 5);
	}
}
