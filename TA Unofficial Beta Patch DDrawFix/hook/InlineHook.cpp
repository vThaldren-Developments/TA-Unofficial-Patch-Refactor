#include "..\oddraw.h"
#include "hook.h"
#include "etc.h"
//InlineSingleHook:
//��ֹ����ʽת����warn��ʾ�������ԭ����ȷ����ת��û�����:
#pragma warning(disable:4311)
#pragma warning(disable:4312)
//
typedef PInlineX86StackBuffer (__stdcall *  defPX86CurrentThreadStackBuffer) (PInlineX86StackBuffer *);
defPX86CurrentThreadStackBuffer Global_PX86CurrentThreadStackBufferFixRtnAddr= X86CurrentThreadStackBufferFixRtnAddr;
defPX86CurrentThreadStackBuffer Global_PX86CurrentThreadStackBuffer= X86CurrentThreadStackBuffer;
const int Global_OffOfEnteredFlagFromInlineX86StackBuffer= (int)&((PInlineX86StackBuffer)0)->EnteredFlag_I;

void __declspec(naked) X86InlineHookRouter (void)
{//���̵߳�ʱ��ͳ����ˡ���Ҫ����Ķ�ջ���ɶ�̬������ڴ棬�Լ�һ���Ƿ��ѽ����HOOK�ı�־��
 //����������һ����־�����
	//__asm lea esp, DWORD PTR [esp+ 4]//����jmp��ʱ�������ջƽ�⡣��call�������ҪNOP��
#define X86INLINEROUTERSTRACKADD4 0x0
	__asm lea esp, DWORD PTR [esp+ 4]//����JMP��ʱ�������ջƽ�⡣��jmp�������ҪNOP��

	__asm pushad
	__asm pushfd

	__asm call TempAddr

#define X86INLINEROUTERSTACKBUFFEROFF 0xb
addrOfStrackBuffer:
	__asm __emit 0x00  ;//�����4��0�����Ϊ�Ժ��pushfd/ad�Ķ����õġ� off 0x7
	__asm __emit 0x00
	__asm __emit 0x00
	__asm __emit 0x00
TempAddr:
 	__asm pop ebx
	__asm push ebx
	__asm call [Global_PX86CurrentThreadStackBufferFixRtnAddr]

	__asm mov ebx, eax;

	__asm mov eax, Global_OffOfEnteredFlagFromInlineX86StackBuffer;
	__asm mov eax, DWORD PTR [ebx+ eax]
	__asm cmp eax, 0xbd88
	__asm je AfterCallRouter
	__asm mov eax, Global_OffOfEnteredFlagFromInlineX86StackBuffer;
	__asm mov DWORD PTR [ebx+ eax], 0xbd88
	__asm mov ecx, 0x9 //�ౣ��1����ջ�е����ݡ���ǰд��0xa�������ˡ�
	__asm lea edi, DWORD PTR[ebx+ 4]
	__asm mov esi, esp
	__asm rep movsd

#define X86INLINEROUTERSTRACKSUB4 0x44
	__asm push ebx //�ƶ�һ�¶�ջ
	__asm mov edi, esp//�ָ��Ĵ���
	__asm lea esi, DWORD PTR[ebx+ 0x4]
	__asm mov ecx, 0x9
	__asm rep movsd
	__asm mov DWORD PTR[esp+ 0x9* 0x4], ebx;
	__asm popfd
	__asm popad


	__asm __emit 0xe8  ;//�����Ҳ��Ҫ������
#define X86INLINEROUTERCALLADDROFF 0x58
	__asm __emit 0x00  ;//�����4��0�����Ϊ�Ժ󱣴�Ҫ���õ�Ŀ��PROC�õġ�
	__asm __emit 0x00
	__asm __emit 0x00
	__asm __emit 0x00
	
	__asm mov edi, eax//����hookrouter����ֵ

	__asm call OffToStrackBuffer
OffToStrackBuffer:
	__asm pop ebx
	__asm sub ebx, offset OffToStrackBuffer
	__asm add ebx, offset addrOfStrackBuffer
	__asm push ebx
	__asm call [Global_PX86CurrentThreadStackBuffer]

	__asm mov ebx, eax;

	__asm mov esp, DWORD PTR[ebx]InlineX86StackBuffer.Esp;//popad�޸Ĳ���esp,ֻ��+0x20����Ҫ�Լ����ú�ESP��

	__asm cmp edi, X86STRACKBUFFERCHANGE//�ж�hook router����ֵ
	__asm jne NormalJmpBack
	//�޸���STRACKBUFFER�����ص�ַ��Ҫ��hook router��ָ������ʲô�ˡ����Ĵ�������Ҫ�ָ���HOOK ROUTER��������Ǹ��ṹ�е�ֵ��

	__asm mov eax, Global_OffOfEnteredFlagFromInlineX86StackBuffer;
	__asm mov DWORD PTR [ebx+ eax], 0//�ͷ����λ�õı�־

	

	__asm push DWORD PTR [ebx]
	__asm sub esp, 0x9* 0x4
	__asm mov ecx, 0x9
	__asm mov edi, esp
	__asm lea esi, DWORD PTR[ebx+ 4]
	__asm rep movsd
	__asm popfd
	__asm popad;

	__asm retn

NormalJmpBack:
	__asm sub esp, 0x9* 0x4//��pushad pushfd��call�Ķ�ջ�ռ������,jmp��ĳ�0x9��
	__asm mov ecx, 0x9 //�ָ���ջ
	__asm mov edi, esp
	__asm lea esi, DWORD PTR[ebx+ 4]
	__asm rep movsd

AfterCallRouter:
	__asm mov eax, Global_OffOfEnteredFlagFromInlineX86StackBuffer;
	__asm mov DWORD PTR [ebx+ eax], 0//�ͷ����λ�õı�־

	__asm popfd
	__asm popad;
	//�����û���أ���Ϊ�Ժ��ԭʼ����+ jmp/call���صĴ����õġ�
#define X86INLINEROUTERENDOFF 0xC1
}

