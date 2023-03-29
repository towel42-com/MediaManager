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
#include "Preferences/Core/Preferences.h"
#include "Preferences/Core/TranscodeNeeded.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/MediaInfo.h"
#include "SABUtils/utils.h"

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

        QStringList CTranscodeModel::dirModelFilter() const
        {
            return NPreferences::NCore::CPreferences::instance()->getVideoDecoderExtensions();
        }

        std::pair< bool, QStandardItem * > CTranscodeModel::processItem( const QStandardItem *item, bool displayOnly )
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

            auto newBaseName = fi.completeBaseName() + "." + NPreferences::NCore::CPreferences::instance()->getMediaContainerExt();
            if ( !transcodeNeeded.formatChangeNeeded() )
                newBaseName += ".new";

            processInfo.fNewNames << fi.absoluteDir().absoluteFilePath( newBaseName );

            processInfo.fItem = new QStandardItem( transcodeNeeded.getMessage( getDispName( processInfo.fOldName ), getDispName( processInfo.fNewNames.front() ) ) );
            processInfo.fItem->setData( processInfo.fOldName, ECustomRoles::eOldName );
            processInfo.fItem->setData( processInfo.fNewNames, ECustomRoles::eNewName );

            bool aOK = true;
            QStandardItem *myItem = nullptr;
            fFirstProcess = true;
            if ( !displayOnly )
            {
                processInfo.fProgressLabel = transcodeNeeded.getProgressLabelHeader( getDispName( processInfo.fOldName ), getDispName( processInfo.fNewNames.front() ) );
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

        QString CTranscodeModel::getProgressLabel( const SProcessInfo &processInfo ) const
        {
            return processInfo.fProgressLabel;
        }

        QStringList CTranscodeModel::headers() const
        {
            return CDirModel::headers() << getMediaHeaders();
            ;
        }

        void CTranscodeModel::postLoad( QTreeView *treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CTranscodeModel::preLoad( QTreeView *treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CTranscodeModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        void CTranscodeModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
        }

        bool CTranscodeModel::preFileFunction( const QFileInfo &fileInfo, std::unordered_set< QString > & /*alreadyAdded*/, TParentTree & /*tree*/, bool countOnly )
        {
            if ( countOnly )
                return true;

            auto mediaInfo = getMediaInfo( fileInfo );
            auto transcodeNeeded = NPreferences::NCore::STranscodeNeeded( mediaInfo );
            return transcodeNeeded.transcodeNeeded();
        }

        void CTranscodeModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {
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
    }
}
