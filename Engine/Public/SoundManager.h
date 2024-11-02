#pragma once
#include "Base.h"
#include "Fmod/fmod.h"
#include "Fmod/fmod.hpp"
#include <io.h>

BEGIN(Engine)

class CSoundManager :
	public CBase
{
private:
	CSoundManager();
	virtual ~CSoundManager();

public:
	void Initialize();
	void Update();			

	FMOD::System* Get_System() { return m_pSystem; }

public:
	enum SOUND_CHANNEL
	{
		CHANNEL_BGM,
		CHANNEL_PLAYER = 1,
		CHANNEL_EVENT = 10,
		CHANNEL_CHECKPOINT = 30,
		CHANNEL_END = 100
	};

public:
	HRESULT		PlayDefault(TCHAR* _TagSound, SOUND_CHANNEL _Channel, _float _fVloume, _bool _bLoop);
	HRESULT		PlayBGM(const TCHAR* _TagSound, _float _fVolume, _bool _bLoop);
	HRESULT		Play3D(TCHAR* _TagSound, _int _Channel, _float _fVolume, _float3 _vPos, _bool _bLoop = false);
	HRESULT		Play3DRemain(TCHAR* _TagSound, _float _fVolume, _float3 _vPos, _bool _bLoop = false);
	HRESULT		Play_NonStack(TCHAR* _TagSound, int _Channel, _float _fVolume, _bool _bLoop);
	HRESULT		Add_Sound(const char* _fullpath, const TCHAR* _filename, const TCHAR* _path);
	HRESULT		Load_Sound(const char* _folderName);
	void		StopSound(SOUND_CHANNEL _Channel);
	void		MuteAll();
	void		StopSoundALL();
	void		SetChannelVolume(SOUND_CHANNEL _Channel, _float _fVolume);
	HRESULT		Set_Listener(_float3 _vListenerPos, _float3 _v_vListenerForward);
	HRESULT		Set_SoundPos(_int _iChannel, _float3 _vPos);

private:
	void CheckChannels();

private:
	map<const TCHAR*, FMOD::Sound*>	m_mapSound; //저장공간
	FMOD::Channel* m_pChannel[100]{ nullptr };		//채널
	FMOD::System* m_pSystem;					//시스템
	vector<_int> m_availableChannels;

private:
	FMOD::Sound* Find_Sound(TCHAR* _TagSound);

public:
	static CSoundManager* Create();
	virtual void Free();
};

END