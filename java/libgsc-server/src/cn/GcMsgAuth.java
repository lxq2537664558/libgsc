package cn;

import gcpb.NetGcWithGas.GcAuthWithGasReq;
import gcpb.NetGcWithGas.GcAuthWithGasRsp;
import actor.N2H;

import com.google.protobuf.ByteString;

import core.Gn2htrans;

public class GcMsgAuth
{
	/** 鉴权请求. */
	public static void gas_req_gc_auth(N2H n2h, Gn2htrans gt, GcAuthWithGasReq req)
	{
		gt.end(GcAuthWithGasRsp.newBuilder().setToken(ByteString.copyFromUtf8("world.")));
	}
}