PInlineX86StackBuffer __stdcall X86CurrentThreadStackBufferFixRtnAddr (PInlineX86StackBuffer * in_Pix86StackBuf)
{
	PInlineX86StackBuffer X86StackBufferAry= *in_Pix86StackBuf;
	DWORD ThreadID_Dw= GetCurrentThreadId ( );
	PInlineX86StackBuffer temp_Pix86Stackbuf= X86StackBufferAry;
	while ((NULL!=temp_Pix86Stackbuf))
	{
		if (ThreadID_Dw==temp_Pix86Stackbuf->TID_Dw)
		{
			temp_Pix86Stackbuf->rtnAddr_Pvoid= temp_Pix86Stackbuf->myInlineHookClass_Pish->RtnAddrOfHook();
			return temp_Pix86Stackbuf;
		}
		temp_Pix86Stackbuf= temp_Pix86Stackbuf->next;
	}

	temp_Pix86Stackbuf= new InlineX86StackBuffer;
	ZeroMemory ( temp_Pix86Stackbuf, sizeof(InlineX86StackBuffer));
	temp_Pix86Stackbuf->TID_Dw= ThreadID_Dw;
	temp_Pix86Stackbuf->next= X86StackBufferAry;
	temp_Pix86Stackbuf->myInlineHookClass_Pish= X86StackBufferAry->myInlineHookClass_Pish;
	temp_Pix86Stackbuf->rtnAddr_Pvoid= temp_Pix86Stackbuf->myInlineHookClass_Pish->RtnAddrOfHook();
	
	*in_Pix86StackBuf= temp_Pix86Stackbuf;

	return temp_Pix86Stackbuf;
}

PInlineX86StackBuffer __stdcall X86CurrentThreadStackBuffer (PInlineX86StackBuffer * in_Pix86StackBuf)
{

	PInlineX86StackBuffer X86StackBufferAry= *in_Pix86StackBuf;
	DWORD ThreadID_Dw= GetCurrentThreadId ( );
	PInlineX86StackBuffer temp_Pix86Stackbuf= X86StackBufferAry;
	while ((NULL!=temp_Pix86Stackbuf))
	{
		if (ThreadID_Dw==temp_Pix86Stackbuf->TID_Dw)
		{
			return temp_Pix86Stackbuf;
		}
		temp_Pix86Stackbuf= temp_Pix86Stackbuf->next;
	}

	temp_Pix86Stackbuf= new InlineX86StackBuffer;
	ZeroMemory ( temp_Pix86Stackbuf, sizeof(InlineX86StackBuffer));
	temp_Pix86Stackbuf->TID_Dw= ThreadID_Dw;
	temp_Pix86Stackbuf->next= X86StackBufferAry;
	temp_Pix86Stackbuf->myInlineHookClass_Pish= X86StackBufferAry->myInlineHookClass_Pish;

	*in_Pix86StackBuf= temp_Pix86Stackbuf;


	return temp_Pix86Stackbuf;
}


