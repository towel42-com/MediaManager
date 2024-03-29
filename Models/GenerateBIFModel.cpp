// The MIT License( MIT )
//
// Copyright( c ) 2020-2023 Scott Aron Bloom
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

#include "GenerateBIFModel.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/BackupFile.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/MediaInfo.h"
#include "SABUtils/BIFFile.h"
#include "SABUtils/GIFWriterDlg.h"

#include <QDir>
#include <QTimer>
#include <QDebug>
#include <QTemporaryDir>

#ifndef NDEBUG
    #define DEBUG_TEMP_DIR
#endif

namespace NMediaManager
{
    namespace NModels
    {
        CGenerateBIFModel::CGenerateBIFModel( NUi::CBasePage *page, QObject *parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CGenerateBIFModel::~CGenerateBIFModel()
        {
        }

        QStringList CGenerateBIFModel::dirModelFilter() const
        {
            return QStringList() << "*.mkv";
        }

        std::pair< bool, std::list< QStandardItem * > > CGenerateBIFModel::processItem( const QStandardItem *item, bool displayOnly )
        {
            if ( item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, std::list< QStandardItem * >() );

            auto processInfo = std::make_shared< SProcessInfo >();
            processInfo->fSetMetainfoTagsOnSuccess = false;
            processInfo->fOldName = item->data( ECustomRoles::eAbsFilePath ).toString();
            auto fi = QFileInfo( processInfo->fOldName );
            auto sz = NPreferences::NCore::CPreferences::instance()->getThumbnailSize( fi );

            processInfo->fItem = new QStandardItem( QString( "Generate Thumbnail Videos from '%1'" ).arg( getDispName( processInfo->fOldName ) ) );
            processInfo->fItem->setData( processInfo->fOldName, ECustomRoles::eOldName );

            if ( NPreferences::NCore::CPreferences::instance()->generateBIF() )
            {
                auto bifName = NPreferences::NCore::CPreferences::instance()->getImageFileName( fi, sz, "bif" );
                processInfo->fNewNames << bifName;

                auto bifItem = new QStandardItem( tr( "BIF File => '%1'" ).arg( getDispName( processInfo->primaryNewName() ) ) );
                processInfo->fItem->appendRow( bifItem );
            }

            if ( NPreferences::NCore::CPreferences::instance()->generateGIF() )
            {
                auto gifName = NPreferences::NCore::CPreferences::instance()->getImageFileName( fi, sz, "gif" );
                processInfo->fNewNames << gifName;

                auto gifItem = new QStandardItem( tr( "GIF File => '%1'" ).arg( getDispName( gifName ) ) );
                processInfo->fItem->appendRow( gifItem );
            }
            processInfo->fItem->setData( processInfo->fNewNames, ECustomRoles::eNewName );

            bool aOK = true;
            QStandardItem *myItem = nullptr;
            fFirstProcess = true;
            if ( !displayOnly )
            {
                processInfo->fMaximum = NSABUtils::CMediaInfo::getNumberOfSeconds( processInfo->fOldName );

                bool isEmbyEXE = false;
                processInfo->fCmd = NPreferences::NCore::CPreferences::instance()->getFFMpegEmbyEXE();
                if ( !processInfo->fCmd.isEmpty() && QFileInfo( processInfo->fCmd ).isExecutable() )
                    isEmbyEXE = true;
                else
                {
                    processInfo->fCmd = NPreferences::NCore::CPreferences::instance()->getFFMpegEXE();
                }

                if ( processInfo->fCmd.isEmpty() || !QFileInfo( processInfo->fCmd ).isExecutable() )
                {
                    QStandardItem *errorItem = nullptr;
                    if ( processInfo->fCmd.isEmpty() )
                        appendError( processInfo->fItem, tr( "ffmpeg is not set properly" ) );
                    else
                        appendError( processInfo->fItem, tr( "ffmpeg '%1' is not an executable" ).arg( processInfo->fCmd ) );
                    aOK = false;
                }

                aOK = aOK && checkProcessItemExists( processInfo->fOldName, processInfo->fItem );
                processInfo->fTimeStamps = NSABUtils::NFileUtils::timeStamps( processInfo->fOldName );

                if ( NPreferences::NCore::CPreferences::instance()->keepTempDir() )
                {
                    processInfo->fTempDir = std::make_shared< QTemporaryDir >();
                    processInfo->fTempDir->setAutoRemove( false );
                }
                else
                {
                    processInfo->fTempDir = std::make_shared< QTemporaryDir >( QFileInfo( processInfo->fOldName ).absoluteDir().absoluteFilePath( "./TempDir-XXXXXX" ) );
                    processInfo->fTempDir->setAutoRemove( true );
                }
                //qDebug() << processInfo->fTempDir->path();

                // eg -f matroska -threads 1 -skip_interval 10 -copyts -i file:"/volume2/video/Movies/Westworld (1973) [tmdbid=2362]/Westworld.mkv" -an -sn -vf "scale=w=320:h=133" -vsync cfr -r 0.1 -f image2 "/var/packages/EmbyServer/var/cache/temp/112d22a09fea457eaea27c4b0c88f790/img_%05d.jpg"
                processInfo->fArgs = QStringList() << "-hide_banner" << "-f" << "matroska"   // input format
                    ;
                auto hwAccel = NPreferences::NCore::CPreferences::instance()->getTranscodeHWAccel();
                if ( !hwAccel.isEmpty() )
                {
                    processInfo->fArgs << "-hwaccel" << hwAccel;
                }
                processInfo->fArgs << "-threads" << "1"   // num threads
                    ;
                if ( isEmbyEXE )
                {
                    processInfo->fArgs << "-skip_interval" << QString::number( NPreferences::NCore::CPreferences::instance()->imageInterval() );   // how often to skip
                }

                processInfo->fArgs << "-copyts"   // ignore processing timestamps
                                  << "-i" << processInfo->fOldName   // input file
                                  << "-an"   // no audio
                                  << "-sn"   // no subtitles
                                  << "-vf" << QString( "scale=w=%1:h=%2" ).arg( sz.width() ).arg( sz.height() ) << "-vsync"
                                  << "cfr"   // constant frame  videwo sync method
                                  << "-f"
                                  << "image2" << processInfo->fTempDir->filePath( "img_%05d.jpg" );
                processInfo->fBackupOrig = false;
                processInfo->fPostProcess = [ this ]( const SProcessInfo * processInfo, QString &msg ) -> bool
                {
                    if ( !processInfo )
                        return false;

                    progressDlg()->setPrimaryValue( progressDlg()->primaryValue() + 1 );
                    if ( !processInfo || !processInfo->fTempDir )
                    {
                        msg = "Temporary directory not set";
                        return false;
                    }
                    auto dir = QDir( processInfo->fTempDir->path() );
                    if ( !dir.exists() )
                    {
                        msg = "Temporary directory does not exist";
                        return false;
                    }

                    bool aOK = true;
                    QString errorMsg;
                    auto allImages = NSABUtils::NFileUtils::findAllFiles( dir, { "img_*.jpg" }, false, true, &errorMsg );
                    if ( !allImages.has_value() || allImages.value().isEmpty() )
                    {
                        msg = QString( "No images exists in dir '%1' of the format 'img_*.jpg' - %2" ).arg( dir.absolutePath() ).arg( errorMsg );
                        return false;
                    }

                    if ( NPreferences::NCore::CPreferences::instance()->generateBIF() )
                    {
                        auto bifFile = std::make_shared< NSABUtils::NBIF::CFile >( allImages.value(), NPreferences::NCore::CPreferences::instance()->imageInterval() * 1000, msg );
                        if ( !bifFile->isValid() )
                            return false;
                        aOK = bifFile->save( processInfo->primaryNewName(), msg );
                    }
                    progressDlg()->setPrimaryValue( progressDlg()->primaryValue() + 1 );
                    if ( aOK && NPreferences::NCore::CPreferences::instance()->generateGIF() )
                    {
                        auto fi = QFileInfo( processInfo->fNewNames.back() );
                        if ( !NSABUtils::NFileUtils::backup( processInfo->fNewNames.back() ) )
                        {
                            CDirModel::appendError( processInfo->fItem, QObject::tr( "%1: FAILED TO BACKUP" ).arg( getDispName( processInfo->fNewNames.back() ) ) );
                            return false;
                        }

                        aOK = NSABUtils::CGIFWriterDlg::saveToGIF(
                            nullptr, processInfo->fNewNames.back(), allImages.value(), NPreferences::NCore::CPreferences::instance()->gifDitherImage(), NPreferences::NCore::CPreferences::instance()->gifFlipImage(),
                            NPreferences::NCore::CPreferences::instance()->gifLoopCount(), NPreferences::NCore::CPreferences::instance()->gifDelay(),
                            [ this, fi, processInfo ]( size_t min, size_t max )
                            {
                                if ( progressDlg() )
                                {
                                    progressDlg()->setCancelButtonText( tr( "Cancel Generating GIF" ) );
                                    progressDlg()->setLabelText( getProgressLabel( processInfo, false ) );
                                    progressDlg()->setSecondaryProgressLabel( "Current Frame" );
                                    progressDlg()->setSecondaryRange( static_cast< int >( min ), static_cast< int >( max ) );
                                }
                            },
                            [ this ]( size_t value )
                            {
                                if ( progressDlg() )
                                    progressDlg()->setSecondaryValue( static_cast< int >( value ) );
                            },
                            [ this ]() -> bool
                            {
                                if ( progressDlg() )
                                    return progressDlg()->wasCanceled();
                                return false;
                            } );
                    }
                    progressDlg()->setPrimaryValue( progressDlg()->primaryValue() + 1 );
                    return aOK;
                };

                fProcessQueue.push_back( processInfo );
                QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
            }
            myItem = processInfo->fItem;
            return std::make_pair( aOK, std::list< QStandardItem * >( { myItem } ) );
        }

        QString CGenerateBIFModel::getProgressLabel( std::shared_ptr< SProcessInfo > processInfo ) const
        {
            return getProgressLabel( processInfo.get(), true );
        }

        QString CGenerateBIFModel::getProgressLabel( const SProcessInfo * processInfo, bool bif ) const
        {
            if ( !processInfo )
                return {};

            auto retVal = QString( "Generating Thumbnail Videos<ul><li>%1</li>to<li>%2</li></ul>" ).arg( getDispName( processInfo->fOldName ) ).arg( getDispName( bif ? processInfo->primaryNewName() : processInfo->fNewNames.back() ) );
            return retVal;
        }

        QStringList CGenerateBIFModel::headers() const
        {
            return CDirModel::headers();
        }

        void CGenerateBIFModel::postLoad( QTreeView *treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CGenerateBIFModel::preLoad( QTreeView *treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CGenerateBIFModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        void CGenerateBIFModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
        }

        bool CGenerateBIFModel::preFileFunction( const QFileInfo &fileInfo, std::unordered_set< QString > & /*alreadyAdded*/, TParentTree & /*tree*/, bool countOnly )
        {
            if ( countOnly )
                return true;

            auto dir = fileInfo.absoluteDir();
            //qDebug() << fileInfo;
            if ( !fileInfo.exists() || !fileInfo.isFile() )
                return false;

            bool needsBIF = false;
            if ( NPreferences::NCore::CPreferences::instance()->generateBIF() )
            {
                auto filter = QFileInfo( NPreferences::NCore::CPreferences::instance()->getImageFileName( fileInfo, "bif" ) ).fileName();
                auto bifFiles = NSABUtils::NFileUtils::findAllFiles( dir, QStringList() << filter, false, false );
                needsBIF = !bifFiles.has_value() || ( bifFiles.value().length() != 1 );
                if ( !needsBIF && !bifFiles.value().empty() )
                {
                    auto bifFile = QFileInfo( bifFiles.value().front() );
                    needsBIF = !bifFile.exists() || !bifFile.isFile();
                }
            }

            bool needsGIF = false;
            if ( NPreferences::NCore::CPreferences::instance()->generateGIF() )
            {
                auto filter = QFileInfo( NPreferences::NCore::CPreferences::instance()->getImageFileName( fileInfo, "gif" ) ).fileName();
                auto gifFiles = NSABUtils::NFileUtils::findAllFiles( dir, QStringList() << filter, false, false );
                needsGIF = !gifFiles.has_value() || ( gifFiles.value().length() != 1 );
                if ( !needsGIF && !gifFiles.value().empty() )
                {
                    auto gifFile = QFileInfo( gifFiles.value().front() );
                    needsGIF = !gifFile.exists() || !gifFile.isFile();
                }
            }

            return needsBIF || needsGIF;
        }

        void CGenerateBIFModel::postDirFunction( bool aOK, const QFileInfo &dirInfo, TParentTree &parentTree, bool /*countOnly*/ )
        {
            if ( !aOK )
                return;
            (void)dirInfo;
            (void)parentTree;
        }

        void CGenerateBIFModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {
        }

        std::optional< std::pair< uint64_t, std::optional< uint64_t > > > CGenerateBIFModel::getCurrentProgress( const QString &string )
        {
            // Skip-Option - Write output: pkt_pts_time:2570 pkt_dts_time:2570 input_pts_time:2570.2
            // time=00:00:00.00
            auto regEx = QRegularExpression( R"((input_pts_[Tt]ime\:(?<secs1>\d+))|(pkt_pts_[Tt]ime\:(?<secs2>\d+)))" );
            QRegularExpressionMatch match;
            auto pos = string.lastIndexOf( regEx, -1, &match );
            if ( pos == -1 || !match.hasMatch() )
                return {};

            auto secs = match.captured( "secs1" );
            if ( secs.isEmpty() )
                secs = match.captured( "secs2" );
            int numSeconds = 0;
            if ( !secs.isEmpty() )
            {
                bool aOK;
                int curr = secs.toInt( &aOK );
                if ( aOK )
                    numSeconds = curr;
            }

            return std::pair< uint64_t, std::optional< uint64_t > >( numSeconds, {} );
        }
    }
}
