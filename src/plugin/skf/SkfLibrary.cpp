/**
 * @file SkfLibrary.cpp
 * @brief SKF 动态库加载器实现
 */

#include "SkfLibrary.h"

namespace wekey {

SkfLibrary::SkfLibrary(const QString& path) : lib_(path) {
    // 尝试加载库
    if (!lib_.load()) {
        return;
    }

    // 加载所有函数符号
    loadSymbols();
}

SkfLibrary::~SkfLibrary() {
    // QLibrary 会自动卸载
}

bool SkfLibrary::isLoaded() const {
    return lib_.isLoaded();
}

QString SkfLibrary::errorString() const {
    return lib_.errorString();
}

void SkfLibrary::loadSymbols() {
    // 设备管理函数 (8 个)
    EnumDev = loadSymbol<skf::PFN_SKF_EnumDev>("SKF_EnumDev");
    ConnectDev = loadSymbol<skf::PFN_SKF_ConnectDev>("SKF_ConnectDev");
    DisConnectDev = loadSymbol<skf::PFN_SKF_DisConnectDev>("SKF_DisConnectDev");
    GetDevInfo = loadSymbol<skf::PFN_SKF_GetDevInfo>("SKF_GetDevInfo");
    SetLabel = loadSymbol<skf::PFN_SKF_SetLabel>("SKF_SetLabel");
    DevAuth = loadSymbol<skf::PFN_SKF_DevAuth>("SKF_DevAuth");
    ChangeDevAuthKey = loadSymbol<skf::PFN_SKF_ChangeDevAuthKey>("SKF_ChangeDevAuthKey");
    WaitForDevEvent = loadSymbol<skf::PFN_SKF_WaitForDevEvent>("SKF_WaitForDevEvent");

    // 应用管理函数 (8 个)
    EnumApplication = loadSymbol<skf::PFN_SKF_EnumApplication>("SKF_EnumApplication");
    CreateApplication = loadSymbol<skf::PFN_SKF_CreateApplication>("SKF_CreateApplication");
    DeleteApplication = loadSymbol<skf::PFN_SKF_DeleteApplication>("SKF_DeleteApplication");
    OpenApplication = loadSymbol<skf::PFN_SKF_OpenApplication>("SKF_OpenApplication");
    CloseApplication = loadSymbol<skf::PFN_SKF_CloseApplication>("SKF_CloseApplication");
    VerifyPIN = loadSymbol<skf::PFN_SKF_VerifyPIN>("SKF_VerifyPIN");
    ChangePIN = loadSymbol<skf::PFN_SKF_ChangePIN>("SKF_ChangePIN");
    UnblockPIN = loadSymbol<skf::PFN_SKF_UnblockPIN>("SKF_UnblockPIN");

    // 容器管理函数 (6 个)
    EnumContainer = loadSymbol<skf::PFN_SKF_EnumContainer>("SKF_EnumContainer");
    CreateContainer = loadSymbol<skf::PFN_SKF_CreateContainer>("SKF_CreateContainer");
    DeleteContainer = loadSymbol<skf::PFN_SKF_DeleteContainer>("SKF_DeleteContainer");
    OpenContainer = loadSymbol<skf::PFN_SKF_OpenContainer>("SKF_OpenContainer");
    CloseContainer = loadSymbol<skf::PFN_SKF_CloseContainer>("SKF_CloseContainer");
    GetContainerType = loadSymbol<skf::PFN_SKF_GetContainerType>("SKF_GetContainerType");

    // 密钥操作函数 (6 个)
    ExportPublicKey = loadSymbol<skf::PFN_SKF_ExportPublicKey>("SKF_ExportPublicKey");
    GenECCKeyPair = loadSymbol<skf::PFN_SKF_GenECCKeyPair>("SKF_GenECCKeyPair");
    ImportECCKeyPair = loadSymbol<skf::PFN_SKF_ImportECCKeyPair>("SKF_ImportECCKeyPair");
    ImportRSAKeyPair = loadSymbol<skf::PFN_SKF_ImportRSAKeyPair>("SKF_ImportRSAKeyPair");
    GenRSAKeyPair = loadSymbol<skf::PFN_SKF_GenRSAKeyPair>("SKF_GenRSAKeyPair");
    GenRandom = loadSymbol<skf::PFN_SKF_GenRandom>("SKF_GenRandom");

    // 对称加密函数 (3 个)
    SetSymmKey = loadSymbol<skf::PFN_SKF_SetSymmKey>("SKF_SetSymmKey");
    EncryptInit = loadSymbol<skf::PFN_SKF_EncryptInit>("SKF_EncryptInit");
    Encrypt = loadSymbol<skf::PFN_SKF_Encrypt>("SKF_Encrypt");

    // 证书操作函数 (2 个)
    ImportCertificate = loadSymbol<skf::PFN_SKF_ImportCertificate>("SKF_ImportCertificate");
    ExportCertificate = loadSymbol<skf::PFN_SKF_ExportCertificate>("SKF_ExportCertificate");

    // 哈希函数 (4 个)
    DigestInit = loadSymbol<skf::PFN_SKF_DigestInit>("SKF_DigestInit");
    Digest = loadSymbol<skf::PFN_SKF_Digest>("SKF_Digest");
    DigestUpdate = loadSymbol<skf::PFN_SKF_DigestUpdate>("SKF_DigestUpdate");
    DigestFinal = loadSymbol<skf::PFN_SKF_DigestFinal>("SKF_DigestFinal");

    // 签名验签函数 (4 个)
    ECCSignData = loadSymbol<skf::PFN_SKF_ECCSignData>("SKF_ECCSignData");
    ECCVerify = loadSymbol<skf::PFN_SKF_ECCVerify>("SKF_ECCVerify");
    RSASignData = loadSymbol<skf::PFN_SKF_RSASignData>("SKF_RSASignData");
    RSAVerify = loadSymbol<skf::PFN_SKF_RSAVerify>("SKF_RSAVerify");

    // 文件操作函数 (5 个)
    CreateFile = loadSymbol<skf::PFN_SKF_CreateFile>("SKF_CreateFile");
    DeleteFile = loadSymbol<skf::PFN_SKF_DeleteFile>("SKF_DeleteFile");
    EnumFiles = loadSymbol<skf::PFN_SKF_EnumFiles>("SKF_EnumFiles");
    ReadFile = loadSymbol<skf::PFN_SKF_ReadFile>("SKF_ReadFile");
    WriteFile = loadSymbol<skf::PFN_SKF_WriteFile>("SKF_WriteFile");
}

template <typename T>
T SkfLibrary::loadSymbol(const char* name) {
    QFunctionPointer ptr = lib_.resolve(name);
    return reinterpret_cast<T>(ptr);
}

}  // namespace wekey
