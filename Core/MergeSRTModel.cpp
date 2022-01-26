// The MIT License( MIT )
//
// Copyright( c ) 2020-2021 Scott Aron Bloom
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files( the "Software" ), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions :
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "MergeSRTModel.h"
#include "LanguageInfo.h"
#include "Preferences.h"
#include "SABUtils/FileUtils.h"

#include "SABUtils/DoubleProgressDlg.h"

#include <QDebug>
#include <QDir>
#include <QTimer>
#include <QTreeView>
#include <algorithm>

namespace NMediaManager
{
    namespace NCore
    {
        CMergeSRTModel::CMergeSRTModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CMergeSRTModel::~CMergeSRTModel()
        {
        }

        QList< QStandardItem * > CMergeSRTModel::getChildMKVFiles( const QStandardItem * item, bool goBelowDirs ) const
        {
            if ( !item )
                return {};
            auto childCount = item->rowCount();
            QList< QStandardItem * > retVal;
            for ( int ii = 0; ii < childCount; ++ii )
            {
                auto child = item->child( ii );
                if ( !child )
                    continue;

                if ( child->data( ECustomRoles::eIsDir ).toBool() )
                {
                    if ( goBelowDirs )
                        retVal << getChildMKVFiles( child, true );
                    continue;
                }

                if ( CPreferences::instance()->isMediaFile( child->data( ECustomRoles::eFullPathRole ).toString() ) )
                    retVal << child;
            }
            return retVal;
        }

        std::list< QStandardItem * > CMergeSRTModel::getChildFiles(const QStandardItem * item, const QString & ext ) const
        {
            auto childCount = item->rowCount();
            std::list< QStandardItem * > retVal;
            for (int ii = 0; ii < childCount; ++ii)
            {
                auto child = item->child(ii);
                if (!child)
                    continue;

                if (child->data(ECustomRoles::eIsDir).toBool())
                    continue;

                auto fullPath = child->data(ECustomRoles::eFullPathRole).toString();
                if (QFileInfo(fullPath).suffix() != ext)
                    continue;
                retVal.push_back(child);
            }
            return retVal;
        }

        std::unordered_map< QString, std::vector< QStandardItem * > > CMergeSRTModel::getChildSRTFiles( const QStandardItem * item, bool sort ) const
        {
            if ( !item )
                return {};

            auto childCount = item->rowCount();
            auto srtFiles = getChildFiles(item, "srt");
            //qDebug() << item->text() << childCount;
            std::unordered_map< QString, std::vector< std::pair< QStandardItem *, bool > > > tmp; // language to <item,bool>
            std::unordered_set< QString > uniqueSRTFiles;
            for ( auto && ii : srtFiles )
            {
                auto fullPath = ii->data(ECustomRoles::eFullPathRole).toString();
                bool isLangFormat;
                if ( !isSubtitleFile(fullPath, &isLangFormat) )
                    continue;

                auto languageItem = getLanguageItem( ii );
                tmp[languageItem->text()].emplace_back(ii, isLangFormat);
            }

            bool allLangFiles = true;
            std::unordered_map< QString, std::vector< QStandardItem * > > retVal;
            for ( auto && ii : tmp )
            {
                std::vector< QStandardItem * > curr;
                for ( auto && jj : ii.second )
                {
                    curr.push_back( jj.first );
                    allLangFiles = allLangFiles && jj.second;
                }
                retVal[ii.first] = curr;
            }

            if ( sort )
            {
                for ( auto && ii : retVal )
                {
                    std::sort( ii.second.begin(), ii.second.end(),
                               [ ]( const QStandardItem * lhs, const QStandardItem * rhs )
                    {
                        auto lhsFI = QFileInfo( lhs->data( ECustomRoles::eFullPathRole ).toString() );
                        auto rhsFI = QFileInfo( rhs->data( ECustomRoles::eFullPathRole ).toString() );

                        return lhsFI.size() < rhsFI.size();
                    } );
                }
            }
            return retVal;
        }

