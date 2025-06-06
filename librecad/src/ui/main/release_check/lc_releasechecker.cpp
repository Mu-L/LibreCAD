/*
 * ********************************************************************************
 * This file is part of the LibreCAD project, a 2D CAD program
 *
 * Copyright (C) 2025 LibreCAD.org
 * Copyright (C) 2025 sand1024
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 * ********************************************************************************
 */

#include "lc_releasechecker.h"

#include <QJsonArray>
#include <qjsondocument.h>

#include "rs_debug.h"
#include "rs_dialogfactory.h"
#include "rs_dialogfactoryinterface.h"
#include "rs_settings.h"

LC_TagInfo::LC_TagInfo(int majorVer, int minorVer, int revisionNum, int bugfixVer, const QString &labelVer, const QString &tagNameVer):m_major(majorVer), m_minor(minorVer), m_revision(
                                                                                                                                           revisionNum), m_bugfix(bugfixVer), m_label(labelVer), m_tagName(tagNameVer) {
}

QString LC_TagInfo::getLabel() const {
    return m_label;
}

QString LC_TagInfo::getTagName() const {
    return m_tagName;
}


bool LC_TagInfo::isSameVersion(const LC_TagInfo& other) const{
    return m_major == other.m_major && m_minor == other.m_minor && m_revision == other.m_revision && m_bugfix == other.m_bugfix;
}

bool LC_TagInfo::isBefore(const LC_TagInfo& other) const{
    if (m_major < other.m_major){
        return true;
    }
    else if (m_major == other.m_major){
        if (m_minor < other.m_minor){
            return true;
        }
        else if (m_minor == other.m_minor) {
            if (m_revision < other.m_revision) {
                return true;
            }
            else if (m_revision == other.m_revision) {
                if (m_bugfix < other.m_bugfix) {
                    return true;
                }
            }
        }
    }
    return false;
}

LC_ReleaseChecker::LC_ReleaseChecker(const QString& ownTagName, bool ownPreRelease):
    m_ownReleaseInfo(getOwnReleaseInfo(ownTagName, ownPreRelease)){
    connect(&m_WebCtrl, &QNetworkAccessManager::finished, this, &LC_ReleaseChecker::infoReceived);
}

LC_ReleaseInfo LC_ReleaseChecker::getOwnReleaseInfo(const QString& tagName, bool preRelease) const{
    bool draft = false;
    const LC_TagInfo ownTagInfo = parseTagInfo(tagName);
    LC_ReleaseInfo result = LC_ReleaseInfo("", draft, preRelease, "", "");
    result.setTagInfo(ownTagInfo);
    return result;
}

#define TEST_JSON_PARSING_
void LC_ReleaseChecker::checkForNewVersion(bool forceCheck) {
  #ifndef TEST_JSON_PARSING
    m_emitSignalIfNoNewVersion = forceCheck;
    QUrl gitHubReleasesUrl("https://api.github.com/repos/LibreCAD/LibreCAD/releases");
    QNetworkRequest request(gitHubReleasesUrl);
    m_WebCtrl.get(request);
  #else
    QFile file = QFile("c:/Local/LibreCADProjects/releases.json");
    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QByteArray responseContent = file.readAll();
    file.close();
    QString resultStr = QString(responseContent);
//    LC_ERR << resultStr;
    processReleasesJSON(responseContent);
#endif
}

void LC_ReleaseChecker::infoReceived(QNetworkReply *reply) {
    if(reply->error() == QNetworkReply::NoError) {
        QByteArray responseContent = reply->readAll();
        // auto resultStr = QString(responseContent);
        processReleasesJSON(responseContent);
    }
    else{
        RS_DIALOGFACTORY->requestWarningDialog(tr("Sorry, some network error occurred during checking for new version."));
    }
    reply->deleteLater();
}

LC_TagInfo LC_ReleaseChecker::parseTagInfo(const QString &tagName) const {
    const QStringList &split = tagName.split(".");
    qsizetype partsSize = split.size();
    int major = 0;
    int minor = 0;
    int revision = 0;
    int bugfix = 0;
    QString label = "";
    if (partsSize > 1){
        major = split[0].toInt();
        minor = split[1].toInt();
        if (partsSize == 3){
            QString remaining = split[2];
            QStringList other = remaining.split("_");
            if (other.size() == 1){
                other = remaining.split("-");
            }
            if (other.size() == 1){
                bool ok;
                revision = remaining.toInt(&ok);
                if (!ok) {
                    label = remaining;
                }
            }
            else if (other.size() == 2){
                revision = other[0].toInt();
                label = other[1];
            }
        }
        else if (partsSize == 4){
            revision = split[2].toInt();
            QString remaining = split[3];
            QStringList other = remaining.split("_");
            if (other.size() == 1){
                other = remaining.split("-");
            }
            if (other.size() == 1){
                bool ok;
                bugfix = remaining.toInt(&ok);
                if (!ok) {
                    label = remaining;
                }
            }
            else if (other.size() == 2){
                bugfix = other[0].toInt();
                label = other[1];
            }
        }
    }
    LC_TagInfo result = LC_TagInfo(major, minor, revision, bugfix, label, tagName);
    return result;
}

