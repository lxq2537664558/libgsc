/*
 * Misc.cpp
 *
 *  Created on: 2011-3-21
 *      Author: xuzewen
 */

#include "Misc.h"
#include "Logger.h"
#ifdef USE_OPENSSL
#include <openssl/rc4.h>
#include <openssl/sha.h>
#include <openssl/md5.h>
#include <openssl/ssl.h>
#include <openssl/rand.h>
#include <openssl/sha.h>
#else
#include "../sec/rc4.h"
#include "../sec/md5.h"
#include "../sec/sha256.h"
#include "../sec/sha512.h"
#include "../sec/sha1.h"
#endif

/** 0 ~ 9, a ~ z, A ~ Z, _. */
static char __0aA__[] = { '_', '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };
/** 0 ~ F. */
static char __0F__[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
/** 0 ~ f. */
static char __0f__[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

#if defined(WINDOWS) && !defined(__MINGW_H)
#if defined(_MSC_VER) || defined(_MSC_EXTENSIONS) || defined(__WATCOMC__)
#define DELTA_EPOCH_IN_USEC 11644473600000000Ui64
#else
#define DELTA_EPOCH_IN_USEC 11644473600000000ULL
#endif
typedef unsigned __int64 u_int64_t;
static u_int64_t filetime_to_unix_epoch(const FILETIME *ft)
{
	u_int64_t res = (u_int64_t) ft->dwHighDateTime << 32;
	res |= ft->dwLowDateTime;
	res /= 10;
	res -= DELTA_EPOCH_IN_USEC;
	return (res);
}
#endif

Misc::Misc()
{

}

/** 是不是一串16进制字符. */
bool Misc::isHexStr(uchar* dat, int len)
{
	if (len < 1)
		return false;
	for (int i = 0; i < len; ++i)
	{
		bool ret = false;
		for (int k = 0; k < 0x10; ++k)
		{
			if (dat[i] == __0F__[k] || dat[i] == __0f__[k])
			{
				ret = true;
				break;
			}
		}
		if (!ret)
			return false;
	}
	return true;
}

/** 将字节流(dat)以大写16进制格式打印到str中, 无换行, 带空格. */
void Misc::hex2str0(uchar* dat, int len, char* str)
{
	int i = 0;
	for (; i < len; ++i)
	{
		str[i * 3 + 0] = __0F__[((dat[i] >> 4) & 0x0F)];
		str[i * 3 + 1] = __0F__[(dat[i] & 0x0F)];
		str[i * 3 + 2] = 0x20;
	}
	str[i * 3 - 1] = 0;
}

/** 将字节流(dat)以大写16进制格式打印到str中, 无换行, 无空格. */
void Misc::hex2str1(uchar* dat, int len, char* str)
{
	for (int i = 0; i < len; ++i)
	{
		str[i * 2 + 0] = __0F__[((dat[i] >> 4) & 0x0F)];
		str[i * 2 + 1] = __0F__[(dat[i] & 0x0F)];
	}
}

/** 将字节流(dat)以大写16进制格式返回, 无换行, 无空格. */
string Misc::hex2str1(uchar* dat, int len)
{
	string str;
	str.resize(len * 2);
	Misc::hex2str1(dat, len, (char*) str.c_str());
	return str;
}

/** 将字节流(str)以大写16进制格式返回, 无换行, 无空格. */
string Misc::hex2str1(const string& str)
{
	string x;
	x.resize(str.length() * 2);
	Misc::hex2str1((uchar*) str.c_str(), str.length(), (char*) x.c_str());
	return str;
}

/** 将字节流(dat)以小写16进制格式打印到str中, 无换行, 无空格. */
void Misc::hex2str2(uchar* dat, int len, char* str)
{
	for (int i = 0; i < len; ++i)
	{
		str[i * 2 + 0] = __0f__[((dat[i] >> 4) & 0x0F)];
		str[i * 2 + 1] = __0f__[(dat[i] & 0x0F)];
	}
}

/** 将字节流(dat)以小写16进制格式返回, 无换行, 无空格. */
string Misc::hex2str2(uchar* dat, int len)
{
	string str;
	str.resize(len * 2);
	Misc::hex2str2(dat, len, (char*) str.c_str());
	return str;
}

/** 16进制字符串(一定是2字节, 如"FF")转换成单字节. */
uchar Misc::hexstr2char(char* hex)
{
	return ((((hex[0] < 65 ? hex[0] - 0x30 : (hex[0] < 97 ? hex[0] - 55 : hex[0] - 87))) << 4) & 0xF0) | //
			(((hex[1] < 65 ? hex[1] - 0x30 : (hex[1] < 97 ? hex[1] - 55 : hex[1] - 87))) & 0x0F);
}

/** 16进制整形字符串(一定是4字节, 如"FFFF")转换成短整形. */
ushort Misc::hexstr2short(char* hex)
{
	ushort ret = 0x00;
	int i = 0;
	for (; i < 4; ++i)
		ret |= (hex[i] < 65 ? hex[i] - 0x30 : (hex[i] < 97 ? hex[i] - 55 : hex[i] - 87)) << (3 - i) * 4;
	return ret;
}

/** 16进制整形字符串(一定是8字节, 如"FFFFFFFF")转换成整形. */
uint Misc::hexstr2int(char* hex)
{
	uint ret = 0x00;
	int i = 0;
	for (; i < 8; ++i)
		ret |= (hex[i] < 65 ? hex[i] - 0x30 : (hex[i] < 97 ? hex[i] - 55 : hex[i] - 87)) << (7 - i) * 4;
	return ret;
}

/** 16进制整形字符串转换成字节数组. */
void Misc::hexstr2dat(char* hex, int len, uchar* dat)
{
	int x = len / 2;
	for (int i = 0; i < x; ++i)
		dat[i] = Misc::hexstr2char(hex + (i * 2));
}

/** 整形转换成16进制字符串. */
void Misc::int2hexstr(uint i, char* str)
{
	str[0] = __0F__[((i >> 28) & 0x0F)];
	str[1] = __0F__[((i >> 24) & 0x0F)];
	str[2] = __0F__[((i >> 20) & 0x0F)];
	str[3] = __0F__[((i >> 16) & 0x0F)];
	str[4] = __0F__[((i >> 12) & 0x0F)];
	str[5] = __0F__[((i >> 8) & 0x0F)];
	str[6] = __0F__[((i >> 4) & 0x0F)];
	str[7] = __0F__[(i & 0x0F)];
}

/** 将字节流(dat)以大写16进制格式打印到标准输出. */
void Misc::printhex(uchar *dat, int len)
{
	int rows = len / 16;
	int ac = len % 16;
	int i;
	for (i = 0; i < rows; ++i)
		printf("%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", //
				dat[(16 * i) + 0], dat[(16 * i) + 1], dat[(16 * i) + 2], dat[(16 * i) + 3], //
				dat[(16 * i) + 4], dat[(16 * i) + 5], dat[(16 * i) + 6], dat[(16 * i) + 7], //
				dat[(16 * i) + 8], dat[(16 * i) + 9], dat[(16 * i) + 10], dat[(16 * i) + 11], //
				dat[(16 * i) + 12], dat[(16 * i) + 13], dat[(16 * i) + 14], dat[(16 * i) + 15], //
				Misc::b2c(dat[(16 * i) + 0]), Misc::b2c(dat[(16 * i) + 1]), Misc::b2c(dat[(16 * i) + 2]), Misc::b2c(dat[(16 * i) + 3]), //
				Misc::b2c(dat[(16 * i) + 4]), Misc::b2c(dat[(16 * i) + 5]), Misc::b2c(dat[(16 * i) + 6]), Misc::b2c(dat[(16 * i) + 7]), //
				Misc::b2c(dat[(16 * i) + 8]), Misc::b2c(dat[(16 * i) + 9]), Misc::b2c(dat[(16 * i) + 10]), Misc::b2c(dat[(16 * i) + 11]), //
				Misc::b2c(dat[(16 * i) + 12]), Misc::b2c(dat[(16 * i) + 13]), Misc::b2c(dat[(16 * i) + 14]), Misc::b2c(dat[(16 * i) + 15]));
	for (i = 0; i < ac; i++)
		printf("%02X ", dat[rows * 16 + i]);
	for (i = 0; ac > 0 && i < 16 - ac; i++)
		printf("%s", "   ");
	for (i = 0; i < ac; i++)
		printf("%c", Misc::b2c(dat[rows * 16 + i]));
	printf("\n");
}

/** 将字节流(dat)以大写16进制格式打印到str. */
void Misc::printhex2str(uchar* dat, int len, char* str)
{
	int rows = len / 16;
	int ac = len % 16;
	int i;
	if (rows > 0)
	{
		for (i = 0; i < rows; ++i)
			sprintf(str + 0x41 * i, "%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c\n", //
					dat[(16 * i) + 0], dat[(16 * i) + 1], dat[(16 * i) + 2], dat[(16 * i) + 3], //
					dat[(16 * i) + 4], dat[(16 * i) + 5], dat[(16 * i) + 6], dat[(16 * i) + 7],  //
					dat[(16 * i) + 8], dat[(16 * i) + 9], dat[(16 * i) + 10], dat[(16 * i) + 11],  //
					dat[(16 * i) + 12], dat[(16 * i) + 13], dat[(16 * i) + 14], dat[(16 * i) + 15],  //
					Misc::b2c(dat[(16 * i) + 0]), Misc::b2c(dat[(16 * i) + 1]), Misc::b2c(dat[(16 * i) + 2]), Misc::b2c(dat[(16 * i) + 3]), //
					Misc::b2c(dat[(16 * i) + 4]), Misc::b2c(dat[(16 * i) + 5]), Misc::b2c(dat[(16 * i) + 6]), Misc::b2c(dat[(16 * i) + 7]), //
					Misc::b2c(dat[(16 * i) + 8]), Misc::b2c(dat[(16 * i) + 9]), Misc::b2c(dat[(16 * i) + 10]), Misc::b2c(dat[(16 * i) + 11]), //
					Misc::b2c(dat[(16 * i) + 12]), Misc::b2c(dat[(16 * i) + 13]), Misc::b2c(dat[(16 * i) + 14]), Misc::b2c(dat[(16 * i) + 15]));
	}
	int offset = 0x41 * rows;
	if (ac != 0)
	{
		for (i = 0; i < 0x30 + ac; ++i)
			str[offset + i] = ' ';
		for (i = 0; i < ac; ++i)
		{
			sprintf(str + offset + (i * 3), "%02X ", dat[(rows * 16) + i]);
			sprintf(str + offset + (0x30 + i), "%c", Misc::b2c(dat[(rows * 16) + i]));
		}
		str[offset + (i * 3)] = 0x20;
		str[offset + (0x30 + i)] = '\n';
	}
}

/** 将字节流(dat)以大写16进制格式返回. */
string Misc::printhex2str(uchar* dat, int len)
{
	char* buf = (char*) calloc(1, (len / 16) * 65 + 65);
	Misc::printhex2str(dat, len, buf);
	string str(buf);
	free(buf);
	return str;
}

/** 将整形(32bit)打印成二进制字符串. */
void Misc::printInt2binStr(uint v, char* str)
{
	for (int i = 0; i < 32; ++i)
		str[31 - i] = (v >> i & 0x01) ? '1' : '0';
}

/** 将长整形(64bit)打印成二进制字符串. */
void Misc::printLong2binStr(ullong v, char* str)
{
	for (int i = 0; i < 64; ++i)
		str[63 - i] = (v >> i & 0x01) ? '1' : '0';
}

/** 将BCD编码转换成字符串形式. */
void Misc::bcd2str(uchar* dat, int len, char* str)
{
	int index = 0;
	int i = 0;
	for (; i < len; ++i)
	{
		uchar bit = (dat[i] & 0xF);
		if (bit > 0x09)
			break;
		str[index++] = (bit + 0x30);
		bit = ((dat[i] >> 0x04) & 0xF);
		if (bit > 0x09)
			break;
		str[index++] = (bit + 0x30);
	}
}

/** 将mobile identity转换成字符串形式,  见: 3GPP TS 24.008 10.5.1.4. */
void Misc::mi2str(uchar* mi, int len, char* str)
{
	uchar toi = mi[0] & 0x07;
	if (toi == 0x01 || toi == 0x02 || toi == 0x03)
	{
		str[0] = ((mi[0] >> 4) & 0x0F) + 0x30;
		int i = 1;
		for (; i < len; ++i)
		{
			uchar low = (mi[i] & 0x0F);
			uchar hig = ((mi[i] >> 0x04) & 0x0F);
			if (hig > 0x09)
			{
				str[i * 2 - 1] = low + 0x30;
				return;
			}
			str[i * 2 - 1] = low + 0x30;
			str[i * 2] = hig + 0x30;
		}
		return;
	}
}

/** 将access point name转换成字符串形式, 见: 3GPP TS 23.003.  */
void Misc::apn2str(uchar* apn, int len, char* str)
{
	if (apn[0] >= len)
		return;
	memcpy(str + strlen(str), apn + 1, apn[0]);
	if (len - 1 > apn[0])
	{
		sprintf(str + strlen(str), "%c", '.');
		Misc::apn2str(apn + apn[0] + 1, len - apn[0] - 1, str);
	}
}

/** 将小区ID转换成字符串形式. */
void Misc::cid2str(uchar* cell, int len, char* str)
{
	short idx = 0, j = 0;
	ushort tmp = 0;
	for (; idx < len; idx++)
	{
		if (idx <= 2)
		{
			str[j++] = (cell[idx] & 0x0F) + '0';
			str[j++] = ((cell[idx] & 0xF0) >> 4) + '0';
		} else
		{
			str[j++] = '-';
			tmp = ((cell[idx] & 0xFF) << 8) + (cell[idx + 1] & 0xFF);
			idx++;
			sprintf(str + j, "%d", (tmp));
			j += strlen(str + j);
		}
	}
	str[3] = '-';
	str[j] = 0;
}

/** 溯栈. */
string Misc::getStack()
{
#ifdef LINUX		/** need -rdynamic linker option support. */
	string ss;
	void* fs[0x100];
	int fc = ::backtrace(fs, 0x100);
	char** strs = ::backtrace_symbols(fs, fc);
	if (strs == NULL)
	{
		ss.assign("no stack.");
		return ss;
	}
	for (int i = 0; i < fc; ++i)
		ss.append(strs[i]).append("\n");
	free(strs);
	return ss;
#else
	return string("unsupported OS.");
#endif
}

/** 随机数设置. */
void Misc::srand()
{
	::srand(DateMisc::getTs());
}

/** 获得一个整形随机数. */
uint Misc::randomInt()
{
#ifdef LINUX
	static int fd = ::open("/dev/urandom", O_RDONLY | O_NONBLOCK);
	uint x;
	::read(fd, &x, sizeof(uint));
	return x;
#else
	return (((::rand() & 0xFFFF) << 16) | (::rand() & 0xFFFF)) * (((::rand() & 0xFFFF) << 16) | (::rand() & 0xFFFF));
#endif
}

/** 生成一个32位(4个字节)的随机数. */
void Misc::random32(uchar* r32)
{
	uint x = Misc::randomInt();
	memcpy(r32, &x, 4);
}

/** 生成一个64位(8个字节)的随机数. */
void Misc::random64(uchar* r64)
{
	Misc::random32(r64);
	Misc::random32(r64 + 0x04);
}

/** 生成一个128位(16个字节)的随机数. */
void Misc::random128(uchar* r128)
{
	Misc::random64(r128);
	Misc::random64(r128 + 0x08);
}

/** 生成一个256位(32个字节)的随机数. */
void Misc::random256(uchar* r256)
{
	Misc::random128(r256);
	Misc::random128(r256 + 0x10);
}

/** 生成一个512位(64个字节)的随机数. */
void Misc::random512(uchar* r512)
{
	Misc::random256(r512);
	Misc::random256(r512 + 0x20);
}

/** 产生一个32位(4个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
void Misc::gen0aAkey32(char* out)
{
	uint x = Misc::randomInt();
	out[0] = __0aA__[(x) % sizeof(__0aA__)];
	out[1] = __0aA__[(x >> 0x08) % sizeof(__0aA__)];
	out[2] = __0aA__[(x >> 0x10) % sizeof(__0aA__)];
	out[3] = __0aA__[(x >> 0x18) % sizeof(__0aA__)];
}

/** 产生一个32位(4个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
string Misc::gen0aAkey32()
{
	char s[0x05];
	Misc::gen0aAkey32(s);
	s[0x04] = 0;
	return string(s);
}

/** 产生一个64位(8个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
void Misc::gen0aAkey64(char* out)
{
	Misc::gen0aAkey32(out);
	Misc::gen0aAkey32(out + 0x04);
}

/** 产生一个64位(8个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
string Misc::gen0aAkey64()
{
	char s[0x09];
	Misc::gen0aAkey64(s);
	s[0x08] = 0;
	return string(s);
}

/** 产生一个128位(16个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
void Misc::gen0aAkey128(char* out)
{
	Misc::gen0aAkey64(out);
	Misc::gen0aAkey64(out + 0x08);
}

/** 产生一个128位(16个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
string Misc::gen0aAkey128()
{
	char s[0x11];
	Misc::gen0aAkey128(s);
	s[0x10] = 0;
	return string(s);
}

/** 产生一个256位(32个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
void Misc::gen0aAkey256(char* out)
{
	Misc::gen0aAkey128(out);
	Misc::gen0aAkey128(out + 0x10);
}

/** 产生一个256位(32个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
string Misc::gen0aAkey256()
{
	char s[0x21];
	Misc::gen0aAkey256(s);
	s[0x20] = 0;
	return string(s);
}

/** 产生一个512位(64个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
void Misc::gen0aAkey512(char* out)
{
	Misc::gen0aAkey256(out);
	Misc::gen0aAkey256(out + 0x20);
}

/** 产生一个512位(64个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
string Misc::gen0aAkey512()
{
	char s[0x41];
	Misc::gen0aAkey512(s);
	s[0x40] = 0;
	return string(s);
}

/** 返回全路径文件中的文件名及对应的size(不超过4G), 成功返回0, 其它为失败. */
int Misc::getFileAtt(char* path, char* name, ullong* size)
{
	int i = strlen(path) - 1;
	int indx = -1;
	for (; i >= 0; --i)
	{
#ifdef WIN32
		if (path[i] == '\\')
#else
		if (path[i] == '/')
#endif
		{
			indx = i;
			break;
		}
	}
	if (name != NULL)
	{
		if (indx != -1)
			memcpy(name, path + indx + 1, strlen(path) - indx - 1);
		else
			memcpy(name, path, strlen(path));
	}
#ifdef WIN32
	struct _stati64 s64;
	if (_stati64(path, &s64) != 0)
	return 1;
	*size = s64.st_size;
	return 0;
#else
	struct stat x;
	if (stat(path, &x) != 0)
		return 1;
	*size = x.st_size;
	return 0;
#endif
}

/** 毫秒级睡眠. */
void Misc::sleep(int mec)
{
#ifdef WIN32
	Sleep(mec);
#else
	usleep(1000 * mec);
#endif
}

/** 返回目录下的所有文件, 不递归. */
void Misc::listFiles(const char* path, list<string>* files)
{
#ifndef WINDOWS
	DIR* dir = ::opendir(path);
	struct dirent *dr;
	while ((dr = ::readdir(dir)) != NULL)
	{
		if (::strcmp(".", dr->d_name) != 0 && ::strcmp("..", dr->d_name) != 0)
			files->push_back(string(dr->d_name));
	}
	::closedir(dir);
#else
	LOG_FAULT("unsupported OS.")
#endif
}

/** 加载文件, 返回全部内容, 返回值需要在外部被free, 此函数适用于加载小的文件. */
uchar* Misc::loadFile(const char* file, int* len)
{
#ifndef WINDOWS
	ullong xlen = 0;
	if (Misc::getFileAtt((char*) file, NULL, &xlen) != 0)
		return NULL;
	int f = ::open(file, O_RDONLY);
	if (f < 1)
		return NULL;
	*len = xlen;
	uchar* dat = (uchar*) ::malloc((size_t) *len);
	if (::read(f, dat, (size_t) *len) != *len)
	{
		::free(dat);
		::close(f);
		return NULL;
	}
	::close(f);
	return dat;
#else
	LOG_FAULT("unsupported OS.")
	return NULL;
#endif

}

/** 检查账号的合法性, 规则是: 不为NULL, 长度不小于min且不超过max值, 字符范围为0 ~ 9, aA ~ zZ和'@', '.', '_'. */
bool Misc::checkNameFormat(const char* name, int min, int max)
{
	if (name == NULL)
		return false;
	int len = strlen(name);
	if (len < min || len > max)
		return false;
	for (int i = 0; i < len; ++i)
	{
		if ((::isdigit(name[i]) == 0) && (::isalpha(name[i]) == 0) && (name[i] != '@') && (name[i] != '.') && (name[i] != '_'))
			return false;
	}
	return true;
}

/** 检查密码的合法性, 规则是: 不为NULL, 长度不小于min且不超过max值, 字符范围为0 ~ 9, aA ~ zZ和'@', '.', '_'. */
bool Misc::checkPwdFormat(const char* name, int min, int max)
{
	return Misc::checkNameFormat(name, min, max);
}

/** 检查email的合法性. */
bool Misc::checkEmailFormat(const char* email, int max)
{
	/** 函数不能避免... @@@ .@. 这种组合. */
	if (email == NULL)
		return false;
	int len = strlen(email);
	if (len < 3 || len > max) //x@x
		return false;
	for (int i = 0; i < len; ++i)
	{
		if (email[i] != 0x2E && email[i] != 0x40 && (isdigit(email[i]) == 0) && (isalpha(email[i]) == 0))
			return false;
	}
	return true;
}

/** 字节转换成可见字符, 如果不可转换, 置为'.'. */
char Misc::b2c(uchar chr)
{
	return (chr > 0x20 && chr < 0x7F) ? chr : '.';
}

/** 检查字符串格式. */
bool Misc::checkRegex(const char* str, const char* pattern)
{
#ifdef LINUX
	int ret;
	regex_t regex;
	ret = regcomp(&regex, pattern, REG_EXTENDED | REG_ICASE);
	if (ret != 0)
		return false;
	ret = regexec(&regex, str, 0, NULL, 0);
	regfree(&regex);
	return ret != REG_NOMATCH;
#else
	LOG_FAULT("unsupported OS.")
	return false;
#endif
}

/** 检查角色名称(UTF-8)格式(汉字算一个单位长度). */
bool Misc::checkRole(const string& role, int min, int max)
{
	int len = 0;
	for (uint i = 0; i < role.length();)
	{
		char word[4] = { 0 };
		strncpy(word, role.c_str() + i, 3);
		if (Misc::checkRegex(word, "^[\u4E00-\u9FFF]"))
			i += 3; /** UTF-8汉字字节长度为3. */
		else
			i += 1;
		len += 1;
	}
	if (len < min || len > max)	// over size
		return false;
	return Misc::checkRegex(role.c_str(), "^[\u4E00-\u9FFF|0-9a-zA-Z][\u4E00-\u9FFF|0-9a-zA-Z]*$");
}

/** 是不是由纯字母组成的字符串. */
bool Misc::isAlphaString(const char* str)
{
	int len = strlen(str);
	for (int i = 0; i < len; ++i)
		if (::isalpha(str[i]) == 0)
			return false;
	return len > 0;
}

/** 是不是由纯数字组成的字符串. */
bool Misc::isDigitString(const char* str)
{
	int len = strlen(str);
	for (int i = 0; i < len; ++i)
		if (::isdigit(str[i]) == 0)
			return false;
	return len > 0;
}

/** 是不是由字母和数字组成的字符串. */
bool Misc::isAlpDiString(const char* str)
{
	int len = strlen(str);
	for (int i = 0; i < len; ++i)
		if (::isalpha(str[i]) == 0 && ::isdigit(str[i]) == 0)
			return false;
	return len > 0;
}

/** 是不是由可见字符(0x20 ~ 0x7E)组成的字符串. */
bool Misc::isVisAdString(const char* str)
{
	int len = strlen(str);
	for (int i = 0; i < len; ++i)
		if (str[i] < 0x20 || str[i] > 0x7E)
			return false;
	return len > 0;
}

/** 分隔字符串. */
void Misc::split(const char* str, const char* key, vector<string>* arrs, int num)
{
	int pos = 0;
	int keylen = strlen(key);
	int len = strlen(str);
	int tlen = len;
	const char* tmp = strstr(str, key);
	while (tmp != NULL)
	{
		int xlen = strlen(tmp);
		if (tlen > xlen)
		{
			if (num == 0 || (int) arrs->size() < num)
			{
				string x;
				x.assign(str + pos, tlen - xlen);
				arrs->push_back(x);
			} else
				return;
		}
		pos += (tlen - xlen) + keylen;
		tlen = xlen - keylen;
		tmp = strstr(str + pos, key);
	}
	if (len > pos)
	{
		if (num == 0 || (int) arrs->size() < num)
		{
			string x;
			x.assign(str + pos, len - pos);
			arrs->push_back(x);
		}
	}
}

/** 分隔url, 返回所有的key-value对. */
void Misc::splitUrl(const char* url, unordered_map<string, string>* kvs)
{
	vector<string> vec;
	Misc::split(url, "&", &vec);
	if (vec.size() < 1)
		return;
	for (vector<string>::iterator it = vec.begin(); it != vec.end(); ++it)
	{
		vector<string> kv;
		Misc::split(it->c_str(), "=", &kv);
		if (kv.size() != 2)
			continue;
		kvs->insert(make_pair(kv[0], kv[1]));
	}
}

/** 返回一个字符串值, 适用于<element att="val" /> */
void Misc::getStrAtt(XMLElement* element, const char* att, string* val)
{
	const char* x = element->Attribute(att);
	if (x != NULL)
	{
		val->clear();
		val->assign(x);
	}
}

/** 返回一个字符串值, 适用于<element><node>val</node></element>*/
void Misc::getStrChild(XMLElement* element, const char* node, string* val)
{
	XMLElement* child = element->FirstChildElement(node);
	if (child == NULL)
		return;
	val->assign(child->GetText() == NULL ? "" : child->GetText());
}

/** 返回一个整数值, 适用于<element att="val" /> */
void Misc::getIntAtt(XMLElement* element, const char* att, int* val)
{
	*val = element->IntAttribute(att);
}

/** 返回一个整数值, 适用于<element><node>val</node></element>*/
void Misc::getIntChild(XMLElement* element, const char* node, int* val)
{
	XMLElement* child = element->FirstChildElement(node);
	if (child == NULL)
		return;
	*val = ::atoi(child->GetText());
}

/** RC4加密. */
void Misc::rc4enc(uchar* key, int klen, uchar* in, int ilen, uchar* out)
{
#ifdef USE_OPENSSL
	RC4_KEY k;
	::RC4_set_key(&k, klen, key);
	::RC4(&k, ilen, in, out);
#else
	struct rc4_state ks;
	rc4_init(&ks, key, klen);
	rc4_crypt(&ks, in, out, ilen);
#endif
}

/** RC4解密. */
void Misc::rc4dec(uchar* key, int klen, uchar* in, int ilen, uchar* out)
{
	Misc::rc4enc(key, klen, in, ilen, out);
}

/** MD5签名. */
void Misc::md5(uchar* in, int ilen, uchar* out)
{
	MD5_CTX ctx;
	MD5_Init(&ctx);
	MD5_Update(&ctx, in, ilen);
	MD5_Final(out, &ctx);
}

/** SHA-1签名. */
void Misc::sha1(uchar* in, int ilen, uchar* out)
{
#ifdef USE_OPENSSL
	::SHA1(in, ilen, out);
#else
	SHA1(in, ilen, out);
#endif
}

/** SHA-256签名. */
void Misc::sha256(uchar* in, int ilen, uchar* out)
{
#ifdef USE_OPENSSL
	::SHA256(in, ilen, out);
#else
	SHA256_CTX sc;
	sha256_init(&sc);
	sha256_update(&sc, in, ilen);
	sha256_final(&sc, out);
#endif
}

/** SHA-256签名(以16进制大写字符串表现形式返回). */
string Misc::sha256(uchar* in, int ilen)
{
	uchar dat[0x20];
	Misc::sha256(in, ilen, dat);
	char str[0x41];
	Misc::hex2str1(dat, 0x20, str);
	str[0x40] = 0;
	return string(str);
}

/** SHA-512签名. */
void Misc::sha512(uchar* in, int ilen, uchar* out)
{
#ifdef USE_OPENSSL
	::SHA512(in, ilen, out);
#else
	sha_512(in, ilen, out, 0);
#endif
}

/** SHA-512签名(以16进制大写字符串表现形式返回). */
string Misc::sha512(uchar* in, int ilen)
{
	uchar dat[0x40];
	Misc::sha512(in, ilen, dat);
	char str[0x81];
	Misc::hex2str1(dat, 0x40, str);
	str[0x80] = 0;
	return string(str);
}

/** 获得环境变量的值. */
string Misc::getEnvStr(const char* key, const char* def)
{
	char* str = ::getenv(key);
	return (str != NULL) ? string(str) : (def == NULL ? "" : def);
}

/** 获得环境变量的值. */
int Misc::getEnvInt(const char* key, int def)
{
#ifndef WINDOWS
	char* str = ::getenv(key);
	return (str == NULL) ? def : (int) ::atoll(str);
#else
	LOG_FAULT("unsupported OS.")
	return 0;
#endif
}

/** 获得并设置(如果未设置)环境变量的值 */
string Misc::getSetEnvStr(const char* key, const char* def)
{
	string val = Misc::getEnvStr(key, def);
	Misc::setEnv(key, val.c_str());
	return val;
}

/** 获得并设置(如果未设置)环境变量的值 */
int Misc::getSetEnvInt(const char* key, int def)
{
	int val = Misc::getEnvInt(key, def);
	Misc::setEnv(key, Misc::itoa(val).c_str());
	return val;
}

/** 设置环境变量. */
void Misc::setEnv(const char* key, const char* val)
{
#ifdef LINUX
	::setenv(key, val, 1);
#else
	LOG_FAULT("unsupported OS.")
#endif
}

/** 获得所有环境变量, 顺序列出. */
void Misc::getEnvs(list<string>* evns)
{
#ifdef LINUX
	char **env = environ;
	while (*env)
		evns->push_back(string(*env++));
	evns->sort();
#else
	LOG_FAULT("unsupported OS.")
#endif
//	evns.sort([](string& str0, string& str1)
//	{
//		return str0.compare(str1) < 0; /** 顺序列出. */
//	});
//	evns.sort([](string& str0, string& str1)
//	{
//		return str0.compare(str1) > 0; /** 倒序列出. */
//	});
}

/** 整型转字符串. */
string Misc::itoa(long long int v)
{
	string str;
#ifdef __MINGW_H
	SPRINTF_STRING(&str, "%I64d", v)
#else
	SPRINTF_STRING(&str, "%lld", v)
#endif
	return str;
}

#ifndef WINDOWS
static void* thread_svc(void* arg)
{
	((void (*)()) arg)();
	return NULL;
}
#else
static void thread_svc(void* arg)
{
	((void (*)()) arg)();
}
#endif

/** 创建一个线程. */
void Misc::newThread(void (*svc)())
{
#ifdef WINDOWS
	_beginthread(thread_svc, 0, (void*)svc);
#else
	pthread_t t;
	pthread_create(&t, NULL, thread_svc, (void*) svc);
	pthread_detach(t);
#endif
}

/** 初始化锁. */
void Misc::initlock(misc_mutex* mutex)
{
#ifndef WINDOWS
	::pthread_mutex_init(&mutex->mutex, NULL);
#else
	InitializeCriticalSection(&mutex->mutex);
#endif
}

/** 获得锁. */
void Misc::lock(misc_mutex& mutex)
{
#ifndef WINDOWS
	::pthread_mutex_lock(&mutex.mutex);
#else
	EnterCriticalSection(&mutex.mutex);
#endif
}

/** 释放锁. */
void Misc::unlock(misc_mutex& mutex)
{
#ifndef WINDOWS
	::pthread_mutex_unlock(&mutex.mutex);
#else
	LeaveCriticalSection(&mutex.mutex);
#endif
}

/** 销毁锁. */
void Misc::deslock(misc_mutex* mutex)
{
#ifndef WINDOWS
	::pthread_mutex_destroy(&mutex->mutex);
#else
	DeleteCriticalSection(&mutex->mutex);
#endif
}

Misc::~Misc()
{

}
