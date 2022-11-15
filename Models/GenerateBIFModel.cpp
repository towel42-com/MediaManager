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

#include "GenerateBIFModel.h"
#include "Preferences/Core/Preferences.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/MKVUtils.h"
#include "SABUtils/BIFFile.h"

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
        CGenerateBIFModel::CGenerateBIFModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CGenerateBIFModel::~CGenerateBIFModel()
        {
        }

        std::pair< bool, QStandardItem * > CGenerateBIFModel::processItem( const QStandardItem * item, bool displayOnly )
        {
            if ( item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, nullptr );

            SProcessInfo processInfo;
            processInfo.fSetMKVTagsOnSuccess = false;
            processInfo.fOldName = item->data( ECustomRoles::eAbsFilePath ).toString();
            auto fi = QFileInfo( processInfo.fOldName );
            processInfo.fNewName = fi.absoluteDir().absoluteFilePath( fi.completeBaseName() + "-320-10.bif" );
            processInfo.fItem = new QStandardItem( QString( "Generate Thumbnail from '%1' => '%2'" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewName ) ) );
            processInfo.fItem->setData( processInfo.fOldName, ECustomRoles::eOldName );
            processInfo.fItem->setData( processInfo.fNewName, ECustomRoles::eNewName );

            bool aOK = true;
            QStandardItem * myItem = nullptr;
            fFirstProcess = true;
            if ( !displayOnly )
            {
                processInfo.fMaximum = NSABUtils::getNumberOfSeconds( processInfo.fOldName );;
                processInfo.fCmd = NPreferences::NCore::CPreferences::instance()->getFFMpegEXE();

                if ( processInfo.fCmd.isEmpty() || !QFileInfo( processInfo.fCmd ).isExecutable() )
                {
                    QStandardItem * errorItem = nullptr;
                    if ( processInfo.fCmd.isEmpty() )
                        appendError( processInfo.fItem, tr( "ffmpeg is not set properly" ) );
                    else
                        appendError( processInfo.fItem, tr( "ffmpeg '%1' is not an executable" ).arg( processInfo.fCmd ) );
                    aOK = false;
                }

                aOK = aOK && checkProcessItemExists( processInfo.fOldName, processInfo.fItem );
                processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps( processInfo.fOldName );

                if ( NPreferences::NCore::CPreferences::instance()->keepTempDir() )
                {
                    processInfo.fTempDir = std::make_shared< QTemporaryDir >();
                    processInfo.fTempDir->setAutoRemove( true );
                }
                else
                {
                    processInfo.fTempDir = std::make_shared< QTemporaryDir >( QFileInfo( processInfo.fOldName ).absoluteDir().absoluteFilePath( "./TempDir-XXXXXX" ) );
                    processInfo.fTempDir->setAutoRemove( false );
                }
                qDebug() << processInfo.fTempDir->path();

// eg -f matroska -threads 1 -skip_interval 10 -copyts -i file:"/volume2/video/Movies/Westworld (1973) [tmdbid=2362]/Westworld.mkv" -an -sn -vf "scale=w=320:h=133" -vsync cfr -r 0.1 -f image2 "/var/packages/EmbyServer/var/cache/temp/112d22a09fea457eaea27c4b0c88f790/img_%05d.jpg"
                processInfo.fArgs = QStringList()
                    << "-f" << "matroska"
                    << "-threads" << "1";
                processInfo.fArgs << "-skip_interval" << QString::number( NPreferences::NCore::CPreferences::instance()->imageInterval() );
                
                processInfo.fArgs
                    << "-copyts"
                    << "-i"
                    << processInfo.fOldName
                    << "-an"
                    << "-sn"
                    << "-vf" << "scale=w=320:h=133"
                    << "-vsync" << "cfr"
                    << "-r" << "0.1"
                    << "-f" << "image2"
                    << processInfo.fTempDir->filePath( "img_%05d.jpg" )
                    ;
                processInfo.fBackupOrig = false;
                processInfo.fPostProcess =
                    [processInfo]( const SProcessInfo * processInfo, QString & msg ) -> bool
                {
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

                    auto bifFile = NSABUtils::NBIF::CFile( dir, "img_*.jpg", NPreferences::NCore::CPreferences::instance()->imageInterval() * 1000, msg );
                    if ( !bifFile.isValid() )
                        return false;
                    return bifFile.save( processInfo->fNewName, msg );
                };

                fProcessQueue.push_back( processInfo );
                QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
            }
            myItem = processInfo.fItem;
            return std::make_pair( aOK, myItem );
        }

        QString CGenerateBIFModel::getProgressLabel( const SProcessInfo & processInfo ) const
        {
            auto retVal = QString( "Generating Thumbnails<ul><li>%1</li>to<li>%2</li></ul>" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewName ) );
            return retVal;
        }

        QStringList CGenerateBIFModel::headers() const
        {
            return CDirModel::headers();
        }

        void CGenerateBIFModel::postLoad( QTreeView * treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CGenerateBIFModel::preLoad( QTreeView * treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CGenerateBIFModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        void CGenerateBIFModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/ )
        {

        }

        bool CGenerateBIFModel::preFileFunction( const QFileInfo & fileInfo, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/ )
        {
            auto dir = fileInfo.absoluteDir();
            //qDebug() << fileInfo;
            QFileInfo bifFile;
            if ( fileInfo.exists() && fileInfo.isFile() )
            {
                auto filter = fileInfo.completeBaseName() + "-320-10.bif";
                auto bifFiles = NSABUtils::NFileUtils::findFilesInDir( dir, QStringList() << filter, false );
                if ( bifFiles.length() != 1 )
                    return true;
                bifFile = bifFiles.front();
            }

            if ( !bifFile.exists() || !bifFile.isFile() )
                return true;

            return false;
        }

        void CGenerateBIFModel::postDirFunction( bool aOK, const QFileInfo & dirInfo, TParentTree & parentTree )
        {
            if ( !aOK )
                return;
            (void)dirInfo;
            (void)parentTree;
        }


        void CGenerateBIFModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {

        }

    }
}
