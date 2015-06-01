package actor;

import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.function.Consumer;

import misc.Log;

/**
 * 
 * 拥有自己独立线程(一个或多个)的Actor, 用于有限的IO阻塞操作, 如数据库.
 * 
 * @author xuzewen
 * @time 2015年1月12日 下午9:24:23
 *
 */
public abstract class ActorBlocking extends Actor
{
	/** 等待处理的Consumer. */
	private ConcurrentLinkedQueue<Consumer<Void>> gfs = new ConcurrentLinkedQueue<>();
	/** 拥有的线程个数. */
	private int tc = 1;
	/** 线程忙? */
	private volatile boolean busy = false;

	public ActorBlocking(int tc)
	{
		super(ActorType.BLOCKING);
		this.tc = tc < 1 ? 1 : tc;
		this.start();
	}

	/** 添加一个Consumer. */
	public void push(Consumer<Void> gf)
	{
		this.gfs.add(gf);
		synchronized (this)
		{
			this.notify();
		}
	}

	/** 线程忙? */
	public boolean isBusy()
	{
		return this.busy;
	}

	private void start()
	{
		ActorBlocking ai = this;
		ExecutorService es = Executors.newFixedThreadPool(this.tc);
		for (int i = 0; i < this.tc; ++i)
			es.execute(new Runnable()
			{
				public void run()
				{
					Log.info("Actor-Blocking worker thread started successfully, name: %s, tid: %d", ai.name, Thread.currentThread().getId());
					while (true)
						ai.run();
				}
			});
	}

	private void run()
	{
		Consumer<Void> c = null;
		synchronized (this)
		{
			c = this.gfs.poll();
			if (c == null)
			{
				try
				{
					this.wait();
				} catch (InterruptedException e)
				{

				}
				c = this.gfs.poll();
			}
		}
		if (c != null) /* 抢占式. */
		{
			this.busy = true;
			c.accept(null);
			this.busy = false;
		}
	}
}