        void CMergeSRTModel::autoDetermineLanguageAttributes( QStandardItem * mediaFileNode )
        {
            //if ( mediaFileNode )
            //    qDebug() << mediaFileNode->text();
            if ( !mediaFileNode || !isMediaFile( mediaFileNode ) )
                return;

            auto srtFiles = getChildSRTFiles( mediaFileNode, true ); // map of language to files sorted by filesize
            if (!srtFiles.empty())
            {
                for (auto && ii : srtFiles)
                {
                    if (ii.second.size() == 3)
                    {
                        // smallest is "Forced"
                        // mid is normal
                        // largest is "SDH"
                        setChecked(getItem(ii.second[0], EColumns::eForced), ECustomRoles::eForcedSubTitleRole, true);
                        setChecked(getItem(ii.second[2], EColumns::eSDH), ECustomRoles::eHearingImparedRole, true);
                    }
                    else if (ii.second.size() == 2)
                    {
                        // if 2 
                        // smallest is normal
                        // largest is sdh
                        setChecked(getItem(ii.second[1], EColumns::eSDH), ECustomRoles::eHearingImparedRole, true);
                    }
                    else if (ii.second.size() == 1)
                    {
                        // do nothing
                    }
                }
            }
        }

        std::list< std::pair< QStandardItem *, QStandardItem * > > CMergeSRTModel::pairSubIDX(const std::list< QStandardItem * > & idxFiles, const std::list< QStandardItem * > & subFiles) const
        {
            if (idxFiles.empty() || subFiles.empty() || (idxFiles.size() != subFiles.size()))
                return {};

            std::unordered_map< QString, QStandardItem * > subMap;
            for (auto && ii : subFiles)
            {
                auto baseName = QFileInfo( ii->data(ECustomRoles::eFullPathRole).toString() ).completeBaseName();
                subMap[baseName] = ii;
            }

            std::list< std::pair< QStandardItem *, QStandardItem * > > retVal;
            for (auto && ii : idxFiles)
            {
                auto baseName = QFileInfo(ii->data(ECustomRoles::eFullPathRole).toString()).completeBaseName();
                auto pos = subMap.find(baseName);
                if (pos == subMap.end())
                    continue;
                retVal.emplace_back( std::make_pair( ii, (*pos).second ) );
            }
            return retVal;
        }

        std::list< std::pair< QFileInfo, QFileInfo > > CMergeSRTModel::getIDXSUBFilesInDir(const QDir & dir) const
        {
            //qDebug() << dir.absolutePath();
            std::list< std::pair< QFileInfo, QFileInfo > > retVal;

            auto subFiles = dir.entryInfoList(QStringList() << "*.sub");
            std::unordered_map< QString, QFileInfo > subMap;
            for (auto && ii : subFiles)
            {
                auto baseName = ii.completeBaseName();
                subMap[baseName] = ii;
            }

            auto idxFiles = dir.entryInfoList(QStringList() << "*.idx");
            for (auto && ii : idxFiles)
            {
                auto baseName = ii.completeBaseName();
                auto pos = subMap.find(baseName);
                if ( pos == subMap.end() )
                    continue;

                retVal.emplace_back( std::make_pair(ii, (*pos).second) );
            }

            auto subDirs = dir.entryInfoList(QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot);
            for (auto && ii : subDirs)
            {
                auto sub = getIDXSUBFilesInDir( QDir(ii.absoluteFilePath() ) );
                retVal.insert(retVal.end(), sub.begin(), sub.end());
            }
            return retVal;
        }

        std::optional< std::pair< QFileInfo, QFileInfo > > CMergeSRTModel::getIDXSUBFilesForMKV(const QFileInfo & fi) const
        {
            if (!fi.exists() || !fi.isFile())
                return {};

            auto dir = fi.absoluteDir();
            //qDebug().noquote().nospace() << "Finding SRT files for '" << getDispName( fi ) << "' in dir '" << getDispName( dir.absolutePath() ) << "'";
            auto idxSubPairs = getIDXSUBFilesInDir( dir );

            if (idxSubPairs.empty() )
                return {};
            if (idxSubPairs.size() == 1 )
                return idxSubPairs.front();

            std::map< QString, QFileInfo > nameBasedMap;
            for (auto && ii : idxSubPairs)
            {
                if (ii.first.completeBaseName().compare(fi.completeBaseName(), Qt::CaseInsensitive) == 0)
                    return ii;
            }
            return {};
        }

