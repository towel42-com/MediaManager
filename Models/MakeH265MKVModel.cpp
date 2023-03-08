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

#include "MakeH265MKVModel.h"
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
        CMakeH265MKVModel::CMakeH265MKVModel( NUi::CBasePage *page, QObject *parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CMakeH265MKVModel::~CMakeH265MKVModel()
        {
        }

        std::optional< NMediaManager::NModels::TItemStatus > CMakeH265MKVModel::computeItemStatus( const QModelIndex &idx ) const
        {
            if ( isRootPath( idx ) )
                return {};

            auto mediaInfo = getMediaInfo( idx );
            if ( !mediaInfo )
                return {};

            if ( idx.column() == getMediaVideoCodecLoc() )
            {
                auto videoCodec = idx.data().toString();
                if ( !NSABUtils::CMediaInfo::isHEVCVideo( videoCodec ) )
                {
                    auto fileInfo = this->fileInfo( idx );
                    auto msg = tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using the H.265 video codec</p>" ).arg( fileInfo.fileName() );
                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg );
                }
            }
            else if ( idx.column() == 0 ) // filename
            {
                auto fileInfo = this->fileInfo( idx );
                if ( fileInfo.suffix().toLower() != "mkv" )
                {
                    auto msg = tr( "<p style='white-space:pre'>File <b>'%1'</b> is not using a MKV container</p>" ).arg( fileInfo.fileName() );
                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg );
                }
            }
            return {};
        }

        QStringList CMakeH265MKVModel::dirModelFilter() const
        {
            return NPreferences::NCore::CPreferences::instance()->getVideoExtensions();
        }

        std::pair< bool, QStandardItem * > CMakeH265MKVModel::processItem( const QStandardItem *item, bool displayOnly )
        {
            if ( item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, nullptr );

            auto path = item->data( ECustomRoles::eAbsFilePath ).toString();

            auto mediaInfo = getMediaInfo( path );
            auto isH265 = mediaInfo && mediaInfo->isHEVCVideo();
            auto fi = QFileInfo( path );
            bool isMKV = ( fi.suffix().toLower() == "mkv" );
            bool convertToH265 = !isH265 && NPreferences::NCore::CPreferences::instance()->getConvertToH265();
            if ( isMKV && !convertToH265 )
                return std::make_pair( true, nullptr );

            SProcessInfo processInfo;
            processInfo.fSetMKVTagsOnSuccess = true;
            processInfo.fOldName = path;
         
            auto newBaseName = fi.completeBaseName() + ".mkv";
            if ( isMKV )
                newBaseName = fi.completeBaseName() + ".mkv.new";

            processInfo.fNewNames << fi.absoluteDir().absoluteFilePath( newBaseName );

            auto msg = tr( "Convert container for '%1' => '%2'" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewNames.front() ) );
            if ( convertToH265 )
                msg += tr( " and transcode into H.265." );
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

                processInfo.fArgs = NPreferences::NCore::CPreferences::instance()->getConvertToMKVArgs( isH265, processInfo.fOldName, processInfo.fNewNames.front() );

                fProcessQueue.push_back( processInfo );
                QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
            }
            myItem = processInfo.fItem;
            return std::make_pair( aOK, myItem );
        }

        QString CMakeH265MKVModel::getProgressLabel( const SProcessInfo &processInfo ) const
        {
            auto retVal = QString( "Converting to H.265 MKV<ul><li>%1</li>to<li>%2</li></ul>" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewNames.front() ) );
            return retVal;
        }

        QStringList CMakeH265MKVModel::headers() const
        {
            return CDirModel::headers() << getMediaHeaders();
            ;
        }

        void CMakeH265MKVModel::postLoad( QTreeView *treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CMakeH265MKVModel::preLoad( QTreeView *treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CMakeH265MKVModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        void CMakeH265MKVModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
        }

        bool CMakeH265MKVModel::preFileFunction( const QFileInfo & fileInfo, std::unordered_set< QString > & /*alreadyAdded*/, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
            auto ext = fileInfo.suffix().toLower();
            if ( ext != "mkv" )
                return true;

            static bool showAll = false;
#if defined(DEBUG) && defined( MAKEMKVMODEL_SHOWALL )
            showAll = true;
#endif;
            if ( showAll )
                return true;

            if ( !NPreferences::NCore::CPreferences::instance()->getConvertToH265() )
                return false;

            auto mediaInfo = getMediaInfo( fileInfo );
            if ( !mediaInfo )
                return false;

            return !mediaInfo->isHEVCVideo();
        }

        void CMakeH265MKVModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {
        }

    }
}
