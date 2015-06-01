package stmp;

import misc.Net;

/**
 * 
 * @author xuzewen
 * 
 */
public class StmpEnc
{
	/** 添加一个字节, t一定是一个简单体. */
	public static void addByte(StmpPdu sp, short t, byte v)
	{
		StmpEnc.addBin(sp, t, new byte[] { v }, 0, 1);
	}

	/** 添加一个字符串, t一定是一个简单体. */
	public static void addStr(StmpPdu sp, short t, String str)
	{
		byte by[] = str.getBytes();
		StmpEnc.addBin(sp, t, by, 0, by.length);
	}

	/** 添加一个short, t一定是一个简单体. */
	public static void addShort(StmpPdu sp, short t, short v)
	{
		StmpEnc.addBin(sp, t, Net.short2byte(v), 0, 2);
	}

	/** 添加一个int, t一定是一个简单体. */
	public static void addInt(StmpPdu sp, short t, int v)
	{
		StmpEnc.addBin(sp, t, Net.int2byte(v), 0, 4);
	}

	/** 添加一个long, t一定是一个简单体. */
	public static void addLong(StmpPdu sp, short t, long v)
	{
		StmpEnc.addBin(sp, t, Net.long2byte(v), 0, 8);
	}

	/** 添加一个字节数组. */
	public static final void addBin(StmpPdu sp, short t, byte[] v)
	{
		StmpEnc.addBin(sp, t, v, 0, v.length);
	}

	/** 添加一个字节数组. */
	public static final void addBin(StmpPdu sp, short t, byte[] v, int offset, int length)
	{
		int ll = Stmp.tlvLen(length);
		int tl;
		if ((t & 0xFF00) != 0)
		{
			tl = 2;
			sp.rm -= (ll + tl + length);
			System.arraycopy(Net.short2byte(t), 0, sp.buff, sp.rm, 2);
		} else
		{
			tl = 1;
			sp.rm -= (ll + tl + length);
			System.arraycopy(new byte[] { (byte) t }, 0, sp.buff, sp.rm, 1);
		}
		if (ll == 1)
		{
			System.arraycopy(new byte[] { (byte) length }, 0, sp.buff, sp.rm + tl, 1);
		} else if (ll == 3)
		{
			System.arraycopy(new byte[] { Stmp.__STMP_LEN_0xFE__ }, 0, sp.buff, sp.rm + tl, 1);
			System.arraycopy(Net.short2byte((short) length), 0, sp.buff, sp.rm + tl + 1, 2);
		} else if (ll == 5)
		{
			System.arraycopy(new byte[] { Stmp.__STMP_LEN_0xFF__ }, 0, sp.buff, sp.rm + tl, 1);
			System.arraycopy(Net.int2byte(length), 0, sp.buff, sp.rm + tl + 1, 4);
		}
		System.arraycopy(v, offset, sp.buff, sp.rm + tl + ll, length);
	}

	/** 记住sp的当前位置, 将从这里准备开始添加一个子构造体的信元(这些信元必需都是简单体). */
	public static final void setPoint(StmpPdu sp)
	{
		sp.p = sp.rm;
	}

	/** 前置一个tag. */
	public static final void addTag(StmpPdu sp, short t)
	{
		StmpEnc.addTag__(sp, t, sp.buff.length - sp.rm);
	}

	/** 前置一个tag, StmpEnc.setPoint总是应该在此函数之前调用. */
	public static final void addTag4Point(StmpPdu sp, short t)
	{
		StmpEnc.addTag__(sp, t, sp.p - sp.rm);
	}

	/** 重置PDU位置指针. */
	public static final void reset(StmpPdu pdu)
	{
		pdu.rm = pdu.buff.length;
	}

	private static final void addTag__(StmpPdu sp, short t, int len)
	{
		int ll = Stmp.tlvLen(len);
		int tl;
		if ((t & 0xFF00) != 0)
		{
			tl = 2;
			sp.rm -= (ll + tl);
			System.arraycopy(Net.short2byte(t), 0, sp.buff, sp.rm, 2);
		} else
		{
			tl = 1;
			sp.rm -= (ll + tl);
			System.arraycopy(new byte[] { (byte) t }, 0, sp.buff, sp.rm, 1);
		}
		if (ll == 1)
		{
			System.arraycopy(new byte[] { (byte) len }, 0, sp.buff, sp.rm + tl, 1);
		} else if (ll == 3)
		{
			System.arraycopy(new byte[] { Stmp.__STMP_LEN_0xFE__ }, 0, sp.buff, sp.rm + tl, 1);
			System.arraycopy(Net.short2byte((short) len), 0, sp.buff, sp.rm + tl + 1, 2);
		} else if (ll == 5)
		{
			System.arraycopy(new byte[] { Stmp.__STMP_LEN_0xFF__ }, 0, sp.buff, sp.rm + tl, 1);
			System.arraycopy(Net.int2byte(len), 0, sp.buff, sp.rm + tl + 1, 4);
		}
	}
}