        QFileInfoList CMergeSRTModel::getSRTFilesInDir( const QDir & dir ) const
        {
            //qDebug() << dir.absolutePath();

            auto srtFiles = dir.entryInfoList( QStringList() << "*.srt" );

            auto subDirs = dir.entryInfoList( QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot );
            for ( auto && ii : subDirs )
            {
                srtFiles << getSRTFilesInDir( QDir( ii.absoluteFilePath() ) );
            }
            return srtFiles;
        }

        QList< QFileInfo > CMergeSRTModel::getSRTFilesForMKV( const QFileInfo & fi ) const
        {
            if ( !fi.exists() || !fi.isFile() )
                return {};

            auto dir = fi.absoluteDir();
            //qDebug().noquote().nospace() << "Finding SRT files for '" << getDispName( fi ) << "' in dir '" << getDispName( dir.absolutePath() ) << "'";
            auto srtFiles = getSRTFilesInDir( dir );

            //qDebug().noquote().nospace() << "Found '" << srtFiles.count() << "' SRT Files";

            if ( srtFiles.size() <= 1 )
                return srtFiles;

            // could be 2_lang, 3_lang etc - return them all
            // or could be name.srt (common for TV shows)
            // or 2_NAME 3_name

            QList< QFileInfo > retVal;
            QList< QFileInfo > namebasedFiles;
            QList< QFileInfo > languageFiles;
            QList< QFileInfo > unknownFiles;

            std::map< QString, QFileInfo > nameBasedMap;
            for (auto && ii : srtFiles)
            {
                auto langInfo = SLanguageInfo(ii);
                if ( langInfo.knownLanguage() && ( langInfo.baseName() != ii.completeBaseName() ) )
                    nameBasedMap[langInfo.baseName()] = ii;
            }
            auto pos = nameBasedMap.find(fi.completeBaseName());
            if (pos != nameBasedMap.end())
            {
                namebasedFiles << (*pos).second;
            }
            else if ( nameBasedMap.size() != srtFiles.size() ) // there are no non-named based srt files
            {
                for (auto && ii : srtFiles)
                {
                    //qDebug().noquote().nospace() << "Checking '" << getDispName( ii ) << "'";
                    //qDebug().noquote().nospace() << "Checking '" << fi.completeBaseName() << "' against '" << ii.completeBaseName() << "'";
                    auto langInfo = SLanguageInfo(ii);
                    if (langInfo.isNameBasedLangFile())
                        continue;
                    if (langInfo.knownLanguage())
                    {
                        //qDebug().noquote().nospace() << "'" << getDispName( ii ) << "' is LANGUAGE based";
                        languageFiles.push_back(ii);
                    }
                    else
                    {
                        //qDebug().noquote().nospace() << "'" << getDispName( ii ) << "' is UNKNOWN type of filename";
                        unknownFiles.push_back(ii);
                    }
                }
            }
            return namebasedFiles << languageFiles;
        }

