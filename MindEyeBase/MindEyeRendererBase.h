#pragma once
#include "MindEyeCommon.h"
#include <crtdbg.h>

template <typename APOClass, typename ParameterClass>
class MindEyeRendererBase
	: public CXAPOParametersBase
{
private:
	ParameterClass m_parameters[3];

	WAVEFORMATEX m_wfx;

	static XAPO_REGISTRATION_PROPERTIES m_regProps;

protected:
	MindEyeRendererBase();
	~MindEyeRendererBase(void);

	const WAVEFORMATEX &WaveFormat() const { return m_wfx; }

	void OnSetParameters(_In_reads_bytes_(cbParams) const void *pParams, UINT32 cbParams) override
	{
		_ASSERT(cbParams == sizeof(ParameterClass));
		cbParams;
		OnSetParameters(*(ParameterClass *)pParams);
	}

	virtual void DoProcess(
		const ParameterClass &params,
		_Inout_updates_all_(cFrames *cChannels) FLOAT32 *__restrict pData,
		UINT32 cFrames,
		UINT32 cChannels) = 0;

	virtual void OnSetParameters(const ParameterClass &) {}

public:
	static HRESULT CreateInstance(void* pInitData, UINT32 cbInitData, APOClass** ppAPO);

	STDMETHOD(LockForProcess)
		(
			UINT32 InputLockedParameterCount,
			_In_reads_opt_(InputLockedParameterCount) const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pInputLockedParameters,
			UINT32 OutputLockedParameterCount,
			_In_reads_opt_(OutputLockedParameterCount) const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS* pOutputLockedParameters) override;

	STDMETHOD_(void, Process)
		(
			UINT32 InputProcessParameterCount,
			_In_reads_opt_(InputProcessParameterCount) const XAPO_PROCESS_BUFFER_PARAMETERS* pInputProcessParameters,
			UINT32 OutputProcessParameterCount,
			_Inout_updates_opt_(OutputProcessParameterCount) XAPO_PROCESS_BUFFER_PARAMETERS* pOutputProcessParameters,
			BOOL IsEnabled) override;
};

template <typename APOClass, typename ParameterClass>
__declspec(selectany) XAPO_REGISTRATION_PROPERTIES MindEyeRendererBase<APOClass, ParameterClass>::m_regProps = {
	__uuidof(APOClass),
	L"MindEyeRenderer",
	L"Copyright (C)2023 LaoQiongGui",
	1,
	0,
	XAPO_FLAG_INPLACE_REQUIRED | XAPO_FLAG_CHANNELS_MUST_MATCH | XAPO_FLAG_FRAMERATE_MUST_MATCH | XAPO_FLAG_BITSPERSAMPLE_MUST_MATCH | XAPO_FLAG_BUFFERCOUNT_MUST_MATCH | XAPO_FLAG_INPLACE_SUPPORTED,
	1, 1, 1, 1};

template <typename APOClass, typename ParameterClass>
HRESULT MindEyeRendererBase<APOClass, ParameterClass>::CreateInstance(void *pInitData, UINT32 cbInitData, APOClass **ppAPO)
{
	_ASSERT(ppAPO);
	HRESULT hr = S_OK;

	*ppAPO = new APOClass;
	if (*ppAPO != NULL)
	{
		hr = (*ppAPO)->Initialize(pInitData, cbInitData);
	}
	else
		hr = E_OUTOFMEMORY;

	return hr;
}

template <typename APOClass, typename ParameterClass>
MindEyeRendererBase<APOClass, ParameterClass>::MindEyeRendererBase()
	: CXAPOParametersBase(&m_regProps, (BYTE *)m_parameters, sizeof(ParameterClass), FALSE)
{
	ZeroMemory(m_parameters, sizeof(m_parameters));
}

template <typename APOClass, typename ParameterClass>
MindEyeRendererBase<APOClass, ParameterClass>::~MindEyeRendererBase()
{
}

template <typename APOClass, typename ParameterClass>
HRESULT MindEyeRendererBase<APOClass, ParameterClass>::LockForProcess(
	UINT32 InputLockedParameterCount,
	const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pInputLockedParameters,
	UINT32 OutputLockedParameterCount,
	const XAPO_LOCKFORPROCESS_BUFFER_PARAMETERS *pOutputLockedParameters)
{
	HRESULT hr = CXAPOParametersBase::LockForProcess(
		InputLockedParameterCount,
		pInputLockedParameters,
		OutputLockedParameterCount,
		pOutputLockedParameters);

	if (SUCCEEDED(hr))
	{
		if (!pInputLockedParameters)
			return E_POINTER;

		memcpy(&m_wfx, pInputLockedParameters[0].pFormat, sizeof(WAVEFORMATEX));
	}
	return hr;
}

template <typename APOClass, typename ParameterClass>
void MindEyeRendererBase<APOClass, ParameterClass>::Process(
	UINT32 InputProcessParameterCount,
	const XAPO_PROCESS_BUFFER_PARAMETERS *pInputProcessParameters,
	UINT32 OutputProcessParameterCount,
	XAPO_PROCESS_BUFFER_PARAMETERS *pOutputProcessParameters,
	BOOL IsEnabled)
{
	_ASSERT(IsLocked());
	_ASSERT(InputProcessParameterCount == 1);
	_ASSERT(OutputProcessParameterCount == 1);
	_ASSERT(pInputProcessParameters != nullptr && pOutputProcessParameters != nullptr);
	_Analysis_assume_(pInputProcessParameters != nullptr && pOutputProcessParameters != nullptr);
	_ASSERT(pInputProcessParameters[0].pBuffer == pOutputProcessParameters[0].pBuffer);

	UNREFERENCED_PARAMETER(OutputProcessParameterCount);
	UNREFERENCED_PARAMETER(InputProcessParameterCount);
	UNREFERENCED_PARAMETER(pOutputProcessParameters);
	UNREFERENCED_PARAMETER(IsEnabled);

	ParameterClass *pParams;
	pParams = (ParameterClass *)BeginProcess();
	if (pInputProcessParameters[0].BufferFlags == XAPO_BUFFER_SILENT)
	{
		memset(pInputProcessParameters[0].pBuffer, 0,
			   pInputProcessParameters[0].ValidFrameCount * m_wfx.nChannels * sizeof(FLOAT32));

		DoProcess(
			*pParams,
			(FLOAT32 *__restrict)pInputProcessParameters[0].pBuffer,
			pInputProcessParameters[0].ValidFrameCount,
			m_wfx.nChannels);
	}
	else if (pInputProcessParameters[0].BufferFlags == XAPO_BUFFER_VALID)
	{
		DoProcess(
			*pParams,
			(FLOAT32 *__restrict)pInputProcessParameters[0].pBuffer,
			pInputProcessParameters[0].ValidFrameCount,
			m_wfx.nChannels);
	}
	EndProcess();
}
