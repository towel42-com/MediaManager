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

        std::unordered_map< QString, std::vector< QStandardItem * > > CMergeSRTModel::getChildSRTFiles( const QStandardItem * item, bool sort ) const
        {
            if ( !item )
                return {};

            auto childCount = item->rowCount();
            //qDebug() << item->text() << childCount;
            std::unordered_map< QString, std::vector< std::pair< QStandardItem *, bool > > > tmp; // language to <item,bool>
            std::unordered_set< QString > uniqueSRTFiles;
            for ( int ii = 0; ii < childCount; ++ii )
            {
                auto child = item->child( ii );
                if ( !child )
                    continue;

                if ( child->data( ECustomRoles::eIsDir ).toBool() )
                    continue;


                auto fullPath = child->data( ECustomRoles::eFullPathRole ).toString();
                bool isLangFormat;
                if ( !CPreferences::instance()->isSubtitleFile( fullPath, &isLangFormat ) )
                    continue;

#ifdef _CHECK_FOR_UNIQUE_SRT_DATA
                auto md5 = child->data( ECustomRoles::eMD5 ).toString();
                if ( md5.isEmpty() )
                {
                    md5 = NUtils::getMd5( fullPath, true );
                    child->setData( md5, ECustomRoles::eMD5 );
                }

                auto pos = uniqueSRTFiles.find( md5 );
                if ( pos != uniqueSRTFiles.end() )
                {
                    continue;
                }
                uniqueSRTFiles.insert( md5 );
#endif
                auto languageItem = getLanguageItem( child );
                tmp[languageItem->text()].push_back( { child, isLangFormat } );
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

            for ( auto && ii : srtFiles )
            {
                if ( ii.second.size() == 3 )
                {
                    // smallest is "Forced"
                    // mid is normal
                    // largest is "SDH"
                    setChecked( getItem( ii.second[0], EColumns::eForced ), ECustomRoles::eForcedSubTitleRole, true );
                    setChecked( getItem( ii.second[2], EColumns::eSDH ), ECustomRoles::eHearingImparedRole, true );
                }
                else if ( ii.second.size() == 2 )
                {
                    // if 2 
                    // smallest is normal
                    // largest is sdh
                    setChecked( getItem( ii.second[1], EColumns::eSDH ), ECustomRoles::eHearingImparedRole, true );
                }
                else if ( ii.second.size() == 1 )
                {
                    // do nothing
                }
            }
        }

        QFileInfoList CMergeSRTModel::getMKVFilesInDir( const QDir & dir ) const
        {
            qDebug() << dir.absolutePath();

            auto srtFiles = dir.entryInfoList( QStringList() << "*.srt" );

            auto subDirs = dir.entryInfoList( QDir::Filter::AllDirs | QDir::Filter::NoDotAndDotDot );
            for ( auto && ii : subDirs )
            {
                srtFiles << getMKVFilesInDir( QDir( ii.absoluteFilePath() ) );
            }
            return srtFiles;
        }

        QList< QFileInfo > CMergeSRTModel::getSRTFilesForMKV( const QFileInfo & fi ) const
        {
            if ( !fi.exists() || !fi.isFile() )
                return {};

            auto dir = fi.absoluteDir();
            //qDebug().noquote().nospace() << "Finding SRT files for '" << getDispName( fi ) << "' in dir '" << getDispName( dir.absolutePath() ) << "'";
            auto srtFiles = getMKVFilesInDir( dir );

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

            for ( auto && ii : srtFiles )
            {
                //qDebug().noquote().nospace() << "Checking '" << getDispName( ii ) << "'";
                //qDebug().noquote().nospace() << "Checking '" << fi.completeBaseName() << "' against '" << ii.completeBaseName() << "'";
                if ( fi.completeBaseName().contains( ii.completeBaseName(), Qt::CaseSensitivity::CaseInsensitive ) )
                {
                    //qDebug().noquote().nospace() << "'" << getDispName( ii ) << "' is NAME based";
                    namebasedFiles.push_back( ii );
                }
                else
                {
                    auto langInfo = SLanguageInfo( ii );
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
                if ( srtFiles.empty() )
                    return std::make_pair( true, nullptr );

                SProcessInfo processInfo;
                processInfo.fSetMKVTagsOnSuccess = true;
                processInfo.fOldName = computeTransformPath( mkvFile, true );
                auto oldFI = QFileInfo( processInfo.fOldName );

                processInfo.fNewName = oldFI.absoluteDir().absoluteFilePath( oldFI.fileName() + ".new" );// prevents the emby system from picking it up

                processInfo.fItem = new QStandardItem( QString( "'%1' => '%2'" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewName ) ) );
                if ( parentItem )
                    parentItem->appendRow( processInfo.fItem );
                else
                    fProcessResults.second->appendRow( processInfo.fItem );

                for ( auto && ii : srtFiles )
                {
                    auto languageItem = new QStandardItem( tr( "Language: %1" ).arg( ii.first ) );
                    processInfo.fItem->appendRow( languageItem );
                    for ( auto && jj : ii.second )
                    {
                        auto defaultItem = getItem( jj, EColumns::eOnByDefault );
                        auto forcedItem = getItem( jj, EColumns::eForced );
                        auto sdhItem = getItem( jj, EColumns::eSDH );

                        auto srtFileItem = new QStandardItem( tr( "'%2' - Default: %3 Forced : %4 SDH : %5" )
                                                              .arg( jj->text() )
                                                              .arg( (defaultItem && defaultItem->checkState() == Qt::Checked) ? "Yes" : "No" )
                                                              .arg( (forcedItem && forcedItem->checkState() == Qt::Checked) ? "Yes" : "No" )
                                                              .arg( (sdhItem && sdhItem->checkState() == Qt::Checked) ? "Yes" : "No" ) );
                        languageItem->appendRow( srtFileItem );
                    }
                }

                if ( !displayOnly )
                {
                    processInfo.fCmd = CPreferences::instance()->getMKVMergeEXE();
                    bool aOK = true;
                    if ( processInfo.fCmd.isEmpty() || !QFileInfo( processInfo.fCmd ).isExecutable() )
                    {
                        QStandardItem * errorItem = nullptr;
                        if ( processInfo.fCmd.isEmpty() )
                            errorItem = new QStandardItem( QString( "ERROR: mkvmerge is not set properly" ) );
                        else
                            errorItem = new QStandardItem( QString( "ERROR: mkvmerge '%1' is not an executable" ).arg( processInfo.fCmd ) );

                        errorItem->setData( ECustomRoles::eIsErrorNode, true );
                        appendError( processInfo.fItem, errorItem );

                        QIcon icon;
                        icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                        errorItem->setIcon( icon );
                        aOK = false;
                    }
                    aOK = aOK && checkProcessItemExists( processInfo.fOldName, processInfo.fItem );
                    for ( auto && ii : srtFiles )
                    {
                        if ( !aOK )
                            break;
                        for ( auto && jj : ii.second )
                        {
                            aOK = aOK && checkProcessItemExists( jj->data( ECustomRoles::eFullPathRole ).toString(), processInfo.fItem );
                        }
                    }
                    // aOK = the MKV and SRT exist and the cmd is an executable
                    processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps( processInfo.fOldName );


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
                    for ( auto && ii : srtFiles )
                    {
                        for ( auto && jj : ii.second )
                        {
                            //qDebug() << jj->text();
                            auto langItem = getItem( jj, EColumns::eLanguage );
                            auto srtFile = jj->data( ECustomRoles::eFullPathRole ).toString();
                            processInfo.fArgs
                                << "--language"
                                << "0:" + langItem->data( ECustomRoles::eISOCodeRole ).toString()
                                << "--default-track"
                                << "0:" + QString( jj->data( ECustomRoles::eDefaultTrackRole ).toBool() ? "yes" : "no" )
                                << "--hearing-impaired-flag"
                                << "0:" + QString( jj->data( ECustomRoles::eHearingImparedRole ).toBool() ? "yes" : "no" )
                                << "--forced-track"
                                << "0:" + QString( jj->data( ECustomRoles::eForcedSubTitleRole ).toBool() ? "yes" : "no" )
                                << "(" << srtFile << ")"
                                ;
                            processInfo.fAncillary.push_back( srtFile );
                            trackOrder << QString( "%1:0" ).arg( nextTrack++ );
                        }
                    }
                    processInfo.fArgs << "--track-order" << trackOrder.join( "," );
                    fProcessQueue.push_back( processInfo );
                    QTimer::singleShot( 0, this, &CMergeSRTModel::slotRunNextProcessInQueue );
                }
                myItem = processInfo.fItem;
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

        void CMergeSRTModel::preAddItems( const QFileInfo & /*fileInfo*/, std::list< NMediaManager::NCore::STreeNodeItem > & /*currItems*/ ) const
        {
            return;
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

        std::list< NMediaManager::NCore::STreeNodeItem > CMergeSRTModel::addItems( const QFileInfo & fileInfo )const
        {
            std::list< NMediaManager::NCore::STreeNodeItem > retVal;
            if ( fileInfo.isFile() )
            {
                auto language = SLanguageInfo( fileInfo );
                auto languageFileItem = STreeNodeItem( language.displayName(), EColumns::eLanguage );
                languageFileItem.setData( language.isoCode(), ECustomRoles::eISOCodeRole );
                retVal.push_back( languageFileItem );

                auto forcedItem = STreeNodeItem( QString(), EColumns::eForced );
                forcedItem.fCheckable = true;
                retVal.push_back( forcedItem );

                auto sdhItem = STreeNodeItem( QString(), EColumns::eSDH );
                sdhItem.fCheckable = true;
                retVal.push_back( sdhItem );

                auto onByDefaultItem = STreeNodeItem( QString(), EColumns::eOnByDefault );
                onByDefaultItem.fCheckable = true;
                retVal.push_back( onByDefaultItem );
            }
            else
            {
                // do nothing for directories
            }
            return retVal;
        }

        void CMergeSRTModel::setupNewItem( const STreeNodeItem & nodeItem, const QStandardItem * /*nameItem*/, QStandardItem * item ) const
        {
            if ( (nodeItem.fType == EColumns::eLanguage) && (item->text().isEmpty()) )
            {
                item->setBackground( Qt::red );
            }
        }

        QStringList CMergeSRTModel::headers() const
        {
            return CDirModel::headers() << tr( "Language" ) << tr( "Forced?" ) << tr( "Hearing Impaired?" ) << tr( "On by Default?" );
        }

        void CMergeSRTModel::postLoad( QTreeView * treeView ) const
        {
            if ( !treeView )
                return;

            treeView->resizeColumnToContents( EColumns::eLanguage );
            treeView->resizeColumnToContents( EColumns::eForced );
            treeView->resizeColumnToContents( EColumns::eSDH );
            treeView->resizeColumnToContents( EColumns::eOnByDefault );
        }

        int CMergeSRTModel::computeNumberOfItems() const
        {
            auto mkvFiles = getChildMKVFiles( invisibleRootItem(), true );
            return mkvFiles.count();
        }

        void CMergeSRTModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem * prevParent, const STreeNode & treeNode )
        {
            if ( treeNode.fIsFile )
            {
                auto isSRT = CPreferences::instance()->isSubtitleFile( treeNode.name() );
                auto useAsParent =
                    [ isSRT, this ]( QStandardItem * item )
                {
                    if ( !item )
                        return true;

                    if ( item->data( ECustomRoles::eIsDir ).toBool() )
                        return true;

                    auto path = item->data( ECustomRoles::eFullPathRole ).toString();
                    if ( isSRT )
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
            auto srtFiles = getSRTFilesForMKV( fileInfo );
            for ( auto && ii : srtFiles )
            {
                //qDebug() << ii.absoluteFilePath();
                if ( alreadyAdded.find( ii.absoluteFilePath() ) != alreadyAdded.end() )
                    continue;
                alreadyAdded.insert( ii.absoluteFilePath() );
                tree.push_back( std::move( getItemRow( ii ) ) );
            }

            return !srtFiles.isEmpty();
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
