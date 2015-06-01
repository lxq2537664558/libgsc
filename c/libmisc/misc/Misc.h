/*
 * Misc.h
 *
 *  Created on: 2011-3-21
 *      Author: xuzewen
 */

#ifndef MISC_H_
#define MISC_H_

#if !defined (__LIBMISC_H__) && !defined (LIBMISC)
#error only libmisc.h can be included directly.
#endif

#include "../macro.h"
#include "tinyxml2.h"
using namespace tinyxml2;

typedef struct
{
#ifndef WINDOWS
	pthread_mutex_t mutex;
#else
	CRITICAL_SECTION mutex;
#endif
} misc_mutex;

class Misc
{
private:
	Misc();
	virtual ~Misc();
	static char b2c(uchar chr); /** 字节转换成可见字符, 如果不可转换, 置为'.'. */
public:
	static bool isHexStr(uchar* dat, int len); /** 是不是一串16进制字符. */
	static void hex2str0(uchar* dat, int len, char* str); /** 将字节流(dat)以大写16进制格式打印到str中, 无换行, 带空格. */
	static void hex2str1(uchar* dat, int len, char* str); /** 将字节流(dat)以大写16进制格式打印到str中, 无换行, 无空格. */
	static string hex2str1(uchar* dat, int len); /** 将字节流(dat)以大写16进制格式返回, 无换行, 无空格. */
	static string hex2str1(const string& str); /** 将字节流(str)以大写16进制格式返回, 无换行, 无空格. */
	static void hex2str2(uchar* dat, int len, char* str); /** 将字节流(dat)以小写16进制格式打印到str中, 无换行, 无空格. */
	static string hex2str2(uchar* dat, int len); /** 将字节流(dat)以小写16进制格式返回, 无换行, 无空格. */
	static uchar hexstr2char(char* hex); /** 16进制字符串(一定是2字节, 如"FF")转换成单字节. */
	static ushort hexstr2short(char* hex); /** 16进制整形字符串(一定是4字节, 如"FFFF")转换成短整形. */
	static uint hexstr2int(char* hex); /** 16进制整形字符串(一定是8字节, 如"FFFFFFFF")转换成整形. */
	static void hexstr2dat(char* hex, int len, uchar* dat); /** 16进制整形字符串转换成字节数组. */
	static void int2hexstr(uint i, char* str); /** 整形转换成16进制字符串. */
	static void printhex(uchar *dat, int len); /** 将字节流(dat)以大写16进制格式打印到标准输出. */
	static void printhex2str(uchar* dat, int len, char* str); /** 将字节流(dat)以大写16进制格式打印到str. */
	static string printhex2str(uchar* dat, int len); /** 将字节流(dat)以大写16进制格式返回. */
	static void printInt2binStr(uint v, char* str); /** 将整形(32bit)打印成二进制字符串. */
	static void printLong2binStr(ullong v, char* str); /** 将长整形(64bit)打印成二进制字符串. */
	static void bcd2str(uchar* dat, int len, char* str); /** 将BCD编码转换成字符串形式. */
	static void mi2str(uchar* mi, int len, char* str); /** 将mobile identity转换成字符串形式,  见: 3GPP TS 24.008 10.5.1.4. */
	static void apn2str(uchar* apn, int len, char* str); /** 将access point name转换成字符串形式, 见: 3GPP TS 23.003.  */
	static void cid2str(uchar* cell, int len, char* str); /** 将小区ID转换成字符串形式. */
	static string getStack(); /** 溯栈. */
	static void srand(); /** 随机数设置. */
	static uint randomInt(); /** 获得一个整形随机数. */
	static void random32(uchar* r32); /** 生成一个32位(4个字节)的随机数. */
	static void random64(uchar* r64); /** 生成一个64位(8个字节)的随机数. */
	static void random128(uchar* r128); /** 生成一个128位(16个字节)的随机数. */
	static void random256(uchar* r256); /** 生成一个256位(32个字节)的随机数. */
	static void random512(uchar* r512); /** 生成一个512位(64个字节)的随机数. */
	static void gen0aAkey32(char* out); /** 产生一个32位(4个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static string gen0aAkey32(); /** 产生一个32位(4个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static void gen0aAkey64(char* out); /** 产生一个64位(8个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static string gen0aAkey64(); /** 产生一个64位(8个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static void gen0aAkey128(char* out); /** 产生一个128位(16个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static string gen0aAkey128(); /** 产生一个128位(16个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static void gen0aAkey256(char* out); /** 产生一个256位(32个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static string gen0aAkey256(); /** 产生一个256位(32个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static void gen0aAkey512(char* out); /** 产生一个512位(64个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static string gen0aAkey512(); /** 产生一个512位(64个字节)随机key, 内容为随机的 0 ~ 9, a ~ z, A ~ Z, _. */
	static int getFileAtt(char* path, char* name, ullong* size); /** 返回全路径文件中的文件名及对应的size(不超过4G), 成功返回0, 其它为失败. */
	static void sleep(int mec); /** 毫秒级睡眠. */
	static void listFiles(const char* path, list<string>* files); /** 返回目录下的所有文件, 不递归. */
	static uchar* loadFile(const char* file, int* len); /** 加载文件, 返回全部内容, 返回值需要在外部被free, 此函数适用于加载小的文件. */
	static bool checkNameFormat(const char* name, int min, int max); /** 检查账号的合法性, 规则是: 不为NULL, 长度不小于min且不超过max值, 字符范围为0 ~ 9, aA ~ zZ和'@', '.', '_'. */
	static bool checkPwdFormat(const char* name, int min, int max); /** 检查密码的合法性, 规则是: 不为NULL, 长度不小于min且不超过max值, 字符范围为0 ~ 9, aA ~ zZ和'@', '.', '_'. */
	static bool checkEmailFormat(const char* email, int max); /** 检查email的合法性. */
	static bool checkRegex(const char* str, const char* pattern); /** 检查字符串格式. */
	static bool checkRole(const string& role, int min, int max); /** 检查角色名称(UTF-8)格式(汉字算一个单位长度). */
	static bool isAlphaString(const char* str); /** 是不是由纯字母组成的字符串. */
	static bool isDigitString(const char* str); /** 是不是由纯数字组成的字符串. */
	static bool isAlpDiString(const char* str); /** 是不是由字母和数字组成的字符串. */
	static bool isVisAdString(const char* str); /** 是不是由可见字符(0x20 ~ 0x7E)组成的字符串. */
	static void split(const char* str, const char* key, vector<string>* arrs, int num = 0 /** 取前面n个, 后面忽略. */); /** 分隔字符串. */
	static void splitUrl(const char* url, unordered_map<string, string>* kvs); /** 分隔url, 返回所有的key-value对. */
	static void getStrAtt(XMLElement* element, const char* att, string* val); /** 返回一个字符串值, 适用于<element att="val"/> */
	static void getStrChild(XMLElement* element, const char* node, string* val); /** 返回一个字符串值, 适用于<element><node>val</node></element>*/
	static void getIntAtt(XMLElement* element, const char* att, int* val); /** 返回一个整数值, 适用于<element att="val"/> */
	static void getIntChild(XMLElement* element, const char* node, int* val); /** 返回一个字符串值, 适用于<element><node>val</node></element>*/
	static void rc4enc(uchar* key, int klen, uchar* in, int ilen, uchar* out); /** RC4加密. */
	static void rc4dec(uchar* key, int klen, uchar* in, int ilen, uchar* out); /** RC4解密. */
	static void md5(uchar* in, int ilen, uchar* out); /** MD5签名. */
	static void sha1(uchar* in, int ilen, uchar* out); /** SHA-1签名. */
	static void sha256(uchar* in, int ilen, uchar* out); /** SHA-256签名. */
	static string sha256(uchar* in, int ilen); /** SHA-256签名(以16进制大写字符串表现形式返回). */
	static void sha512(uchar* in, int ilen, uchar* out); /** SHA-512签名. */
	static string sha512(uchar* in, int ilen); /** SHA-512签名(以16进制大写字符串表现形式返回). */
	static string getEnvStr(const char* key, const char* def = NULL); /** 获得环境变量的值. */
	static int getEnvInt(const char* key, int def = 0);/** 获得环境变量的值. */
	static string getSetEnvStr(const char* key, const char* def = NULL); /** 获得并设置(如果未设置)环境变量的值 */
	static int getSetEnvInt(const char* key, int def = 0); /** 获得并设置(如果未设置)环境变量的值 */
	static void setEnv(const char* key, const char* val); /** 设置环境变量. */
	static void getEnvs(list<string>* evns); /** 获得所有环境变量, 顺序列出. */
	static string itoa(long long int v); /** 整型转字符串. */
	static void newThread(void (*svc)()); /** 创建一个线程. */
	static void initlock(misc_mutex* mutex); /** 初始化锁. */
	static void lock(misc_mutex& mutex); /** 获得锁. */
	static void unlock(misc_mutex& mutex); /** 释放锁. */
	static void deslock(misc_mutex* mutex); /** 销毁锁. */
};

#endif /* MISC_H_ */
