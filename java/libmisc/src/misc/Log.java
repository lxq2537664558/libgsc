package misc;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.IOException;
import java.io.PrintStream;
import java.io.RandomAccessFile;
import java.lang.management.ManagementFactory;
import java.lang.reflect.Array;
import java.lang.reflect.Field;
import java.net.Socket;
import java.util.Arrays;
import java.util.Comparator;
import java.util.Date;
import java.util.concurrent.ConcurrentLinkedQueue;

import stmp.Stmp;
import stmp.StmpEnc;
import stmp.StmpPdu;

/**
 * 
 * 日志处理.
 * 
 * @author xuzewen
 * @create on 2008-10-01
 * 
 */
public class Log
{
	/** 使能标准输出. */
	public static final byte OUTPUT_STDOUT = 0x01;
	/** 使能文件输出. */
	public static final byte OUTPUT_FILE = 0x02;
	/** 使能网络输出. */
	public static final byte OUTPUT_NET = 0x04;
	//
	private static final byte RECORD = 0x00, TRACE = 0x01, DEBUG = 0x02, INFO = 0x03, WARN = 0x04, ERROR = 0x05, FAULT = 0x06, OPER = 0x07;
	private static byte level = DEBUG;
	private static byte output = Log.OUTPUT_STDOUT;
	private static String ne = null;
	private static String path = "./";
	private static String host = null;
	private static int port = 0;
	private static int pid = 0;
	private static final ConcurrentLinkedQueue<String> queue = new ConcurrentLinkedQueue<>();
	private static Socket sock = null;
	private static byte hb[] = null;
	private static long hbts = 0;

	static
	{
		byte hb[] = "heart-beat".getBytes();
		StmpPdu pdu = new StmpPdu(hb.length);
		StmpEnc.addBin(pdu, Stmp.STMP_TAG_ATT, hb);
		StmpEnc.addShort(pdu, Stmp.STMP_TAG_CMD, (short) 0x0000);
		StmpEnc.addTag(pdu, Stmp.STMP_TAG_TRANS_UNI);
		Log.hb = pdu.bytes();
		try
		{
			Log.pid = Misc.forceInt0(ManagementFactory.getRuntimeMXBean().getName().split("@")[0]);
		} catch (Exception e)
		{
			e.printStackTrace();
		}
	}

	public static final void init(String ne /* network-equipment, 进程唯一标识, 用于日志归属识别. */, String path, String host, int port, int o)
	{
		Log.ne = ne;
		Log.path = path == null ? Log.path : path;
		Log.host = host;
		Log.port = port;
		Log.output |= o;
		//
		if (Log.isOutPutNet())
		{
			new Thread(new Runnable()
			{
				public void run()
				{
					Log.svc();
				}
			}).start();
		}
	}

	/** 一些时候, 进程可能不能立即知道自己的唯一标识, 因此这里提供一个更新方法. */
	public static final void updateNe(String ne)
	{
		Log.ne = ne;
	}

	public static final void setLevel(String lev)
	{
		if ("RECORD".equals(lev))
			Log.setRecord();
		if ("TRACE".equals(lev))
			Log.setTrace();
		if ("DEBUG".equals(lev))
			Log.setDebug();
		if ("INFO".equals(lev))
			Log.setInfo();
		if ("WARN".equals(lev))
			Log.setWarn();
		if ("ERROR".equals(lev))
			Log.setError();
		if ("FAULT".equals(lev))
			Log.setFault();
		if ("OPER".equals(lev))
			Log.setOper();
	}

	public static final void setRecord()
	{
		Log.level = RECORD;
	}

	public static final void setTrace()
	{
		Log.level = TRACE;
	}

	public static final void setDebug()
	{
		Log.level = DEBUG;
	}

	public static final void setInfo()
	{
		Log.level = INFO;
	}

	public static final void setWarn()
	{
		Log.level = WARN;
	}

	public static final void setError()
	{
		Log.level = ERROR;
	}