        QStandardItem * CMergeSRTModel::processSUBIDXSubTitle(QStandardItem * mkvFile, const std::list< std::pair< QStandardItem *, QStandardItem * > > & subidxFiles, QStandardItem * parentItem, bool displayOnly) const
        {
            SProcessInfo processInfo;
            processInfo.fSetMKVTagsOnSuccess = true;
            processInfo.fOldName = computeTransformPath(mkvFile, true);
            auto oldFI = QFileInfo(processInfo.fOldName);

            processInfo.fNewName = oldFI.absoluteDir().absoluteFilePath(oldFI.fileName() + ".new");// prevents the emby system from picking it up
            processInfo.fItem = new QStandardItem(QString("'%1' => '%2'").arg(getDispName(processInfo.fOldName)).arg(getDispName(processInfo.fNewName)));
            if (parentItem)
                parentItem->appendRow(processInfo.fItem);
            else
                fProcessResults.second->appendRow(processInfo.fItem);

            std::list< std::pair< std::pair< QStandardItem *, QStandardItem * >, SLanguageInfo > > allLangInfos;
            std::unordered_map<  QStandardItem *, std::unordered_map< int, std::pair< bool, bool > > > langMap;
            for (auto && ii : subidxFiles)
            {
                auto idxItem = new QStandardItem(tr("IDX File: %1").arg(ii.first->text()));
                processInfo.fItem->appendRow(idxItem);
                auto path = ii.first->data(ECustomRoles::eFullPathRole).toString();
                auto langInfo = SLanguageInfo(QFileInfo(path));
                allLangInfos.emplace_back( std::make_pair(ii, langInfo ));
                auto langs = langInfo.allLanguageInfos();

                for (auto && currLang : langs)
                {
                    auto dispName = currLang.first;
                    auto && currLangInfos = currLang.second;

                    for (size_t fileNum = 0; fileNum < currLangInfos.size(); ++fileNum)
                    {
                        auto && lang = currLangInfos[fileNum].fLanguage;
                        auto index = currLangInfos[fileNum].fIndex;

                        auto label = tr("'%1' Index: %2 - Forced : %4 SDH : %5")
                            .arg(lang->displayName())
                            .arg(index);

                        if (currLangInfos.size() == 3)
                        {
                            if (fileNum == 0)
                            {
                                label = label.arg("Yes").arg("No");
                                langMap[ii.first][index] = std::make_pair(true, false);
                            }
                            else if (fileNum == 1)
                            {
                                label = label.arg("No").arg("No");
                                langMap[ii.first][index] = std::make_pair(false, false);
                            }
                            else
                            {
                                label = label.arg("No").arg("Yes");
                                langMap[ii.first][index] = std::make_pair(false, true);
                            }
                        }
                        else if (currLangInfos.size() == 2)
                        {
                            if (fileNum == 0)
                            {
                                label = label.arg("No").arg("No");
                                langMap[ii.first][index] = std::make_pair(false, false);
                            }
                            else
                            {
                                label = label.arg("No").arg("Yes");
                                langMap[ii.first][index] = std::make_pair(false, true);
                            }
                        }
                        else
                        {
                            label = label.arg("No").arg("No");
                            langMap[ii.first][index] = std::make_pair(false, false);
                        }
                        auto langItem = new QStandardItem(label);
                        idxItem->appendRow(langItem);
                    }
                }
            }

            if (!displayOnly)
            {
                processInfo.fCmd = CPreferences::instance()->getMKVMergeEXE();
                bool aOK = true;
                if (processInfo.fCmd.isEmpty() || !QFileInfo(processInfo.fCmd).isExecutable())
                {
                    QStandardItem * errorItem = nullptr;
                    if (processInfo.fCmd.isEmpty())
                        errorItem = new QStandardItem(QString("ERROR: mkvmerge is not set properly"));
                    else
                        errorItem = new QStandardItem(QString("ERROR: mkvmerge '%1' is not an executable").arg(processInfo.fCmd));

                    errorItem->setData(ECustomRoles::eIsErrorNode, true);
                    appendError(processInfo.fItem, errorItem);

                    QIcon icon;
                    icon.addFile(QString::fromUtf8(":/resources/error.png"), QSize(), QIcon::Normal, QIcon::Off);
                    errorItem->setIcon(icon);
                    aOK = false;
                }

                aOK = aOK && checkProcessItemExists(processInfo.fOldName, processInfo.fItem);
                for (auto && ii : subidxFiles)
                {
                    if (!aOK)
                        break;
                    aOK = aOK && checkProcessItemExists(ii.first->data(ECustomRoles::eFullPathRole).toString(), processInfo.fItem);
                    aOK = aOK && checkProcessItemExists(ii.second->data(ECustomRoles::eFullPathRole).toString(), processInfo.fItem);
                }

                 //aOK = the MKV and SRT exist and the cmd is an executable
                processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps(processInfo.fOldName);

                processInfo.fArgs = QStringList()
                    << "--ui-language" << "en"
                    << "--priority" << "lower"
                    << "--output" << processInfo.fNewName
                    << "--language" << "0:en"
                    << "--language" << "1:en"
                    << "--language" << "3:en"
                    << "(" << processInfo.fOldName << ")"
                    << "--title" << oldFI.completeBaseName()
                    ;

                QStringList trackOrder = { "0:0", "0:1", "0:3" };
                int langFileCnt = 1;
                for (auto && langInfo : allLangInfos)
                {
                    int nextTrack = 1;
                    std::list< SMultLangInfo > orderByIdx;
                    auto currIDX = langInfo.first.first->data(ECustomRoles::eFullPathRole).toString();
                    auto currSUB = langInfo.first.second->data(ECustomRoles::eFullPathRole).toString();
                    auto langs = langInfo.second.allLanguageInfos();
                    for (auto && ii : langs)
                    {
                        for (auto && jj : ii.second)
                            orderByIdx.push_back(jj);
                    }

                    orderByIdx.sort(
                        [](const SMultLangInfo & lhs, const SMultLangInfo & rhs)
                        {
                            return lhs.fIndex < rhs.fIndex;
                        });

                    for (auto && ii : orderByIdx)
                    {
                        bool sdh = false;
                        bool forced = false;

                        auto langPos = langMap.find(langInfo.first.first);
                        if (langPos != langMap.end())
                        {
                            auto indexPos = (*langPos).second.find(ii.fIndex);
                            if (indexPos != (*langPos).second.end())
                            {
                                sdh = (*indexPos).second.first;
                                forced = (*indexPos).second.second;
                            }
                        }
                        processInfo.fArgs
                            << "--language"
                            << QString("%1:%2").arg(ii.fIndex).arg(ii.fLanguage->isoCode())
                            << "--default-track"
                            << QString( "%1:%2" ).arg( ii.fIndex ).arg( "no" )
                            << "--hearing-impaired-flag"
                            << QString("%1:%2").arg(ii.fIndex).arg(sdh ? "yes" : "no")
                            << "--forced-track"
                            << QString("%1:%2").arg(ii.fIndex).arg(forced ? "yes" : "no")
                            ;
                        trackOrder << QString("%1:%2").arg( langFileCnt ).arg(ii.fIndex);
                    }
                    processInfo.fArgs
                        << "(" << currIDX << ")"
                        << "(" << currSUB << ")"
                        ;
                    processInfo.fAncillary.push_back(currIDX);
                    processInfo.fAncillary.push_back(currSUB);

                    langFileCnt++;
                }
                processInfo.fArgs << "--track-order" << trackOrder.join(",");
                fProcessQueue.push_back(processInfo);
                QTimer::singleShot(0, this, &CMergeSRTModel::slotRunNextProcessInQueue);
            }
            return processInfo.fItem;
        }