InlineSingleHook::InlineSingleHook ()
{
	InitHookClass ( NULL, 5, 0, NULL);
	if (NULL!=m_NewBytes_Pbyte)
	{
		Hook ( );
	}
}


InlineSingleHook::InlineSingleHook (unsigned int AddrToHook_Pvoid, DWORD Len_Dw, int HookMode, InlineX86HookRouter RouterAddr)
{
	InitHookClass ( reinterpret_cast<LPBYTE> (AddrToHook_Pvoid), Len_Dw, HookMode, (LPVOID)RouterAddr);
	if (NULL!=m_NewBytes_Pbyte)
	{
		Hook ( );
	}
}

InlineSingleHook::InlineSingleHook (unsigned int AddrToHook_Pvoid, DWORD Len_Dw, int HookMode, LPVOID RouterAddr)
{
	InitHookClass ( reinterpret_cast<LPBYTE> (AddrToHook_Pvoid), Len_Dw, HookMode, RouterAddr);
	if (NULL!=m_NewBytes_Pbyte)
	{
		Hook ( );
	}
}

InlineSingleHook::~InlineSingleHook ()
{
// 	if (Inline_5Bytes!=m_NewBytes_Pbyte)
// 	{//�ⲿ��buffer���ù�
// 
// 	}
	if (NULL!=m_OrgBytes_Pbyte)
	{
		UnHook ( );
		delete []m_OrgBytes_Pbyte;
		m_OrgBytes_Pbyte= NULL;
	}


	if (NULL!=m_HookRouter_Pproc)
	{
		PInlineX86StackBuffer X86StackBufferAry= *((PInlineX86StackBuffer *)&m_HookRouter_Pproc[X86INLINEROUTERSTACKBUFFEROFF]);
		while (NULL!=X86StackBufferAry)
		{
			PInlineX86StackBuffer temp_Ix86Stackbuf= X86StackBufferAry->next;
			delete [] X86StackBufferAry;
			X86StackBufferAry= temp_Ix86Stackbuf;
		}

		delete [] m_HookRouter_Pproc;
		m_HookRouter_Pproc= NULL;
	}
	if (NULL!=m_RedirectOrgOpcodes_Pbyte)
	{
		delete [] m_RedirectOrgOpcodes_Pbyte;
		m_RedirectOrgOpcodes_Pbyte= NULL;
	}
}

LPBYTE InlineSingleHook::NewBytesForInlineHook (LPBYTE NewBytes_Pvoid)
{
	LPBYTE tempRtn_Pbyte= m_NewBytes_Pbyte;
	m_NewBytes_Pbyte= NewBytes_Pvoid;
	return tempRtn_Pbyte;
}

LPVOID InlineSingleHook::SetParamOfHook (LPVOID ParamOfHook_Pvoid)
{
	LPVOID rtn= ParamOfHook;
	ParamOfHook= ParamOfHook_Pvoid;
	return rtn;
}

