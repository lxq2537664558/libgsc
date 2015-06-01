package net;

import misc.Log;
import misc.Misc;
import misc.Net;
import stmp.StmpDec;
import stmp.StmpNode;
import actor.Actor.ActorType;
import actor.ActorNet;
import actor.N2H;
import core.Cfg;

/**
 *
 * @Created on: 2015年5月19日 下午1:21:36
 * @Author: xuzewen
 * 
 */
public class WebSocket
{
	private static final String secKey = "Sec-WebSocket-Key:";
	private static final byte WS_FRAME_CONTINUATION = 0x00;
	private static final byte WS_FRAME_TEXT = 0x01;
	private static final byte WS_FRAME_BINARY = 0x02;
	private static final byte WS_FRAME_CLOSE = 0x08;
	private static final byte WS_FRAME_PING = 0x09;
	private static final byte WS_FRAME_PONG = 0x0A;

	/** 检查握手协议. */
	public static final int checkHandShake(ActorNet an, byte by[], int ofst, int len)
	{
		boolean r = false;
		int of = 0;
		for (int i = len - 1; i >= 4; i -= 4)
		{
			if (by[i] == 0x0A && by[i - 1] == 0x0D && by[i - 2] == 0x0A && by[i - 3] == 0x0D)
			{
				of = i;
				r = true;
				break;
			}
		}
		if (!r) /* 不是个完整协议报文. */
			return 0;
		String swk = WebSocket.parseSwk(new String(by, ofst, of));
		if (swk == null)
			return -1;
		WebSocket.sendHandShake(an, swk);
		return of + 1;
	}

	/** 业务报文解析. */
	public static final int unpack(ActorNet an, byte by[], int ofst, int len)
	{
		// 81 85 C7 5B 2E F3 AF 3E 42 9F A8
		// 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		// +-+-+-+-+-------+-+-------------+-------------------------------+
		// |F|R|R|R| opcode|M| Payload len | Extended payload length |
		// |I|S|S|S| (4) |A| (7) | (16/64) |
		// |N|V|V|V| |S| | (if payload len==126/127) |
		// | |1|2|3| |K| | |
		// +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - -
		int o = ofst;
		int l = len;
		for (;;)
		{
			if (l < 2)
				/** 不完整的报文, */
				break;
			if ((by[o] & 0x80) == 0) /** FIN, 是否为最后一帧. */
			{
				if (Log.isDebug())
					Log.debug("unsupported multi-frame.");
				return -1;
			}
			byte opcode = (byte) (by[0] & 0x0F);
			/** 操作码. */
			if ((by[o + 1] & 0x80) == 0) /** MASK, 是否使用了掩码. */
			{
				if (Log.isDebug())
					Log.debug("client MUST use mask.");
				return -1;
			}
			switch (opcode)
			{
			case WS_FRAME_CLOSE:/** 直接关闭. */
			{
				if (Log.isDebug())
					Log.debug("got a web socket close request, we will close this socket immediate: %s", an);
			}
				return -1;
			case WS_FRAME_CONTINUATION:/** 不处理. */
			case WS_FRAME_PING:
			case WS_FRAME_PONG:
			{
				if (Log.isDebug())
					Log.debug("unsupported opcode: %02X", opcode);
				return -1;
			}
			case WS_FRAME_TEXT:
			case WS_FRAME_BINARY:
				break; /* 去下面. */
			default:
				if (Log.isDebug())
					Log.debug("unsupported opcode: %02X", opcode);
				return -1;
			}
			if (by[o + 1] == 0x80)
			{
				if (Log.isDebug())
					Log.debug("need 1 byte at least.");
				return -1;
			}
			if (by[o + 1] == 0xFF) /* 不支持超过两个字节长度的报文. */
			{
				if (Log.isDebug())
					Log.debug("unsupported over 64K PDU.");
				return -1;
			}
			boolean sm = ((short) ((by[o + 1]) & 0x00FF) < 0x00FE);
			int size = 0;
			if (sm)
				size = 1/* 首字节 */+ 1 /* len字段本身 */+ 4 /* 4字节掩码. */+ (by[o + 1] & 0x7F) /* 内容长度. */; /* 小帧. */
			else
				size = 1 /* 首字节 */+ 3 /* len字段本身 */+ 4 /* 4字节掩码. */+ Net.byte2short(by, 2) /* 内容长度. */; /* 有两字节表示长度. */
			if (size > Cfg.libgsc_peer_mtu)
			{
				if (Log.isDebug())
					Log.debug("packet format error, we will close this connection, peer: %s, size: %08X", Net.getRemoteAddr(an.sc), size);
				return -1;
			}
			if (l < size) /* 还未到齐. */
				break;
			//
			if (an.type == ActorType.N2H && ((N2H) an).lz) /* 这里有一个问题: 一次性收到多个消息时, 如果单个消息处理完, 需要延迟关闭套接字, 后面的将还会继续处理, 所以这里作一次判断. */
			{
				o += size; /* 直接越过, 不处理, 等待客户端的websocket-close报文, 或者延迟关闭执行. */
				l -= size;
				continue;
			}
			//
			byte[] mask = new byte[4];
			System.arraycopy(by, sm ? o + 2 : o + 4, mask, 0, 4);
			int of = sm ? 6 : 8;
			for (int i = of; i < size; i++)
				by[i] = (byte) (by[i] ^ mask[(i - of) % 4]); /* 解码. */
			an.lts = System.currentTimeMillis(); /* 更新最后收到消息的时间戳. */
			if (!WebSocket.ws_frame_binary(an, by, of, size - of))
				return -1;
			o += size;
			l -= size;
		}
		return o - ofst;
	}

	/** 文本/二进制帧. */
	private static boolean ws_frame_binary(ActorNet an, byte[] by, int ofst, int len)
	{
		StmpNode root = StmpDec.unpack(by, ofst, len);
		if (root == null)
		{
			if (Log.isDebug())
				Log.debug("STMP protocol error.");
			return false;
		}
		if (Log.isRecord())
			Log.record("\n  <-- peer: %s\n%s", Net.getRemoteAddr(an.sc), StmpDec.printNode2Str(root));
		return an.evnMsg(root);
	}

	/** 从握手协议中取出Sec-WebSocket-Key. */
	private static final String parseSwk(String str)
	{
		int i0 = str.indexOf(WebSocket.secKey);
		if (i0 == -1)
			return null;
		int i1 = str.indexOf("\n", i0 + WebSocket.secKey.length());
		if (i1 == -1)
			return null;
		return Misc.trim(str.substring(i0 + WebSocket.secKey.length(), i1));
	}

	/** 发送握手响应. */
	private static final void sendHandShake(ActorNet an, String swk)
	{
		String str = swk + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
		str = Misc.printf2Str("HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: WebSocket\r\nSec-WebSocket-Accept: %s\r\n\r\n", Misc.base64enc(Misc.sha1(str.getBytes())));
		an.send(str.getBytes()); /* 发送响应. */
	}
}