        QStandardItem * CMergeSRTModel::processSRTSubTitle(QStandardItem * mkvFile, const std::unordered_map< QString, std::vector< QStandardItem * > > & srtFiles, QStandardItem * parentItem, bool displayOnly ) const
        {
            SProcessInfo processInfo;
            processInfo.fSetMKVTagsOnSuccess = true;
            processInfo.fOldName = computeTransformPath(mkvFile, true);
            auto oldFI = QFileInfo(processInfo.fOldName);

            processInfo.fNewName = oldFI.absoluteDir().absoluteFilePath(oldFI.fileName() + ".new");// prevents the emby system from picking it up

            processInfo.fItem = new QStandardItem(QString("'%1' => '%2'").arg(getDispName(processInfo.fOldName)).arg(getDispName(processInfo.fNewName)));
            if (parentItem)
                parentItem->appendRow(processInfo.fItem);
            else
                fProcessResults.second->appendRow(processInfo.fItem);

            for (auto && ii : srtFiles)
            {
                auto languageItem = new QStandardItem(tr("Language: %1").arg(ii.first));
                processInfo.fItem->appendRow(languageItem);
                for (auto && jj : ii.second)
                {
                    auto defaultItem = getItem(jj, EColumns::eOnByDefault);
                    auto forcedItem = getItem(jj, EColumns::eForced);
                    auto sdhItem = getItem(jj, EColumns::eSDH);

                    auto srtFileItem = new QStandardItem(tr("'%2' - Default: %3 Forced : %4 SDH : %5")
                        .arg(jj->text())
                        .arg((defaultItem && defaultItem->checkState() == Qt::Checked) ? "Yes" : "No")
                        .arg((forcedItem && forcedItem->checkState() == Qt::Checked) ? "Yes" : "No")
                        .arg((sdhItem && sdhItem->checkState() == Qt::Checked) ? "Yes" : "No"));
                    languageItem->appendRow(srtFileItem);
                }
            }

            if (!displayOnly)
            {
                processInfo.fCmd = CPreferences::instance()->getMKVMergeEXE();
                bool aOK = true;
                if (processInfo.fCmd.isEmpty() || !QFileInfo(processInfo.fCmd).isExecutable())
                {
                    QStandardItem * errorItem = nullptr;
                    if (processInfo.fCmd.isEmpty())
                        errorItem = new QStandardItem(QString("ERROR: mkvmerge is not set properly"));
                    else
                        errorItem = new QStandardItem(QString("ERROR: mkvmerge '%1' is not an executable").arg(processInfo.fCmd));

                    errorItem->setData(ECustomRoles::eIsErrorNode, true);
                    appendError(processInfo.fItem, errorItem);

                    QIcon icon;
                    icon.addFile(QString::fromUtf8(":/resources/error.png"), QSize(), QIcon::Normal, QIcon::Off);
                    errorItem->setIcon(icon);
                    aOK = false;
                }
                aOK = aOK && checkProcessItemExists(processInfo.fOldName, processInfo.fItem);
                for (auto && ii : srtFiles)
                {
                    if (!aOK)
                        break;
                    for (auto && jj : ii.second)
                    {
                        aOK = aOK && checkProcessItemExists(jj->data(ECustomRoles::eFullPathRole).toString(), processInfo.fItem);
                    }
                }
                // aOK = the MKV and SRT exist and the cmd is an executable
                processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps(processInfo.fOldName);


                processInfo.fArgs = QStringList()
                    << "--ui-language" << "en"
                    << "--priority" << "lower"
                    << "--output" << processInfo.fNewName
                    << "--language" << "0:en"
                    << "--language" << "1:en"
                    << "(" << processInfo.fOldName << ")"
                    << "--title" << oldFI.completeBaseName()
                    ;
                QStringList trackOrder = { "0:0", "0:1" };
                int nextTrack = 1;
                for (auto && ii : srtFiles)
                {
                    for (auto && jj : ii.second)
                    {
                        //qDebug() << jj->text();
                        auto langItem = getItem(jj, EColumns::eLanguage);
                        auto srtFile = jj->data(ECustomRoles::eFullPathRole).toString();
                        processInfo.fArgs
                            << "--language"
                            << "0:" + langItem->data(ECustomRoles::eISOCodeRole).toString()
                            << "--default-track"
                            << "0:" + QString(jj->data(ECustomRoles::eDefaultTrackRole).toBool() ? "yes" : "no")
                            << "--hearing-impaired-flag"
                            << "0:" + QString(jj->data(ECustomRoles::eHearingImparedRole).toBool() ? "yes" : "no")
                            << "--forced-track"
                            << "0:" + QString(jj->data(ECustomRoles::eForcedSubTitleRole).toBool() ? "yes" : "no")
                            << "(" << srtFile << ")"
                            ;
                        processInfo.fAncillary.push_back(srtFile);
                        trackOrder << QString("%1:0").arg(nextTrack++);
                    }
                }
                processInfo.fArgs << "--track-order" << trackOrder.join(",");
                fProcessQueue.push_back(processInfo);
                QTimer::singleShot(0, this, &CMergeSRTModel::slotRunNextProcessInQueue);
            }
            return processInfo.fItem;
        }

