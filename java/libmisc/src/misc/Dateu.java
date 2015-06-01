package misc;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Calendar;
import java.util.Date;
import java.util.Locale;

/**
 * 
 * 日期处理.
 * 
 * @author xuzewen
 * @create on 2008-10-01
 * 
 */
public class Dateu
{
	public static final long SECOND = 1 * 1000;
	public static final long MINUTE = 60 * SECOND;
	public static final long HOUR = 60 * MINUTE;
	public static final long DAY = 24 * HOUR;
	public static final long WEEK = 7 * DAY;

	/** 入参格式: Sat Nov 01 14:01:55 CST 2014. */
	public static final Date parseLocale(String date)
	{
		if (date == null)
			return null;
		SimpleDateFormat sdf = new SimpleDateFormat("EEE MMM dd HH:mm:ss zzz yyyy", Locale.US);
		try
		{
			return sdf.parse(date);
		} catch (ParseException e)
		{
			return null;
		}
	}

	/** 入参格式: yyyyMMdd. */
	public static final Date parseDateyyyyMMdd(String date)
	{
		if (date == null)
			return null;
		SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMdd");
		try
		{
			return sdf.parse(date);
		} catch (ParseException e)
		{
			return null;
		}
	}

	/** 入参格式: yyyy-MM-dd. */
	public static final Date parseDateyyyy_MM_dd(String date)
	{
		if (date == null)
			return null;
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd");
		try
		{
			return sdf.parse(date);
		} catch (ParseException e)
		{
			return null;
		}
	}

	/** 解析yyyyMMddHH格式的日期. */
	public static final Date parseDateyyyyMMddHH(String date)
	{
		if (date == null)
			return null;
		SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHH");
		try
		{
			return sdf.parse(date);
		} catch (ParseException e)
		{
			return null;
		}
	}

	/** 解析yyyyMMddHHmm格式的日期. */
	public static final Date parseDateyyyyMMddHHmm(String date)
	{
		if (date == null)
			return null;
		SimpleDateFormat sdf = new SimpleDateFormat("yyyyMMddHHmm");
		try
		{
			return sdf.parse(date);
		} catch (ParseException e)
		{
			return null;
		}
	}

	/** 解析yyyy-MM-dd HH:mm:ss格式的日期. */
	public static final Date parseDateyyyy_MM_dd_HH_mm_ss(String date)
	{
		if (date == null)
			return null;
		SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
		try
		{
			return sdf.parse(date);
		} catch (ParseException e)
		{
			return null;
		}
	}

	/** 返回格式: yyyy-MM-dd */
	public static final String parseDateyyyy_MM_dd(Date date)
	{
		return parse("yyyy-MM-dd", date);
	}

	/** 返回格式:yyyy-MM-dd HH:mm:ss */
	public static final String parseDateyyyyMMddHHmmss(Date date)
	{
		return parse("yyyy-MM-dd HH:mm:ss", date);
	}

	/** 返回格式:yyyy/MM/dd HH:mm */
	public static final String parseDateyyyyMMddHHmm2(Date date)
	{
		return parse("yyyy/MM/dd HH:mm", date);
	}

	/** 返回格式:yyyyMMdd */
	public static final String parseDateyyyyMMdd(Date date)
	{
		return parse("yyyyMMdd", date);
	}

	/** 返回格式:yyyyMMddHH */
	public static final String parseDateyyyyMMddHH(Date date)
	{
		return parse("yyyyMMddHH", date);
	}

	/** 返回格式:yyyyMMddHHmmss */
	public static final String parseDateyyyyMMddHHmmss2(Date date)
	{
		return parse("yyyyMMddHHmmss", date);
	}

	/** 返回格式:yyyyMMddHHmm */
	public static final String parseDateyyyyMMddHHmm(Date date)
	{
		return parse("yyyyMMddHHmm", date);
	}

	/** 返回格式:MMddHHmmss */
	public static final String parseDateMMddHHmmss(Date date)
	{
		return parse("MMddHHmmss", date);
	}

	/** 返回格式:HH:mm:ss */
	public static final String parseDateHHmmss(Date date)
	{
		return parse("HH:mm:ss", date);
	}

	/** 返回格式: HH:mm:ss.ms */
	public static final String parseDateHHmmssms(Date date)
	{
		long ms = date.getTime() % 1000;
		return parse("HH:mm:ss", date) + "." + (ms > 99 ? ms : (ms > 9 ? ("0" + ms) : ("00" + ms)));
	}

