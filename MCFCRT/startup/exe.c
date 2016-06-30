// 这个文件是 MCF 的一部分。
// 有关具体授权说明，请参阅 MCFLicense.txt。
// Copyleft 2013 - 2016, LH_Mouse. All wrongs reserved.

#include "exe.h"
#include "generic.h"
#include "../env/mcfwin.h"
#include "../env/_seh_top.h"
#include "../env/standard_streams.h"
#include "../ext/wcpcpy.h"
#include "../ext/itow.h"
#include "../env/bail.h"
#include "../env/module.h"

// -static -Wl,-e@__MCFCRT_ExeStartup,--disable-runtime-pseudo-reloc,--disable-auto-import

// __MCFCRT_ExeStartup 模块入口点。
_Noreturn __MCFCRT_C_STDCALL
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown)
	__asm__("@__MCFCRT_ExeStartup");

__attribute__((__used__))
int __MCFCRT_do_not_link_exe_startup_code_and_dll_startup_code_together = 0;

_Noreturn __attribute__((__noinline__))
static void BailWithErrorCode(const wchar_t *pwszMessage, DWORD dwErrorCode){
	wchar_t awcBuffer[512];
	wchar_t *pwcWrite;
	pwcWrite = _MCFCRT_wcppcpy(awcBuffer, awcBuffer + 448, pwszMessage);
	pwcWrite = _MCFCRT_wcpcpy(pwcWrite, L"\n\n错误代码：");
	pwcWrite = _MCFCRT_itow_u(pwcWrite, dwErrorCode);
	*pwcWrite = 0;
	_MCFCRT_Bail(awcBuffer);
}

__MCFCRT_C_STDCALL
static BOOL CrtCtrlHandler(DWORD dwCtrlType){
	if(_MCFCRT_OnCtrlEvent){
		const bool bIsSigInt = ((dwCtrlType == CTRL_C_EVENT) || (dwCtrlType == CTRL_BREAK_EVENT));
		_MCFCRT_OnCtrlEvent(bIsSigInt);
		return true;
	}

	static const wchar_t kKilledMessage[] = L"进程被 Ctrl-C 终止，因为没有找到自定义的 _MCFCRT_OnCtrlEvent() 响应函数。";
	_MCFCRT_WriteStandardErrorString(kKilledMessage, sizeof(kKilledMessage) / sizeof(wchar_t) - 1, true);

	TerminateProcess(GetCurrentProcess(), (DWORD)STATUS_CONTROL_C_EXIT);
	__builtin_trap();
}

// 如果 EXE 只链接了 KERNEL32.DLL 和 NTDLL.DLL 那么 TLS 回调就收不到 DLL_PROCESS_ATTACH 通知。这里需要处理这种情况。
static bool g_bTlsCallbackActiveUponInit = false;

__MCFCRT_C_STDCALL __attribute__((__noinline__))
static void ExeTlsCallback(LPVOID pInstance, DWORD dwReason, LPVOID pReserved){
	(void)pInstance;
	(void)pReserved;

	g_bTlsCallbackActiveUponInit = true;

	__MCFCRT_SEH_TOP_BEGIN
	{
		if(!__MCFCRT_TlsCallbackGeneric((void *)pInstance, (unsigned)dwReason, !pReserved)){
			BailWithErrorCode(L"MCFCRT 初始化失败。", GetLastError());
		}
	}
	__MCFCRT_SEH_TOP_END

	// 忽略错误。
	if(dwReason == DLL_PROCESS_ATTACH){
		SetConsoleCtrlHandler(&CrtCtrlHandler, true);
	} else if(dwReason == DLL_PROCESS_DETACH){
		SetConsoleCtrlHandler(&CrtCtrlHandler, false);
	}
}

extern const IMAGE_TLS_DIRECTORY _tls_used;

__extension__ __attribute__((__section__(".tls$AAA")))
static const char tls_begin[0] = { };
__extension__ __attribute__((__section__(".tls$ZZZ")))
static const char tls_end[0]   = { };

__attribute__((__section__(".CRT$AAA")))
static const PIMAGE_TLS_CALLBACK callback_begin = &ExeTlsCallback;
__attribute__((__section__(".CRT$ZZZ"), __used__))
static const PIMAGE_TLS_CALLBACK callback_end   = nullptr;

__attribute__((__section__(".data")))
static DWORD tls_index = 0xDEADBEEF;

__attribute__((__section__(".rdata"), __used__))
const IMAGE_TLS_DIRECTORY _tls_used = { (UINT_PTR)&tls_begin, (UINT_PTR)&tls_end, (UINT_PTR)&tls_index, (UINT_PTR)&callback_begin, 0, 0 };

_Noreturn __MCFCRT_C_STDCALL __attribute__((__noinline__))
DWORD __MCFCRT_ExeStartup(LPVOID pUnknown){
	(void)pUnknown;

	DWORD dwExitCode;

	__MCFCRT_SEH_TOP_BEGIN
	{
		if(!g_bTlsCallbackActiveUponInit){
			ExeTlsCallback(_MCFCRT_GetModuleBase(), DLL_PROCESS_ATTACH, false);
		}

		dwExitCode = _MCFCRT_Main();
	}
	__MCFCRT_SEH_TOP_END

	ExitProcess(dwExitCode);
	__builtin_trap();
}