        std::pair< bool, QStandardItem * > CMergeSRTModel::processItem( const QStandardItem * item, QStandardItem * parentItem, bool displayOnly ) const
        {
            if ( !item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, nullptr );

            auto mkvFiles = getChildMKVFiles( item, false );
            if ( mkvFiles.empty() )
                return std::make_pair( true, nullptr );

            bool aOK = true;
            QStandardItem * myItem = nullptr;
            fFirstProcess = true;
            for ( auto && mkvFile : mkvFiles )
            {
                auto srtFiles = getChildSRTFiles( mkvFile, false );
                if ( !srtFiles.empty() )
                    myItem = processSRTSubTitle( mkvFile, srtFiles, parentItem, displayOnly );
                else
                {
                    auto idxFiles = getChildFiles(mkvFile, "idx");
                    auto subFiles = getChildFiles(mkvFile, "sub");
                    auto subIDXPairFiles = pairSubIDX(idxFiles, subFiles);
                    if (!subIDXPairFiles.empty())
                        myItem = processSUBIDXSubTitle(mkvFile, subIDXPairFiles, parentItem, displayOnly);
                }
            }
            if ( mkvFiles.count() > 1 )
                return std::make_pair( aOK, myItem );
            else
                return std::make_pair( aOK, parentItem );
        }