	/** 返回格式:yyyy-MM-dd HH:mm:ss.ms */
	public static final String parseDateyyyyMMddHHmmssms(Date date)
	{
		long ms = date.getTime() % 1000;
		return parse("yyyy-MM-dd HH:mm:ss", date) + "." + (ms > 99 ? ms : (ms > 9 ? ("0" + ms) : ("00" + ms)));
	}

	/** 置为凌晨00:00:00 000,Calendar提供的set函数, 并不能清除毫秒. */
	public static final Date set000000(Date date)
	{
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		cal.set(cal.get(Calendar.YEAR), cal.get(Calendar.MONTH), cal.get(Calendar.DAY_OF_MONTH), 0, 0, 0);
		return clearMinute(cal.getTime());
	}

	/** 当前时间的hour,小于10时前面补个零. */
	public static final String hour(Date date)
	{
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		int hour = cal.get(Calendar.HOUR_OF_DAY);
		return hour > 9 ? hour + "" : "0" + hour;
	}

	/** 返回分钟(0 ~ 59). */
	public static final int minuteInt(Date date)
	{
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		return cal.get(Calendar.MINUTE);
	}

	/** 返回小时(0 ~ 23). */
	public static final int hourInt(Date date)
	{
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		return cal.get(Calendar.HOUR_OF_DAY);
	}

	/** 返回天. */
	public static final int dayInt(Date date)
	{
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		return cal.get(Calendar.DAY_OF_MONTH);
	}

	/** 返回月份. */
	public static final int monthInt(Date date)
	{
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		return cal.get(Calendar.MONTH) + 1;
	}

	/** 返回年份. */
	public static final int yearInt(Date date)
	{
		Calendar cal = Calendar.getInstance();
		cal.setTime(date);
		return cal.get(Calendar.YEAR);
	}

	/** 判断两个日期是否在同一天. */
	public static final boolean isSameDay(Date arg0, Date arg1)
	{
		return (Dateu.yearInt(arg0) == Dateu.yearInt(arg1)) && //
				(Dateu.monthInt(arg0) == Dateu.monthInt(arg1)) && //
				(Dateu.dayInt(arg0) == Dateu.dayInt(arg1));
	}

	/** 构造一个给定的时间. */
	public static final Date createDate(int year, int month, int day, int hourOfDay, int minute, int second)
	{
		Calendar cal = Calendar.getInstance();
		cal.set(year, month, day, hourOfDay, minute, second);
		return cal.getTime();
	}

	/** 时间滚动, 按分钟, up == true向今后滚动, 否则向以前滚动. */
	public static final Date dateRollOfMinute(Date date, int amount, boolean up)
	{
		return up ? new Date(date.getTime() + ((long) amount) * Dateu.MINUTE) : new Date(date.getTime() - ((long) amount) * Dateu.MINUTE);
	}

	/** 时间滚动, 按小时, up == true向今后滚动, 否则向以前滚动. */
	public static final Date dateRollOfHour(Date date, int amount, boolean up)
	{
		return up ? new Date(date.getTime() + ((long) amount) * Dateu.HOUR) : new Date(date.getTime() - ((long) amount) * Dateu.HOUR);
	}

	/** 时间滚动, 按天, up == true向今后滚动, 否则向以前滚动. */
	public static final Date dateRollOfDay(Date date, int amount, boolean up)
	{
		return up ? new Date(date.getTime() + ((long) amount) * Dateu.DAY) : new Date(date.getTime() - ((long) amount) * Dateu.DAY);
	}

	/** 时间滚动, 按月, up == true向今后滚动, 否则向以前滚动. */
	public static final Date dateRollOfMonth(Date date, boolean up)
	{
		Calendar ca = Calendar.getInstance();
		ca.setTime(date);
		ca.roll(Calendar.MONTH, up);
		if (ca.getTime().getTime() < date.getTime())
			ca.roll(Calendar.YEAR, true);
		return ca.getTime();
	}

	/** 清除分钟. */
	public static final Date clearMinute(Date date)
	{
		return new Date(date.getTime() - (date.getTime() % Dateu.HOUR));
	}

	/** 清除小时. */
	public static final Date clearHour(Date date)
	{
		return Dateu.set000000(date);
	}

	/** 秒转换为毫秒, 出现这个函数的原因时, 当前时间的秒数 * 1000后总是整数(4字节)溢出, 此函数则可避免出错. */
	public static final long sec2msec(long sec)
	{
		return sec * 1000L;
	}

	/** 按格式解析日期. */
	private static final String parse(String format, Date date)
	{
		try
		{
			return date == null ? null : new SimpleDateFormat(format).format(date);
		} catch (Exception e)
		{
			return null;
		}
	}
}
