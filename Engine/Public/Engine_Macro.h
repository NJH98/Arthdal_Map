#pragma once

#define D3DCOLOR_ARGB(a,r,g,b) \
    ((D3DCOLOR)((((a)&0xff)<<24)|(((b)&0xff)<<16)|(((g)&0xff)<<8)|((r)&0xff)))

#define BEGIN(SpaceName) namespace SpaceName {
#define END }

#ifdef ENGINE_EXPORTS
#define ENGINE_DLL _declspec(dllexport)
#else
#define ENGINE_DLL _declspec(dllimport)
#endif

#define CONFIRM_BOX(message) (MessageBox(nullptr, message, L"Confirm", MB_YESNO | MB_ICONQUESTION) == IDYES)

#define MSG_BOX(message)								\
::MessageBox(nullptr, message, L"error", MB_OK)

#define NO_COPY(ClassName)								\
ClassName(const ClassName&) = delete;					\
ClassName& operator=(const ClassName&) = delete;

#define DECLARE_SINGLETON(ClassName)					\
		NO_COPY(ClassName)								\
public :												\
	static ClassName* Get_Instance();					\
	static unsigned int Destroy_Instance();				\
private:												\
	static ClassName* m_pInstance;

#define IMPLEMENT_SINGLETON(ClassName)					\
ClassName* ClassName::m_pInstance = nullptr;			\
ClassName* ClassName::Get_Instance()					\
{														\
	if (nullptr == m_pInstance)							\
		m_pInstance = new ClassName;					\
	return m_pInstance;									\
}														\
unsigned int  ClassName::Destroy_Instance()				\
{														\
	unsigned int	iRefCnt = { 0 };					\
														\
	if (nullptr != m_pInstance)							\
	{													\
		iRefCnt = m_pInstance->Release();				\
		m_pInstance = nullptr;							\
	}													\
	return iRefCnt;										\
}

