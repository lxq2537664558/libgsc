package actor;

import java.util.function.Consumer;

import net.Gworker;
import misc.Log;
import misc.Misc;
import core.Gsc;

/**
 * 
 * actor抽象基类.
 * 
 * @author xuzewen
 * @time 2015年1月12日 下午2:09:35
 *
 */
public abstract class Actor
{
	public enum ActorType
	{
		/** 进程内Actor, 附着在某个Gworker上. */
		ITC,
		/** 拥有自己独立线程或线程池的actor, 主要用于IO阻塞操作, 如数据库查询. */
		BLOCKING,
		/** 即network to host, 网络到主机的连接. */
		N2H,
		/** 即host to network, 主机到网络的连接. */
		H2N
	}

	/** 工作线程索引, -1时无效. */
	public int wk = -1;
	/** Actor类型. */
	public ActorType type;
	/** Actor名称. */
	public String name;

	public Actor(ActorType type)
	{
		this.type = type;
		this.name = this.getClass().getSimpleName();
	}

	public void future(Consumer<Void> c)
	{
		if (this.type.ordinal() == ActorType.BLOCKING.ordinal()) /* 直接入队. */
		{
			((ActorBlocking) this).push(c);
			return;
		}
		if (this.wk == -1)
		{
			Log.fault("it`s a bug, t: %s, %s", this.name, Misc.getStackInfo());
			return;
		}
		if (Gsc.getWorkerIndex() != this.wk) /* 不在同一个线程. */
			Gsc.wks[this.wk].push(c);
		else
			c.accept(null);
	}

	public Gworker getGworker()
	{
		return this.wk == -1 ? null : Gsc.wks[this.wk];
	}
}
