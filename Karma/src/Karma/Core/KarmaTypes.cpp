#include "KarmaTypes.h"

// Variables.
FUrlConfig FURL::UrlConfig;
bool FURL::m_bDefaultsInitialized = false;

void FUrlConfig::Init()
{
	m_DefaultProtocol = "";
	m_DefaultName = "";
	m_DefaultHost = "";
	m_DefaultPortal = "";
	m_DefaultSaveExt = "";

	m_DefaultPort = 80;
}

void FUrlConfig::Reset()
{
	m_DefaultProtocol = "";
	m_DefaultName = "";
	m_DefaultHost = "";
	m_DefaultPortal = "";
	m_DefaultSaveExt = "";

	m_DefaultPort = 80;
}

FURL::FURL(const std::string& LocalFilename)
	: m_Protocol(UrlConfig.m_DefaultProtocol)
	, m_Host(UrlConfig.m_DefaultHost)
	, m_Port(UrlConfig.m_DefaultPort)
	, m_Valid(1)
	, m_Portal(UrlConfig.m_DefaultPortal)
{
	// strip off any extension from map name
	if (LocalFilename != "")
	{
		m_Map = LocalFilename;
		/*
		if (FPackageName::IsValidLongPackageName(LocalFilename))
		{
			m_Map = LocalFilename;
		}
		else
		{
			m_Map = FPaths::GetBaseFilename(LocalFilename);
		}*/
	}
	else
	{
		//m_Map = UGameMapsSettings::GetGameDefaultMap();
		m_Map = "NoName";
	}
}
