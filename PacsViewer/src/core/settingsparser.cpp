/*************************************************************************************
  Copyright (C) 2014 Laboratori de Gràfics i Imatge, Universitat de Girona &
  Institut de Diagnòstic per la Imatge.
  Girona 2014. All rights reserved.
  http://starviewer.udg.edu

  This file is part of the Starviewer (Medical Imaging Software) open source project.
  It is subject to the license terms in the LICENSE file found in the top-level
  directory of this distribution and at http://starviewer.udg.edu/license. No part of
  the Starviewer (Medical Imaging Software) open source project, including this file,
  may be copied, modified, propagated, or distributed except according to the
  terms contained in the LICENSE file.
 *************************************************************************************/

#include "settingsparser.h"

// localhostname, ip
#include <QHostInfo>
#include <QRegExp>
#include <QDir>
// For the systemEnvironment
#include <QProcess>
#include "logging.h"

namespace udg {

// Delimiting character for keywords
const QChar delimiterChar('%');

SettingsParser::SettingsParser()
{
    initializeParseableStringsTable();
}

SettingsParser::~SettingsParser()
{
}

QString SettingsParser::parse(const QString &stringToParse)
{
    // We will construct a regular expression that will find the defined keys.
    // It will also take into account whether the key is accompanied by a mask suffix
    // and will apply it in case you have one
    QRegExp regExp;

    // We get the existing keys and put them together into a single string to form the regular expression% KEY1% |% KEY2% | ... |% KEYN%
    QStringList keys = m_parseableStringsTable.uniqueKeys();
    QString keysPattern = "(%" + keys.join("%|%") + "%)";
    // Màscara de truncatge [n:c], on 'c' pot ser qualsevol caràcter o res, excepte un whitespace (\S==non-whitespace character)
    QString maskPattern = "(\\[\\d+:\\S?\\])?";
    // ALL the application of the mask assumes that we truncate from "right to left" (right Justified) and that we do the padding as a prefix of
    // the truncated string. This behavior could be made more flexible by adding more parameters to the truncation expression to indicate
    // if truncation and / or padding are done on the right or left

    // Regular expression: Any of the keys, which can be optionally accompanied by a truncation mask
    regExp.setPattern(keysPattern + maskPattern);

    // String on anirem parsejant els resultats
    QString parsedString = stringToParse;
    // índex de l'string on comença el patró trobat
    int keyIndex = 0;
    // Key found
    QString capturedKey;
    // Mask found
    QString capturedMask;
    // Key we want to replace
    QString keyToReplace;
    // String parsing the mask
    QString maskedString;
    // Number of characters to truncate --- >> instead of truncate, put width
    int truncate = 0;
    // Character with which we will paddle
    QChar paddingChar;

    // As long as there are expressions, we capture and parse them
    // The "replace" is done one by one, as we can have repeated keys and it is necessary to do it step by step,
    // as we treat each regular expression
    while ((keyIndex = regExp.indexIn(parsedString)) != -1)
    {
        // La clau trobada, 1a part de l'expressió regular
        capturedKey = regExp.cap(1);
        // La màscara trobada, 2a part de l'expressió regular
        capturedMask = regExp.cap(2);
        // Li eliminem els '%'
        keyToReplace = QString(capturedKey).replace("%", "");
        // Si s'ha trobat sufix de màscara, el parsejem
        if (!capturedMask.isEmpty())
        {
            // Obtenim les sub-parts de la màscara ([n:c])
            QRegExp maskRegExp("\\[(\\d+):(\\S)?\\]");
            if (maskRegExp.indexIn(capturedMask) != -1)
            {
                // Nombre de caràcters a truncar
                truncate = maskRegExp.cap(1).toInt();
                // Trunquem
                maskedString = QString(m_parseableStringsTable.value(keyToReplace)).right(truncate);
                // Si hi ha caràcter de padding, tractem de fer el padding
                if (!maskRegExp.cap(2).isEmpty())
                {
                    // Caràcter de padding
                    paddingChar = maskRegExp.cap(2).at(0);
                    maskedString = maskedString.rightJustified(truncate, paddingChar);
                }
                // Substituim el valor a parsejar i la màscara
                parsedString.replace(keyIndex, capturedKey.size() + capturedMask.size(), maskedString);
            }
            else
            {
                DEBUG_LOG("EP! Hem comés algun error de sintaxi amb l'expressió regular!");
            }
        }
        // Altrament, substituim únicament la clau
        else
        {
            parsedString.replace(keyIndex, capturedKey.size(), m_parseableStringsTable.value(keyToReplace));
        }
    }

    return parsedString;
}

void SettingsParser::initializeParseableStringsTable()
{
    // We fill in the values of the different keywords
    QString localHostName = QHostInfo::localHostName();
    m_parseableStringsTable["HOSTNAME"] = localHostName;

    // Obtaining the ip
    QStringList ipV4Addresses = getLocalHostIPv4Addresses();
    QString ip;
    if (!ipV4Addresses.isEmpty())
    {
        // We assume that the first one in the list is the good IP
        ip = ipV4Addresses.first();

        m_parseableStringsTable["IP"] = ip;

        // "Split" ip prefixes
        QStringList ipParts = ip.split(".");
        // This should never fail as the list of IPs must contain correctly
        // formatted values as these have been previously validated..
        if (ipParts.count() == 4)
        {
            m_parseableStringsTable["IP.1"] = ipParts.at(0);
            m_parseableStringsTable["IP.2"] = ipParts.at(1);
            m_parseableStringsTable["IP.3"] = ipParts.at(2);
            m_parseableStringsTable["IP.4"] = ipParts.at(3);
        }
    }
    else
    {
        // We have no IP address
        m_parseableStringsTable["IP"] = "N/A";
        m_parseableStringsTable["IP.1"] = "[N/A]";
        m_parseableStringsTable["IP.2"] = "[N/A]";
        m_parseableStringsTable["IP.3"] = "[N/A]";
        m_parseableStringsTable["IP.4"] = "[N/A]";
        WARN_LOG("No IPv4 address was recognized on the computer.");
    }

    // Home path
    m_parseableStringsTable["HOMEPATH"] = QDir::homePath();

    // Username
    QStringList environmentList = QProcess::systemEnvironment();
    // Windows
    int index = environmentList.indexOf("USERNAME");
    if (index != -1)
    {
        m_parseableStringsTable["USERNAME"] = environmentList.at(index);
    }
    else
    {
        // Linux, Mac
        index = environmentList.indexOf("USER");
        if (index != -1)
        {
            m_parseableStringsTable["USERNAME"] = environmentList.at(index);
        }
    }
}

bool SettingsParser::isIPv4Address(const QString &ipAddress)
{
    QString zeroTo255Range("([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])");
    QRegExp ipv4AddressRegularExpression("^" + zeroTo255Range + "\\." + zeroTo255Range + "\\." + zeroTo255Range + "\\." + zeroTo255Range + "$");

    return ipv4AddressRegularExpression.exactMatch(ipAddress);
}

QStringList SettingsParser::getLocalHostIPv4Addresses()
{
    QStringList ipV4List;

    QHostInfo hostInfo = QHostInfo::fromName(QHostInfo::localHostName());
    /// TODO You could also choose to use QNetworkInterface :: allAddresses (),
    /// although it returns the address 127.0.0.1 in addition,
    /// instead of using hostInfo.addresses ()
    foreach (const QHostAddress &ip, hostInfo.addresses())
    {
        QString ipString = ip.toString();
        if (isIPv4Address(ipString))
        {
            ipV4List << ipString;
            DEBUG_LOG(ipString + " -> It is a valid IPv4 address");
        }
        else
        {
            DEBUG_LOG(ipString + " -> It is NOT a valid IPv4 address");
        }
    }

    return ipV4List;
}

} // End namespace udg
