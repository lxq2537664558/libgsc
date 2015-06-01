/*
 * sha1.h
 *
 *  Created on: Mar 27, 2015 3:02:12 PM
 *      Author: xuzewen
 */

#ifndef SHA1_H_
#define SHA1_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

unsigned char* SHA1(const unsigned char *d, size_t n, unsigned char *md);

#endif /* SHA1_H_ */
