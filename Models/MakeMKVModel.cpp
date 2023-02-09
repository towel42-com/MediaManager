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
#include "SABUtils/MKVUtils.h"

#include <QDir>
#include <QTimer>

namespace NMediaManager
{
    namespace NModels
    {
        CMakeMKVModel::CMakeMKVModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CMakeMKVModel::~CMakeMKVModel()
        {
        }

        QStringList CMakeMKVModel::dirModelFilter() const
        {
            return NPreferences::NCore::CPreferences::instance()->getNonMKVMediaExtensions();
        }

        std::pair< bool, QStandardItem * > CMakeMKVModel::processItem( const QStandardItem * item, bool displayOnly )
        {
            if ( item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, nullptr );

            SProcessInfo processInfo;
            processInfo.fSetMKVTagsOnSuccess = true;
            processInfo.fOldName = item->data( ECustomRoles::eAbsFilePath ).toString();
            auto fi = QFileInfo( processInfo.fOldName );
            processInfo.fNewNames << fi.absoluteDir().absoluteFilePath( fi.completeBaseName() + ".mkv" );
            processInfo.fItem = new QStandardItem( QString( "Convert '%1' => '%2'" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewNames.front() ) ) );
            processInfo.fItem->setData( processInfo.fOldName, ECustomRoles::eOldName );
            processInfo.fItem->setData( processInfo.fNewNames, ECustomRoles::eNewName );

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

                processInfo.fArgs = QStringList()
                    << "-y"
                    << "-fflags"
                    << "+genpts"
                    << "-i"
                    << processInfo.fOldName
                    << "-c:v" << "copy"
                    << "-c:a" << "copy"
                    << processInfo.fNewNames;
                fProcessQueue.push_back( processInfo );
                QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
            }
            myItem = processInfo.fItem;
            return std::make_pair( aOK, myItem );
        }

        QString CMakeMKVModel::getProgressLabel( const SProcessInfo & processInfo ) const
        {
            auto retVal = QString( "Converting to MKV<ul><li>%1</li>to<li>%2</li></ul>" ).arg( getDispName( processInfo.fOldName ) ).arg( getDispName( processInfo.fNewNames.front() ) );
            return retVal;
        }

        QStringList CMakeMKVModel::headers() const
        {
            return CDirModel::headers()
                << getMediaHeaders();
            ;
        }

        void CMakeMKVModel::postLoad( QTreeView * treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CMakeMKVModel::preLoad( QTreeView * treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CMakeMKVModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        void CMakeMKVModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/ )
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
