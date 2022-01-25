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
#include "Preferences.h"
#include "SABUtils/DoubleProgressDlg.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/MKVUtils.h"

#include <QDir>
#include <QTimer>

namespace NMediaManager
{
    namespace NCore
    {
        CMakeMKVModel::CMakeMKVModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CMakeMKVModel::~CMakeMKVModel()
        {
        }

        int64_t CMakeMKVModel::getNumberOfSeconds( const QString & fileName ) const
        {
            return NSABUtils::getNumberOfSeconds( fileName, NCore::CPreferences::instance()->getFFProbeEXE() );
        }

        std::pair< bool, QStandardItem * > CMakeMKVModel::processItem( const QStandardItem * item, QStandardItem * parentItem, bool displayOnly ) const
        {
            if ( item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, nullptr );

            SProcessInfo processInfo;
            processInfo.fSetMKVTagsOnSuccess = true;
            processInfo.fOldName = item->data( ECustomRoles::eFullPathRole ).toString();
            auto fi = QFileInfo( processInfo.fOldName );
            processInfo.fNewName = fi.absoluteDir().absoluteFilePath( fi.completeBaseName() + ".mkv" );
            processInfo.fItem = new QStandardItem( QString( "Convert '%1' => '%2'" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewName ) ) );
            processInfo.fItem->setData( processInfo.fOldName, ECustomRoles::eOldName );
            processInfo.fItem->setData( processInfo.fNewName, ECustomRoles::eNewName );
            if ( parentItem )
                parentItem->appendRow( processInfo.fItem );
            else
                fProcessResults.second->appendRow( processInfo.fItem );

            bool aOK = true;
            QStandardItem * myItem = nullptr;
            fFirstProcess = true;
            if ( !displayOnly )
            {
                int numSeconds = getNumberOfSeconds( processInfo.fOldName );
                progressDlg()->setSecondaryMaximum( numSeconds );

                processInfo.fCmd = CPreferences::instance()->getFFMpegEXE();
                if ( processInfo.fCmd.isEmpty() || !QFileInfo( processInfo.fCmd ).isExecutable() )
                {
                    QStandardItem * errorItem = nullptr;
                    if ( processInfo.fCmd.isEmpty() )
                        errorItem = new QStandardItem( QString( "ERROR: ffmpeg is not set properly" ) );
                    else
                        errorItem = new QStandardItem( QString( "ERROR: ffmpeg '%1' is not an executable" ).arg( processInfo.fCmd ) );

                    errorItem->setData( ECustomRoles::eIsErrorNode, true );
                    appendError( processInfo.fItem, errorItem );

                    QIcon icon;
                    icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                    errorItem->setIcon( icon );
                    aOK = false;
                }

                aOK = aOK && checkProcessItemExists( processInfo.fOldName, processInfo.fItem );
                processInfo.fTimeStamps = NSABUtils::NFileUtils::timeStamps( processInfo.fOldName );

                processInfo.fArgs = QStringList()
                    << "-y"
                    << "-fflags"
                    << "+genpts"
                    << "-i"
                    << processInfo.fOldName
                    << "-c:v" << "copy"
                    << "-c:a" << "copy"
                    << processInfo.fNewName;
                fProcessQueue.push_back( processInfo );
                QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
            }
            myItem = processInfo.fItem;
            return std::make_pair( aOK, myItem );
        }

        QString CMakeMKVModel::getProgressLabel( const SProcessInfo & processInfo ) const
        {
            return QString( "Converting '%1' to '%2'" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewName ) );
        }

        QStringList CMakeMKVModel::headers() const
        {
            return CDirModel::headers()
                << tr( "Title" ) << tr( "Media Date" ) << tr( "Comment" )
                ;
        }

        void CMakeMKVModel::setupNewItem( const SDirNodeItem & /*nodeItem*/, const QStandardItem * /*nameItem*/, QStandardItem * /*item*/ ) const
        {

        }

        void CMakeMKVModel::postLoad( QTreeView * treeView)
        {
            CDirModel::postLoad(treeView);
        }

        void CMakeMKVModel::preLoad(QTreeView * treeView)
        {
            CDirModel::preLoad(treeView);
        }

        void CMakeMKVModel::postProcess(bool /*displayOnly*/)
        {
            if (progressDlg())
                progressDlg()->setValue(0);
        }

        int CMakeMKVModel::computeNumberOfItems() const
        {
            return NSABUtils::itemCount( fProcessResults.second.get(), true );
        }

        void CMakeMKVModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/ )
        {

        }

        bool CMakeMKVModel::preFileFunction( const QFileInfo & /*fileInfo*/, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/ )
        {
            return true;
        }

        void CMakeMKVModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {

        }

    }
}
