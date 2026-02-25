/**
 * @file SkfLibrary.h
 * @brief SKF 动态库加载器
 *
 * 封装 SKF 供应商库的动态加载，提供类型安全的函数指针访问
 */

#pragma once

#include <QLibrary>
#include <QString>

#include "SkfApi.h"

namespace wekey {

/**
 * @brief SKF 动态库加载器
 *
 * 负责加载 SKF 供应商提供的动态库（.dll/.so/.dylib），
 * 并解析所有 34 个 SKF API 函数指针。
 *
 * 使用方式：
 * @code
 * SkfLibrary lib("/path/to/vendor/skf.dll");
 * if (lib.isLoaded()) {
 *     ULONG ret = lib.EnumDev(TRUE, buffer, &size);
 * }
 * @endcode
 */
class SkfLibrary {
public:
    /**
     * @brief 构造函数
     * @param path 动态库路径
     *
     * 自动加载库并解析所有函数符号
     */
    explicit SkfLibrary(const QString& path);

    /**
     * @brief 析构函数
     *
     * QLibrary 会自动卸载库
     */
    ~SkfLibrary();

    // 禁止拷贝和移动
    SkfLibrary(const SkfLibrary&) = delete;
    SkfLibrary& operator=(const SkfLibrary&) = delete;
    SkfLibrary(SkfLibrary&&) = delete;
    SkfLibrary& operator=(SkfLibrary&&) = delete;

    /**
     * @brief 检查库是否成功加载
     * @return true=已加载，false=加载失败
     */
    [[nodiscard]] bool isLoaded() const;

    /**
     * @brief 获取加载错误信息
     * @return 错误描述字符串
     */
    [[nodiscard]] QString errorString() const;

    //=== 设备管理函数指针 (8 个) ===

    skf::PFN_SKF_EnumDev EnumDev = nullptr;
    skf::PFN_SKF_ConnectDev ConnectDev = nullptr;
    skf::PFN_SKF_DisConnectDev DisConnectDev = nullptr;
    skf::PFN_SKF_GetDevInfo GetDevInfo = nullptr;
    skf::PFN_SKF_SetLabel SetLabel = nullptr;
    skf::PFN_SKF_DevAuth DevAuth = nullptr;
    skf::PFN_SKF_ChangeDevAuthKey ChangeDevAuthKey = nullptr;
    skf::PFN_SKF_WaitForDevEvent WaitForDevEvent = nullptr;

    //=== 应用管理函数指针 (8 个) ===

    skf::PFN_SKF_EnumApplication EnumApplication = nullptr;
    skf::PFN_SKF_CreateApplication CreateApplication = nullptr;
    skf::PFN_SKF_DeleteApplication DeleteApplication = nullptr;
    skf::PFN_SKF_OpenApplication OpenApplication = nullptr;
    skf::PFN_SKF_CloseApplication CloseApplication = nullptr;
    skf::PFN_SKF_VerifyPIN VerifyPIN = nullptr;
    skf::PFN_SKF_ChangePIN ChangePIN = nullptr;
    skf::PFN_SKF_UnblockPIN UnblockPIN = nullptr;

    //=== 容器管理函数指针 (6 个) ===

    skf::PFN_SKF_EnumContainer EnumContainer = nullptr;
    skf::PFN_SKF_CreateContainer CreateContainer = nullptr;
    skf::PFN_SKF_DeleteContainer DeleteContainer = nullptr;
    skf::PFN_SKF_OpenContainer OpenContainer = nullptr;
    skf::PFN_SKF_CloseContainer CloseContainer = nullptr;
    skf::PFN_SKF_GetContainerType GetContainerType = nullptr;

    //=== 密钥操作函数指针 (6 个) ===

    skf::PFN_SKF_ExportPublicKey ExportPublicKey = nullptr;
    skf::PFN_SKF_GenECCKeyPair GenECCKeyPair = nullptr;
    skf::PFN_SKF_ImportECCKeyPair ImportECCKeyPair = nullptr;
    skf::PFN_SKF_ImportRSAKeyPair ImportRSAKeyPair = nullptr;
    skf::PFN_SKF_GenRSAKeyPair GenRSAKeyPair = nullptr;
    skf::PFN_SKF_GenRandom GenRandom = nullptr;

    //=== 对称加密函数指针 (3 个) ===

    skf::PFN_SKF_SetSymmKey SetSymmKey = nullptr;
    skf::PFN_SKF_EncryptInit EncryptInit = nullptr;
    skf::PFN_SKF_Encrypt Encrypt = nullptr;

    //=== 证书操作函数指针 (2 个) ===

    skf::PFN_SKF_ImportCertificate ImportCertificate = nullptr;
    skf::PFN_SKF_ExportCertificate ExportCertificate = nullptr;

    //=== 哈希函数指针 (4 个) ===

    skf::PFN_SKF_DigestInit DigestInit = nullptr;
    skf::PFN_SKF_Digest Digest = nullptr;
    skf::PFN_SKF_DigestUpdate DigestUpdate = nullptr;
    skf::PFN_SKF_DigestFinal DigestFinal = nullptr;

    //=== 签名验签函数指针 (4 个) ===

    skf::PFN_SKF_ECCSignData ECCSignData = nullptr;
    skf::PFN_SKF_ECCVerify ECCVerify = nullptr;
    skf::PFN_SKF_RSASignData RSASignData = nullptr;
    skf::PFN_SKF_RSAVerify RSAVerify = nullptr;

    //=== 文件操作函数指针 (5 个) ===

    skf::PFN_SKF_CreateFile CreateFile = nullptr;
    skf::PFN_SKF_DeleteFile DeleteFile = nullptr;
    skf::PFN_SKF_EnumFiles EnumFiles = nullptr;
    skf::PFN_SKF_ReadFile ReadFile = nullptr;
    skf::PFN_SKF_WriteFile WriteFile = nullptr;

private:
    /**
     * @brief 加载所有函数符号
     *
     * 从动态库中解析所有 34 个 SKF API 函数
     */
    void loadSymbols();

    /**
     * @brief 加载单个函数符号（模板辅助函数）
     * @tparam T 函数指针类型
     * @param name 函数名称
     * @return 函数指针（失败时返回 nullptr）
     */
    template <typename T>
    T loadSymbol(const char* name);

    QLibrary lib_;  ///< Qt 动态库加载器
};

}  // namespace wekey
