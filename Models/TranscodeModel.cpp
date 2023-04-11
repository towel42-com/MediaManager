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

#include "TranscodeModel.h"

#include "Core/LanguageInfo.h"

#include "Preferences/Core/Preferences.h"
#include "Preferences/Core/TranscodeNeeded.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/MediaInfo.h"
#include "SABUtils/utils.h"
#include "SABUtils/StringUtils.h"

#include <QDir>
#include <QTimer>

namespace NMediaManager
{
    namespace NModels
    {
        CTranscodeModel::CTranscodeModel( NUi::CBasePage *page, QObject *parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CTranscodeModel::~CTranscodeModel()
        {
        }

        void CTranscodeModel::clear()
        {
            CDirModel::clear();
            fSRTFileCache.clear();
        }

        QStringList CTranscodeModel::dirModelFilter() const
        {
            return NPreferences::NCore::CPreferences::instance()->getVideoExtensions( QStringList() << "*.nfo" );
        }

        std::optional< NMediaManager::NModels::TItemStatus > CTranscodeModel::computeItemStatus( const QModelIndex &idx ) const
        {
            if ( isRootPath( idx ) )
                return {};

            auto mediaInfo = getMediaInfo( idx );
            if ( !mediaInfo )
                return {};

            if ( !mediaInfo->aOK() )
                return {};

            auto transcodeNeeded = NMediaManager::NPreferences::NCore::STranscodeNeeded( mediaInfo );
            auto fileInfo = this->fileInfo( idx );

            if ( idx.column() == 0 )   // filename
            {
                auto msg = transcodeNeeded.getFormatMessage();
                if ( msg.has_value() )
                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg.value() );
            }
            else if ( idx.column() == getMediaVideoCodecLoc() )
            {
                auto msg = transcodeNeeded.getVideoCodecMessage();
                if ( msg.has_value() )
                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg.value() );
            }
            else if ( idx.column() == getMediaAudioCodecLoc() )
            {
                auto msg = transcodeNeeded.getAudioCodecMessage();
                if ( msg.has_value() )
                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg.value() );
            }
            return {};
        }

        QString CTranscodeModel::getProgressLabel( const SProcessInfo &processInfo ) const
        {
            return processInfo.fProgressLabel;
        }

        QStringList CTranscodeModel::headers() const
        {
            return CDirModel::headers() << tr( "Language" ) << tr( "Forced?" ) << tr( "Hearing Impaired?" ) << tr( "On by Default?" ) << getMediaHeaders();
        }

        void CTranscodeModel::postLoad( QTreeView *treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CTranscodeModel::preLoad( QTreeView *treeView )
        {
            CDirModel::preLoad( treeView );
        }

        std::pair< std::function< bool( const QVariant &path ) >, int > CTranscodeModel::getExcludeFuncForItemCount() const
        {
            return std::make_pair(
                []( const QVariant &value )
                {
                    auto retVal = !NPreferences::NCore::CPreferences::instance()->isMediaFile( value.toString() );
                    return retVal;
                },
                ECustomRoles::eAbsFilePath );
        }

        void CTranscodeModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *&prevParent, const STreeNode &treeNode )
        {
            if ( treeNode.fIsFile )
            {
                auto nodePath = treeNode.fullPath();
                auto isSubFile = NPreferences::NCore::CPreferences::instance()->isSubtitleFile( treeNode.fullPath(), false );
                auto useAsParent = [ isSubFile, this ]( QStandardItem *item )
                {
                    if ( !item )
                        return true;

                    if ( item->data( ECustomRoles::eIsDir ).toBool() )
                        return true;

                    auto path = item->data( ECustomRoles::eAbsFilePath ).toString();
                    if ( isSubFile )
                    {
                        return NPreferences::NCore::CPreferences::instance()->isMediaFile( path );
                    }

                    return false;
                };

                while ( !useAsParent( prevParent ) )
                    prevParent = prevParent->parent();
            }
        }

        bool CTranscodeModel::preFileFunction( const QFileInfo &fileInfo, std::unordered_set< QString > &alreadyAdded, TParentTree &tree, bool countOnly )
        {
            if ( countOnly )
                return true;

            if ( !NPreferences::NCore::CPreferences::instance()->isMediaFile( fileInfo ) )
                return false;

            //qDebug() << fileInfo;
            auto srtFiles = getSRTFilesForMKV( fileInfo, countOnly );
            auto dir = fileInfo.absoluteDir();
            bool retVal = false;
            if ( !srtFiles.isEmpty() )
            {
                //for ( auto && ii : tree )
                //    qDebug() << ii;
                for ( auto &&ii : srtFiles )
                {
                    //qDebug() << ii.absoluteFilePath();
                    if ( alreadyAdded.find( ii.absoluteFilePath() ) != alreadyAdded.end() )
                        continue;
                    alreadyAdded.insert( ii.absoluteFilePath() );
                    auto srtRow = getItemRow( ii );

                    //qDebug() << dir;
                    //qDebug() << srtRow.fileInfo();
                    //qDebug() << srtRow.fileInfo().absoluteDir();

                    if ( dir != srtRow.fileInfo().absoluteDir() )
                    {
                        srtRow.updateName( dir );
                    }
                    tree.emplace_back( std::move( srtRow ) );
                    retVal = true;
                }
            }
            auto idxSub = getIDXSUBFilesForMKV( fileInfo );
            if ( idxSub.has_value() )
            {
                if ( ( alreadyAdded.find( idxSub.value().first.absoluteFilePath() ) == alreadyAdded.end() ) && ( alreadyAdded.find( idxSub.value().second.absoluteFilePath() ) == alreadyAdded.end() ) )
                {
                    alreadyAdded.insert( idxSub.value().first.absoluteFilePath() );
                    alreadyAdded.insert( idxSub.value().second.absoluteFilePath() );

                    auto idxRow = getItemRow( idxSub.value().first );
                    if ( dir != idxRow.fileInfo().absoluteDir() )
                        idxRow.updateName( dir );

                    auto subRow = getItemRow( idxSub.value().second );
                    if ( dir != subRow.fileInfo().absoluteDir() )
                        subRow.updateName( dir );

                    tree.emplace_back( idxRow );
                    tree.emplace_back( subRow );
                    retVal = true;
                }
            }

            if ( !retVal )
            {
                auto mediaInfo = getMediaInfo( fileInfo );
                auto transcodeNeeded = NPreferences::NCore::STranscodeNeeded( mediaInfo );
                retVal = !transcodeNeeded.isLoaded() || transcodeNeeded.transcodeNeeded();
            }
            return retVal;
        }

        void CTranscodeModel::postFileFunction( bool aOK, const QFileInfo &fileInfo, TParentTree & /*tree*/, bool countOnly )
        {
            if ( countOnly )
                return;

            if ( aOK && NPreferences::NCore::CPreferences::instance()->isMediaFile( fileInfo ) )
            {
                auto pos = fPathMapping.find( fileInfo.absoluteFilePath() );
                if ( pos != fPathMapping.end() )
                {
                    auto mkvItem = ( *pos ).second;
                    if ( mkvItem )
                    {
                        //qDebug() << rootItem->text() << rootItem->data( ECustomRoles::eFullPathRole ).toString();
                        autoDetermineLanguageAttributes( mkvItem );
                    }
                }
            }
        }

        void CTranscodeModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        std::pair< bool, bool > CTranscodeModel::setupProcessItem( SProcessInfo &processInfo, const QString &path, const std::list< NCore::SLanguageInfo > &srtFiles, const std::list< std::pair< NCore::SLanguageInfo, QString > > &subIDXFiles, bool displayOnly ) const
        {
            auto fi = QFileInfo( path );
            auto mediaInfo = getMediaInfo( fi );
            Q_ASSERT( mediaInfo );
            auto transcodeNeeded = NPreferences::NCore::STranscodeNeeded( mediaInfo );
            if ( !transcodeNeeded.transcodeNeeded() && srtFiles.empty() && subIDXFiles.empty() )
                return { true, true };

            if ( !processInfo.fItem )
            {
                processInfo.fOldName = path;
                processInfo.fSetMKVTagsOnSuccess = true;

                auto newBaseName = fi.completeBaseName() + "." + NPreferences::NCore::CPreferences::instance()->getMediaContainerExt();
                processInfo.fNewNames << fi.absoluteDir().absoluteFilePath( newBaseName );

                if ( QFileInfo( processInfo.primaryNewName() ) == QFileInfo( processInfo.fOldName ) )
                {
                    processInfo.fNewNames.front() += ".new";
                }
                processInfo.fItem = new QStandardItem( QString( "Convert '%1' => '%2'" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.primaryNewName() ) ) );
                processInfo.fItem->setData( processInfo.fOldName, ECustomRoles::eOldName );
                processInfo.fItem->setData( processInfo.fNewNames, ECustomRoles::eNewName );
                processInfo.fItem->setData( path, ECustomRoles::eAbsFilePath );

                if ( !displayOnly )
                {
                    processInfo.fMaximum = mediaInfo->getNumberOfSeconds();

                    processInfo.fCmd = NPreferences::NCore::CPreferences::instance()->getFFMpegEXE();
                    if ( processInfo.fCmd.isEmpty() || !QFileInfo( processInfo.fCmd ).isExecutable() )
                    {
                        QStandardItem *errorItem = nullptr;
                        if ( processInfo.fCmd.isEmpty() )
                            appendError( processInfo.fItem, tr( "ffmpeg is not set properly" ) );
                        else
                            appendError( processInfo.fItem, tr( "ffmpeg '%1' is not an executable" ).arg( processInfo.fCmd ) );
                        return { false, false };
                    }
                    processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps( processInfo.fOldName );
                }
            }
            if ( !checkProcessItemExists( processInfo.fOldName, processInfo.fItem ) )
                return { false, false };

            for ( auto &&langFile : srtFiles )
            {
                if ( !checkProcessItemExists( langFile.path(), processInfo.fItem ) )
                    return { false, false };
                processInfo.fAncillary << langFile.path();
            }

            for ( auto &&langFile : subIDXFiles )
            {
                if ( !checkProcessItemExists( langFile.first.path(), processInfo.fItem ) )
                    return { false, false };
                processInfo.fAncillary << langFile.first.path();
                if ( !checkProcessItemExists( langFile.second, processInfo.fItem ) )
                    return { false, false };
                processInfo.fAncillary << langFile.second;
            }

            return { false, true };
        }

        std::pair< bool, QStandardItem * > CTranscodeModel::processItem( const QStandardItem *item, bool displayOnly )
        {
            if ( !isMediaFile( item ) || item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, nullptr );

            auto path = item->data( ECustomRoles::eAbsFilePath ).toString();

            //qDebug() << path;

            auto srtFiles = getChildSRTFiles( item, false );

            auto idxFiles = getChildFiles( item, "idx" );
            auto subFiles = getChildFiles( item, "sub" );
            auto subIDXPairFiles = pairSubIDX( idxFiles, subFiles );

            SProcessInfo processInfo;
            bool nothingToDo = false;
            bool aOK = false;
            std::tie( nothingToDo, aOK ) = setupProcessItem( processInfo, path, srtFiles.second, subIDXPairFiles.second, displayOnly );
            if ( !aOK )
                return std::make_pair( false, processInfo.fItem );

            if ( nothingToDo )
                return std::make_pair( true, nullptr );
            fFirstProcess = true;

            aOK = processTranscoding( processInfo, srtFiles.second, subIDXPairFiles.second, item, displayOnly );

            if ( aOK )
            {
                if ( !srtFiles.first.empty() )
                    aOK = processSRTSubTitle( processInfo, item, srtFiles.first );   // all actually processing is done in
            }

            if ( aOK )
            {
                if ( !subIDXPairFiles.first.empty() )
                    aOK = processSUBIDXSubTitle( processInfo, item, subIDXPairFiles.first );
            }

            if ( aOK && !displayOnly )
            {
                fProcessQueue.push_back( processInfo );
                QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
            }
            return std::move( std::make_pair( aOK, processInfo.fItem ) );
        }

        bool CTranscodeModel::processTranscoding( SProcessInfo &processInfo, const std::list< NCore::SLanguageInfo > &srtFiles, const std::list< std::pair< NCore::SLanguageInfo, QString > > &subIDXFiles, const QStandardItem *item, bool displayOnly )
        {
            if ( !isMediaFile( item ) || item->data( ECustomRoles::eIsDir ).toBool() )
                return true;

            auto path = item->data( ECustomRoles::eAbsFilePath ).toString();
            auto mediaInfo = getMediaInfo( path );
            //qDebug() << path;

            auto fi = QFileInfo( path );
            auto transcodeNeeded = NPreferences::NCore::STranscodeNeeded( mediaInfo );
            if ( !transcodeNeeded.transcodeNeeded() && srtFiles.empty() && subIDXFiles.empty() )
                return true;

            auto actions = transcodeNeeded.getActions();
            for ( auto &&ii : actions )
            {
                auto item = new QStandardItem( ii );
                processInfo.fItem->appendRow( item );
            }

            bool aOK = true;
            if ( !displayOnly )
            {
                auto tmp = processInfo.fAncillary;
                for ( auto &&ii : tmp )
                    ii = getDispName( ii );
                processInfo.fProgressLabel = transcodeNeeded.getProgressLabelHeader( getDispName( processInfo.fOldName ), tmp, getDispName( processInfo.primaryNewName() ) );

                processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps( processInfo.fOldName );
                processInfo.fArgs = NPreferences::NCore::CPreferences::instance()->getTranscodeArgs( mediaInfo, processInfo.fOldName, processInfo.primaryNewName(), srtFiles, subIDXFiles );
            }
            return aOK;
        }

        bool CTranscodeModel::processSRTSubTitle( SProcessInfo &processInfo, const QStandardItem *mkvFile, const std::unordered_map< QString, std::vector< QStandardItem * > > &srtFiles ) const
        {
            if ( !mkvFile )
                return true;

            auto path = mkvFile->data( ECustomRoles::eAbsFilePath ).toString();
            auto aOK = setupProcessItem( processInfo, path, {}, {}, true );

            for ( auto &&ii : srtFiles )
            {
                auto languageItem = new QStandardItem( tr( "Language: %1" ).arg( ii.first ) );
                processInfo.fItem->appendRow( languageItem );
                for ( auto &&jj : ii.second )
                {
                    auto defaultItem = getItem( jj, EColumns::eOnByDefault );
                    auto forcedItem = getItem( jj, EColumns::eForced );
                    auto sdhItem = getItem( jj, EColumns::eSDH );

                    auto srtFileItem = new QStandardItem( tr( "'%2' - Default: %3 Forced : %4 SDH : %5" ).arg( jj->text() ).arg( ( defaultItem && defaultItem->checkState() == Qt::Checked ) ? "Yes" : "No" ).arg( ( forcedItem && forcedItem->checkState() == Qt::Checked ) ? "Yes" : "No" ).arg( ( sdhItem && sdhItem->checkState() == Qt::Checked ) ? "Yes" : "No" ) );
                    languageItem->appendRow( srtFileItem );
                }
            }
            return true;
        }

        bool CTranscodeModel::processSUBIDXSubTitle( SProcessInfo &processInfo, const QStandardItem *mkvFile, const std::list< std::pair< QStandardItem *, QStandardItem * > > &subIDXFiles ) const
        {
            if ( !mkvFile )
                return true;

            auto path = mkvFile->data( ECustomRoles::eAbsFilePath ).toString();
            auto aOK = setupProcessItem( processInfo, path, {}, {}, true );

            std::list< std::pair< std::pair< QStandardItem *, QStandardItem * >, NCore::SLanguageInfo > > allLangInfos;
            std::unordered_map< QStandardItem *, std::unordered_map< int, std::pair< bool, bool > > > langMap;
            for ( auto &&ii : subIDXFiles )
            {
                auto idxItem = new QStandardItem( tr( "IDX File: %1 - SUB File: %2" ).arg( ii.first->text() ).arg( ii.second->text() ) );
                processInfo.fItem->appendRow( idxItem );
                auto path = ii.first->data( ECustomRoles::eAbsFilePath ).toString();
                auto langInfo = NCore::SLanguageInfo( QFileInfo( path ) );
                allLangInfos.emplace_back( std::make_pair( ii, langInfo ) );
                auto langs = langInfo.allLanguageInfos();

                for ( auto &&currLang : langs )
                {
                    auto dispName = currLang.first;
                    auto &&currLangInfos = currLang.second;

                    for ( size_t fileNum = 0; fileNum < currLangInfos.size(); ++fileNum )
                    {
                        auto &&lang = currLangInfos[ fileNum ].fLanguage;
                        auto index = currLangInfos[ fileNum ].fIndex;

                        auto label = tr( "'%1' Index: %2 - Forced : %4 SDH : %5" ).arg( lang->displayName() ).arg( index );

                        if ( currLangInfos.size() >= 3 )
                        {
                            if ( fileNum == 0 )
                            {
                                label = label.arg( "Yes" ).arg( "No" );
                                langMap[ ii.first ][ index ] = std::make_pair( true, false );
                            }
                            else if ( fileNum == 1 )
                            {
                                label = label.arg( "No" ).arg( "No" );
                                langMap[ ii.first ][ index ] = std::make_pair( false, false );
                            }
                            else if ( fileNum == ( currLangInfos.size() - 1 ) )
                            {
                                label = label.arg( "No" ).arg( "Yes" );
                                langMap[ ii.first ][ index ] = std::make_pair( false, true );
                            }
                        }
                        else if ( currLangInfos.size() == 2 )
                        {
                            if ( fileNum == 0 )
                            {
                                label = label.arg( "No" ).arg( "No" );
                                langMap[ ii.first ][ index ] = std::make_pair( false, false );
                            }
                            else
                            {
                                label = label.arg( "No" ).arg( "Yes" );
                                langMap[ ii.first ][ index ] = std::make_pair( false, true );
                            }
                        }
                        else
                        {
                            label = label.arg( "No" ).arg( "No" );
                            langMap[ ii.first ][ index ] = std::make_pair( false, false );
                        }
                        auto langItem = new QStandardItem( label );
                        idxItem->appendRow( langItem );
                    }
                }
            }

            //if ( !displayOnly )
            //{
            //    processInfo.fProgressLabel = computeProgressLabel( processInfo );

            //    processInfo.fCmd = NPreferences::NCore::CPreferences::instance()->getMKVMergeEXE();
            //    bool aOK = true;
            //    if ( processInfo.fCmd.isEmpty() || !QFileInfo( processInfo.fCmd ).isExecutable() )
            //    {
            //        QStandardItem *errorItem = nullptr;
            //        if ( processInfo.fCmd.isEmpty() )
            //            appendError( processInfo.fItem, tr( "mkvmerge is not set properly" ) );
            //        else
            //            appendError( processInfo.fItem, tr( "mkvmerge '%1' is not an executable" ).arg( processInfo.fCmd ) );

            //        aOK = false;
            //    }

            //    for ( auto &&ii : subIDXFiles )
            //    {
            //        if ( !aOK )
            //            break;
            //        aOK = aOK && checkProcessItemExists( ii.first->data( ECustomRoles::eAbsFilePath ).toString(), processInfo.fItem );
            //        aOK = aOK && checkProcessItemExists( ii.second->data( ECustomRoles::eAbsFilePath ).toString(), processInfo.fItem );
            //    }

            //    //aOK = the MKV and SRT exist and the cmd is an executable
            //    processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps( processInfo.fOldName );

            //    processInfo.fArgs = QStringList() << "--ui-language"
            //                                      << "en"
            //                                      << "--priority"
            //                                      << "lower"
            //                                      << "--output" << processInfo.fNewNames << "--language"
            //                                      << "0:en"
            //                                      << "--language"
            //                                      << "1:en"
            //                                      << "--language"
            //                                      << "3:en"
            //                                      << "(" << processInfo.fOldName << ")"
            //                                      << "--title" << oldFI.completeBaseName();

            //    QStringList trackOrder = { "0:0", "0:1", "0:3" };
            //    int langFileCnt = 1;
            //    for ( auto &&langInfo : allLangInfos )
            //    {
            //        int nextTrack = 1;
            //        std::list< NCore::SMultLangInfo > orderByIdx;
            //        auto currIDX = langInfo.first.first->data( ECustomRoles::eAbsFilePath ).toString();
            //        auto currSUB = langInfo.first.second->data( ECustomRoles::eAbsFilePath ).toString();
            //        auto langs = langInfo.second.allLanguageInfos();
            //        for ( auto &&ii : langs )
            //        {
            //            for ( auto &&jj : ii.second )
            //                orderByIdx.push_back( jj );
            //        }

            //        orderByIdx.sort( []( const NCore::SMultLangInfo &lhs, const NCore::SMultLangInfo &rhs ) { return lhs.fIndex < rhs.fIndex; } );

            //        for ( auto &&ii : orderByIdx )
            //        {
            //            bool sdh = false;
            //            bool forced = false;

            //            auto langPos = langMap.find( langInfo.first.first );
            //            if ( langPos != langMap.end() )
            //            {
            //                auto indexPos = ( *langPos ).second.find( ii.fIndex );
            //                if ( indexPos != ( *langPos ).second.end() )
            //                {
            //                    sdh = ( *indexPos ).second.first;
            //                    forced = ( *indexPos ).second.second;
            //                }
            //            }
            //            processInfo.fArgs << "--language" << QString( "%1:%2" ).arg( ii.fIndex ).arg( ii.fLanguage->isoCode() ) << "--default-track" << QString( "%1:%2" ).arg( ii.fIndex ).arg( "no" ) << "--hearing-impaired-flag"
            //                              << QString( "%1:%2" ).arg( ii.fIndex ).arg( sdh ? "yes" : "no" ) << "--forced-track" << QString( "%1:%2" ).arg( ii.fIndex ).arg( forced ? "yes" : "no" );
            //            trackOrder << QString( "%1:%2" ).arg( langFileCnt ).arg( ii.fIndex );
            //        }
            //        processInfo.fArgs << "(" << currIDX << ")"
            //                          << "(" << currSUB << ")";

            //        langFileCnt++;
            //    }
            //    processInfo.fArgs << "--track-order" << trackOrder.join( "," );
            //}
            return true;
        }

        std::pair< std::list< std::pair< QStandardItem *, QStandardItem * > >, std::list< std::pair< NCore::SLanguageInfo, QString > > > CTranscodeModel::pairSubIDX( const std::list< QStandardItem * > &idxFiles, const std::list< QStandardItem * > &subFiles ) const
        {
            if ( idxFiles.empty() || subFiles.empty() || ( idxFiles.size() != subFiles.size() ) )
                return {};

            std::unordered_map< QString, QStandardItem * > subMap;
            for ( auto &&ii : subFiles )
            {
                auto baseName = QFileInfo( ii->data( ECustomRoles::eAbsFilePath ).toString() ).completeBaseName();
                subMap[ baseName ] = ii;
            }

            std::list< std::pair< QStandardItem *, QStandardItem * > > retVal;
            std::list< std::pair< NCore::SLanguageInfo, QString > > langInfoFiles;
            for ( auto &&ii : idxFiles )
            {
                auto currIDX = ii->data( ECustomRoles::eAbsFilePath ).toString();

                auto baseName = QFileInfo( currIDX ).completeBaseName();
                auto pos = subMap.find( baseName );
                if ( pos == subMap.end() )
                    continue;
                retVal.emplace_back( std::make_pair( ii, ( *pos ).second ) );

                auto currSUB = ( *pos ).second->data( ECustomRoles::eAbsFilePath ).toString();
                auto langInfo = NCore::SLanguageInfo( QFileInfo( currIDX ) );
                langInfoFiles.emplace_back( std::make_pair( langInfo, currSUB ) );
            }
            return std::move( std::make_pair( retVal, langInfoFiles ) );
        }

        QString CTranscodeModel::computeProgressLabel( const SProcessInfo &processInfo ) const
        {
            auto dir = QFileInfo( processInfo.primaryNewName() ).absolutePath();
            auto fname = QFileInfo( processInfo.fOldName ).fileName();

            auto retVal = QString( "Merging MKV %1<ul><li>%2</li>" ).arg( getDispName( dir ) ).arg( fname );
            for ( auto &&ii : processInfo.fAncillary )
            {
                auto fname = QFileInfo( ii ).fileName();
                retVal += QString( "<li>%1</li>" ).arg( fname );
            }
            retVal += "</ul>";
            fname = QFileInfo( processInfo.primaryNewName() ).fileName();
            retVal += QString( "to create %1" ).arg( fname );
            return retVal;
        }

        std::optional< std::pair< uint64_t, std::optional< uint64_t > > > CTranscodeModel::getCurrentProgress( const QString &string )
        {
            // time=00:00:00.00
            auto numSeconds = getNumSeconds( string );
            if ( !numSeconds.has_value() )
                return {};
            return std::pair< uint64_t, std::optional< uint64_t > >( numSeconds.value(), {} );
        }

        std::optional< uint64_t > CTranscodeModel::getNumSeconds( const QString &string ) const
        {
            auto regEx = QRegularExpression( "[Tt]ime\\=\\s*(?<hours>\\d{2}):(?<mins>\\d{2}):(?<secs>\\d{2})" );

            QRegularExpressionMatch match;
            auto pos = string.lastIndexOf( regEx, -1, &match );
            if ( ( pos == -1 ) || !match.hasMatch() )
                return 0;

            auto hours = match.captured( "hours" );
            auto mins = match.captured( "mins" );
            auto secs = match.captured( "secs" );

            uint64_t numSeconds = 0;
            if ( !hours.isEmpty() )
            {
                bool aOK;
                int curr = hours.toInt( &aOK );
                if ( aOK )
                    numSeconds += curr * 60 * 60;
            }

            if ( !mins.isEmpty() )
            {
                bool aOK;
                int curr = mins.toInt( &aOK );
                if ( aOK )
                    numSeconds += curr * 60;
            }

            if ( !secs.isEmpty() )
            {
                bool aOK;
                int curr = secs.toInt( &aOK );
                if ( aOK )
                    numSeconds += curr;
            }
            return numSeconds;
        }

        std::optional< std::chrono::milliseconds > CTranscodeModel::getMSRemaining( const QString &string, const std::pair< uint64_t, std::optional< uint64_t > > &currProgress ) const
        {
            if ( !currProgress.second.has_value() )
                return {};

            auto regEx2 = QRegularExpression( "[Ss]peed\\s*\\=\\s*(?<multiplier>\\d+(\\.\\d+)?)" );
            QRegularExpressionMatch match;
            auto pos = string.lastIndexOf( regEx2, -1, &match );
            if ( ( pos == -1 ) || !match.hasMatch() )
                return {};

            auto mult = match.captured( "multiplier" );
            bool aOK = false;
            auto multVal = mult.toDouble( &aOK );
            if ( !aOK || ( multVal == 0.0 ) )
                return {};

            auto msecsRemaining = std::chrono::milliseconds( static_cast< uint64_t >( std::round( ( currProgress.second.value() - currProgress.first ) * 1000 / multVal ) ) );
            return msecsRemaining;
        }

        void CTranscodeModel::autoDetermineLanguageAttributes( QStandardItem *mediaFileNode ) const
        {
            //if ( mediaFileNode )
            //    qDebug() << mediaFileNode->text();
            if ( !mediaFileNode || !isMediaFile( mediaFileNode ) )
                return;

            auto srtFiles = getChildSRTFiles( mediaFileNode, true );   // map of language to files sorted by filesize
            for ( auto &&ii : srtFiles.first )
            {
                if ( ii.second.size() >= 3 )
                {
                    // smallest is "Forced"
                    // mid is normal
                    // largest is "SDH"
                    setChecked( getItem( ii.second.front(), EColumns::eForced ), ECustomRoles::eForcedSubTitleRole, true );
                    setChecked( getItem( ii.second.back(), EColumns::eSDH ), ECustomRoles::eHearingImparedRole, true );
                }
                else if ( ii.second.size() == 2 )
                {
                    // if 2
                    // smallest is normal
                    // largest is sdh
                    setChecked( getItem( ii.second[ 1 ], EColumns::eSDH ), ECustomRoles::eHearingImparedRole, true );
                }
                else if ( ii.second.size() == 1 )
                {
                    // do nothing
                }
            }
        }

        std::optional< std::pair< QFileInfo, QFileInfo > > CTranscodeModel::getIDXSUBFilesForMKV( const QFileInfo &fi ) const
        {
            if ( !fi.exists() || !fi.isFile() )
                return {};

            auto dir = fi.absoluteDir();
            //qDebug().noquote().nospace() << "Finding SRT files for '" << getDispName( fi ) << "' in dir '" << getDispName( dir.absolutePath() ) << "'";
            auto idxSubPairs = getIDXSUBFilesInDir( dir );

            if ( idxSubPairs.empty() )
                return {};
            if ( idxSubPairs.size() == 1 )
                return idxSubPairs.front();

            std::map< QString, QFileInfo > nameBasedMap;
            for ( auto &&ii : idxSubPairs )
            {
                if ( ii.first.completeBaseName().compare( fi.completeBaseName(), Qt::CaseInsensitive ) == 0 )
                    return ii;
            }
            return {};
        }

        QList< QFileInfo > CTranscodeModel::getSRTFilesForMKV( const QFileInfo &mkvFile, bool countOnly ) const
        {
            if ( !mkvFile.exists() || !mkvFile.isFile() )
                return {};

            std::optional< QList< QFileInfo > > srtFiles;

            auto dir = mkvFile.absoluteDir();
            auto ii = fSRTFileCache.find( dir.absolutePath() );
            if ( ii == fSRTFileCache.end() )
            {
                //qDebug().noquote().nospace() << "Finding SRT files for '" << getDispName( mkvFile ) << "' in dir '" << getDispName( dir.absolutePath() ) << "'";
                srtFiles = NSABUtils::NFileUtils::findAllFiles( dir, QStringList() << "*.srt", true );
                fSRTFileCache[ dir.absolutePath() ] = srtFiles;
            }
            else
                srtFiles = ( *ii ).second;

            //qDebug().noquote().nospace() << "Found '" << srtFiles.count() << "' SRT Files";

            if ( !srtFiles.has_value() )
                return {};

            if ( srtFiles.value().count() <= 1 )
            {
                return srtFiles.value();
            }

            if ( countOnly )
                return srtFiles.value();

            // could be 2_lang, 3_lang etc - return them all
            // or could be name.srt (common for TV shows)
            // or 2_NAME 3_name

            QList< QFileInfo > retVal;
            QList< QFileInfo > namebasedFiles;
            QList< QFileInfo > languageFiles;
            QList< QFileInfo > unknownFiles;

            std::map< QString, QList< QFileInfo > > nameBasedMap;
            for ( auto &&ii : srtFiles.value() )
            {
                if ( isNameBasedMatch( mkvFile, ii ) )
                    nameBasedMap[ mkvFile.completeBaseName() ].push_back( ii );
            }
            auto pos = nameBasedMap.find( mkvFile.completeBaseName() );
            if ( pos != nameBasedMap.end() )
            {
                namebasedFiles << ( *pos ).second;
            }
            else
            {
                for ( auto &&ii : srtFiles.value() )
                {
                    //qDebug().noquote().nospace() << "Checking '" << getDispName( ii ) << "'";
                    //qDebug().noquote().nospace() << "Checking '" << fi.completeBaseName() << "' against '" << ii.completeBaseName() << "'";
                    auto langInfo = NCore::SLanguageInfo( ii );
                    if ( langInfo.isNameBasedLangFile() )
                        continue;
                    if ( langInfo.knownLanguage() )
                    {
                        //qDebug().noquote().nospace() << "'" << getDispName( ii ) << "' is LANGUAGE based";
                        languageFiles.push_back( ii );
                    }
                    else
                    {
                        //qDebug().noquote().nospace() << "'" << getDispName( ii ) << "' is UNKNOWN type of filename";
                        unknownFiles.push_back( ii );
                    }
                }
            }
            return namebasedFiles << languageFiles;
        }

        std::pair< std::unordered_map< QString, std::vector< QStandardItem * > >, std::list< NCore::SLanguageInfo > > CTranscodeModel::getChildSRTFiles( const QStandardItem *item, bool sort ) const
        {
            if ( !item )
                return {};

            auto childCount = item->rowCount();
            auto srtFiles = getChildFiles( item, "srt" );
            //qDebug() << item->text() << childCount;
            std::unordered_map< QString, std::vector< std::pair< QStandardItem *, bool > > > tmp;   // language to <item,bool>
            std::unordered_set< QString > uniqueSRTFiles;
            for ( auto &&ii : srtFiles )
            {
                auto fullPath = ii->data( ECustomRoles::eAbsFilePath ).toString();
                bool isLangFormat;
                if ( !isSubtitleFile( fullPath, &isLangFormat ) )
                    continue;

                auto languageItem = getLanguageItem( ii );
                auto langName = languageItem->data( ECustomRoles::eLanguageName ).toString();
                tmp[ langName ].emplace_back( ii, isLangFormat );
            }

            bool allLangFiles = true;

            std::list< NCore::SLanguageInfo > allLangFilesList;
            std::unordered_map< QString, std::vector< QStandardItem * > > retVal;
            for ( auto &&ii : tmp )
            {
                std::vector< QStandardItem * > curr;
                for ( auto &&jj : ii.second )
                {
                    curr.push_back( jj.first );
                    auto path = jj.first->data( ECustomRoles::eAbsFilePath ).toString();
                    auto langInfo = NCore::SLanguageInfo( path );
                    allLangFilesList.emplace_back( langInfo );
                    allLangFiles = allLangFiles && jj.second;
                }
                retVal[ ii.first ] = curr;
            }

            if ( sort )
            {
                for ( auto &&ii : retVal )
                {
                    std::sort(
                        ii.second.begin(), ii.second.end(),
                        []( const QStandardItem *lhs, const QStandardItem *rhs )
                        {
                            auto lhsFI = QFileInfo( lhs->data( ECustomRoles::eAbsFilePath ).toString() );
                            auto rhsFI = QFileInfo( rhs->data( ECustomRoles::eAbsFilePath ).toString() );

                            return lhsFI.size() < rhsFI.size();
                        } );
                }
            }
            return std::move( std::make_pair( retVal, allLangFilesList ) );
        }

        std::list< std::pair< QFileInfo, QFileInfo > > CTranscodeModel::getIDXSUBFilesInDir( const QDir &dir ) const
        {
            //qDebug() << dir.absolutePath();
            std::list< std::pair< QFileInfo, QFileInfo > > retVal;

            auto subFiles = dir.entryInfoList( QStringList() << "*.sub" );
            std::unordered_map< QString, QFileInfo > subMap;
            for ( auto &&ii : subFiles )
            {
                auto baseName = ii.completeBaseName();
                subMap[ baseName ] = ii;
            }

            auto idxFiles = dir.entryInfoList( QStringList() << "*.idx" );
            for ( auto &&ii : idxFiles )
            {
                auto baseName = ii.completeBaseName();
                auto pos = subMap.find( baseName );
                if ( pos == subMap.end() )
                    continue;

                retVal.emplace_back( std::make_pair( ii, ( *pos ).second ) );
            }

            auto subDirs = dir.entryInfoList( QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot );
            for ( auto &&ii : subDirs )
            {
                auto sub = getIDXSUBFilesInDir( QDir( ii.absoluteFilePath() ) );
                retVal.insert( retVal.end(), sub.begin(), sub.end() );
            }
            return retVal;
        }

        bool CTranscodeModel::nameMatch( const QString &mkvBaseName, const QString &origSubtitleFile ) const
        {
            auto nextChar = ( origSubtitleFile.length() > mkvBaseName.length() ) ? origSubtitleFile[ mkvBaseName.length() ] : QChar( 0 );
            if ( origSubtitleFile.startsWith( mkvBaseName ) && !nextChar.isLetterOrNumber() )
                return true;

            auto subtitleFile = origSubtitleFile;
            subtitleFile.remove( mkvBaseName, Qt::CaseInsensitive );
            if ( subtitleFile == origSubtitleFile )
                return false;

            if ( subtitleFile.isEmpty() )
                return true;

            return NSABUtils::NStringUtils::startsOrEndsWithNumber( subtitleFile );
        }

        bool CTranscodeModel::isNameBasedMatch( const QFileInfo &mkvFile, const QFileInfo &srtFile ) const
        {
            //qDebug() << mkvFile;
            //qDebug() << srtFile;
            auto langInfo = NCore::SLanguageInfo( srtFile );
            if ( !langInfo.knownLanguage() && nameMatch( mkvFile.completeBaseName(), langInfo.baseName() ) )
                return true;
            else
            {
                auto dir = mkvFile.absoluteDir();

                auto relPath = dir.relativeFilePath( srtFile.absoluteFilePath() );
                auto dirs = relPath.split( QRegularExpression( R"([\/\\])" ) );
                for ( auto &curr : dirs )
                {
                    if ( ( curr.toLower() == "subs" ) || ( curr.toLower() == "subtitles" ) )
                        continue;
                    if ( curr.toLower() == "." )
                        continue;
                    if ( nameMatch( mkvFile.completeBaseName(), curr ) )
                    {
                        return true;
                        break;
                    }
                }
            }
            return false;
        }

        bool CTranscodeModel::isSubtitleFile( const QFileInfo &fileInfo, bool *isLangFileFormat ) const
        {
            if ( !CDirModel::isSubtitleFile( fileInfo, isLangFileFormat ) )
                return false;
            return fileInfo.suffix() != "sub";
        }

        QList< QStandardItem * > CTranscodeModel::getChildMKVFiles( const QStandardItem *item, bool goBelowDirs ) const
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

                if ( NPreferences::NCore::CPreferences::instance()->isMediaFile( child->data( ECustomRoles::eAbsFilePath ).toString() ) )
                    retVal << child;
            }
            return retVal;
        }

        std::list< QStandardItem * > CTranscodeModel::getChildFiles( const QStandardItem *item, const QString &ext ) const
        {
            auto childCount = item->rowCount();
            std::list< QStandardItem * > retVal;
            for ( int ii = 0; ii < childCount; ++ii )
            {
                auto child = item->child( ii );
                if ( !child )
                    continue;

                if ( child->data( ECustomRoles::eIsDir ).toBool() )
                    continue;

                auto fullPath = child->data( ECustomRoles::eAbsFilePath ).toString();
                if ( QFileInfo( fullPath ).suffix() != ext )
                    continue;
                retVal.push_back( child );
            }
            return retVal;
        }

        QStandardItem *CTranscodeModel::getLanguageItem( const QStandardItem *item ) const
        {
            return getItem( item, EColumns::eLanguage );
        }

        std::list< SDirNodeItem > CTranscodeModel::addAdditionalItems( const QFileInfo &fileInfo ) const
        {
            std::list< SDirNodeItem > retVal;
            if ( fileInfo.isFile() )
            {
                auto language = NCore::SLanguageInfo( fileInfo );
                auto langName = language.displayName();
                if ( !isSubtitleFile( fileInfo ) )
                    langName.clear();

                auto languageFileItem = SDirNodeItem( langName, EColumns::eLanguage );
                if ( !language.isMultiLanguage() )
                    languageFileItem.setData( language.isoCode(), ECustomRoles::eISOCodeRole );
                languageFileItem.setData( language.language(), ECustomRoles::eLanguageName );
                retVal.emplace_back( languageFileItem );

                auto forcedItem = SDirNodeItem( QString(), EColumns::eForced );
                forcedItem.fCheckable = { true, true, Qt::CheckState::Unchecked };
                retVal.emplace_back( forcedItem );

                auto sdhItem = SDirNodeItem( QString(), EColumns::eSDH );
                sdhItem.fCheckable = { true, true, Qt::CheckState::Unchecked };
                retVal.emplace_back( sdhItem );

                auto onByDefaultItem = SDirNodeItem( QString(), EColumns::eOnByDefault );
                onByDefaultItem.fCheckable = { !language.isMultiLanguage(), true, Qt::CheckState::Unchecked };
                retVal.emplace_back( onByDefaultItem );
            }
            else
            {
                retVal.emplace_back( SDirNodeItem() );
                retVal.emplace_back( SDirNodeItem() );
                retVal.emplace_back( SDirNodeItem() );
                retVal.emplace_back( SDirNodeItem() );
            }

            auto mediaItems = CDirModel::addAdditionalItems( fileInfo );
            for ( auto &&ii : mediaItems )
                retVal.emplace_back( ii );

            return retVal;
        }

        void CTranscodeModel::setupNewItem( const SDirNodeItem &nodeItem, const QStandardItem *nameItem, QStandardItem *item ) const
        {
            if ( ( nodeItem.fType == EColumns::eLanguage ) && ( item->text().isEmpty() ) )
            {
                auto path = nameItem->data( ECustomRoles::eAbsFilePath ).toString();
                if ( isSubtitleFile( path ) )
                    item->setBackground( Qt::red );

                auto language = NCore::SLanguageInfo( path );
                fAllLangInfos[ item ] = { nullptr, language };   // need to handl subidx
            }
        }
    }
}
