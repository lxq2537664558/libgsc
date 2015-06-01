package misc;

import java.io.ByteArrayOutputStream;
import java.io.DataInputStream;
import java.io.DataOutputStream;
import java.io.IOException;
import java.io.PrintStream;
import java.net.InetSocketAddress;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

/**
 *
 * @author xzw
 * @created on: Apr 21, 2014 2:06:19 PM
 * 
 */
public final class Net
{
	/** 两个字转换成short. */
	public static final short byte2short(byte by[], int ofst)
	{
		int value = 0;
		for (int i = ofst; i < ofst + 2; i++)
			value += ((by[i] & 0xFF) << (8 * (2 - (i - ofst) - 1)));
		return (short) value;
	}

	/** 四字节转换成int. */
	public static final int byte2int(byte by[], int ofst)
	{
		int value = 0;
		for (int i = ofst; i < ofst + 4; i++)
			value += ((by[i] & 0xFF) << (8 * (4 - (i - ofst) - 1)));
		return value;
	}

	/** 八字节转换成long. */
	public static final long byte2long(byte by[], int ofst)
	{
		long value = 0;
		for (int i = ofst; i < ofst + 8; i++)
			value += (long) ((long) (by[i] & 0xFF)) << (8 * (8 - (i - ofst) - 1));
		return value;
	}

	/** short转两字节, 小头在前. */
	public static final byte[] short2byte(short arg)
	{
		return new byte[] { (byte) ((arg >> 8) & 0xFF), (byte) (arg & 0xFF) };
	}

	/** int转四字节, 小头在前. */
	public static final byte[] int2byte(int arg)
	{
		byte by[] = new byte[4];
		for (int i = 0; i < 4; i++)
			by[i] = (byte) (arg >> (24 - i * 8));
		return by;
	}

	/** long转8字节, 小头在前. */
	public static final byte[] long2byte(long arg)
	{
		byte by[] = new byte[8];
		for (int i = 0; i < 8; i++)
			by[i] = (byte) (arg >> (56 - i * 8));
		return by;
	}

	/** 将long字节序倒换. */
	public static final long h2l_long(int h)
	{
		long n = 0L;
		n |= ((h << 56) & 0xFF00000000000000L);
		n |= ((h << 40) & 0x00FF000000000000L);
		n |= ((h << 24) & 0x0000FF0000000000L);
		n |= ((h << 8) & 0x000000FF00000000L);
		n |= ((h >> 8) & 0x00000000FF000000L);
		n |= ((h >> 24) & 0x0000000000FF0000L);
		n |= ((h >> 40) & 0x000000000000FF00L);
		n |= ((h >> 56) & 0x00000000000000FFL);
		return n;
	}

	/** 将int字节序倒换. */
	public static final int h2l_int(int h)
	{
		return ((h >> 24) & 0xFF) + ((((h >> 16) & 0xFF) << 8) & 0xFF00) + ((((h >> 8) & 0xFF) << 16) & 0xFF0000) + (((h & 0xFF) << 24) & 0xFF000000);
	}

	/** 将short字节序倒换. */
	public static final short h2l_short(short h)
	{
		return (short) (((h >> 8) & 0xFF) + (((h & 0xFF) << 8) & 0xFF00));
	}

	/** 16进制整形字符串(一定是4字节, 如"FFFF")转换成短整形. */
	public static final short hex2short(String hex)
	{
		char hx[] = hex.toCharArray();
		int ret = 0x00;
		for (int i = 0; i < 4; ++i)
			ret += (hx[i] < 65 ? hx[i] - 0x30 : (hx[i] < 97 ? hx[i] - 55 : hx[i] - 87)) << (3 - i) * 4;
		return (short) (ret & 0xFFFF);
	}

	/** 16进制整形字符串(一定是8字节, 如"FFFFFFFF")转换成整形. */
	public static final int hex2int(String hex)
	{
		char hx[] = hex.toCharArray();
		long ret = 0x00;
		for (int i = 0; i < 8; ++i)
			ret += (hx[i] < 65 ? hx[i] - 0x30 : (hx[i] < 97 ? hx[i] - 55 : hx[i] - 87)) << (7 - i) * 4;
		return (short) (ret & 0xFFFFFFFF);
	}

	/** 将字节流转换成十六进制字符串(紧凑格式). */
	public static final String byte2HexStr(byte by[])
	{
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		PrintStream ps = new PrintStream(bos);
		for (int i = 0; i < by.length; i++)
			ps.printf("%02X", by[i]);
		return bos.toString();
	}

