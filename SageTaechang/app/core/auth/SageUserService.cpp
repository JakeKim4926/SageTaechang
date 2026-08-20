#include "pch.h"
#include "app/core/auth/SageUserService.h"
#include "SageDefine.h"

#include <bcrypt.h>
#pragma comment(lib, "bcrypt.lib")

SageUserService::SageUserService(SageUserRepository* pRepository) {
    m_pRepository = pRepository;
}

SageUserService::~SageUserService() {}

BOOL SageUserService::Login(
    const CString& strLoginId,
    const CString& strPassword,
    SageUserDto& outDto,
    BOOL& bSuccess,
    CString& strError
) {
    SageUserDto dto;
    BOOL bFound;

    bSuccess = FALSE;
    outDto = SageUserDto();

    if (m_pRepository->SelectByLoginId(strLoginId, dto, bFound, strError) == FALSE)
        return FALSE;

    if (bFound == FALSE)
        return TRUE;

    CString strHash = HashPassword(strPassword);

    if (strHash.CompareNoCase(dto.strPwHash) != 0)
        return TRUE;

    bSuccess = TRUE;
    outDto = dto;

    return TRUE;
}

BOOL SageUserService::AddUser(
    const SageUserDto& dto,
    const CString& strPlainPassword,
    int& nNewUserId,
    CString& strError
) {
    BOOL bExists;

    if (ValidateLoginId(dto.strLoginId, strError) == FALSE)
        return FALSE;

    if (ValidatePassword(strPlainPassword, strError) == FALSE)
        return FALSE;

    if (m_pRepository->ExistsByLoginId(dto.strLoginId, bExists, strError) == FALSE)
        return FALSE;

    if (bExists == TRUE) {
        strError = _T("이미 존재하는 아이디입니다.");
        return FALSE;
    }

    SageUserDto insertDto = dto;
    insertDto.strPwHash = HashPassword(strPlainPassword);

    return m_pRepository->Insert(insertDto, nNewUserId, strError);
}

BOOL SageUserService::ChangePassword(
    int nUserId,
    const CString& strNewPassword,
    CString& strError
) {
    if (ValidatePassword(strNewPassword, strError) == FALSE)
        return FALSE;

    CString strHash = HashPassword(strNewPassword);

    return m_pRepository->UpdatePassword(nUserId, strHash, strError);
}

BOOL SageUserService::LoadAll(
    CArray<SageUserDto, SageUserDto&>& arrUsers,
    CString& strError
) {
    return m_pRepository->SelectAll(arrUsers, strError);
}

BOOL SageUserService::RemoveUser(int nUserId, CString& strError) {
    return m_pRepository->Delete(nUserId, strError);
}

CString SageUserService::HashPassword(const CString& strPassword) {
    CStringA strPasswordA(CT2A(strPassword.GetString(), CP_UTF8));

    BCRYPT_ALG_HANDLE hAlg = NULL;
    BCRYPT_HASH_HANDLE hHash = NULL;
    BYTE* pbHashObject = NULL;
    BYTE rgbHash[32];
    DWORD cbData = 0;
    DWORD cbHashObject = 0;
    DWORD cbHash = 0;
    CString strResult;

    if (BCryptOpenAlgorithmProvider(&hAlg, BCRYPT_SHA256_ALGORITHM, NULL, 0) != 0)
        return strResult;

    BCryptGetProperty(hAlg, BCRYPT_OBJECT_LENGTH, (PBYTE)&cbHashObject, sizeof(DWORD), &cbData, 0);
    pbHashObject = new BYTE[cbHashObject]();

    BCryptGetProperty(hAlg, BCRYPT_HASH_LENGTH, (PBYTE)&cbHash, sizeof(DWORD), &cbData, 0);

    if (BCryptCreateHash(hAlg, &hHash, pbHashObject, cbHashObject, NULL, 0, 0) != 0) {
        delete[] pbHashObject;
        BCryptCloseAlgorithmProvider(hAlg, 0);
        return strResult;
    }

    BCryptHashData(hHash, (PBYTE)strPasswordA.GetString(), (ULONG)strPasswordA.GetLength(), 0);
    BCryptFinishHash(hHash, rgbHash, cbHash, 0);

    BCryptDestroyHash(hHash);
    BCryptCloseAlgorithmProvider(hAlg, 0);
    delete[] pbHashObject;

    for (DWORD i = 0; i < cbHash; i++) {
        CString strByte;
        strByte.Format(_T("%02x"), rgbHash[i]);
        strResult += strByte;
    }

    return strResult;
}

BOOL SageUserService::ValidateLoginId(const CString& strLoginId, CString& strError) {
    if (strLoginId.IsEmpty()) {
        strError = _T("아이디를 입력하세요.");
        return FALSE;
    }

    if (strLoginId.GetLength() < SAGE_USER_LOGIN_ID_MIN_LEN) {
        strError.Format(_T("아이디는 %d자 이상이어야 합니다."), SAGE_USER_LOGIN_ID_MIN_LEN);
        return FALSE;
    }

    if (strLoginId.GetLength() > SAGE_USER_LOGIN_ID_MAX_LEN) {
        strError.Format(_T("아이디는 %d자 이하이어야 합니다."), SAGE_USER_LOGIN_ID_MAX_LEN);
        return FALSE;
    }

    return TRUE;
}

BOOL SageUserService::ValidatePassword(const CString& strPassword, CString& strError) {
    if (strPassword.IsEmpty()) {
        strError = _T("비밀번호를 입력하세요.");
        return FALSE;
    }

    if (strPassword.GetLength() < SAGE_USER_PW_MIN_LEN) {
        strError = SAGE_UI_CHANGE_PW_TOO_SHORT;
        return FALSE;
    }

    if (strPassword.GetLength() > SAGE_USER_PW_MAX_LEN) {
        strError = SAGE_UI_CHANGE_PW_TOO_LONG;
        return FALSE;
    }

    for (int i = 0; i < strPassword.GetLength(); ++i) {
        TCHAR ch = strPassword[i];
        BOOL bAlpha = (ch >= _T('A') && ch <= _T('Z')) || (ch >= _T('a') && ch <= _T('z'));
        BOOL bDigit = (ch >= _T('0') && ch <= _T('9'));
        if (!bAlpha && !bDigit) {
            strError = SAGE_UI_CHANGE_PW_INVALID_CHAR;
            return FALSE;
        }
    }

    return TRUE;
}