void InlineSingleHook::InitHookClass (const LPBYTE AddrToHook_Pvoid, DWORD Len_Dw, int HookMode, LPVOID RouterAddr, LPVOID ParamOfHook_Pvoid)
{
	m_NewBytes_Pbyte= NULL;
	m_LenToModify_Dw= 0;
	AddrToHook ( AddrToHook_Pvoid);
	ThisHookMode ( HookMode);
	ParamOfHook= ParamOfHook_Pvoid;

	ZeroMemory ( Inline_5Bytes, sizeof(Inline_5Bytes)* sizeof(BYTE));

/*
	if (! IsBadWritePtr ( , 1))
	{//��ô�ж�m_OrgBytes_Pbyte�ǲ�����Ч��ָ���أ�
		delete [] m_OrgBytes_Pbyte;
	}
*/
	m_OrgBytes_Pbyte= NULL;

	if (NULL==AddrToHook ())
	{
		return ;
	}

	switch (HookMode)
	{
	case INLINE_UNPROTECTEVINMENT:
		//
		SingleHook::InitHookClass ( AddrToHook_Pvoid, Len_Dw, HookMode);
		break;
	case INLINE_5BYTESNOREDIECTCALL:
	case INLINE_5BYTESLAGGERCALL:
		if (NULL==m_NewBytes_Pbyte)
		{//��û��ָ��NewBytes_Pvoid,���Լ���ʼ��һ����
			Inline_5Bytes[0]= 0xe8;
			m_NewBytes_Pbyte= Inline_5Bytes;
		}
		
	case INLINE_5BYTESNOREDIECTJMP:
	case INLINE_5BYTESLAGGERJMP:

		if (5>Len_Dw)
		{//JMP��CALL��ʽ�Ĳ�����5�ֽ�����ʵ�֡�
			HookMode= ERRORMODE;
		}	

		if (NULL==m_NewBytes_Pbyte)
		{//��û��ָ��NewBytes_Pvoid,���Լ���ʼ��һ����
			Inline_5Bytes[0]= 0xe9;
			m_NewBytes_Pbyte= Inline_5Bytes;
		}
		
		//����ǳ�ʼ��HOOK��wapper m_HookRouter_Pproc
		DWORD HookRouterLen;
		HookRouterLen= X86INLINEROUTERENDOFF+ Len_Dw* 0x4+ 0x4+ 0x5;
		m_HookRouter_Pproc= new BYTE[HookRouterLen];//X86InlineHookRouter+ orgcode+ jmp back, orgcode����ȫ��Ҫ������4����С��jecxz
		LPBYTE CurrentPtrIn_m_HookRouter_Pproc;
		DWORD tempForProtect_Dw;
		VirtualProtect ( m_HookRouter_Pproc, HookRouterLen, PAGE_EXECUTE_READWRITE, &tempForProtect_Dw);
		
#ifdef DEBUG
		//VC��debugģʽ�����õĺ�������0xe9 ?? ?? ?? ??�����ĵ�ַ��
		LPBYTE tempCopyFrom_Pproc;
		tempCopyFrom_Pproc= *((LPBYTE *)((DWORD)&X86InlineHookRouter+ 1));
		tempCopyFrom_Pproc= (DWORD)&X86InlineHookRouter+ tempCopyFrom_Pproc+ 5;
		memcpy ( m_HookRouter_Pproc, tempCopyFrom_Pproc, X86INLINEROUTERENDOFF);
#else
		memcpy ( m_HookRouter_Pproc, X86InlineHookRouter, X86INLINEROUTERENDOFF);
#endif

		CurrentPtrIn_m_HookRouter_Pproc= &m_HookRouter_Pproc[X86INLINEROUTERSTACKBUFFEROFF]; 
		PInlineX86StackBuffer tempPtrToX86StackBufForRouter;
		tempPtrToX86StackBufForRouter= new InlineX86StackBuffer;
		ZeroMemory ( tempPtrToX86StackBufForRouter, sizeof(InlineX86StackBuffer));
		tempPtrToX86StackBufForRouter->rtnAddr_Pvoid= RtnAddrOfHook ();
		tempPtrToX86StackBufForRouter->myInlineHookClass_Pish= this;
		*((PInlineX86StackBuffer *)CurrentPtrIn_m_HookRouter_Pproc)= tempPtrToX86StackBufForRouter;//���ñ����ջ�Ļ����ַ��
		
		if (INLINE_5BYTESLAGGERJMP==HookMode)
		{//��JMP==hookmodeʱ���ѻָ���ջ+4�Ĵ���nop����
			CurrentPtrIn_m_HookRouter_Pproc= &m_HookRouter_Pproc[X86INLINEROUTERSTRACKADD4];
			*((DWORD *)CurrentPtrIn_m_HookRouter_Pproc)= 0x90909090u;//����Ϊ4�ֽ�(lea esp, DWORD PTR[esp+ 4])��nop nop nop nop
// 
// 			CurrentPtrIn_m_HookRouter_Pproc= &m_HookRouter_Pproc[X86INLINEROUTERSTRACKSUB4];
// 			*((BYTE *)CurrentPtrIn_m_HookRouter_Pproc)= 0x90u;//����Ϊ4�ֽ�(lea esp, DWORD PTR[esp+ 4])��nop nop nop nop
		}

		CurrentPtrIn_m_HookRouter_Pproc= &m_HookRouter_Pproc[X86INLINEROUTERCALLADDROFF];
		*((DWORD *)CurrentPtrIn_m_HookRouter_Pproc)= (DWORD) ((LPBYTE)RouterAddr- &m_HookRouter_Pproc[X86INLINEROUTERCALLADDROFF]+ 1- 0x5);//����HOOK����ת����Ŀ�ꡣע��X86INLINEROUTERCALLADDROFF����ָ���0xe8������0xe8֮��1���ֽڣ���Ҫ+1


		CurrentPtrIn_m_HookRouter_Pproc= &m_HookRouter_Pproc[X86INLINEROUTERENDOFF];

		//Len_Dw= 0;

		if ((INLINE_5BYTESLAGGERCALL==HookMode)||
			INLINE_5BYTESLAGGERJMP==HookMode)
		{
			DWORD RedirectOpCodeLen;
			m_RedirectOrgOpcodes_Pbyte= NULL;
			RedirectOpCodeLen= X86RedirectOpcodeToNewBase ( CurrentPtrIn_m_HookRouter_Pproc, AddrToHook_Pvoid, &Len_Dw, &m_RedirectOrgOpcodes_Pbyte);

			m_RedirectOrgOpcodesPtrInStub_Pvoid= CurrentPtrIn_m_HookRouter_Pproc;
			memcpy ( CurrentPtrIn_m_HookRouter_Pproc, m_RedirectOrgOpcodes_Pbyte, RedirectOpCodeLen);
			CurrentPtrIn_m_HookRouter_Pproc= CurrentPtrIn_m_HookRouter_Pproc+ RedirectOpCodeLen;
		}

		//if (INLINE_5BYTESLAGGERCALL==HookMode||INLINE_5BYTESLAGGERJMP==HookMode)
		{//jmp�����������ģ���jmp��ȥ
		//���������Ͳ�������������֡�
			CurrentPtrIn_m_HookRouter_Pproc[0]= 0xe9;
			*((DWORD *)&CurrentPtrIn_m_HookRouter_Pproc [1])= (DWORD)((AddrToHook_Pvoid+ Len_Dw)- CurrentPtrIn_m_HookRouter_Pproc - 0x5);
		}
		
		*((DWORD *)(&Inline_5Bytes[1]))= (DWORD) (reinterpret_cast <LPBYTE>(m_HookRouter_Pproc)- AddrToHook_Pvoid- 0x5);
	default:
		if (0<Len_Dw)
		{
			m_OrgBytes_Pbyte= new BYTE[Len_Dw];
			memcpy ( m_OrgBytes_Pbyte, AddrToHook_Pvoid, Len_Dw);
		}
		m_LenToModify_Dw= Len_Dw;
		break;
	}
}