	/** 将字节流转换成十六进制字符串. */
	public static final String printBytes(byte by[], int ofst, int length)
	{
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		PrintStream ps = new PrintStream(bos);
		int rows = length / 16;
		int ac = length % 16;
		for (int i = 0; i < rows; ++i)
			ps.printf("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", //
					by[ofst + (16 * i) + 0], //
					by[ofst + (16 * i) + 1], //
					by[ofst + (16 * i) + 2], //
					by[ofst + (16 * i) + 3], //
					by[ofst + (16 * i) + 4], //
					by[ofst + (16 * i) + 5], //
					by[ofst + (16 * i) + 6], //
					by[ofst + (16 * i) + 7], //
					by[ofst + (16 * i) + 8], //
					by[ofst + (16 * i) + 9], //
					by[ofst + (16 * i) + 10], //
					by[ofst + (16 * i) + 11], //
					by[ofst + (16 * i) + 12], //
					by[ofst + (16 * i) + 13], //
					by[ofst + (16 * i) + 14], //
					by[ofst + (16 * i) + 15], //
					Net.toc(by[ofst + (16 * i) + 0]), //
					Net.toc(by[ofst + (16 * i) + 1]), //
					Net.toc(by[ofst + (16 * i) + 2]), //
					Net.toc(by[ofst + (16 * i) + 3]), //
					Net.toc(by[ofst + (16 * i) + 4]), //
					Net.toc(by[ofst + (16 * i) + 5]), //
					Net.toc(by[ofst + (16 * i) + 6]), //
					Net.toc(by[ofst + (16 * i) + 7]), //
					Net.toc(by[ofst + (16 * i) + 8]), //
					Net.toc(by[ofst + (16 * i) + 9]), //
					Net.toc(by[ofst + (16 * i) + 10]), //
					Net.toc(by[ofst + (16 * i) + 11]), //
					Net.toc(by[ofst + (16 * i) + 12]), //
					Net.toc(by[ofst + (16 * i) + 13]), //
					Net.toc(by[ofst + (16 * i) + 14]), //
					Net.toc(by[ofst + (16 * i) + 15]));
		for (int i = 0; i < ac; i++)
			ps.printf("%02X ", by[ofst + rows * 16 + i]);
		for (int i = 0; ac > 0 && i < 16 - ac; i++)
			ps.printf("%s", "   ");
		for (int i = 0; i < ac; i++)
			ps.printf("%c", toc(by[ofst + rows * 16 + i]));
		return bos.toString();
	}

	/** 四字节整数打印成二进制字符串. */
	public static final String int2BinStr(int val)
	{
		String str = Integer.toBinaryString(val);
		int len = str.length();
		if (len >= 0x20)
			return str;
		char chr[] = new char[0x20 - len];
		for (int i = 0; i < chr.length; ++i)
			chr[i] = '0';
		return new String(chr) + str;
	}

	/** 八字节整数打印成二进制字符串. */
	public static final String long2BinStr(long val)
	{
		String str = Long.toBinaryString(val);
		int len = str.length();
		if (len >= 0x40)
			return str;
		char chr[] = new char[0x40 - len];
		for (int i = 0; i < chr.length; ++i)
			chr[i] = '0';
		return new String(chr) + str;
	}

	/** 将IP(ipv4)转换成四字节整数. */
	public static final long byte2ip(byte addr[])
	{
		if (addr == null || addr.length != 0x04)
		{
			Log.error("ipv6?");
			return 0;
		}
		return ((addr[0] << 24) & 0xFF000000L) + ((addr[1] << 16) & 0x00FF0000L) + ((addr[2] << 8) & 0x0000FF00L) + (addr[3] & 0x000000FFL);
	}

	/** 将IP地址转换成字符串表现形式. */
	public static final String ip2str(long ip)
	{
		return ((ip >> 24) & 0xFF) + "." + ((ip >> 16) & 0xFF) + "." + ((ip >> 8) & 0xFF) + "." + (ip & 0xFF);
	}

	/** 字符串(如电话号码)转成BCD数组. */
	public static final byte[] str2bcd(String str)
	{
		try
		{
			if (!Misc.isDigital(str))
				return null;
			byte by[] = str.getBytes("ISO-8859-1");
			int k = by.length / 2;
			byte ret[] = new byte[(by.length % 2) + k];
			for (int i = 0; i < k; ++i)
				ret[i] = (byte) ((by[i * 2] & 0x0F) + (((by[i * 2 + 1] & 0x0F) << 4) & 0xF0));
			if (by.length % 2 == 1)
				ret[ret.length - 1] = (byte) ((by[by.length - 1] & 0x0F) + 0xF0);
			return ret;
		} catch (Exception e)
		{
			return null;
		}
	}

	/** 将缓冲区的内容读出并扔掉. */
	public static final void readAndDicard(SocketChannel sc, ByteBuffer bb)
	{
		try
		{
			while (true)
			{
				bb.position(0);
				if (sc.read(bb) < 1)
					break;
			}
		} catch (Exception e)
		{
			Log.debug(Log.trace(e));
		}
	}

	/** 返回套接字地址的字符串表现形式. */
	public static final String getAddr(InetSocketAddress addr)
	{
		return addr.getAddress().getHostAddress() + ":" + addr.getPort();
	}

	/** 返回套接字远端地址的字符串表现形式. */
	public static final String getRemoteAddr(SocketChannel sc)
	{
		try
		{
			return Net.getAddr((InetSocketAddress) sc.getRemoteAddress());
		} catch (Exception e)
		{
			return "exception";
		}
	}

	/** 构造一个InetSocketAddress. */
	public static final InetSocketAddress getAddr(String host, int port)
	{
		return new InetSocketAddress(host, port);
	}

	/** 尝试在套接字上发送一段数据. */
	public static final boolean send(DataOutputStream dos, byte by[])
	{
		try
		{
			dos.write(by);
			return true;
		} catch (IOException e)
		{
			Log.debug(Log.trace(e));
			return false;
		}
	}

	/** 关闭套接字通道. */
	public static final void closeSocketChannel(SocketChannel sc)
	{
		try
		{
			if (sc != null)
				sc.close();
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
		}
	}

	/** 关闭套接字. */
	public static final void closeSocket(Socket s)
	{
		if (s != null)
		{
			try
			{
				s.close();
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
			}
			s = null;
		}
	}

	/** 关闭套接字. */
	public static final void closeSocket(DataInputStream dis, DataOutputStream dos, Socket sock)
	{
		Misc.closeInputStream(dis);
		Misc.closeOutputStream(dos);
		Net.closeSocket(sock);
	}

	/** 返回可打印字符. */
	private static final char toc(byte chr)
	{
		return (chr > 0x20 && chr < 0x7F) ? (char) chr : '.';
	}
}