void LC_ReleaseChecker::processReleasesJSON(const QByteArray &responseContent) {
    QJsonParseError error;
    QJsonDocument jsonDocument = QJsonDocument::fromJson(responseContent, &error);
    if (jsonDocument.isEmpty()){
        LC_ERR << error.errorString();
        RS_DIALOGFACTORY->requestWarningDialog(tr("Unable to parse response from the server"));
    }
    else {
        if (jsonDocument.isArray()) {
            QJsonArray array = jsonDocument.array();
            QVector<LC_ReleaseInfo> releases;
            QVector<LC_ReleaseInfo> preReleases;
            LC_TagInfo ownTagInfo = m_ownReleaseInfo.getTagInfo();
            LC_GROUP_GUARD("Startup");
            bool ignorePreReleases = LC_GET_BOOL("IgnorePreReleaseVersions", true);
            QString ignoredReleaseTagStr = LC_GET_STR("IgnoredRelease", "0.0.0.0");
            QString ignoredPreReleaseTag = LC_GET_STR("IgnoredPreRelease", "0.0.0.0");

            LC_TagInfo ignoredRelease = parseTagInfo(ignoredReleaseTagStr);
            LC_TagInfo ignoredPreRelease = parseTagInfo(ignoredPreReleaseTag);
            if (m_ownReleaseInfo.m_isPrerelease){
                ignorePreReleases = false;
            }

            for (const QJsonValue &value: array) {
                const QString &tagName = value["tag_name"].toString();
                const bool draft = value["draft"].toBool();
                const bool prerelease = value["prerelease"].toBool();
                const QString &publishedAt = value["published_at"].toString();
                const QString &body = value["body"].toString();
                const QString &htmlUrl = value["html_url"].toString();

                LC_TagInfo tagInfo = parseTagInfo(tagName);

//                LC_ERR << tagName << " as  | " << tagInfo.major << "." << tagInfo.minor << "." << tagInfo.revision << "." << tagInfo.bugfix<< "-" << tagInfo.label;

                LC_ReleaseInfo releaseInfo = LC_ReleaseInfo(publishedAt, draft, prerelease, htmlUrl, body);
                releaseInfo.setTagInfo(tagInfo);
                // first, filter everything by version numbers
                if (ownTagInfo.isBefore(tagInfo)){
                    // also, if needed, skip pre-release builds
                    if (prerelease){
                        if (ignorePreReleases) {
//                            LC_ERR << "Skipped as ignored pre-release";
                            continue;
                        }
                        else if (tagInfo.isBefore(ignoredPreRelease)){
//                            LC_ERR << "Skipped as before current";
                            continue;
                        }
                        preReleases << releaseInfo;
                    }
                    else{
                        if (tagInfo.isBefore(ignoredRelease)){
//                            LC_ERR << "Skipped as before ignored";
                            continue;
                        }
                        else{
                            releases << releaseInfo;
                        }
                    }
                }
//              LC_ERR << publishedAt << " - as - " << releaseInfo.publishedDate.toString(Qt::ISODate);
            }

            qsizetype availableReleases = releases.size();
            qsizetype availablePreReleases = preReleases.size();

            // it might be that there are several releases with the same version but different suffix (i.e - latest).
            // for sure, in order to distinguish them it's more reliable to check and compare release dates.
            // however, that will require additional magic with embedding release date (similar to LC_Version), so
            // we can avoid such situations just by proper numbering of releases. In other words, each version should be
            // unique (say, different by last "revision" component of version number.

            if (availableReleases > 0){
                sortReleasesInfo(releases);
//                LC_ERR << availableReleases;
                LC_ReleaseInfo &info = releases.last();
                if (!info.getTagInfo().isSameVersion(ignoredRelease) && !info.getTagInfo().isSameVersion(ownTagInfo)) {
                    m_latestRelease = info;
                }
                else{
                    m_latestRelease.m_valid = false;
                    availableReleases = 0;
                }
            }
//            LC_ERR << "=======";
            if (availablePreReleases > 0){
                sortReleasesInfo(preReleases);
//                LC_ERR << availablePreReleases;
                LC_ReleaseInfo &info = preReleases.last();
                if (!info.getTagInfo().isSameVersion(ignoredPreRelease) && !info.getTagInfo().isSameVersion(ownTagInfo)) {
                    m_latestPreRelease = info;
                }
                else {
                    m_latestRelease.m_valid = false;
                    availablePreReleases = 0;
                }
            }
            if (availableReleases > 0 || availablePreReleases > 0 || m_emitSignalIfNoNewVersion){
                emit updatesAvailable();
                m_emitSignalIfNoNewVersion = false;
            }
        }
    }
}

void LC_ReleaseChecker::sortReleasesInfo(QVector<LC_ReleaseInfo> &list) const {
    std::sort(list.begin(), list.end(), [](const LC_ReleaseInfo& v0, const LC_ReleaseInfo& v1) {
        bool result = v0.m_tagInfo.isSameVersion(v1.m_tagInfo);
        if (result){
            // if releases has the same version (mostly pre-releases) - just sort them by date and use the freshest one
            result = v1.m_publishedDate < v0.m_publishedDate;
        }
        else {
            result = v0.m_tagInfo.isBefore(v1.m_tagInfo);
        }
        
        return result;
    });
//    for (auto ri: list){
//        LC_ERR << ri.tagInfo.tagName;
//    }
}

const LC_ReleaseInfo &LC_ReleaseChecker::getLatestRelease() const {
    return m_latestRelease;
}

const LC_ReleaseInfo &LC_ReleaseChecker::getLatestPreRelease() const {
    return m_latestPreRelease;
}
