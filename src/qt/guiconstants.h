// Copyright (c) 2011-2016 The Bitcoin Core developers
// Copyright (c) 2022 The Junkcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef BITCOIN_QT_GUICONSTANTS_H
#define BITCOIN_QT_GUICONSTANTS_H

#include "validation.h"
#include <QColor>

/* Milliseconds between model updates */
static const int MODEL_UPDATE_DELAY = 250;

/* AskPassphraseDialog -- Maximum passphrase length */
static const int MAX_PASSPHRASE_SIZE = 1024;

/* BitcoinGUI -- Size of icons in status bar */
static const int STATUSBAR_ICONSIZE = 16;

static const bool DEFAULT_SPLASHSCREEN = true;

/* Theme colors */
static const QColor THEME_NAVY_BLUE(26, 32, 44);
static const QColor THEME_GOLD(218, 165, 32);
static const QColor THEME_TEXT(255, 255, 255);
static const QColor THEME_BORDER(180, 180, 180); /* Light gray for borders and lines */
static const QColor THEME_CHECKBOX(240, 240, 240); /* Brighter white for checkboxes */

/* Font constants */
static const QString FONT_FAMILY = "Roboto";
static const int FONT_SIZE_NORMAL = 10;
static const int FONT_SIZE_LARGE = 12;
static const int FONT_SIZE_SMALL = 9;

/* Invalid field background style */
#define STYLE_INVALID "background:#FF8080"

/* Checkbox and radio button style for better visibility on dark background */
#define STYLE_CHECKBOX_RADIOBUTTON "\
    QCheckBox, QRadioButton, QTreeWidget::item {\
        color: white;\
    }\
    QCheckBox::indicator, QRadioButton::indicator, QTreeWidget::indicator {\
        width: 16px;\
        height: 16px;\
        background-color: white;\
        border: 1px solid gray;\
    }\
    QCheckBox::indicator:checked, QTreeWidget::indicator:checked {\
        background-color: white;\
        border: 1px solid gray;\
    }\
    QCheckBox::indicator:checked, QTreeWidget::indicator:checked {\
        background-color: white;\
        border: 1px solid gray;\
    }\
    QRadioButton::indicator:checked {\
        background-color: white;\
        border: 1px solid gray;\
    }\
"

/* Transaction list -- unconfirmed transaction */
static const QColor COLOR_UNCONFIRMED(128, 128, 128);
/* Transaction list -- negative amount */
static const QColor COLOR_NEGATIVE(255, 88, 88);
/* Transaction list -- bare address (without label) */
#define COLOR_BAREADDRESS QColor(140, 140, 140)
/* Transaction list -- TX status decoration - open until date */
#define COLOR_TX_STATUS_OPENUNTILDATE QColor(64, 64, 255)
/* Transaction list -- TX status decoration - danger, tx needs attention */
#define COLOR_TX_STATUS_DANGER QColor(200, 100, 100)
/* Transaction list -- TX status decoration - default color */
#define COLOR_BLACK QColor(0, 0, 0)

/* Tooltips longer than this (in characters) are converted into rich text,
   so that they can be word-wrapped.
 */
static const int TOOLTIP_WRAP_THRESHOLD = 80;

/* Maximum allowed URI length */
static const int MAX_URI_LENGTH = 255;

/* QRCodeDialog -- size of exported QR Code image */
#define QR_IMAGE_SIZE 300

/* Number of frames in spinner animation */
#define SPINNER_FRAMES 36

#define QAPP_ORG_NAME "JunkCoin"
#define QAPP_ORG_DOMAIN "JUNKCOIN.DOMAIN"
#define QAPP_APP_NAME_DEFAULT "JunkCoin-Qt"
#define QAPP_APP_NAME_TESTNET "JunkCoin-Qt-testnet"

/* One gigabyte (GB) in bytes */
static constexpr uint64_t GB_BYTES{1000000000};

/**
 * Convert configured prune target bytes to displayed GB. Round up to avoid underestimating max disk usage.
 */
constexpr inline int PruneBytestoGB(uint64_t bytes) { return (bytes + GB_BYTES - 1) / GB_BYTES; }

/**
 * Convert displayed prune target GB to configured MiB. Round down so roundtrip GB -> MiB -> GB conversion is stable.
 */
constexpr inline int64_t PruneGBtoMiB(int gb) { return gb * GB_BYTES / 1024 / 1024; }

// Default prune target displayed in GUI.
static constexpr int DEFAULT_PRUNE_TARGET_GB{PruneBytestoGB(MIN_DISK_SPACE_FOR_BLOCK_FILES)};

#endif // BITCOIN_QT_GUICONSTANTS_H
