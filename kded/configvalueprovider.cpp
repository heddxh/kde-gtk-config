/*
 * Copyright (C) 2019 Mikhail Zolotukhin <zomial@protonmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QFont>
#include <QString>
#include <QColor>
#include <QMap>
#include <QList>

#include <KConfig>
#include <KSharedConfig>
#include <KConfigGroup>
#include <KColorScheme>
#include <KColorUtils>

#include <gtk/gtk.h>

#include "configvalueprovider.h"

ConfigValueProvider::ConfigValueProvider() :
    kdeglobalsConfig(KSharedConfig::openConfig(QStringLiteral("kdeglobals"))),
    inputConfig(KSharedConfig::openConfig(QStringLiteral("kcminputrc"))),
    kwinConfig(KSharedConfig::openConfig(QStringLiteral("kwinrc")))
{

}

QString ConfigValueProvider::fontName() const
{
    static const QFont defaultFont(QStringLiteral("Noto Sans"), 10);

    KConfigGroup configGroup = kdeglobalsConfig->group(QStringLiteral("General"));
    QString fontAsString = configGroup.readEntry(QStringLiteral("font"), defaultFont.toString());
    static QFont font;
    font.fromString(fontAsString);
    const QString fontStyle = fontStyleHelper(font);
    return font.family() + QStringLiteral(", ") + fontStyle + ' ' + QString::number(font.pointSize());
}

QString ConfigValueProvider::fontStyleHelper(const QFont &font) const
{
    // BUG: 333146
    // Since Qt sometimes gives us wrong font style name,
    // we ought to use this big helper function to construct
    // the style ourselves. Some fonts will not work
    auto weight = font.weight();
    QString result;
    if (weight > QFont::Normal) {
        if (weight >= QFont::Black) {
            result = QStringLiteral("Black");
        } else if (weight >= QFont::ExtraBold) {
            result = QStringLiteral("Extra Bold");
        } else if (weight >= QFont::Bold) {
            result = QStringLiteral("Bold");
        } else if (weight >= QFont::DemiBold) {
            result = QStringLiteral("Demi Bold");
        } else if (weight >= QFont::Medium) {
            result = QStringLiteral("Medium");
        }
    } else {
        if (weight <= QFont::Thin) {
            result = QStringLiteral("Thin");
        } else if (weight <= QFont::ExtraLight) {
            result = QStringLiteral("Extra Light");
        } else if (weight <= QFont::Light) {
            result = QStringLiteral("Light");
        }
    }

    auto style = font.style();
    if (style == QFont::StyleItalic) {
        result += QLatin1Char(' ') + QStringLiteral("Italic");
    } else if (style == QFont::StyleOblique) {
        result += QLatin1Char(' ') + QStringLiteral("Oblique");
    }

    auto stretch = font.stretch();
    if (stretch == QFont::UltraCondensed) {
        result += QLatin1Char(' ') + QStringLiteral("UltraCondensed");
    } else if (stretch == QFont::ExtraCondensed) {
        result += QLatin1Char(' ') + QStringLiteral("ExtraCondensed");
    } else if (stretch == QFont::Condensed) {
        result += QLatin1Char(' ') + QStringLiteral("Condensed");
    } else if (stretch == QFont::SemiCondensed) {
        result += QLatin1Char(' ') + QStringLiteral("SemiCondensed");
    } else if (stretch == QFont::Unstretched) {
        result += QLatin1Char(' ') + QStringLiteral("Unstretched");
    } else if (stretch == QFont::SemiExpanded) {
        result += QLatin1Char(' ') + QStringLiteral("SemiExpanded");
    } else if (stretch == QFont::Expanded) {
        result += QLatin1Char(' ') + QStringLiteral("Expanded");
    } else if (stretch == QFont::ExtraExpanded) {
        result += QLatin1Char(' ') + QStringLiteral("ExtraExpanded");
    } else if (stretch == QFont::UltraExpanded) {
        result += QLatin1Char(' ') + QStringLiteral("UltraExpanded");
    }

    return result.simplified();
}


QString ConfigValueProvider::iconThemeName() const
{
    KConfigGroup configGroup = kdeglobalsConfig->group(QStringLiteral("Icons"));
    return configGroup.readEntry(QStringLiteral("Theme"), QStringLiteral("breeze"));
}

QString ConfigValueProvider::cursorThemeName() const
{
    KConfigGroup configGroup = inputConfig->group(QStringLiteral("Mouse"));
    return configGroup.readEntry(QStringLiteral("cursorTheme"), QStringLiteral("breeze_cursors"));
}

bool ConfigValueProvider::iconsOnButtons() const
{
    KConfigGroup configGroup = kdeglobalsConfig->group(QStringLiteral("KDE"));
    return configGroup.readEntry(QStringLiteral("ShowIconsOnPushButtons"), true);
}

bool ConfigValueProvider::iconsInMenus() const
{
    KConfigGroup configGroup = kdeglobalsConfig->group(QStringLiteral("KDE"));
    return configGroup.readEntry(QStringLiteral("ShowIconsInMenuItems"), true);
}

int ConfigValueProvider::toolbarStyle() const
{
    KConfigGroup configGroup = kdeglobalsConfig->group(QStringLiteral("Toolbar style"));
    QString kdeConfigValue = configGroup.readEntry(QStringLiteral("ToolButtonStyle"), "TextBesideIcon");

    if (kdeConfigValue == QStringLiteral("NoText")) {
        return GtkToolbarStyle::GTK_TOOLBAR_ICONS;
    } else if (kdeConfigValue == QStringLiteral("TextOnly")) {
        return GtkToolbarStyle::GTK_TOOLBAR_TEXT;
    } else if (kdeConfigValue == QStringLiteral("TextBesideIcon")) {
        return GtkToolbarStyle::GTK_TOOLBAR_BOTH_HORIZ;
    } else {
        return GtkToolbarStyle::GTK_TOOLBAR_BOTH;
    }
}

bool ConfigValueProvider::scrollbarBehavior() const
{
    KConfigGroup configGroup = kdeglobalsConfig->group(QStringLiteral("KDE"));
    bool kdeConfigValue = configGroup.readEntry(QStringLiteral("ScrollbarLeftClickNavigatesByPage"), true);
    return !kdeConfigValue; // GTK setting is inverted
}

bool ConfigValueProvider::preferDarkTheme() const
{
    KConfigGroup colorsConfigGroup = kdeglobalsConfig->group(QStringLiteral("Colors:Window"));
    QColor windowBackgroundColor = colorsConfigGroup.readEntry(QStringLiteral("BackgroundNormal"), QColor(239, 240, 241));
    const int windowBackgroundGray = qGray(windowBackgroundColor.rgb());

    // We use heuristic to determine if current color scheme is dark or not
    return windowBackgroundGray < 192;
}

QString ConfigValueProvider::windowDecorationsButtonsOrder() const
{
    KConfigGroup configGroup = kwinConfig->group(QStringLiteral("org.kde.kdecoration2"));
    QString buttonsOnLeftKdeConfigValue = configGroup.readEntry(QStringLiteral("ButtonsOnLeft"), "MS");
    QString buttonsOnRightKdeConfigValue = configGroup.readEntry(QStringLiteral("ButtonsOnRight"), "HIAX");

    QString buttonsOnLeftInGtkNotation = windowDecorationButtonsOrderInGtkNotation(buttonsOnLeftKdeConfigValue);
    QString buttonsOnRightInGtkNotation = windowDecorationButtonsOrderInGtkNotation(buttonsOnRightKdeConfigValue);

    return buttonsOnLeftInGtkNotation + QStringLiteral(":") + buttonsOnRightInGtkNotation;
}

bool ConfigValueProvider::enableAnimations() const
{
    KConfigGroup generalCfg = kdeglobalsConfig->group(QStringLiteral("KDE"));
    const qreal animationSpeedModifier = qMax(0.0, generalCfg.readEntry("AnimationDurationFactor", 1.0));

    return !qFuzzyIsNull(animationSpeedModifier);
}

QMap<QString, QColor> ConfigValueProvider::colors() const
{
    using KCS = KColorScheme;

    // Color Schemes Collection
    QHash<QString, QHash<QString, KCS>> csc {
        {QStringLiteral("active"), {
            {QStringLiteral("view"), KCS(QPalette::Active, KCS::View)},
            {QStringLiteral("window"), KCS(QPalette::Active, KCS::Window)},
            {QStringLiteral("button"), KCS(QPalette::Active, KCS::Button)},
            {QStringLiteral("selection"), KCS(QPalette::Active, KCS::Selection)},
            {QStringLiteral("tooltip"), KCS(QPalette::Active, KCS::Tooltip)},
            {QStringLiteral("complementary"), KCS(QPalette::Active, KCS::Complementary)},
        }},
        {QStringLiteral("inactive"), {
            {QStringLiteral("view"), KCS(QPalette::Inactive, KCS::View)},
            {QStringLiteral("window"), KCS(QPalette::Inactive, KCS::Window)},
            {QStringLiteral("button"), KCS(QPalette::Inactive, KCS::Button)},
            {QStringLiteral("selection"), KCS(QPalette::Inactive, KCS::Selection)},
            {QStringLiteral("tooltip"), KCS(QPalette::Inactive, KCS::Tooltip)},
            {QStringLiteral("complementary"), KCS(QPalette::Inactive, KCS::Complementary)},
        }},
        {QStringLiteral("disabled"), {
            {QStringLiteral("view"), KCS(QPalette::Disabled, KCS::View)},
            {QStringLiteral("window"), KCS(QPalette::Disabled, KCS::Window)},
            {QStringLiteral("button"), KCS(QPalette::Disabled, KCS::Button)},
            {QStringLiteral("selection"), KCS(QPalette::Disabled, KCS::Selection)},
            {QStringLiteral("tooltip"), KCS(QPalette::Disabled, KCS::Tooltip)},
            {QStringLiteral("complementary"), KCS(QPalette::Disabled, KCS::Complementary)},
        }},
    };

    // Color mixing
    QColor windowForegroundColor = csc["active"]["window"].foreground(KCS::NormalText).color();
    QColor windowBackgroundColor = csc["active"]["window"].background(KCS::NormalBackground).color();
    QColor bordersColor = KColorUtils::mix(windowBackgroundColor, windowForegroundColor, 0.25);

    QColor inactiveWindowForegroundColor = csc["inactive"]["window"].foreground(KCS::NormalText).color();
    QColor inactiveWindowBackgroundColor = csc["inactive"]["window"].background(KCS::NormalBackground).color();
    QColor inactiveBordersColor = KColorUtils::mix(inactiveWindowBackgroundColor, inactiveWindowForegroundColor, 0.25);

    QColor disabledWindowForegroundColor = csc["disabled"]["window"].foreground(KCS::NormalText).color();
    QColor disabledWindowBackgroundColor = csc["disabled"]["window"].background(KCS::NormalBackground).color();
    QColor disabledBordersColor = KColorUtils::mix(disabledWindowBackgroundColor, disabledWindowForegroundColor, 0.25);

    QColor unfocusedDisabledWindowForegroundColor = csc["disabled"]["window"].foreground(KCS::NormalText).color();
    QColor unfocusedDisabledWindowBackgroundColor = csc["disabled"]["window"].background(KCS::NormalBackground).color();
    QColor unfocusedDisabledBordersColor = KColorUtils::mix(unfocusedDisabledWindowBackgroundColor, unfocusedDisabledWindowForegroundColor, 0.25);

    QColor tooltipForegroundColor = csc["active"]["tooltip"].foreground(KCS::NormalText).color();
    QColor tooltipBackgroundColor = csc["active"]["tooltip"].background(KCS::NormalBackground).color();
    QColor tooltipBorderColor = KColorUtils::mix(tooltipBackgroundColor, tooltipForegroundColor, 0.25);

    KConfigGroup windowManagerConfig = kdeglobalsConfig->group(QStringLiteral("WM"));

    return {
        /*
         * Normal (Non Backdrop, Non Insensitive)
         */

        // General Colors
        { "theme_fg_color_breeze", csc["active"]["window"].foreground(KCS::NormalText).color()},
        { "theme_bg_color_breeze", csc["active"]["window"].background(KCS::NormalBackground).color()},
        { "theme_text_color_breeze", csc["active"]["view"].foreground(KCS::NormalText).color()},
        { "theme_base_color_breeze", csc["active"]["view"].background(KCS::NormalBackground).color()},
        { "theme_view_hover_decoration_color_breeze", csc["active"]["view"].decoration(KCS::HoverColor).color()},
        { "theme_hovering_selected_bg_color_breeze", csc["active"]["selection"].decoration(KCS::HoverColor).color()},
        { "theme_selected_bg_color_breeze", csc["active"]["selection"].background(KCS::NormalBackground).color()},
        { "theme_selected_fg_color_breeze", csc["active"]["selection"].foreground(KCS::NormalText).color()},
        { "theme_view_active_decoration_color_breeze", csc["active"]["view"].decoration(KCS::HoverColor).color()},

        // Button Colors
        { "theme_button_background_normal_breeze", csc["active"]["button"].background(KCS::NormalBackground).color()},
        { "theme_button_decoration_hover_breeze", csc["active"]["button"].decoration(KCS::HoverColor).color()},
        { "theme_button_decoration_focus_breeze", csc["active"]["button"].decoration(KCS::FocusColor).color()},
        { "theme_button_foreground_normal_breeze", csc["active"]["button"].foreground(KCS::NormalText).color()},
        { "theme_button_foreground_active_breeze", csc["active"]["selection"].foreground(KCS::NormalText).color()},


        // Misc Colors
        { "borders_breeze", bordersColor},
        { "warning_color_breeze", csc["active"]["view"].foreground(KCS::NeutralText).color()},
        { "success_color_breeze", csc["active"]["view"].foreground(KCS::PositiveText).color()},
        { "error_color_breeze", csc["active"]["view"].foreground(KCS::NegativeText).color()},

        /*
         * Backdrop (Inactive)
         */

        // General
        { "theme_unfocused_fg_color_breeze", csc["inactive"]["window"].foreground(KCS::NormalText).color()},
        { "theme_unfocused_text_color_breeze", csc["inactive"]["view"].foreground(KCS::NormalText).color()},
        { "theme_unfocused_bg_color_breeze", csc["inactive"]["window"].background(KCS::NormalBackground).color()},
        { "theme_unfocused_base_color_breeze", csc["inactive"]["view"].background(KCS::NormalBackground).color()},
        { "theme_unfocused_selected_bg_color_alt_breeze", csc["inactive"]["selection"].background(KCS::NormalBackground).color()},
        { "theme_unfocused_selected_bg_color_breeze", csc["inactive"]["selection"].background(KCS::NormalBackground).color()},
        { "theme_unfocused_selected_fg_color_breeze", csc["inactive"]["selection"].foreground(KCS::NormalText).color()},

        // Button
        { "theme_button_background_backdrop_breeze", csc["inactive"]["button"].background(KCS::NormalBackground).color()},
        { "theme_button_decoration_hover_backdrop_breeze", csc["inactive"]["button"].decoration(KCS::HoverColor).color()},
        { "theme_button_decoration_focus_backdrop_breeze", csc["inactive"]["button"].decoration(KCS::FocusColor).color()},
        { "theme_button_foreground_backdrop_breeze", csc["inactive"]["button"].foreground(KCS::NormalText).color()},
        { "theme_button_foreground_active_backdrop_breeze", csc["inactive"]["selection"].foreground(KCS::NormalText).color()},

        // Misc Colors
        { "unfocused_borders_breeze", inactiveBordersColor},
        { "warning_color_backdrop_breeze", csc["inactive"]["view"].foreground(KCS::NeutralText).color()},
        { "success_color_backdrop_breeze", csc["inactive"]["view"].foreground(KCS::PositiveText).color()},
        { "error_color_backdrop_breeze", csc["inactive"]["view"].foreground(KCS::NegativeText).color()},

        /*
         * Insensitive (Disabled)
         */

        // General
        { "insensitive_fg_color_breeze",csc["disabled"]["window"].foreground(KCS::NormalText).color()},
        { "insensitive_base_fg_color_breeze", csc["disabled"]["view"].foreground(KCS::NormalText).color()},
        { "insensitive_bg_color_breeze", csc["disabled"]["window"].background(KCS::NormalBackground).color()},
        { "insensitive_base_color_breeze", csc["disabled"]["view"].background(KCS::NormalBackground).color()},
        { "insensitive_selected_bg_color_breeze", csc["disabled"]["selection"].background(KCS::NormalBackground).color()},
        { "insensitive_selected_fg_color_breeze", csc["disabled"]["selection"].foreground(KCS::NormalText).color()},

        // Button
        { "theme_button_background_insensitive_breeze", csc["disabled"]["button"].background(KCS::NormalBackground).color()},
        { "theme_button_decoration_hover_insensitive_breeze", csc["disabled"]["button"].decoration(KCS::HoverColor).color()},
        { "theme_button_decoration_focus_insensitive_breeze", csc["disabled"]["button"].decoration(KCS::FocusColor).color()},
        { "theme_button_foreground_insensitive_breeze", csc["disabled"]["button"].foreground(KCS::NormalText).color()},
        { "theme_button_foreground_active_insensitive_breeze", csc["disabled"]["selection"].foreground(KCS::NormalText).color()},

        // Misc Colors
        { "insensitive_borders_breeze", disabledBordersColor},
        { "warning_color_insensitive_breeze", csc["disabled"]["view"].foreground(KCS::NeutralText).color()},
        { "success_color_insensitive_breeze", csc["disabled"]["view"].foreground(KCS::PositiveText).color()},
        { "error_color_insensitive_breeze", csc["disabled"]["view"].foreground(KCS::NegativeText).color()},

        /*
         * Insensitive Backdrop (Inactive Disabled)
         * These pretty much have the same appearance as regular inactive colors,
         * but they're separate in case we decide to make them different in the future.
         */

        // General
        { "insensitive_unfocused_fg_color_breeze",csc["disabled"]["window"].foreground(KCS::NormalText).color()},
        { "theme_unfocused_view_text_color_breeze", csc["disabled"]["view"].foreground(KCS::NormalText).color()},
        { "insensitive_unfocused_bg_color_breeze", csc["disabled"]["window"].background(KCS::NormalBackground).color()},
        { "theme_unfocused_view_bg_color_breeze", csc["disabled"]["view"].background(KCS::NormalBackground).color()},
        { "insensitive_unfocused_selected_bg_color_breeze", csc["disabled"]["selection"].background(KCS::NormalBackground).color()},
        { "insensitive_unfocused_selected_fg_color_breeze", csc["disabled"]["selection"].foreground(KCS::NormalText).color()},

        // Button
        { "theme_button_background_backdrop_insensitive_breeze", csc["disabled"]["button"].background(KCS::NormalBackground).color()},
        { "theme_button_decoration_hover_backdrop_insensitive_breeze", csc["disabled"]["button"].decoration(KCS::HoverColor).color()},
        { "theme_button_decoration_focus_backdrop_insensitive_breeze", csc["disabled"]["button"].decoration(KCS::FocusColor).color()},
        { "theme_button_foreground_backdrop_insensitive_breeze", csc["disabled"]["button"].foreground(KCS::NormalText).color()},
        { "theme_button_foreground_active_backdrop_insensitive_breeze", csc["disabled"]["selection"].foreground(KCS::NormalText).color()},

        // Misc Colors
        { "unfocused_insensitive_borders_breeze", unfocusedDisabledBordersColor},
        { "warning_color_insensitive_backdrop_breeze", csc["disabled"]["view"].foreground(KCS::NeutralText).color()},
        { "success_color_insensitive_backdrop_breeze", csc["disabled"]["view"].foreground(KCS::PositiveText).color()},
        { "error_color_insensitive_backdrop_breeze", csc["disabled"]["view"].foreground(KCS::NegativeText).color()},

        /*
         * Ignorant Colors (These colors do not care about backdrop or insensitive states)
         */

        { "link_color_breeze", csc["active"]["view"].foreground(KCS::LinkText).color()},
        { "link_visited_color_breeze", csc["active"]["view"].foreground(KCS::VisitedText).color()},

        { "tooltip_text_breeze", tooltipForegroundColor},
        { "tooltip_background_breeze", tooltipBackgroundColor},
        { "tooltip_border_breeze", tooltipBorderColor},

        { "content_view_bg_breeze", csc["active"]["view"].background(KCS::NormalBackground).color()},

        { "theme_titlebar_background_breeze", windowManagerConfig.readEntry("activeBackground", QColor())},
        { "theme_titlebar_foreground_breeze", windowManagerConfig.readEntry("activeForeground", QColor())},
        { "theme_titlebar_background_light_breeze", csc["active"]["window"].background(KCS::NormalBackground).color()},
        { "theme_titlebar_foreground_backdrop_breeze", windowManagerConfig.readEntry("inactiveForeground", QColor())},
        { "theme_titlebar_background_backdrop_breeze", windowManagerConfig.readEntry("inactiveBackground", QColor())},
        { "theme_titlebar_foreground_insensitive_breeze", windowManagerConfig.readEntry("inactiveForeground", QColor())},
        { "theme_titlebar_foreground_insensitive_backdrop_breeze", windowManagerConfig.readEntry("inactiveForeground", QColor())},

        // Titlebar colors
        { "theme_titlebar_background_breeze", windowManagerConfig.readEntry("activeBackground", QColor())},
        { "theme_titlebar_foreground_breeze", windowManagerConfig.readEntry("activeForeground", QColor())},
        { "theme_titlebar_background_light_breeze", csc["active"]["window"].background(KCS::NormalBackground).color()},
        { "theme_titlebar_foreground_backdrop_breeze", windowManagerConfig.readEntry("inactiveForeground", QColor())},
        { "theme_titlebar_background_backdrop_breeze", windowManagerConfig.readEntry("inactiveBackground", QColor())},
        { "theme_titlebar_foreground_insensitive_breeze", windowManagerConfig.readEntry("inactiveForeground", QColor())},
        { "theme_titlebar_foreground_insensitive_backdrop_breeze", windowManagerConfig.readEntry("inactiveForeground", QColor())},
    };
}

QString ConfigValueProvider::windowDecorationButtonsOrderInGtkNotation(const QString &kdeConfigValue) const
{
    QString gtkNotation;

    for (const QChar &buttonAbbreviation : kdeConfigValue) {
        if (buttonAbbreviation == 'X') {
            gtkNotation += QStringLiteral("close,");
        } else if (buttonAbbreviation == 'I') {
            gtkNotation += QStringLiteral("minimize,");
        } else if (buttonAbbreviation == 'A') {
            gtkNotation += QStringLiteral("maximize,");
        } else if (buttonAbbreviation == 'M') {
            gtkNotation += QStringLiteral("icon,");
        }
    }
    gtkNotation.chop(1);

    return gtkNotation;
}
