WaveBuffer.cpp
#include "stdafx.h"
#include "WaveBuffer.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CWaveBuffer::CWaveBuffer(HWAVEIN hwi)
{
	m_buffer=new CHAR[BUFSIZE];
	m_waveh.lpData = m_buffer;
	m_waveh.dwBufferLength = BUFSIZE;
	m_waveh.dwBytesRecorded = 0;
	m_waveh.dwUser = 0;
	m_waveh.dwFlags = 0;
	m_waveh.dwLoops = 1;
	m_waveh.lpNext = NULL;
	m_waveh.reserved = 0;
	
	m_iotype = WAVEIN;
	m_hwio = (HWAVEIN)hwi;
}

CWaveBuffer::CWaveBuffer(HWAVEOUT hwo)
{
	m_buffer = new CHAR[BUFSIZE];
	
	m_waveh.lpData = m_buffer;
	m_waveh.dwBufferLength = BUFSIZE;
	m_waveh.dwBytesRecorded = 0;
	m_waveh.dwUser = 0;
	m_waveh.dwFlags = 0;
	m_waveh.dwLoops = 0;
	m_waveh.lpNext = NULL;
	m_waveh.reserved = 0;
	
	m_iotype = WAVEOUT;
	m_hwio = (HWAVEIN)hwo;
}

CWaveBuffer::CWaveBuffer(HWAVEOUT hwo, LPSTR p_buf, DWORD dwNum)
{	
	m_buffer = new CHAR[dwNum];
	CopyMemory(m_buffer, p_buf, dwNum);
	
	m_waveh.lpData = m_buffer;
	m_waveh.dwBufferLength = dwNum;
	m_waveh.dwBytesRecorded = 0;
	m_waveh.dwUser = 0;
	m_waveh.dwFlags = 0;
	m_waveh.dwLoops = 0;
	m_waveh.lpNext = NULL;
	m_waveh.reserved = 0;
	
	m_iotype = WAVEOUT;
	m_hwio = (HWAVEIN)hwo;
}

CWaveBuffer::~CWaveBuffer()
{	
	if (m_iotype == WAVEIN)
	{
		waveInUnprepareHeader((HWAVEIN)m_hwio, &m_waveh, sizeof(WAVEHDR));
	}
	else
	{
		waveOutUnprepareHeader((HWAVEOUT)m_hwio, &m_waveh, sizeof(WAVEHDR));
	}
	
	if (m_buffer == NULL)
		return;

	delete []m_buffer;
	m_buffer = NULL;
}

MMRESULT CWaveBuffer::Prepare()
{
	if (m_iotype == WAVEIN) 
	{
		return waveInPrepareHeader((HWAVEIN)m_hwio, &m_waveh, sizeof(WAVEHDR));
	}
	else
	{
		return waveOutPrepareHeader((HWAVEOUT)m_hwio, &m_waveh, sizeof(WAVEHDR));
	}
}

MMRESULT CWaveBuffer::AddToWaveInBuffer()
{
	if (m_iotype == WAVEOUT)
		return 0;

	return waveInAddBuffer((HWAVEIN)m_hwio, &m_waveh, sizeof(WAVEHDR));
}

void CWaveBuffer::ResetWaveInBuffer()
{
	if (m_iotype == WAVEOUT)
		return;
	
	m_waveh.lpData = m_buffer;
	m_waveh.dwBufferLength = BUFSIZE;
	m_waveh.dwBytesRecorded = 0;
	m_waveh.dwUser = 0;
	m_waveh.dwFlags = 0;
	m_waveh.dwLoops = 1;
	m_waveh.lpNext = NULL;
	m_waveh.reserved = 0;
	
	m_iotype = WAVEIN;
}

void CWaveBuffer::ResetWaveOutBuffer()
{
	if (m_iotype == WAVEIN)
		return;

	m_waveh.lpData = m_buffer;
	m_waveh.dwBufferLength = BUFSIZE;
	m_waveh.dwBytesRecorded = 0;
	m_waveh.dwUser = 0;
	m_waveh.dwFlags = 0;
	m_waveh.dwLoops = 0;
	m_waveh.lpNext = NULL;
	m_waveh.reserved = 0;
	
	m_iotype = WAVEOUT;
}

void CWaveBuffer::UseWaveOutBuffer(LPSTR p_buf, DWORD dwNum)
{
	if (m_iotype == WAVEIN)
		return;

	CopyMemory(m_buffer, p_buf, dwNum);
	m_waveh.dwBufferLength = dwNum;
}

MMRESULT CWaveBuffer::Play()
{
	if (m_iotype == WAVEIN)
		return 0;

	return waveOutWrite((HWAVEOUT)m_hwio, &m_waveh, sizeof(WAVEHDR));
}

/////////////////////////////////////////////////// ////////////////////
//CWaveFormat
///////////////////////////////////////////////////////////////////////
CWaveFormat::CWaveFormat()
{
	m_wfx.wFormatTag = WAVE_FORMAT_PCM;
	m_wfx.nChannels = 1;
	m_wfx.nSamplesPerSec = 11025;
	m_wfx.nAvgBytesPerSec = 11025;
	m_wfx.nBlockAlign = 1;
	m_wfx.wBitsPerSample = 8;
	m_wfx.cbSize = 0;
}

CWaveFormat::~CWaveFormat()
{
}
