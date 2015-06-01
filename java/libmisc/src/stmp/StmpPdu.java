package stmp;

/**
 * 
 * @author xuzewen
 * 
 */
public class StmpPdu
{
	public int rm = 0;
	public int p = 0;
	public byte buff[] = null;

	public StmpPdu(int size)
	{
		this.rm = size + 64; /* 预留64字节给tag. */
		this.p = 0;
		this.buff = new byte[this.rm];
	}

	public String toString()
	{
		return StmpDec.printPdu2Str(this);
	}

	public byte[] bytes()
	{
		byte by[] = new byte[this.buff.length - this.rm];
		System.arraycopy(this.buff, this.rm, by, 0, by.length);
		return by;
	}
}
