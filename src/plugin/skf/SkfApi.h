/**
 * @file SkfApi.h
 * @brief SKF API 函数指针定义
 *
 * 定义 SKF (智能密码钥匙) C API 的所有函数指针类型和错误码
 */

#pragma once

#include "SkfTypes.h"

extern "C" {

namespace wekey {
namespace skf {

//=== SKF 错误码定义 ===

constexpr ULONG SAR_OK = 0x00000000;                ///< 成功
constexpr ULONG SAR_FAIL = 0x0A000001;              ///< 失败
constexpr ULONG SAR_UNKNOWNERR = 0x0A000002;        ///< 未知错误
constexpr ULONG SAR_NOTSUPPORTYETERR = 0x0A000003;  ///< 不支持的功能
constexpr ULONG SAR_FILEERR = 0x0A000004;           ///< 文件错误
constexpr ULONG SAR_INVALIDHANDLEERR = 0x0A000005;  ///< 无效句柄
constexpr ULONG SAR_INVALIDPARAMERR = 0x0A000006;   ///< 无效参数
constexpr ULONG SAR_READFILEERR = 0x0A000007;       ///< 读文件错误
constexpr ULONG SAR_WRITEFILEERR = 0x0A000008;      ///< 写文件错误
constexpr ULONG SAR_NAMELENERR = 0x0A000009;        ///< 名称长度错误
constexpr ULONG SAR_KEYUSAGEERR = 0x0A00000A;       ///< 密钥用途错误
constexpr ULONG SAR_MODULUSLENERR = 0x0A00000B;     ///< 模长度错误
constexpr ULONG SAR_NOTINITIALIZEERR = 0x0A00000C;  ///< 未初始化
constexpr ULONG SAR_OBJERR = 0x0A00000D;            ///< 对象错误
constexpr ULONG SAR_MEMORYERR = 0x0A00000E;         ///< 内存错误
constexpr ULONG SAR_TIMEOUTERR = 0x0A00000F;        ///< 超时
constexpr ULONG SAR_INDATALENERR = 0x0A000010;      ///< 输入数据长度错误
constexpr ULONG SAR_INDATAERR = 0x0A000011;         ///< 输入数据错误
constexpr ULONG SAR_GENRANDERR = 0x0A000012;        ///< 生成随机数错误
constexpr ULONG SAR_HASHOBJERR = 0x0A000013;        ///< 哈希对象错误
constexpr ULONG SAR_HASHERR = 0x0A000014;           ///< 哈希错误
constexpr ULONG SAR_GENRSAKEYERR = 0x0A000015;      ///< 生成 RSA 密钥错误
constexpr ULONG SAR_RSAMODULUSLENERR = 0x0A000016;  ///< RSA 模长度错误
constexpr ULONG SAR_CSPIMPRTPUBKEYERR = 0x0A000017; ///< CSP 导入公钥错误
constexpr ULONG SAR_RSAENCERR = 0x0A000018;         ///< RSA 加密错误
constexpr ULONG SAR_RSADECERR = 0x0A000019;         ///< RSA 解密错误
constexpr ULONG SAR_HASHNOTEQUALERR = 0x0A00001A;   ///< 哈希不相等
constexpr ULONG SAR_KEYNOTFOUNTERR = 0x0A00001B;    ///< 密钥未找到
constexpr ULONG SAR_CERTNOTFOUNTERR = 0x0A00001C;   ///< 证书未找到
constexpr ULONG SAR_NOTEXPORTERR = 0x0A00001D;      ///< 不可导出
constexpr ULONG SAR_DECRYPTPADERR = 0x0A00001E;     ///< 解密填充错误
constexpr ULONG SAR_MACLENERR = 0x0A00001F;         ///< MAC 长度错误
constexpr ULONG SAR_BUFFER_TOO_SMALL = 0x0A000020;  ///< 缓冲区太小
constexpr ULONG SAR_KEYINFOTYPEERR = 0x0A000021;    ///< 密钥信息类型错误
constexpr ULONG SAR_NOT_EVENTERR = 0x0A000022;      ///< 无事件错误
constexpr ULONG SAR_DEVICE_REMOVED = 0x0A000023;    ///< 设备已移除
constexpr ULONG SAR_PIN_INCORRECT = 0x0A000024;     ///< PIN 码错误
constexpr ULONG SAR_PIN_LOCKED = 0x0A000025;        ///< PIN 码已锁定
constexpr ULONG SAR_PIN_INVALID = 0x0A000026;       ///< PIN 码无效
constexpr ULONG SAR_PIN_LEN_RANGE = 0x0A000027;     ///< PIN 码长度错误
constexpr ULONG SAR_USER_ALREADY_LOGGED_IN = 0x0A000028;   ///< 用户已登录
constexpr ULONG SAR_USER_PIN_NOT_INITIALIZED = 0x0A000029; ///< 用户 PIN 未初始化
constexpr ULONG SAR_USER_TYPE_INVALID = 0x0A00002A;        ///< 用户类型无效
constexpr ULONG SAR_APPLICATION_NAME_INVALID = 0x0A00002B; ///< 应用名称无效
constexpr ULONG SAR_APPLICATION_EXISTS = 0x0A00002C;       ///< 应用已存在
constexpr ULONG SAR_USER_NOT_LOGGED_IN = 0x0A00002D;       ///< 用户未登录
constexpr ULONG SAR_APPLICATION_NOT_EXISTS = 0x0A00002E;   ///< 应用不存在
constexpr ULONG SAR_FILE_ALREADY_EXIST = 0x0A00002F;       ///< 文件已存在
constexpr ULONG SAR_NO_ROOM = 0x0A000030;                  ///< 空间不足
constexpr ULONG SAR_FILE_NOT_EXIST = 0x0A000031;           ///< 文件不存在
constexpr ULONG SAR_REACH_MAX_CONTAINER_COUNT = 0x0A000032; ///< 达到最大容器数

//=== 设备管理函数指针类型 ===

/**
 * @brief 枚举设备
 * @param bPresent TRUE=枚举当前存在的设备，FALSE=枚举所有设备
 * @param szNameList 设备名称列表（以 \0\0 结尾）
 * @param pulSize 输入：缓冲区大小；输出：实际大小
 */
using PFN_SKF_EnumDev = ULONG(SKF_API*)(BOOL bPresent, LPSTR szNameList, PULONG pulSize);

/**
 * @brief 连接设备
 * @param szName 设备名称
 * @param phDev 返回设备句柄
 */
using PFN_SKF_ConnectDev = ULONG(SKF_API*)(LPCSTR szName, DEVHANDLE* phDev);

/**
 * @brief 断开设备连接
 * @param hDev 设备句柄
 */
using PFN_SKF_DisConnectDev = ULONG(SKF_API*)(DEVHANDLE hDev);

/**
 * @brief 获取设备信息
 * @param hDev 设备句柄
 * @param pDevInfo 返回设备信息
 */
using PFN_SKF_GetDevInfo = ULONG(SKF_API*)(DEVHANDLE hDev, DEVINFO* pDevInfo);

/**
 * @brief 设置设备标签
 * @param hDev 设备句柄
 * @param szLabel 新标签
 */
using PFN_SKF_SetLabel = ULONG(SKF_API*)(DEVHANDLE hDev, LPCSTR szLabel);

/**
 * @brief 设备认证
 * @param hDev 设备句柄
 * @param pbAuthData 认证数据
 * @param ulLen 数据长度
 */
using PFN_SKF_DevAuth = ULONG(SKF_API*)(DEVHANDLE hDev, BYTE* pbAuthData, ULONG ulLen);

/**
 * @brief 修改设备认证密钥
 * @param hDev 设备句柄
 * @param pbAuthData 新认证数据
 * @param ulLen 数据长度
 */
using PFN_SKF_ChangeDevAuthKey = ULONG(SKF_API*)(DEVHANDLE hDev, BYTE* pbAuthData, ULONG ulLen);

/**
 * @brief 等待设备事件
 * @param szDevName 设备名称（可为空）
 * @param pulDevNameLen 设备名称长度
 * @param pulEvent 返回事件类型
 */
using PFN_SKF_WaitForDevEvent = ULONG(SKF_API*)(LPSTR szDevName, PULONG pulDevNameLen, PULONG pulEvent);

//=== 应用管理函数指针类型 ===

/**
 * @brief 枚举应用
 * @param hDev 设备句柄
 * @param szAppName 应用名称列表（以 \0\0 结尾）
 * @param pulSize 输入：缓冲区大小；输出：实际大小
 */
using PFN_SKF_EnumApplication = ULONG(SKF_API*)(DEVHANDLE hDev, LPSTR szAppName, PULONG pulSize);

/**
 * @brief 创建应用
 * @param hDev 设备句柄
 * @param szAppName 应用名称
 * @param szAdminPin 管理员 PIN
 * @param dwAdminPinRetryCount 管理员 PIN 重试次数
 * @param szUserPin 用户 PIN
 * @param dwUserPinRetryCount 用户 PIN 重试次数
 * @param dwCreateFileRights 创建文件权限
 * @param phApplication 返回应用句柄
 */
using PFN_SKF_CreateApplication = ULONG(SKF_API*)(DEVHANDLE hDev, LPCSTR szAppName, LPCSTR szAdminPin,
                                                   DWORD dwAdminPinRetryCount, LPCSTR szUserPin,
                                                   DWORD dwUserPinRetryCount, DWORD dwCreateFileRights,
                                                   HAPPLICATION* phApplication);

/**
 * @brief 删除应用
 * @param hDev 设备句柄
 * @param szAppName 应用名称
 */
using PFN_SKF_DeleteApplication = ULONG(SKF_API*)(DEVHANDLE hDev, LPCSTR szAppName);

/**
 * @brief 打开应用
 * @param hDev 设备句柄
 * @param szAppName 应用名称
 * @param phApplication 返回应用句柄
 */
using PFN_SKF_OpenApplication = ULONG(SKF_API*)(DEVHANDLE hDev, LPCSTR szAppName, HAPPLICATION* phApplication);

/**
 * @brief 关闭应用
 * @param hApplication 应用句柄
 */
using PFN_SKF_CloseApplication = ULONG(SKF_API*)(HAPPLICATION hApplication);

/**
 * @brief 验证 PIN
 * @param hApplication 应用句柄
 * @param ulPINType PIN 类型（0=管理员，1=用户）
 * @param szPIN PIN 码
 * @param pulRetryCount 返回剩余重试次数
 */
using PFN_SKF_VerifyPIN = ULONG(SKF_API*)(HAPPLICATION hApplication, ULONG ulPINType, LPCSTR szPIN,
                                          PULONG pulRetryCount);

/**
 * @brief 修改 PIN
 * @param hApplication 应用句柄
 * @param ulPINType PIN 类型
 * @param szOldPIN 旧 PIN
 * @param szNewPIN 新 PIN
 * @param pulRetryCount 返回剩余重试次数
 */
using PFN_SKF_ChangePIN = ULONG(SKF_API*)(HAPPLICATION hApplication, ULONG ulPINType, LPCSTR szOldPIN,
                                          LPCSTR szNewPIN, PULONG pulRetryCount);

/**
 * @brief 解锁 PIN
 * @param hApplication 应用句柄
 * @param szAdminPIN 管理员 PIN
 * @param szNewUserPIN 新用户 PIN
 * @param pulRetryCount 返回剩余重试次数
 */
using PFN_SKF_UnblockPIN = ULONG(SKF_API*)(HAPPLICATION hApplication, LPCSTR szAdminPIN, LPCSTR szNewUserPIN,
                                           PULONG pulRetryCount);

//=== 容器管理函数指针类型 ===

/**
 * @brief 枚举容器
 * @param hApplication 应用句柄
 * @param szContainerName 容器名称列表（以 \0\0 结尾）
 * @param pulSize 输入：缓冲区大小；输出：实际大小
 */
using PFN_SKF_EnumContainer = ULONG(SKF_API*)(HAPPLICATION hApplication, LPSTR szContainerName, PULONG pulSize);

/**
 * @brief 创建容器
 * @param hApplication 应用句柄
 * @param szContainerName 容器名称
 * @param phContainer 返回容器句柄
 */
using PFN_SKF_CreateContainer = ULONG(SKF_API*)(HAPPLICATION hApplication, LPCSTR szContainerName,
                                                HCONTAINER* phContainer);

/**
 * @brief 删除容器
 * @param hApplication 应用句柄
 * @param szContainerName 容器名称
 */
using PFN_SKF_DeleteContainer = ULONG(SKF_API*)(HAPPLICATION hApplication, LPCSTR szContainerName);

/**
 * @brief 打开容器
 * @param hApplication 应用句柄
 * @param szContainerName 容器名称
 * @param phContainer 返回容器句柄
 */
using PFN_SKF_OpenContainer = ULONG(SKF_API*)(HAPPLICATION hApplication, LPCSTR szContainerName,
                                              HCONTAINER* phContainer);

/**
 * @brief 关闭容器
 * @param hContainer 容器句柄
 */
using PFN_SKF_CloseContainer = ULONG(SKF_API*)(HCONTAINER hContainer);

/**
 * @brief 获取容器类型
 * @param hContainer 容器句柄
 * @param pulContainerType 返回容器类型
 */
using PFN_SKF_GetContainerType = ULONG(SKF_API*)(HCONTAINER hContainer, PULONG pulContainerType);

//=== 密钥操作函数指针类型 ===

/**
 * @brief 导出公钥
 * @param hContainer 容器句柄
 * @param bSignFlag TRUE=签名公钥，FALSE=加密公钥
 * @param pbBlob 公钥数据
 * @param pulBlobLen 公钥长度
 */
using PFN_SKF_ExportPublicKey = ULONG(SKF_API*)(HCONTAINER hContainer, BOOL bSignFlag, BYTE* pbBlob, PULONG pulBlobLen);

/**
 * @brief 生成 ECC 密钥对
 * @param hContainer 容器句柄
 * @param ulAlgId 算法标识
 * @param pBlob 返回公钥
 */
using PFN_SKF_GenECCKeyPair = ULONG(SKF_API*)(HCONTAINER hContainer, ULONG ulAlgId, ECCPUBLICKEYBLOB* pBlob);

/**
 * @brief 导入 ECC 加密密钥对（信封格式）
 * @param hContainer 容器句柄
 * @param pEnvelopedKeyBlob 加密密钥信封数据（GMT-0016 ENVELOPEDKEYBLOB）
 */
using PFN_SKF_ImportECCKeyPair = ULONG(SKF_API*)(HCONTAINER hContainer, ENVELOPEDKEYBLOB* pEnvelopedKeyBlob);

/**
 * @brief 导入 RSA 加密密钥对
 * @param hContainer 容器句柄
 * @param ulSymAlgId 对称算法标识
 * @param pbWrappedKey 被加密的对称密钥
 * @param ulWrappedKeyLen 对称密钥长度
 * @param pbEncryptedData 被加密的私钥数据
 * @param ulEncryptedDataLen 私钥数据长度
 */
using PFN_SKF_ImportRSAKeyPair = ULONG(SKF_API*)(HCONTAINER hContainer, ULONG ulSymAlgId,
                                                  BYTE* pbWrappedKey, ULONG ulWrappedKeyLen,
                                                  BYTE* pbEncryptedData, ULONG ulEncryptedDataLen);

/**
 * @brief 生成 RSA 密钥对
 * @param hContainer 容器句柄
 * @param ulBitsLen 密钥位长度
 * @param pBlob 返回公钥
 */
using PFN_SKF_GenRSAKeyPair = ULONG(SKF_API*)(HCONTAINER hContainer, ULONG ulBitsLen, RSAPUBLICKEYBLOB* pBlob);

/**
 * @brief 生成随机数
 * @param hDev 设备句柄
 * @param pbRandom 随机数缓冲区
 * @param ulRandomLen 随机数长度
 */
using PFN_SKF_GenRandom = ULONG(SKF_API*)(DEVHANDLE hDev, BYTE* pbRandom, ULONG ulRandomLen);

//=== 对称加密函数指针类型 ===

using HANDLE = void*;  ///< 通用句柄

/**
 * @brief 设置对称密钥
 * @param hDev 设备句柄
 * @param pbKey 密钥数据
 * @param ulAlgID 算法标识（如 SGD_SM4_ECB）
 * @param phKey 返回密钥句柄
 */
using PFN_SKF_SetSymmKey = ULONG(SKF_API*)(DEVHANDLE hDev, BYTE* pbKey, ULONG ulAlgID, HANDLE* phKey);

/**
 * @brief 初始化加密
 * @param hKey 密钥句柄
 * @param encryptParam 加密参数（BLOCKCIPHERPARAM）
 */
using PFN_SKF_EncryptInit = ULONG(SKF_API*)(HANDLE hKey, BLOCKCIPHERPARAM encryptParam);

/**
 * @brief 对称加密
 * @param hKey 密钥句柄
 * @param pbData 明文数据
 * @param ulDataLen 明文长度
 * @param pbEncryptedData 密文缓冲区
 * @param pulEncryptedLen 输入：缓冲区大小；输出：密文长度
 */
using PFN_SKF_Encrypt = ULONG(SKF_API*)(HANDLE hKey, BYTE* pbData, ULONG ulDataLen,
                                         BYTE* pbEncryptedData, PULONG pulEncryptedLen);

//=== 证书操作函数指针类型 ===

/**
 * @brief 导入证书
 * @param hContainer 容器句柄
 * @param bSignFlag TRUE=签名证书，FALSE=加密证书
 * @param pbCert 证书数据
 * @param ulCertLen 证书长度
 */
using PFN_SKF_ImportCertificate = ULONG(SKF_API*)(HCONTAINER hContainer, BOOL bSignFlag, BYTE* pbCert,
                                                  ULONG ulCertLen);

/**
 * @brief 导出证书
 * @param hContainer 容器句柄
 * @param bSignFlag TRUE=签名证书，FALSE=加密证书
 * @param pbCert 证书缓冲区
 * @param pulCertLen 输入：缓冲区大小；输出：实际大小
 */
using PFN_SKF_ExportCertificate = ULONG(SKF_API*)(HCONTAINER hContainer, BOOL bSignFlag, BYTE* pbCert,
                                                  PULONG pulCertLen);

//=== 哈希函数指针类型 ===

/**
 * @brief 初始化哈希计算
 * @param hDev 设备句柄
 * @param ulAlgID 算法标识 (SGD_SM3)
 * @param pPubKey 公钥 (用于 SM2 预处理，可为 NULL)
 * @param pucID 用户 ID (用于 SM2 预处理，可为 NULL)
 * @param ulIDLen 用户 ID 长度
 * @param phHash 返回哈希句柄
 */
using PFN_SKF_DigestInit = ULONG(SKF_API*)(DEVHANDLE hDev, ULONG ulAlgID, ECCPUBLICKEYBLOB* pPubKey,
                                           BYTE* pucID, ULONG ulIDLen, HANDLE* phHash);

/**
 * @brief 哈希计算
 * @param hHash 哈希句柄
 * @param pbData 数据
 * @param ulDataLen 数据长度
 * @param pbHashData 哈希结果
 * @param pulHashLen 哈希长度
 */
using PFN_SKF_Digest = ULONG(SKF_API*)(HANDLE hHash, BYTE* pbData, ULONG ulDataLen,
                                        BYTE* pbHashData, PULONG pulHashLen);

/**
 * @brief 更新哈希数据
 * @param hHash 哈希句柄
 * @param pbData 数据
 * @param ulDataLen 数据长度
 */
using PFN_SKF_DigestUpdate = ULONG(SKF_API*)(HANDLE hHash, BYTE* pbData, ULONG ulDataLen);

/**
 * @brief 完成哈希计算
 * @param hHash 哈希句柄
 * @param pbHashData 哈希结果
 * @param pulHashLen 哈希长度
 */
using PFN_SKF_DigestFinal = ULONG(SKF_API*)(HANDLE hHash, BYTE* pbHashData, PULONG pulHashLen);

//=== 签名验签函数指针类型 ===

/**
 * @brief ECC 签名
 * @param hContainer 容器句柄
 * @param pbData 待签名数据 (SM3 哈希值，32 字节)
 * @param ulDataLen 数据长度 (应为 32)
 * @param pSignature 返回签名值
 */
using PFN_SKF_ECCSignData = ULONG(SKF_API*)(HCONTAINER hContainer, BYTE* pbData, ULONG ulDataLen,
                                            ECCSIGNATUREBLOB* pSignature);

/**
 * @brief ECC 验签
 * @param hDev 设备句柄
 * @param pPubKey 公钥
 * @param pbData 原始数据
 * @param ulDataLen 数据长度
 * @param pSignature 签名值
 */
using PFN_SKF_ECCVerify = ULONG(SKF_API*)(DEVHANDLE hDev, ECCPUBLICKEYBLOB* pPubKey, BYTE* pbData, ULONG ulDataLen,
                                          ECCSIGNATUREBLOB* pSignature);

/**
 * @brief RSA 签名
 * @param hContainer 容器句柄
 * @param pbData 待签名数据（哈希值）
 * @param ulDataLen 数据长度
 * @param pbSignature 签名结果缓冲区
 * @param pulSignLen 输入：缓冲区大小；输出：签名长度
 */
using PFN_SKF_RSASignData = ULONG(SKF_API*)(HCONTAINER hContainer, BYTE* pbData, ULONG ulDataLen,
                                            BYTE* pbSignature, PULONG pulSignLen);

/**
 * @brief RSA 验签
 * @param hDev 设备句柄
 * @param pRSAPubKeyBlob 公钥
 * @param pbData 原始数据（哈希值）
 * @param ulDataLen 数据长度
 * @param pbSignature 签名数据
 * @param ulSignLen 签名长度
 */
using PFN_SKF_RSAVerify = ULONG(SKF_API*)(DEVHANDLE hDev, RSAPUBLICKEYBLOB* pRSAPubKeyBlob,
                                          BYTE* pbData, ULONG ulDataLen,
                                          BYTE* pbSignature, ULONG ulSignLen);

//=== 文件操作函数指针类型 ===

/**
 * @brief 创建文件
 * @param hApplication 应用句柄
 * @param szFileName 文件名
 * @param ulFileSize 文件大小
 * @param ulReadRights 读权限
 * @param ulWriteRights 写权限
 */
using PFN_SKF_CreateFile = ULONG(SKF_API*)(HAPPLICATION hApplication, LPCSTR szFileName, ULONG ulFileSize,
                                           ULONG ulReadRights, ULONG ulWriteRights);

/**
 * @brief 删除文件
 * @param hApplication 应用句柄
 * @param szFileName 文件名
 */
using PFN_SKF_DeleteFile = ULONG(SKF_API*)(HAPPLICATION hApplication, LPCSTR szFileName);

/**
 * @brief 枚举文件
 * @param hApplication 应用句柄
 * @param szFileName 文件名列表（以 \0\0 结尾）
 * @param pulSize 输入：缓冲区大小；输出：实际大小
 */
using PFN_SKF_EnumFiles = ULONG(SKF_API*)(HAPPLICATION hApplication, LPSTR szFileName, PULONG pulSize);

/**
 * @brief 读取文件
 * @param hApplication 应用句柄
 * @param szFileName 文件名
 * @param ulOffset 偏移量
 * @param ulSize 读取大小
 * @param pbOutData 输出缓冲区
 * @param pulOutLen 输出长度
 */
using PFN_SKF_ReadFile = ULONG(SKF_API*)(HAPPLICATION hApplication, LPCSTR szFileName, ULONG ulOffset, ULONG ulSize,
                                         BYTE* pbOutData, PULONG pulOutLen);

/**
 * @brief 写入文件
 * @param hApplication 应用句柄
 * @param szFileName 文件名
 * @param ulOffset 偏移量
 * @param pbData 数据
 * @param ulSize 数据大小
 */
using PFN_SKF_WriteFile = ULONG(SKF_API*)(HAPPLICATION hApplication, LPCSTR szFileName, ULONG ulOffset, BYTE* pbData,
                                          ULONG ulSize);

}  // namespace skf
}  // namespace wekey

}  // extern "C"
