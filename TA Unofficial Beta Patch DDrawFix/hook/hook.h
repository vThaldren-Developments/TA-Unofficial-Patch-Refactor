#ifndef HOOK_H74DJN6554FD
#define HOOK_H74DJN6554FD

	class SingleHook
	{//�������Hook��elmt��������ʵ�־����һ��HOOK��
	public:
		LPVOID m_AddrToHook;
		int m_HookMode_I;
		LPBYTE m_NewBytes_Pbyte;
		LPBYTE m_HookRouter_Pproc;
		LPBYTE m_OrgBytes_Pbyte;
		DWORD m_LenToModify_Dw;
		LPBYTE mallocedBuf;

	public:
		SingleHook ();
		SingleHook (LPBYTE AddrToHook_Pvoid, DWORD Len_Dw, int HookMode= 0, LPBYTE NewBytes_Pbyte= NULL);//
		SingleHook (unsigned int AddrToHook_Pvoid, DWORD Len_Dw, int HookMode= 0, LPBYTE NewBytes_Pbyte= NULL);

		~SingleHook();

		LPVOID AddrToRtn (void);

		virtual void Hook (void);
		virtual void UnHook (void);

		virtual LPVOID AddrToHook (LPVOID Addr_Pvoid);
		virtual int ThisHookMode (int HookMode);
		virtual const int ThisHookMode (void);
		virtual const LPVOID AddrToHook (void);
		virtual DWORD LenToHook (DWORD Len_D);
	//private:
		virtual void InitHookClass (LPBYTE AddrToHook_Pvoid, DWORD Len_Dw, int HookMode= 0, LPBYTE NewBytes_Pbyte= NULL);;
		
	};

	//INLINE_UNPROTECTEVINMENT
	//������������hookRouter��new byte��Ҫ�Լ�ʵ�֡�
	// INLINE_5BYTESNOREDIECTCALL (0x1004)
	// INLINE_5BYTESNOREDIECTJMP (0x1005)�ᱣ��������������Ѹ����˵Ĵ��븴�����Զ�ִ�С�
	//	#define INLINE_5BYTESCALL 0x1001
	//  #define INLINE_5BYTESJMP 0x1002
	//�����inlineHOOK�ı���������hookrouter������Ҫ�õĸ�ʽ!
	// 	HOOKROUTERBEGIN_NAKED(blablabalabla)
	// 		AfxMessageBox ( _T("�����HOOK�ĺ�����"));
	// 	HOOKROUTEREND(blablabalabla)
	//  �ں��������ջ�ͼĴ�����HOOK�ط���һģһ����ֻҪ���޸ĵʹ���ջ(esp+4֮��)��ֵ���Ͳ����ƻ���ջ�ͼĴ���������Ҳ�����޸���Щֵ��
	//  ����Ҫ��֤���hookRouterֻ��һ��inlineHookʹ�ã�������Ȼ�����̲߳�ͬ���ˡ�

	class InlineSingleHook;
	typedef struct tagInlineX86StackBuffer
	{
		LPVOID rtnAddr_Pvoid;
		DWORD EFlags_Dw;
		DWORD Edi;
		DWORD Esi;
		DWORD Ebp;
		DWORD Esp;
		DWORD Ebx;
		DWORD Edx;
		DWORD Ecx;
		DWORD Eax;
		DWORD RtnEsp;//���Ᵽ���һ��
		int EnteredFlag_I;
		DWORD TID_Dw;
		InlineSingleHook * myInlineHookClass_Pish;
		tagInlineX86StackBuffer * next;
	}InlineX86StackBuffer, * PInlineX86StackBuffer;
	PInlineX86StackBuffer __stdcall X86CurrentThreadStackBufferFixRtnAddr (PInlineX86StackBuffer * in_Pix86StackBuf);
	PInlineX86StackBuffer __stdcall X86CurrentThreadStackBuffer (PInlineX86StackBuffer * in_Pix86StackBuf);

#define INLINE_5BYTESLAGGERCALL (0x1001)
#define INLINE_5BYTESLAGGERJMP (0x1002)
#define INLINE_UNPROTECTEVINMENT (0x1003)

#define INLINE_5BYTESNOREDIECTCALL (0x1004)
#define INLINE_5BYTESNOREDIECTJMP (0x1005)

#define INLINE_MODIFYCODE (0x1006)
#define INLINE_SINGLEJMP (0x1007)

#define ERRORMODE 0x99987

#define X86STRACKBUFFERCHANGE 0x7798FFAA // HOOK ROUTER���޸��˷��ص�ַʱ����뷵��������޸��˼Ĵ�����ֵʱ���÷���������޸Ķ�ջ��ֵҲ���á������Լ������˷��ص�ַ����Ҫ�Լ�������Щ��HOOK����ָ���ˡ����Դ�InlineSingleHook::RedirectedOpcodeInStub���stub�з���λ�õĴ��롣



	typedef int  (__stdcall * InlineX86HookRouter) (PInlineX86StackBuffer X86StrackBuffer);

	class InlineSingleHook: public SingleHook
	{
	public:
		LPVOID ParamOfHook;
	private:
		LPBYTE m_RedirectOrgOpcodes_Pbyte;
		LPBYTE m_RedirectOrgOpcodesPtrInStub_Pvoid;
		
		BYTE Inline_5Bytes[0x10];

	public:
		LPBYTE NewBytesForInlineHook (LPBYTE NewBytes_Pvoid= NULL);
		InlineSingleHook ();
		InlineSingleHook (unsigned int AddrToHook_Pvoid, DWORD Len_Dw, int HookMode= 0, LPVOID RouterAddr= NULL);
		InlineSingleHook (unsigned int AddrToHook_Pvoid, DWORD Len_Dw, int HookMode, InlineX86HookRouter RouterAddr);
		~InlineSingleHook ();

		
		const LPVOID RtnAddrOfHook (void);
		const LPBYTE RedirectedOrgOpcodes (void);
		const LPVOID RedirectedOpcodeInStub (void);
		LPVOID SetParamOfHook (LPVOID ParamOfHook_Pvoid);

		virtual void Hook (void);
		
	private:
		void InitHookClass (const LPBYTE AddrToHook_Pvoid, DWORD Len_Dw, int HookMode= 0, LPVOID RouterAddr= NULL, LPVOID ParamOfHook_Pvoid= NULL);
	};

	

	class ModifyHook: public SingleHook
	{
		//public:

	private:
		
		LPBYTE m_RedirectOrgOpcodes_Pbyte;
		LPBYTE m_RedirectOrgOpcodesPtrInStub_Pvoid;
		DWORD m_OverWriteLen;
		DWORD m_OffToRedirect;
	public:
		ModifyHook ();

		ModifyHook (LPBYTE AddrToHook_Pvoid, int HookMode, DWORD MaxLen, LPBYTE OverWriteOpcode, DWORD OverWriteLen, DWORD OffToRedirect= 0);
		ModifyHook (unsigned int AddrToHook_Pvoid, int HookMode, DWORD MaxLen, LPBYTE OverWriteOpcode, DWORD OverWriteLen, DWORD OffToRedirect= 0);
		~ModifyHook ();

		virtual void Hook (void);

	private:
		void InitHookClass (LPBYTE AddrToHook_Pvoid, int HookMode, DWORD MaxLen, LPBYTE OverWriteOpcode, DWORD OverWriteLen, DWORD OffToRedirect= 0);
	};

#endif //HOOK_H74DJN6554FD