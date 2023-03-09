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

#include "MakeMKVModel.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/MediaInfo.h"

#include <QDir>
#include <QTimer>

namespace NMediaManager
{
    namespace NModels
    {
        CMakeMKVModel::CMakeMKVModel( NUi::CBasePage *page, QObject *parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CMakeMKVModel::~CMakeMKVModel()
        {
        }

        std::optional< NMediaManager::NModels::TItemStatus > CMakeMKVModel::computeItemStatus( const QModelIndex &idx ) const
        {
            if ( isRootPath( idx ) )
                return {};

            auto mediaInfo = getMediaInfo( idx );
            if ( !mediaInfo )
                return {};

            auto transcodeNeeded = NMediaManager::NPreferences::NCore::STranscodeNeeded( mediaInfo );
            auto fileInfo = this->fileInfo( idx );

            if ( idx.column() == 0 )   // filename
            {
                if ( transcodeNeeded.fFormat )
                {
                    auto msg = tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using a %2 container</p>" ).arg( fileInfo.fileName() ).arg( NPreferences::NCore::CPreferences::instance()->getForceMediaFormatName() );
                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg );
                }
            }
            else if ( idx.column() == getMediaVideoCodecLoc() )
            {
                if ( transcodeNeeded.fVideo )
                {
                    auto fileInfo = this->fileInfo( idx );
                    auto msg = tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using the H.265 video codec</p>" ).arg( fileInfo.fileName() );
                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg );
                }
            }
            else if ( idx.column() == getMediaAudioCodecLoc() )
            {
                if ( transcodeNeeded.fAudio )
                {
                    auto fileInfo = this->fileInfo( idx );
                    auto msg = tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using the %2 audio codec</p>" ).arg( fileInfo.fileName() ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );
                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg );
                }
            }
            return {};
        }

        QStringList CMakeMKVModel::dirModelFilter() const
        {
            return NPreferences::NCore::CPreferences::instance()->getVideoExtensions();
        }

        std::pair< bool, QStandardItem * > CMakeMKVModel::processItem( const QStandardItem *item, bool displayOnly )
        {
            if ( item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, nullptr );

            auto path = item->data( ECustomRoles::eAbsFilePath ).toString();

            auto mediaInfo = getMediaInfo( path );

            auto fi = QFileInfo( path );
            auto transcodeNeeded = NPreferences::NCore::STranscodeNeeded( mediaInfo );
            if ( !transcodeNeeded.transcodeNeeded() )
                return std::make_pair( true, nullptr );

            SProcessInfo processInfo;
            processInfo.fSetMKVTagsOnSuccess = true;
            processInfo.fOldName = path;

            auto newBaseName = fi.completeBaseName() + "." + NPreferences::NCore::CPreferences::instance()->getForceMediaFormatExt();
            if ( !transcodeNeeded.fFormat )
                newBaseName += ".new";

            processInfo.fNewNames << fi.absoluteDir().absoluteFilePath( newBaseName );

            auto msg = tr( "Convert file from '%1' => '%2'" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewNames.front() ) );
            if ( transcodeNeeded.fVideo && transcodeNeeded.fAudio )
                msg += tr( ", transcode into H.265, and transcode to %1." ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );
            else if ( transcodeNeeded.fVideo && !transcodeNeeded.fAudio )
                msg += tr( " and transcode into H.265." );
            else if ( !transcodeNeeded.fVideo && transcodeNeeded.fAudio )
                msg += tr( " transcode to %1." ).arg( NPreferences::NCore::CPreferences::instance()->getTranscodeToAudioCodec() );

            processInfo.fItem = new QStandardItem( msg );
            processInfo.fItem->setData( processInfo.fOldName, ECustomRoles::eOldName );
            processInfo.fItem->setData( processInfo.fNewNames, ECustomRoles::eNewName );

            bool aOK = true;
            QStandardItem *myItem = nullptr;
            fFirstProcess = true;
            if ( !displayOnly )
            {
                auto mediaInfo = getMediaInfo( fi );
                Q_ASSERT( mediaInfo );

                processInfo.fMaximum = mediaInfo->getNumberOfSeconds();
                processInfo.fCmd = NPreferences::NCore::CPreferences::instance()->getFFMpegEXE();
                if ( processInfo.fCmd.isEmpty() || !QFileInfo( processInfo.fCmd ).isExecutable() )
                {
                    QStandardItem *errorItem = nullptr;
                    if ( processInfo.fCmd.isEmpty() )
                        appendError( processInfo.fItem, tr( "ffmpeg is not set properly" ) );
                    else
                        appendError( processInfo.fItem, tr( "ffmpeg '%1' is not an executable" ).arg( processInfo.fCmd ) );
                    aOK = false;
                }

                aOK = aOK && checkProcessItemExists( processInfo.fOldName, processInfo.fItem );
                processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps( processInfo.fOldName );

                processInfo.fArgs = NPreferences::NCore::CPreferences::instance()->getTranscodeArgs( mediaInfo, processInfo.fOldName, processInfo.fNewNames.front() );

                fProcessQueue.push_back( processInfo );
                QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
            }
            myItem = processInfo.fItem;
            return std::make_pair( aOK, myItem );
        }

        QString CMakeMKVModel::getProgressLabel( const SProcessInfo &processInfo ) const
        {
            auto retVal = QString( "Converting to H.265 %3<ul><li>%1</li>to<li>%2</li></ul>" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewNames.front() ) ).arg( NPreferences::NCore::CPreferences::instance()->getForceMediaFormatName() );
            return retVal;
        }

        QStringList CMakeMKVModel::headers() const
        {
            return CDirModel::headers() << getMediaHeaders();
            ;
        }

        void CMakeMKVModel::postLoad( QTreeView *treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CMakeMKVModel::preLoad( QTreeView *treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CMakeMKVModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        void CMakeMKVModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
        }

        bool CMakeMKVModel::preFileFunction( const QFileInfo &fileInfo, std::unordered_set< QString > & /*alreadyAdded*/, TParentTree & /*tree*/, bool countOnly )
        {
            if ( countOnly )
                return true;

            auto mediaInfo = getMediaInfo( fileInfo );
            auto transcodeNeeded = NPreferences::NCore::STranscodeNeeded( mediaInfo );
            return transcodeNeeded.transcodeNeeded();
        }

        void CMakeMKVModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {
        }

    }
}