void InlineSingleHook::Hook (void)
{
	switch ( ThisHookMode ( ))
	{
	case INLINE_5BYTESNOREDIECTCALL:
	case INLINE_5BYTESNOREDIECTJMP:
	case INLINE_5BYTESLAGGERCALL:
	case INLINE_5BYTESLAGGERJMP:
		LPBYTE nopBts;
		nopBts= new BYTE[m_LenToModify_Dw];
		memset ( nopBts, 0x90, m_LenToModify_Dw);
		
		MemWriteWithBackup ( AddrToHook (), m_LenToModify_Dw, NULL, nopBts);
		MemWriteWithBackup ( AddrToHook (), 5, NULL, m_NewBytes_Pbyte);
		delete [] nopBts;
		break;
	case INLINE_UNPROTECTEVINMENT:
	default:
		SingleHook::Hook ( );
	}
}


const LPVOID InlineSingleHook::RtnAddrOfHook (void)
{
	return (const LPBYTE)AddrToHook ()+ m_LenToModify_Dw;
}

const LPBYTE InlineSingleHook::RedirectedOrgOpcodes (void)
{
	return m_RedirectOrgOpcodes_Pbyte;
}

const LPVOID InlineSingleHook::RedirectedOpcodeInStub (void)
{
	return m_RedirectOrgOpcodesPtrInStub_Pvoid;
}