	public static final void setFault()
	{
		Log.level = FAULT;
	}

	public static final void setOper()
	{
		Log.level = OPER;
	}

	public static final boolean isRecord()
	{
		return Log.level <= RECORD;
	}

	public static final boolean isTrace()
	{
		return Log.level <= TRACE;
	}

	public static final boolean isDebug()
	{
		return Log.level <= DEBUG;
	}

	public static final boolean isInfo()
	{
		return Log.level <= INFO;
	}

	public static final boolean isWarn()
	{
		return Log.level <= WARN;
	}

	public static final boolean isError()
	{
		return Log.level <= ERROR;
	}

	public static final boolean isFault()
	{
		return Log.level <= FAULT;
	}

	public static final boolean isOper()
	{
		return Log.level <= OPER;
	}

	public static final void record(String format, Object... args)
	{
		if (Log.level <= RECORD)
			Log.log("RECO", format, args);
	}

	public static final void trace(String format, Object... args)
	{
		if (Log.level <= TRACE)
			Log.log("TRAC", format, args);
	}

	public static final void debug(String format, Object... args)
	{
		if (Log.level <= DEBUG)
			Log.log("DEBU", format, args);
	}

	public static final void info(String format, Object... args)
	{
		if (Log.level <= INFO)
			Log.log("INFO", format, args);
	}

	public static final void warn(String format, Object... args)
	{
		if (Log.level <= WARN)
			Log.log("WARN", format, args);
	}

	public static final void error(String format, Object... args)
	{
		if (Log.level <= ERROR)
			Log.log("ERRO", format, args);
	}

	public static final void fault(String format, Object... args)
	{
		if (Log.level <= FAULT)
			Log.log("FAUL", format, args);
	}

	public static final void oper(String format, Object... args)
	{
		Log.log("OPER", format, args);
	}

	private static final void log(String level, String format, Object... args)
	{
		ByteArrayOutputStream bos = new ByteArrayOutputStream();
		Date now = new Date();
		StringBuilder prefix = new StringBuilder();
		StackTraceElement[] stacks = new Throwable().getStackTrace();
		prefix.append(Dateu.parseDateHHmmssms(now)).append("[").append(level).append("]").append(Thread.currentThread().getId()).append("(").append(stacks[2].getClassName()).append(" ").append(stacks[2].getMethodName()).append(" ").append(stacks[2].getLineNumber()).append(") ");
		PrintStream ps = new PrintStream(bos);
		ps.printf("%s", prefix);
		ps.printf(format, args);
		//
		String str = bos.toString();
		Log.queue.add(str); /* push to queue. */
		//
		if (Log.isOutPutStdout())
			System.out.println(str);
	}

	private static final void svc()
	{
		while (true)
		{
			String str = Log.queue.poll();
			if (str == null)
			{
				Misc.sleep(20);
				Log.heartbeat(System.currentTimeMillis());
				continue;
			}
			if (Log.isOutPutFile())
			{
				RandomAccessFile raf = null;
				try
				{
					raf = new RandomAccessFile(new File(Log.path + Dateu.parseDateyyyy_MM_dd(new Date()) + ".log"), "rw");
					raf.seek(raf.length());
					raf.write(str.getBytes());
					raf.write('\n');
				} catch (Exception e)
				{
				}
				Misc.closeRaf(raf);
			}
			if (Log.isOutPutNet())
			{
				while (Log.ne == null)
					Misc.sleep(20); /* 等待NE被设置. */
				byte by[] = (Log.ne + "," + str).getBytes();
				StmpPdu pdu = new StmpPdu(by.length);
				StmpEnc.addBin(pdu, Stmp.STMP_TAG_ATT, by);
				StmpEnc.addShort(pdu, Stmp.STMP_TAG_CMD, (short) 0x0000);
				StmpEnc.addTag(pdu, Stmp.STMP_TAG_TRANS_UNI);
				while (true)
				{
					Log.conn();
					try
					{
						Log.sock.getOutputStream().write(pdu.bytes());
						Log.hbts = System.currentTimeMillis();
						break;
					} catch (IOException e)
					{
						Log.sock = null;
						Log.error(Log.trace(e));
						Misc.sleep(500);
					}
				}
			}
		}
	}

