#pragma once

struct TaechangAppSettings
{
    CString m_strPdfToTextPath;
};

class TaechangAppSettingsService
{
public:
    CString GetSettingsPath() const;
    BOOL Load(TaechangAppSettings& outSettings) const;
    CString GetDefaultPdfToTextPath() const;
};
