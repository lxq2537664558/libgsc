package actor;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.SocketChannel;

import misc.Log;
import misc.Misc;
import misc.Net;
import stmp.StmpDec;
import stmp.StmpNode;

public abstract class ActorNet extends Actor
{
	public enum NetProtocol
	{
		/** 未知. */
		NP_NONE,
		/** STMP. */
		NP_STMP,
		/** WebSocket. */
		NP_WEBSOCKET,
	};

	/** 连接上的协议类型. */
	public NetProtocol np = NetProtocol.NP_NONE;
	/** websocket-handshake是否完成, 仅在np == NP_WEBSOCKET时有效. */
	public boolean wshs = false;
	/** 连接是否已建立. */
	public boolean est = false;
	/** 连接建立时间. */
	public long gts = 0L;
	/** 最后一次收到消息的时间戳. */
	public long mts = 0L;
	/** 连接失去时间(延迟关闭时使用, 这是一个未来时间). */
	public long lts = 0L;
	/** 连接标识. */
	public SocketChannel sc = null;
	/** 缓冲区. */
	public ByteBuffer bb = null;

	public ActorNet(ActorType type, SocketChannel sc, ByteBuffer bb)
	{
		super(type);
		this.sc = sc;
		this.bb = bb;
	}

	/** 消息出栈, 成功递送到本端发送缓冲区时返回true. */
	public void send(byte[] by)
	{
		if (Log.isRecord())
		{
			if (this.np == NetProtocol.NP_WEBSOCKET && !this.wshs) /* 是websocket连接, 但还未完成握手流程. */
				Log.record("\n  --> PEER: %s\n%s", Net.getRemoteAddr(this.sc), Misc.printBytes(by));
			else
				Log.record("\n  --> PEER: %s\n%s", Net.getRemoteAddr(this.sc), StmpDec.print2Str(by));
		}
		if (!this.est)
			return;
		if (this.np == NetProtocol.NP_WEBSOCKET && this.wshs) /* websocket握手已完成. */
			this.sendWebSocket(by);
		else
			this.sendStmp(by);
	}

	/** 强制关闭连接(回调evnDis). */
	public final void close()
	{
		this.getGworker().removeActorNet(this);
		this.evnDis();
	}

	/** 静默关闭, 不触发evnDis. */
	protected void closeSlient()
	{
		if (!this.est)
		{
			Log.warn("connection was lost: %s", this);
			return;
		}
		this.getGworker().removeActorNet(this);
	}

	/** 连接失去事件. */
	public abstract void evnDis();

	/** 网络消息送达事件(STMP). */
	public abstract boolean evnMsg(StmpNode root);

	/** STMP报文出栈. */
	private void sendStmp(byte by[])
	{
		try
		{
			if (this.sc.write(ByteBuffer.wrap(by)) != by.length)
			{
				this.close();
				return;
			}
		} catch (IOException e)
		{
			if (Log.isTrace())
				Log.trace(Log.trace(e));
			this.close();
			return;
		}
	}

	/** websocket报文出栈. */
	private final void sendWebSocket(byte by[])
	{
		byte dat[] = null;
		int size = 0;
		if (by.length < 0x7E) /* 小帧. */
		{
			size = by.length + 2;
			dat = new byte[size];
			dat[0] = (byte) 0x82;
			dat[1] = (byte) by.length;
			System.arraycopy(by, 0, dat, 2, by.length);
		} else if (by.length < 0x10000) /* 中帧. */
		{
			size = by.length + 4;
			dat = new byte[size];
			dat[0] = (byte) 0x82;
			dat[1] = (byte) 0x7E;
			dat[2] = Net.short2byte((short) by.length)[0];
			dat[3] = Net.short2byte((short) by.length)[1];
			System.arraycopy(by, 0, dat, 4, by.length);
		} else
		{ /* 大帧. */
			size = by.length + 10;
			dat = new byte[size];
			dat[0] = (byte) 0x82;
			dat[1] = (byte) 0x7F;
			byte tmp[] = Net.int2byte(by.length);
			dat[2] = 0;
			dat[3] = 0;
			dat[4] = 0;
			dat[5] = 0;
			dat[6] = tmp[0];
			dat[7] = tmp[1];
			dat[8] = tmp[2];
			dat[9] = tmp[3];
			System.arraycopy(by, 0, dat, 10, by.length);
		}
		this.sendStmp(dat);
	}
}