        QStandardItem * CMergeSRTModel::getLanguageItem( const QStandardItem * item ) const
        {
            return getItem( item, EColumns::eLanguage );
        }

        bool CMergeSRTModel::postExtProcess( const SProcessInfo & info, QStringList & msgList )
        {
            bool aOK = CDirModel::postExtProcess( info, msgList );
            QStringList retVal;
            if ( aOK && !QFile::rename( info.fNewName, info.fOldName ) )
            {
                msgList << QString( "ERROR: %1: FAILED TO MOVE ITEM TO %2" ).arg( getDispName( info.fNewName ) ).arg( getDispName( info.fOldName ) );
                aOK = false;
            }

            return aOK;
        }

        bool CMergeSRTModel::isSubtitleFile(const QFileInfo & fileInfo, bool * isLangFileFormat) const
        {
            if (!CDirModel::isSubtitleFile(fileInfo, isLangFileFormat))
                return false;
            return fileInfo.suffix() != "sub";
        }

        std::list< NMediaManager::NCore::SDirNodeItem > CMergeSRTModel::addAdditionalItems( const QFileInfo & fileInfo )const
        {
            std::list< NMediaManager::NCore::SDirNodeItem > retVal;
            if ( fileInfo.isFile() )
            {
                auto language = SLanguageInfo( fileInfo );
                auto langName = language.displayName();
                if (!isSubtitleFile(fileInfo))
                    langName.clear();

                auto languageFileItem = SDirNodeItem( langName, CMergeSRTModel::EColumns::eLanguage );
                if ( !language.isMultiLanguage() )
                    languageFileItem.setData( language.isoCode(), ECustomRoles::eISOCodeRole );
                retVal.push_back( languageFileItem );

                auto forcedItem = SDirNodeItem( QString(), EColumns::eForced );
                forcedItem.fCheckable = true;
                retVal.push_back( forcedItem );

                auto sdhItem = SDirNodeItem( QString(), EColumns::eSDH );
                sdhItem.fCheckable = true;
                retVal.push_back( sdhItem );

                auto onByDefaultItem = SDirNodeItem( QString(), EColumns::eOnByDefault );
                onByDefaultItem.fCheckable = !language.isMultiLanguage();
                retVal.push_back( onByDefaultItem );
            }
            else
            {
                // do nothing for directories
            }
            return retVal;
        }

        void CMergeSRTModel::setupNewItem( const SDirNodeItem & nodeItem, const QStandardItem * nameItem, QStandardItem * item ) const
        {
            if ( (nodeItem.fType == EColumns::eLanguage) && (item->text().isEmpty()) )
            {
                auto path = nameItem->data(ECustomRoles::eFullPathRole).toString();
                if ( isSubtitleFile( path ) )
                    item->setBackground( Qt::red );
            }
        }

