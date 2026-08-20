#pragma once

struct SageAppSettings
{
    CString m_strPdfToTextPath;
};

class SageAppSettingsService
{
public:
    CString GetSettingsPath() const;
    BOOL Load(SageAppSettings& outSettings) const;
    CString GetDefaultPdfToTextPath() const;
};