	private static final void conn()
	{
		while (Log.sock == null)
		{
			Socket sock = null;
			try
			{
				sock = new Socket(Log.host, Log.port);
			} catch (Exception e)
			{
				Log.error(Log.trace(e));
			}
			if (sock == null)
				Misc.sleep(3 * 1000);
			else
			{
				Log.sock = sock;
				Log.info("got a connection from GLS: %s:%d", Log.host, Log.port);
			}
		}
	}

	private static final void heartbeat(long now)
	{
		if (!Log.isOutPutNet())
			return;
		if (Log.sock == null)
			return;
		if (now - Log.hbts < 15 * 1000)
			return;
		Log.hbts = now;
		try
		{
			Log.sock.getOutputStream().write(Log.hb);
		} catch (IOException e)
		{
			Log.sock = null;
			Log.error(Log.trace(e));
		}
	}

	public static final String trace(Exception e)
	{
		StringBuilder strb = new StringBuilder(0x200);
		StackTraceElement ste[] = e.getStackTrace();
		strb.append(e).append(Misc.LINE);
		for (int i = 0; i < ste.length; i++)
			strb.append(ste[i].toString()).append(Misc.LINE);
		return strb.toString();
	}

	public static final int getPid()
	{
		return Log.pid;
	}

	private static final boolean isOutPutStdout()
	{
		return (Log.output & Log.OUTPUT_STDOUT) != 0;
	}

	private static final boolean isOutPutFile()
	{
		return (Log.output & Log.OUTPUT_FILE) != 0;
	}

	private static final boolean isOutPutNet()
	{
		return (Log.output & Log.OUTPUT_NET) != 0;
	}

	public static final void logBean(Object bean, String className, StringBuilder strb)
	{
		Field field[] = bean.getClass().getDeclaredFields();
		Arrays.sort(field, new Comparator<Field>()
		{
			public int compare(Field o1, Field o2)
			{
				return o1.getName().compareTo(o2.getName()) < 0 ? -1 : 1;
			}
		});
		try
		{
			for (int i = 0; i < field.length; i++)
			{
				if (field[i].getType().getName().equals(bean.getClass().getName())) /* 禁止递归自己. */
					continue;
				field[i].setAccessible(true);
				if (!field[i].isAccessible())
					continue;
				String fieldName = field[i].getName();
				Object o = field[i].get(bean);
				Class<?> t = field[i].getType();
				if (o != null && t.isArray())
				{
					int arr_length = Array.getLength(o);
					for (int k = 0; k < arr_length; k++)
					{
						Object obj = Array.get(o, k);
						if (obj != null && obj.getClass().isInstance(obj) && !Log.isSimpleObject(obj))
						{
							String index = o.getClass().getSimpleName();
							index = index.substring(0, index.length() - 1) + k + "]";
							Log.logBean(obj, className + "." + index + "." + obj.getClass().getSimpleName(), strb);
						} else
							strb.append(className + "." + fieldName + "[" + k + "] = " + obj).append(Misc.LINE);
					}
				} else if (o != null && o instanceof Date)
					strb.append(className + "." + fieldName + " = " + Dateu.parseDateyyyyMMddHHmmss((Date) o)).append(Misc.LINE);
				else if (o != null && t.isInstance(o) && !Log.isSimpleObject(o))
					Log.logBean(o, className + "." + o.getClass().getSimpleName(), strb);
				else
					strb.append(className + "." + fieldName + " = " + o).append(Misc.LINE);
			}
		} catch (Exception e)
		{
			Log.error(Log.trace(e));
		}
	}

	private static final boolean isSimpleObject(Object o)
	{
		return o instanceof String || o instanceof Number || o instanceof Character || o instanceof Boolean;
	}
}
