/*
 * Base64.c
 *
 *  Created on: 2012-7-8
 *      Author: xuzewen
 */

#include "Base64.h"

static const char base64_alphabet[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const uchar mime_base64_rank[256] = { 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 62, 255, 255, 255, 63, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 255, 255, 255, 0, 255, 255, 255, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 255, 255, 255, 255, 255, 255, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255 };

Base64::Base64()
{

}

int Base64::encode_step(const uchar* in, int len, int break_lines, char* out, int* state, int* save)
{
	char *outptr;
	const unsigned char* inptr;
	if (len <= 0)
		return 0;
	inptr = in;
	outptr = out;

	if (len + ((char *) save)[0] > 2)
	{
		const unsigned char *inend = in + len - 2;
		int c1, c2, c3;
		int already;

		already = *state;

		switch (((char *) save)[0])
		{
		case 1:
			c1 = ((unsigned char *) save)[1];
			goto skip1;
		case 2:
			c1 = ((unsigned char *) save)[1];
			c2 = ((unsigned char *) save)[2];
			goto skip2;
		}

		/*
		 * yes, we jump into the loop, no i'm not going to change it,
		 * it's beautiful!
		 */
		while (inptr < inend)
		{
			c1 = *inptr++;
			skip1: c2 = *inptr++;
			skip2: c3 = *inptr++;
			*outptr++ = base64_alphabet[c1 >> 2];
			*outptr++ = base64_alphabet[c2 >> 4 | ((c1 & 0x3) << 4)];
			*outptr++ = base64_alphabet[((c2 & 0x0f) << 2) | (c3 >> 6)];
			*outptr++ = base64_alphabet[c3 & 0x3f];
			/* this is a bit ugly ... */
			if (break_lines && (++already) >= 19)
			{
				*outptr++ = '\n';
				already = 0;
			}
		}

		((char *) save)[0] = 0;
		len = 2 - (inptr - inend);
		*state = already;
	}

	if (len > 0)
	{
		char *saveout;

		/* points to the slot for the next char to save */
		saveout = &(((char *) save)[1]) + ((char *) save)[0];

		/* len can only be 0 1 or 2 */
		switch (len)
		{
		case 2:
			*saveout++ = *inptr++;
			*saveout++ = *inptr++;
			break;
		case 1:
			*saveout++ = *inptr++;
			break;
		default:
			break;
		}
		((char *) save)[0] += len;
	}

	return outptr - out;
}

/**
 * Base64::encode_close:
 * @break_lines: whether to break long lines
 * @out: pointer to destination buffer
 * @state: Saved state from Base64::encode_step()
 * @save: Saved state from Base64::encode_step()
 *
 * Flush the status from a sequence of calls to Base64::encode_step().
 *
 * The output buffer must be large enough to fit all the data that will
 * be written to it. It will need up to 4 bytes, or up to 5 bytes if
 * line-breaking is enabled.
 *
 * Return value: The number of bytes of output that was written
 *
 * Since: 2.12
 */
int Base64::encode_close(int break_lines, char *out, int *state, int *save)
{
	int c1, c2;
	char *outptr = out;
	c1 = ((unsigned char *) save)[1];
	c2 = ((unsigned char *) save)[2];

	switch (((char *) save)[0])
	{
	case 2:
		outptr[2] = base64_alphabet[((c2 & 0x0f) << 2)];
		goto skip;
	case 1:
		outptr[2] = '=';
		skip: outptr[0] = base64_alphabet[c1 >> 2];
		outptr[1] = base64_alphabet[c2 >> 4 | ((c1 & 0x3) << 4)];
		outptr[3] = '=';
		outptr += 4;
		break;
	}
	if (break_lines)
		*outptr++ = '\n';

	*save = 0;
	*state = 0;

	return outptr - out;
}

/**
 * g_base64_encode:
 * @data: the binary data to encode
 * @len: the length of @data
 *
 * Encode a sequence of binary data into its Base-64 stringified
 * representation.
 *
 * Return value: a newly allocated, zero-terminated Base-64 encoded
 *               string representing @data. The returned string must
 *               be freed with g_free().
 *
 * Since: 2.12
 */
char* Base64::encode(const unsigned char *data, int len)
{
	char *out;
	int state = 0, outlen;
	int save = 0;

	out = (char*) malloc((len / 3 + 1) * 4 + 1);

	outlen = Base64::encode_step(data, len, 0, out, &state, &save);
	outlen += Base64::encode_close(0, out + outlen, &state, &save);
	out[outlen] = '\0';

	return (char *) out;
}

/**
 * Base64::decode_step:
 * @in: binary input data
 * @len: max length of @in data to decode
 * @out: output buffer
 * @state: Saved state between steps, initialize to 0
 * @save: Saved state between steps, initialize to 0
 *
 * Incrementally decode a sequence of binary data from its Base-64 stringified
 * representation. By calling this function multiple times you can convert
 * data in chunks to avoid having to have the full encoded data in memory.
 *
 * The output buffer must be large enough to fit all the data that will
 * be written to it. Since base64 encodes 3 bytes in 4 chars you need
 * at least: (@len / 4) * 3 + 3 bytes (+ 3 may be needed in case of non-zero
 * state).
 *
 * Return value: The number of bytes of output that was written
 *
 * Since: 2.12
 **/
int Base64::decode_step(const char *in, int len, unsigned char *out, int *state, unsigned int *save)
{
	const unsigned char *inptr;
	unsigned char *outptr;
	const unsigned char *inend;
	unsigned char c, rank;
	unsigned char last[2] = { 0 };
	unsigned int v;
	int i;

	if (len <= 0)
		return 0;

	inend = (const unsigned char *) in + len;
	outptr = out;

	/* convert 4 base64 bytes to 3 normal bytes */
	v = *save;
	i = *state;
	inptr = (const unsigned char *) in;
	last[0] = last[1] = 0;
	while (inptr < inend)
	{
		c = *inptr++;
		rank = mime_base64_rank[c];
		if (rank != 0xff)
		{
			last[1] = last[0];
			last[0] = c;
			v = (v << 6) | rank;
			i++;
			if (i == 4)
			{
				*outptr++ = v >> 16;
				if (last[1] != '=')
					*outptr++ = v >> 8;
				if (last[0] != '=')
					*outptr++ = v;
				i = 0;
			}
		}
	}

	*save = v;
	*state = i;

	return outptr - out;
}

/**
 * g_base64_decode:
 * @text: zero-terminated string with base64 text to decode
 * @out_len: The length of the decoded data is written here
 *
 * Decode a sequence of Base-64 encoded text into binary data
 *
 * Return value: a newly allocated buffer containing the binary data
 *               that @text represents. The returned buffer must
 *               be freed with g_free().
 *
 * Since: 2.12
 */
uchar* Base64::decode(const char *text, int *out_len)
{
	unsigned char *ret;
	int input_length;
	int state = 0;
	unsigned int save = 0;
	input_length = strlen(text);
	/* We can use a smaller limit here, since we know the saved state is 0,
	 +1 used to avoid calling g_malloc0(0), and hence retruning NULL */
	ret = (unsigned char*) calloc(1, (input_length / 4) * 3 + 1);
	*out_len = Base64::decode_step(text, input_length, ret, &state, &save);
	return ret;
}

Base64::~Base64()
{

}