        QStringList CMergeSRTModel::headers() const
        {
            return CDirModel::headers() 
                << tr( "Language" ) << tr( "Forced?" ) << tr( "Hearing Impaired?" ) << tr( "On by Default?" );
        }

        void CMergeSRTModel::postLoad( QTreeView * treeView )
        {
            CDirModel::postLoad(treeView);
        }

        void CMergeSRTModel::preLoad(QTreeView * treeView)
        {
            CDirModel::preLoad(treeView);
        }
                
        int CMergeSRTModel::computeNumberOfItems() const
        {
            auto mkvFiles = getChildMKVFiles( invisibleRootItem(), true );
            return mkvFiles.count();
        }

        void CMergeSRTModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& prevParent, const STreeNode & treeNode )
        {
            if ( treeNode.fIsFile )
            {
                auto isSubFile = CPreferences::instance()->isSubtitleFile( treeNode.name() );
                auto useAsParent =
                    [isSubFile, this ]( QStandardItem * item )
                {
                    if ( !item )
                        return true;

                    if ( item->data( ECustomRoles::eIsDir ).toBool() )
                        return true;

                    auto path = item->data( ECustomRoles::eFullPathRole ).toString();
                    if (isSubFile)
                    {
                        return CPreferences::instance()->isMediaFile( path );
                    }

                    return false;
                };

                while ( !useAsParent( prevParent ) )
                    prevParent = prevParent->parent();
            }
        }

        bool CMergeSRTModel::preFileFunction( const QFileInfo & fileInfo, std::unordered_set<QString> & alreadyAdded, TParentTree & tree )
        {
            auto dir = fileInfo.absoluteDir();
            //qDebug() << fileInfo;
            auto srtFiles = getSRTFilesForMKV( fileInfo );
            if (!srtFiles.isEmpty())
            {
                //for ( auto && ii : tree )
                //    qDebug() << ii;
                for (auto && ii : srtFiles)
                {
                    //qDebug() << ii.absoluteFilePath();
                    if (alreadyAdded.find(ii.absoluteFilePath()) != alreadyAdded.end())
                        continue;
                    alreadyAdded.insert(ii.absoluteFilePath());
                    tree.push_back(std::move(getItemRow(ii)));
                }

                //for ( auto && ii : tree )
                //    qDebug() << ii;
                return true;
            }
            auto idxSub = getIDXSUBFilesForMKV(fileInfo);
            if (idxSub.has_value())
            {
                tree.emplace_back(getItemRow(idxSub.value().first));
                tree.emplace_back(getItemRow(idxSub.value().second));
                return true;
            }
            
            return false;
        }

        void CMergeSRTModel::postFileFunction( bool aOK, const QFileInfo & fileInfo )
        {
            if ( aOK && CPreferences::instance()->isMediaFile( fileInfo ) )
            {
                auto pos = fPathMapping.find( fileInfo.absoluteFilePath() );
                if ( pos != fPathMapping.end() )
                {
                    auto mkvItem = (*pos).second;
                    if ( mkvItem )
                    {
                        //qDebug() << rootItem->text() << rootItem->data( ECustomRoles::eFullPathRole ).toString();
                        autoDetermineLanguageAttributes( mkvItem );
                    }
                }
            }
        }

        void CMergeSRTModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }


        QString CMergeSRTModel::getProgressLabel( const SProcessInfo & processInfo ) const
        {
            auto dir = QFileInfo( processInfo.fNewName ).absolutePath();
            auto fname = QFileInfo( processInfo.fOldName ).fileName();

            auto retVal = QString( "Merging MKV %1<ul><li>%2</li>" ).arg( getDispName( dir ) ).arg( fname );
            for ( auto && ii : processInfo.fAncillary )
            {
                auto fname = QFileInfo( ii ).fileName();
                retVal += QString( "<li>%1</li>" ).arg( fname );
            }
            retVal += "</ul>";
            fname = QFileInfo( processInfo.fNewName ).fileName();
            retVal += QString( "to create %1" ).arg( fname );
            return retVal;
        }
    }
}
