// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "../../env/_crtdef.h"

wchar_t *wcschr(const wchar_t *s, wchar_t c){
	register const wchar_t *rp = s;
	// 如果 rp 是对齐到字的，就不用考虑越界的问题。
	// 因为内存按页分配的，也自然对齐到页，并且也对齐到字。
	// 每个字内的字节的权限必然一致。
	while(((uintptr_t)rp & (sizeof(uintptr_t) - 1)) != 0){
		const wchar_t rc = *rp;
		if(rc == c){
			return (wchar_t *)rp;
		}
		if(rc == 0){
			return nullptr;
		}
		++rp;
	}
	for(;;){
		uintptr_t w = *(const uintptr_t *)rp;
		for(unsigned i = 0; i < sizeof(uintptr_t) / 2; ++i){
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
			const wchar_t rc = (wchar_t)w;
			w >>= 16;
#else
			const wchar_t rc = (wchar_t)(w >> (sizeof(uintptr_t) * 8 - 16));
			w <<= 16;
#endif
			if(rc == c){
				return (wchar_t *)rp;
			}
			if(rc == 0){
				return nullptr;
			}
			++rp;
		}
	}
}