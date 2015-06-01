#ifndef RC4_H_
#define RC4_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

struct rc4_state
{
	unsigned char perm[256];
	unsigned char index1;
	unsigned char index2;
};

extern void rc4_init(struct rc4_state *state, const unsigned char *key, int keylen);
extern void rc4_crypt(struct rc4_state *state, const unsigned char *inbuf, unsigned char *outbuf, int buflen);

#endif	/* RC4_H_ */
