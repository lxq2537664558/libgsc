package misc;

import java.io.BufferedReader;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.PrintStream;
import java.io.RandomAccessFile;
import java.lang.reflect.Method;
import java.net.URLDecoder;
import java.net.URLEncoder;
import java.security.Key;
import java.security.MessageDigest;
import java.sql.CallableStatement;
import java.sql.Connection;
import java.sql.PreparedStatement;
import java.sql.ResultSet;
import java.text.DecimalFormat;
import java.util.ArrayList;
import java.util.Base64;
import java.util.Comparator;
import java.util.Map;
import java.util.Map.Entry;
import java.util.concurrent.ThreadLocalRandom;

import javax.crypto.Cipher;
import javax.crypto.spec.SecretKeySpec;

import com.google.gson.Gson;
import com.google.gson.reflect.TypeToken;
import com.google.protobuf.Message;
import com.googlecode.protobuf.format.JsonFormat;

/**
 * 
 * 常用工具函数.
 * 
 * @author xuzewen
 * @create on 2007-12-01
 * 
 */
public final class Misc
{
	public static final String UTF_8 = "UTF-8";
	public static final String GBK = "GBK";
	public static final String ISO_8859_1 = "ISO-8859-1";
	public static final String COMMA = ",";
	public static final String COMMA2 = ";";
	public static final String SPACE = " ";
	public static final String LINE = "\n";
	public static final String LINE_R_N = "\r\n";
	public static final String _LINE = "_";
	public static final String UNIX_FILE_SEPARATOR = "/";
	public static final int SUCCESS = 0;
	public static final int FAILED = -1;
	private static char __0aA__[] = { '_', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
	private static final DecimalFormat df = new DecimalFormat("#0.00");
	private static final PrintStream out = new PrintStream(System.out);
	private static final BufferedReader in = new BufferedReader(new InputStreamReader(System.in));
	private static final ThreadLocalRandom rand = ThreadLocalRandom.current();
	private static final Gson gson = new Gson();

	/** 从标准输入读取一行. */
	public static final String readLine()
	{
		try
		{
			return Misc.in.readLine();
		} catch (IOException e)
		{
			return null;
		}
	}

	/** 去除首尾空格. */
	public static final String trim(String arg)
	{
		return arg == null ? null : ("".equals(arg.trim()) ? null : arg.trim());
	}

	/** 异常时返回零, 否则值总是 >=0. */
	public static final long forceLongO(String arg)
	{
		try
		{
			long value = Long.parseLong(trim(arg));
			return value < 0 ? 0 : value;
		} catch (Exception e)
		{
			return 0;
		}
	}

	/** 异常时返回-1. */
	public static final int int_1(String arg)
	{
		try
		{
			return Integer.parseInt(trim(arg));
		} catch (Exception e)
		{
			return -1;
		}
	}

	/** 异常时返回-1, 否则值总是 >=-1. */
	public static final int forceInt_1(String arg)
	{
		try
		{
			int value = Integer.parseInt(trim(arg));
			return value < -1 ? -1 : value;
		} catch (Exception e)
		{
			return -1;
		}
	}

	/** 异常时返回零. */
	public static final int int0(String arg)
	{
		try
		{
			return Integer.parseInt(trim(arg));
		} catch (Exception e)
		{
			return 0;
		}
	}

	/** 异常时返回零, 否则值总是 >=0. */
	public static final int forceInt0(String arg)
	{
		try
		{
			int value = Integer.parseInt(trim(arg));
			return value < 0 ? 0 : value;
		} catch (Exception e)
		{
			return 0;
		}
	}

	/** 异常时返回1. */
	public static final int int1(String arg)
	{
		try
		{
			return Integer.parseInt(trim(arg));
		} catch (Exception e)
		{
			return 1;
		}
	}

	/** 异常时返回1, 否则值总是 >= 1 */
	public static final int forceInt1(String arg)
	{
		try
		{
			int value = Integer.parseInt(trim(arg));
			return value < 1 ? 1 : value;
		} catch (Exception e)
		{
			return 1;
		}
	}

	/** 异常时返回0, 否则总是 >= 0. */
	public static final float forceFloat0(String arg)
	{
		try
		{
			float value = Float.parseFloat(trim(arg));
			return value < 0 ? 0 : value;
		} catch (Exception e)
		{
			return 0;
		}
	}

	/** 异常时返回1.0, 否则总是 >= 1.0. */
	public static final float forceFloat1(String arg)
	{
		try
		{
			float value = Float.parseFloat(trim(arg));
			return value < 1.0f ? 1.0f : value;
		} catch (Exception e)
		{
			return 1.0f;
		}
	}

	/** 异常时返回-1, 否则值总是 >= -1 */
	public static final float forceFloat_1(String arg)
	{
		try
		{
			float value = Float.parseFloat(trim(arg));
			return value < -1.00f ? -1.00f : value;
		} catch (Exception e)
		{
			return -1;
		}
	}

	/** 异常时返回0. */
	public static final float float0(String arg)
	{
		try
		{
			return Float.parseFloat(trim(arg));
		} catch (Exception e)
		{
			return 0;
		}
	}

	/** 将逗号表达式分隔的整形字符串转换成整形数组, 如:"1,2,3,4,5", 异常时返回null. */
	public static final int[] parseIntArr(String str)
	{
		String arr[] = str.split(Misc.COMMA);
		int ia[] = new int[arr.length];
		try
		{
			for (int i = 0; i < ia.length; ++i)
				ia[i] = Integer.parseInt(Misc.trim(arr[i]));
			return ia;
		} catch (Exception e)
		{
			return null;
		}
	}

	/** 判断字符串是否为NULL. */
	public static final boolean isNull(String arg)
	{
		return Misc.trim(arg) == null;
	}

	/** parse为一个byte. */
	public static final Byte parseByte(String arg)
	{
		try
		{
			return Byte.parseByte(trim(arg));
		} catch (Exception e)
		{
			return null;
		}
	}

	/** return URLEncoder.encode(str, UTF_8); */
	public static final String encode(String str)
	{
		try
		{
			return URLEncoder.encode(trim(str), UTF_8);
		} catch (Exception e)
		{
			return str;
		}
	}

	/** return URLDecoder.decode(str, UTF_8); */
	public static final String decode(String str)
	{
		try
		{
			return URLDecoder.decode(trim(str), UTF_8);
		} catch (Exception e)
		{
			return str;
		}
	}

	/** 是不是一串数字. */
	public static final boolean isDigital(String arg)
	{
		arg = trim(arg);
		if (arg == null)
			return false;
		char[] array = arg.toCharArray();
		for (char chr : array)
		{
			if (!Character.isDigit(chr))
				return false;
		}
		return true;
	}

	/** 替代c的printf. */
	public static final void printf(String format, Object... args)
	{
		Misc.out.printf(format, args);
	}

	/** 打印byte[]到标准输出. */
	public static final void printf(byte by[])
	{
		Misc.printf("%s\n", Net.printBytes(by, 0, by.length));
	}

	/** 打印byte[]到标准输出. */
	public static final void printf(byte by[], int offset, int length)
	{
		Misc.printf(Net.printBytes(by, offset, length));
	}

	/** 替代c的printf, 将打印的结果以字符串方式返回. */
	public static final String printf2Str(String format, Object... args)
	{
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		PrintStream ps = new PrintStream(bos);
		ps.printf(format, args);
		return bos.toString();
	}

	/** 将字节流转换成十六进制字符串. */
	public static final String printBytes(byte by[])
	{
		return Net.printBytes(by, 0, by.length);
	}

	/** 将字节流转换成十六进制字符串. */
	public static final String printBytes(byte by[], int offset, int length)
	{
		return Net.printBytes(by, offset, length);
	}

	/** format to #0.00. */
	public static final String decimalformat(double val)
	{
		return Misc.df.format(val);
	}

	/** 判断是不是一个可能合法的email格式. */
	public static final boolean isEmail(String email)
	{
		if (email == null)
			return false;
		byte by[] = email.getBytes();
		for (int i = 0; i < by.length; i++)
		{
			if (by[i] != 0x2E && by[i] != 0x40 && (by[i] < 0x30 || by[i] > 0x39) && (by[i] < 0x61 || by[i] > 0x7A))
				return false;
		}
		return true;
	}

	/** 生成一个随机数(正长整形). */
	public static final long randLang()
	{
		return Misc.rand.nextLong() & 0x7FFFFFFFFFFFFFFFL;
	}

	/** 生成一个随机数(正整形). */
	public static final int randInt()
	{
		return (int) (Misc.rand.nextLong() & 0x7FFFFFFF);
	}

	/** 产生一个128位(16个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	public static final String gen0aAkey128()
	{
		byte by[] = new byte[16];
		for (int i = 0; i < 4; ++i)
		{
			int x = Misc.randInt();
			by[4 * i + 0] = (byte) __0aA__[(x) % __0aA__.length];
			by[4 * i + 1] = (byte) __0aA__[(x >> 0x08) % __0aA__.length];
			by[4 * i + 2] = (byte) __0aA__[(x >> 0x10) % __0aA__.length];
			by[4 * i + 3] = (byte) __0aA__[(x >> 0x18) % __0aA__.length];
		}
		return new String(by);
	}

	/** 产生一个256位(32个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	public static final String gen0aAkey256()
	{
		byte by[] = new byte[32];
		for (int i = 0; i < 8; ++i)
		{
			int x = Misc.randInt();
			by[4 * i + 0] = (byte) __0aA__[(x) % __0aA__.length];
			by[4 * i + 1] = (byte) __0aA__[(x >> 0x08) % __0aA__.length];
			by[4 * i + 2] = (byte) __0aA__[(x >> 0x10) % __0aA__.length];
			by[4 * i + 3] = (byte) __0aA__[(x >> 0x18) % __0aA__.length];
		}
		return new String(by);
	}

	/** 产生一个512位(64个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	public static final String gen0aAkey512()
	{
		byte by[] = new byte[64];
		for (int i = 0; i < 8; ++i)
		{
			int x = Misc.randInt();
			by[8 * i + 0] = (byte) __0aA__[(x) % __0aA__.length];
			by[8 * i + 1] = (byte) __0aA__[(x >> 0x08) % __0aA__.length];
			by[8 * i + 2] = (byte) __0aA__[(x >> 0x10) % __0aA__.length];
			by[8 * i + 3] = (byte) __0aA__[(x >> 0x18) % __0aA__.length];
			x = Misc.randInt();
			by[8 * i + 4] = (byte) __0aA__[(x) % __0aA__.length];
			by[8 * i + 5] = (byte) __0aA__[(x >> 0x08) % __0aA__.length];
			by[8 * i + 6] = (byte) __0aA__[(x >> 0x10) % __0aA__.length];
			by[8 * i + 7] = (byte) __0aA__[(x >> 0x18) % __0aA__.length];
		}
		return new String(by);
	}

	/** sleep毫秒. */
	public static final void sleep(long ms)
	{
		try
		{
			Thread.sleep(ms);
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
		}
	}

	/** 导致当前线程挂起. */
	public static final void hold()
	{
		try
		{
			Thread.currentThread().join();
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	/** 按行加载整个文件. */
	public static final ArrayList<String> loadFileLines(String path)
	{
		RandomAccessFile raf = null;
		try
		{
			ArrayList<String> lines = new ArrayList<>();
			raf = new RandomAccessFile(path, "r");
			String line = raf.readLine();
			while (line != null)
			{
				lines.add(line);
				line = raf.readLine();
			}
			return lines;
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
			return null;
		} finally
		{
			Misc.closeRaf(raf);
		}
	}

	/** 关闭数据库连接. */
	public static final void closeConn(Connection conn, PreparedStatement pst, ResultSet rst, CallableStatement call)
	{
		if (rst != null)
		{
			try
			{
				rst.close();
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
			}
		}
		rst = null;
		if (pst != null)
		{
			try
			{
				pst.close();
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
			}
		}
		pst = null;
		if (call != null)
		{
			try
			{
				call.close();
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
			}
		}
		call = null;
		if (conn != null)
		{
			try
			{
				conn.close();
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
			}
		}
		conn = null;
	}

	/** 关闭输入流. */
	public static final void closeInputStream(InputStream ins)
	{
		if (ins != null)
		{
			try
			{
				ins.close();
			} catch (IOException e)
			{
				Log.error(Log.trace(e));
			}
			ins = null;
		}
	}

	/** 关闭输出流. */
	public static final void closeOutputStream(OutputStream ops)
	{
		if (ops != null)
		{
			try
			{

				ops.close();
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
			}
			ops = null;
		}
	}

	/** 关闭随机文件读取句柄. */
	public static final void closeRaf(RandomAccessFile raf)
	{
		if (raf != null)
		{
			try
			{
				raf.close();
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
			}
			raf = null;
		}
	}

	/** BASE-ENCODE. */
	public static final String base64enc(byte by[])
	{
		return Base64.getEncoder().encodeToString(by);
	}

	/** BASE-DECODE. */
	public static final byte[] base64dec(String str)
	{
		return Base64.getDecoder().decode(str);
	}

	/** RC4加密. */
	public static final byte[] rc4enc(byte key[], byte[] org)
	{
		return Misc.rc4enc(key, org, 0, org.length);
	}

	/** RC4加密. */
	public static final byte[] rc4enc(byte key[], byte[] org, int ofst, int len)
	{
		try
		{
			Key k = new SecretKeySpec(key, "RC4");
			Cipher cip = Cipher.getInstance("RC4");
			cip.init(Cipher.ENCRYPT_MODE, k);
			return cip.doFinal(org, ofst, len);
		} catch (Exception ex)
		{
			Log.error("call RC4-enc function failed: %s", ex.toString());
			return null;
		}
	}

	/** RC4解密. */
	public static final byte[] rc4dec(byte key[], byte[] crypto)
	{
		return Misc.rc4dec(key, crypto, 0, crypto.length);
	}

	/** RC4解密. */
	public static final byte[] rc4dec(byte key[], byte[] crypto, int ofst, int len)
	{
		try
		{
			Key k = new SecretKeySpec(key, "RC4");
			Cipher cip = Cipher.getInstance("RC4");
			cip.init(Cipher.DECRYPT_MODE, k);
			return cip.doFinal(crypto, ofst, len);
		} catch (Exception ex)
		{
			Log.error("call RC4-dec function failed: %s", ex.toString());
			return null;
		}
	}

	/** MD5摘要算法. */
	public static final byte[] md5(byte[] by)
	{
		try
		{
			return MessageDigest.getInstance("MD5").digest(by);
		} catch (Exception ex)
		{
			Log.error("call MD5 function failed: %s", ex.toString());
			return null;
		}
	}

	/** MD5摘要算法, 以大写16进制字符串方式返回. */
	public static final String md5Str(byte by[])
	{
		return Net.byte2HexStr(Misc.md5(by));
	}

	/** SHA-1摘要算法. */
	public static final byte[] sha1(byte[] by)
	{
		try
		{
			return MessageDigest.getInstance("SHA-1").digest(by);
		} catch (Exception ex)
		{
			Log.error("call SHA-1 function failed: %s", ex.toString());
			return null;
		}
	}

	/** SHA-256摘要算法. */
	public static final byte[] sha256(byte[] by)
	{
		try
		{
			return MessageDigest.getInstance("SHA-256").digest(by);
		} catch (Exception ex)
		{
			Log.error("call SHA-256 function failed: %s", ex.toString());
			return null;
		}
	}

	/** SHA-256摘要算法, 以大写16进制字符串方式返回. */
	public static final String sha256Str(byte by[])
	{
		return Net.byte2HexStr(Misc.sha256(by));
	}

	/** SHA-512摘要算法. */
	public static final byte[] sha512(byte[] by)
	{
		try
		{
			return MessageDigest.getInstance("SHA-512").digest(by);
		} catch (Exception ex)
		{
			Log.error("call SHA-512 function failed: %s", ex.toString());
			return null;
		}
	}

	/** SHA-512摘要算法, 以大写16进制字符串方式返回. */
	public static final String sha512Str(byte by[])
	{
		return Net.byte2HexStr(Misc.sha512(by));
	}

	/** 以JNI标准格式返回函数的参数列表. */
	public static final String getMethodPars(Class<?> cls, String funName)
	{
		Method m[] = cls.getDeclaredMethods();
		StringBuilder strb = new StringBuilder();
		for (int i = 0; m != null && i < m.length; i++)
		{
			if (m[i].getName().equals(funName))
			{
				Class<?> x[] = m[i].getParameterTypes();
				for (int j = 0; x != null && j < x.length; j++)
					strb.append("L").append(x[j].getName()).append(";");
			}
		}
		String str = strb.toString();
		if (str.length() < 2)
			return null;
		return strb.toString().replace('.', '/');
	}

	/** 通过函数名返回函数指针, 不支持重载函数. */
	public static final Method findMethodByName(Class<?> cls, String name)
	{
		Method m[] = cls.getDeclaredMethods();
		for (int i = 0; i < m.length; i++)
		{
			if (m[i].getName().equals(name))
				return m[i];
		}
		return null;
	}

	/** 静态函数调用, 适用于不关心返回值的场景. */
	public static final void invoke(Method m, Object... args)
	{
		try
		{
			m.invoke(null, args);
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
		}
	}

	/** 将对象序列化. */
	public static final byte[] serialObject(Object o)
	{
		try
		{
			ByteArrayOutputStream baos = new ByteArrayOutputStream();
			ObjectOutputStream oos = new ObjectOutputStream(baos);
			oos.writeObject(o);
			return baos.toByteArray();
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
			return null;
		}
	}

	/** 溯栈. */
	public static final String getStackInfo()
	{
		StackTraceElement[] stacks = new Throwable().getStackTrace();
		StringBuilder strb = new StringBuilder();
		for (int i = 1; i < stacks.length; i++)
			strb.append(stacks[i].toString()).append(Misc.LINE);
		return strb.toString();
	}

	/** pb对象打印. */
	public static final String pb2str(Message msg)
	{
		return JsonFormat.printToString(msg);
	}

	/** pb对象打印到标准输出. */
	public static final void printPb(Message msg)
	{
		System.out.println(Misc.pb2str(msg));
	}

	/** 自动强制转换. */
	@SuppressWarnings("unchecked")
	public static final <T> T get(Object o)
	{
		return (T) o;
	}

	/** json字符串转换成对象数组. */
	public static final <T> ArrayList<T> json2List(String str, Class<T> t)
	{
		return Misc.gson.fromJson(str, new TypeToken<ArrayList<T>>()
		{
		}.getType());
	}

	/** json字符串转换成对象. */
	public static final <T> T json2Obj(String json, Class<T> t)
	{
		return Misc.gson.fromJson(json, t);
	}

	/** 对象转换成json字符串. */
	public static final String obj2json(Object o)
	{
		return Misc.gson.toJson(o);
	}

	/** 设置系统环境变量. */
	public static final void setEnv(String key, Object val)
	{
		System.setProperty(key, val == null ? "" : val.toString());
	}

	/** 获得环境变量的值. */
	public static final String getEnvStr(String key, String def)
	{
		String str = Misc.trim(System.getProperty(key));
		if (str == null)
		{
			str = Misc.trim(System.getenv(key));
			return str == null ? def : str;
		}
		return str;
	}

	/** 获得环境变量的值. */
	public static final int getEnvInt(String key, int def)
	{
		String val = Misc.getEnvStr(key, null);
		return val == null ? def : Misc.forceInt0(val);
	}

	/** 获得并设置(如果未设置)环境变量的值 */
	public static final String getSetEnvStr(String key, String def)
	{
		String val = Misc.getEnvStr(key, def);
		Misc.setEnv(key, val);
		return val;
	}

	/** 获得并设置(如果未设置)环境变量的值 */
	public static final int getSetEnvInt(String key, int def)
	{
		Integer val = Misc.getEnvInt(key, def);
		Misc.setEnv(key, val);
		return val;
	}

	/** 将系统环境变量顺序列出. */
	public static final ArrayList<Map.Entry<Object, Object>> getEnvs()
	{
		ArrayList<Map.Entry<Object, Object>> arr = new ArrayList<Map.Entry<Object, Object>>();
		System.getProperties().entrySet().forEach(o -> arr.add(o));
		arr.sort(new Comparator<Map.Entry<Object, Object>>()
		{
			public int compare(Entry<Object, Object> o1, Entry<Object, Object> o2)
			{
				return o1.getKey().toString().compareTo(o2.getKey().toString());
			}
		});
		return arr;
	}
}
