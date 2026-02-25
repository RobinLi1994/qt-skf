/**
 * @file UiHelper.h
 * @brief UI 样式辅助工具
 *
 * 参考 Ant Design 设计规范，提供统一的样式常量和辅助函数。
 * 色彩体系：主色 #1677ff，成功 #52c41a，警告 #faad14，错误 #ff4d4f
 * 间距体系：基础单位 4px，常用 8/12/16/24px
 *
 * 注意：ElaPushButton 自绘 paintEvent，QSS 对其无效，
 * 必须使用 setLightDefaultColor / setLightTextColor 等原生 API。
 */

#pragma once

#include <QBuffer>
#include <QDialog>
#include <QFont>
#include <QFontMetrics>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QTableWidget>
#include <QHeaderView>

#include <ElaComboBox.h>
#include <ElaIcon.h>
#include <ElaLineEdit.h>
#include <ElaPushButton.h>
#include <ElaScrollPageArea.h>
#include <ElaText.h>

namespace wekey {
namespace UiHelper {

// ============================================================
// Ant Design 间距体系 (基础单位 4px)
// ============================================================
inline constexpr int kSpaceXS  = 4;
inline constexpr int kSpaceSM  = 8;
inline constexpr int kSpaceMD  = 12;
inline constexpr int kSpaceLG  = 16;
inline constexpr int kSpaceXL  = 24;
inline constexpr int kSpaceXXL = 32;

// ============================================================
// Ant Design 圆角体系
// ============================================================
inline constexpr int kRadiusSM   = 4;   // 小组件（Tag、小按钮）
inline constexpr int kRadiusMD   = 6;   // 中等组件（按钮、输入框、下拉框）
inline constexpr int kRadiusLG   = 8;   // 大组件（卡片、表格）

// ============================================================
// 表格行高
// ============================================================
inline constexpr int kTableRowHeight = 48;

/**
 * @brief 配置表格为 Ant Design 风格
 *
 * 斑马纹、无网格线、统一行高、表头样式
 */
inline void styleTable(QTableWidget* table) {
    table->verticalHeader()->setVisible(false);
    table->verticalHeader()->setDefaultSectionSize(kTableRowHeight);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setAlternatingRowColors(true);
    table->setShowGrid(false);
    table->setFocusPolicy(Qt::NoFocus);
    table->setSelectionMode(QAbstractItemView::SingleSelection);

    // QTableWidget 是原生 Qt 组件，QSS 生效
    table->setStyleSheet(
        "QTableWidget {"
        "  border: 1px solid #f0f0f0;"
        "  border-radius: 8px;"
        "  background-color: #ffffff;"
        "  alternate-background-color: #fafafa;"
        "  gridline-color: transparent;"
        "}"
        "QTableWidget::item {"
        "  padding: 8px 16px;"
        "  border-bottom: 1px solid #f0f0f0;"
        "}"
        "QTableWidget::item:selected {"
        "  background-color: #e6f4ff;"
        "  color: #000000;"
        "}"
        "QTableWidget::item:hover {"
        "  background-color: #f0f0f0;"
        "}"
        "QHeaderView::section {"
        "  background-color: #fafafa;"
        "  color: #000000;"
        "  font-weight: 600;"
        "  padding: 8px 16px;"
        "  border: none;"
        "  border-bottom: 1px solid #f0f0f0;"
        "}"
    );
}

/**
 * @brief Ant Design 主按钮（蓝色填充）
 *
 * 通过 ElaPushButton 原生 API 设置颜色，确保自绘 paintEvent 生效。
 */
inline void stylePrimaryButton(ElaPushButton* btn) {
    // 背景色
    btn->setLightDefaultColor(QColor("#1677ff"));
    btn->setDarkDefaultColor(QColor("#1668dc"));
    // hover 背景色
    btn->setLightHoverColor(QColor("#4096ff"));
    btn->setDarkHoverColor(QColor("#3c89e8"));
    // 按下背景色
    btn->setLightPressColor(QColor("#0958d9"));
    btn->setDarkPressColor(QColor("#1554ad"));
    // 文字色（白色）
    btn->setLightTextColor(QColor("#ffffff"));
    btn->setDarkTextColor(QColor("#ffffff"));
    // 圆角 + 尺寸（确保文字与边框有足够间距）
    btn->setBorderRadius(6);
    btn->setFixedHeight(36);
    btn->setMinimumWidth(80);
}

/**
 * @brief Ant Design 默认按钮（保持 Ela 默认样式，仅调整圆角）
 *
 * Ela 默认按钮已经是白底灰边，与 Ant Design 默认按钮风格接近。
 */
inline void styleDefaultButton(ElaPushButton* btn) {
    btn->setBorderRadius(6);
    btn->setFixedHeight(36);
    btn->setMinimumWidth(72);
}

/**
 * @brief Ant Design 危险按钮（红色背景）
 */
inline void styleDangerButton(ElaPushButton* btn) {
    btn->setLightDefaultColor(QColor("#ff4d4f"));
    btn->setDarkDefaultColor(QColor("#d9363e"));
    btn->setLightHoverColor(QColor("#ff7875"));
    btn->setDarkHoverColor(QColor("#e84749"));
    btn->setLightPressColor(QColor("#d9363e"));
    btn->setDarkPressColor(QColor("#b02a2f"));
    btn->setLightTextColor(QColor("#ffffff"));
    btn->setDarkTextColor(QColor("#ffffff"));
    btn->setBorderRadius(6);
    btn->setFixedHeight(36);
    btn->setMinimumWidth(72);
}

/**
 * @brief Ant Design 文字链接按钮（表格操作列用，蓝色文字无背景）
 */
inline void styleLinkButton(ElaPushButton* btn) {
    // 透明背景
    btn->setLightDefaultColor(QColor(0, 0, 0, 0));
    btn->setDarkDefaultColor(QColor(0, 0, 0, 0));
    btn->setLightHoverColor(QColor(22, 119, 255, 20));
    btn->setDarkHoverColor(QColor(22, 119, 255, 30));
    btn->setLightPressColor(QColor(22, 119, 255, 40));
    btn->setDarkPressColor(QColor(22, 119, 255, 50));
    // 蓝色文字
    btn->setLightTextColor(QColor("#1677ff"));
    btn->setDarkTextColor(QColor("#4096ff"));
    btn->setBorderRadius(4);
    btn->setFixedHeight(28);
    btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btn->setCursor(Qt::PointingHandCursor);
}

/**
 * @brief Ant Design 危险文字链接按钮（删除操作用，红色文字无背景）
 */
inline void styleDangerLinkButton(ElaPushButton* btn) {
    btn->setLightDefaultColor(QColor(0, 0, 0, 0));
    btn->setDarkDefaultColor(QColor(0, 0, 0, 0));
    btn->setLightHoverColor(QColor(255, 77, 79, 20));
    btn->setDarkHoverColor(QColor(255, 77, 79, 30));
    btn->setLightPressColor(QColor(255, 77, 79, 40));
    btn->setDarkPressColor(QColor(255, 77, 79, 50));
    btn->setLightTextColor(QColor("#ff4d4f"));
    btn->setDarkTextColor(QColor("#ff7875"));
    btn->setBorderRadius(4);
    btn->setFixedHeight(28);
    btn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    btn->setCursor(Qt::PointingHandCursor);
}

// ============================================================
// Ant Design 操作链接（表格操作列用，纯文字+颜色，无按钮边框）
// ============================================================

/**
 * @brief 将 Ela 图标转为 base64 HTML img 标签
 * @param icon Ela 图标类型
 * @param color 图标颜色
 * @param size 图标大小
 * @return HTML img 标签字符串
 */
inline QString iconToHtmlImg(ElaIconType::IconName icon, const QColor& color, int size = 14) {
    QIcon qicon = ElaIcon::getInstance()->getElaIcon(icon, size, color);
    QPixmap pix = qicon.pixmap(size, size);
    QByteArray ba;
    QBuffer buffer(&ba);
    buffer.open(QIODevice::WriteOnly);
    pix.save(&buffer, "PNG");
    return QString("<img src='data:image/png;base64,%1' width='%2' height='%2' style='vertical-align:middle;'>")
        .arg(QString(ba.toBase64())).arg(size);
}

/**
 * @brief 创建带图标的蓝色操作链接（单个 QLabel，图标内嵌 HTML，点击图标或文字均触发）
 * @param icon Ela 图标类型
 * @param text 显示文字
 * @param parent 父组件
 * @return QLabel*，使用 linkActivated 信号响应点击
 */
inline QLabel* createActionLink(ElaIconType::IconName icon, const QString& text, QWidget* parent = nullptr) {
    QString imgTag = iconToHtmlImg(icon, QColor("#1677ff"));
    auto* label = new QLabel(
        QString("<a href='#' style='color:#1677ff;text-decoration:none;'>%1 %2</a>").arg(imgTag, text), parent);
    label->setCursor(Qt::PointingHandCursor);
    return label;
}

/**
 * @brief 创建蓝色操作链接（无图标）
 */
inline QLabel* createActionLink(const QString& text, QWidget* parent = nullptr) {
    auto* label = new QLabel(
        QString("<a href='#' style='color:#1677ff;text-decoration:none;'>%1</a>").arg(text), parent);
    label->setCursor(Qt::PointingHandCursor);
    return label;
}

/**
 * @brief 创建带图标的红色危险操作链接
 */
inline QLabel* createDangerLink(ElaIconType::IconName icon, const QString& text, QWidget* parent = nullptr) {
    QString imgTag = iconToHtmlImg(icon, QColor("#ff4d4f"));
    auto* label = new QLabel(
        QString("<a href='#' style='color:#ff4d4f;text-decoration:none;'>%1 %2</a>").arg(imgTag, text), parent);
    label->setCursor(Qt::PointingHandCursor);
    return label;
}

/**
 * @brief 创建红色危险操作链接（无图标）
 */
inline QLabel* createDangerLink(const QString& text, QWidget* parent = nullptr) {
    auto* label = new QLabel(
        QString("<a href='#' style='color:#ff4d4f;text-decoration:none;'>%1</a>").arg(text), parent);
    label->setCursor(Qt::PointingHandCursor);
    return label;
}

/**
 * @brief 创建带图标的灰色禁用操作链接（不可点击）
 */
inline QLabel* createDisabledLink(ElaIconType::IconName icon, const QString& text, QWidget* parent = nullptr) {
    QString imgTag = iconToHtmlImg(icon, QColor("#bfbfbf"));
    auto* label = new QLabel(
        QString("<span style='color:#bfbfbf;'>%1 %2</span>").arg(imgTag, text), parent);
    label->setEnabled(false);
    return label;
}

/**
 * @brief 创建灰色禁用操作链接（无图标，不可点击）
 */
inline QLabel* createDisabledLink(const QString& text, QWidget* parent = nullptr) {
    auto* label = new QLabel(
        QString("<span style='color:#bfbfbf;'>%1</span>").arg(text), parent);
    label->setEnabled(false);
    return label;
}

// ============================================================
// Ant Design Tag 标签样式（用于表格状态列）
// QLabel 是原生 Qt 组件，QSS 生效
// ============================================================

/**
 * @brief 创建 Tag 标签的内部辅助：QLabel + 居中 wrapper
 *
 * 返回一个 QWidget 容器，内部居中放置带样式的 QLabel，
 * 适合用于 QTableWidget::setCellWidget。
 */
inline QWidget* createTagWidget(const QString& text, const QString& bgColor,
                                const QString& textColor, const QString& borderColor,
                                QWidget* parent = nullptr) {
    auto* wrapper = new QWidget(parent);
    // 用对象名限定 QSS，避免被表格父级样式覆盖
    wrapper->setObjectName("tagWrapper");
    wrapper->setStyleSheet("#tagWrapper { background: transparent; border: none; }");
    auto* layout = new QHBoxLayout(wrapper);
    layout->setContentsMargins(8, 4, 8, 4);
    layout->setAlignment(Qt::AlignCenter);
    auto* label = new QLabel(text, wrapper);
    label->setObjectName("tagLabel");
    label->setAlignment(Qt::AlignCenter);
    label->setMinimumHeight(22);
    // 根据文字宽度 + padding + border 计算最小宽度，防止文字被截断
    QFont tagFont;
    tagFont.setPixelSize(12);
    int textWidth = QFontMetrics(tagFont).horizontalAdvance(text);
    label->setMinimumWidth(textWidth + 24);  // 10px padding * 2 + 2px border * 2
    label->setStyleSheet(
        QString("#tagLabel {"
                "  background-color: %1;"
                "  color: %2;"
                "  border: 1px solid %3;"
                "  border-radius: 4px;"
                "  padding: 2px 10px;"
                "  font-size: 12px;"
                "}").arg(bgColor, textColor, borderColor)
    );
    layout->addWidget(label);
    return wrapper;
}

/**
 * @brief 创建 Ant Design 成功 Tag（绿色，如"已激活"、"已登录"、"已生成"）
 */
inline QWidget* createSuccessTag(const QString& text, QWidget* parent = nullptr) {
    return createTagWidget(text, "#f6ffed", "#52c41a", "#b7eb8f", parent);
}

/**
 * @brief 创建 Ant Design 默认 Tag（灰色，如"未激活"、"未登录"、"未生成"）
 */
inline QWidget* createDefaultTag(const QString& text, QWidget* parent = nullptr) {
    return createTagWidget(text, "#fafafa", "#595959", "#d9d9d9", parent);
}

/**
 * @brief 创建 Ant Design 蓝色 Tag（信息，如"SM2"、"RSA"）
 */
inline QWidget* createInfoTag(const QString& text, QWidget* parent = nullptr) {
    return createTagWidget(text, "#e6f4ff", "#1677ff", "#91caff", parent);
}

/**
 * @brief 创建 Ant Design 警告 Tag（橙色，如"已过期"）
 */
inline QWidget* createWarningTag(const QString& text, QWidget* parent = nullptr) {
    return createTagWidget(text, "#fffbe6", "#faad14", "#ffe58f", parent);
}

// ============================================================
// 组件圆角统一
// ============================================================

/**
 * @brief 统一设置 ElaComboBox 圆角
 */
inline void styleComboBox(ElaComboBox* combo) {
    combo->setBorderRadius(kRadiusMD);
}

/**
 * @brief 统一设置 ElaLineEdit 圆角
 */
inline void styleLineEdit(ElaLineEdit* edit) {
    edit->setBorderRadius(kRadiusMD);
}

/**
 * @brief 统一设置 ElaScrollPageArea 卡片圆角
 */
inline void styleCard(ElaScrollPageArea* area) {
    area->setBorderRadius(kRadiusLG);
    // ElaScrollPageArea 默认 setFixedHeight(75)，取消固定高度让内容自适应
    area->setMinimumHeight(0);
    area->setMaximumHeight(QWIDGETSIZE_MAX);
    area->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

// ============================================================
// 对话框辅助
// ============================================================

/**
 * @brief 创建 Ant Design 风格的水平分隔线
 */
inline QFrame* createDivider(QWidget* parent = nullptr) {
    auto* line = new QFrame(parent);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Plain);
    line->setStyleSheet("QFrame { color: #f0f0f0; }");
    line->setFixedHeight(1);
    return line;
}

/**
 * @brief 配置对话框为 Ant Design 风格（间距、背景）
 *
 * QDialog 是原生 Qt 组件，QSS 生效。
 */
inline void styleDialog(QDialog* dialog) {
    dialog->setStyleSheet(
        "QDialog { background-color: #ffffff; }"
        "QLabel { color: rgba(0,0,0,0.85); font-size: 14px; }"
    );
}

}  // namespace UiHelper
}  // namespace wekey
