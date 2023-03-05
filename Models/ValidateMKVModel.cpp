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

#include "ValidateMKVModel.h"
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
        CValidateMKVModel::CValidateMKVModel( NUi::CBasePage *page, QObject *parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CValidateMKVModel::~CValidateMKVModel()
        {
        }

        QStringList CValidateMKVModel::dirModelFilter() const
        {
            return QStringList() << "*.mkv";
        }

        std::pair< bool, QStandardItem * > CValidateMKVModel::processItem( const QStandardItem *item, bool displayOnly )
        {
            if ( item->data( ECustomRoles::eIsDir ).toBool() )
                return std::make_pair( true, nullptr );

            SProcessInfo processInfo;
            processInfo.fSetMKVTagsOnSuccess = true;
            processInfo.fOldName = item->data( ECustomRoles::eAbsFilePath ).toString();
            auto fi = QFileInfo( processInfo.fOldName );
            processInfo.fItem = new QStandardItem( QString( "Validate '%1'" ).arg( getDispName( processInfo.fOldName ) ) );
            processInfo.fItem->setData( processInfo.fOldName, ECustomRoles::eOldName );

            bool aOK = true;
            QStandardItem *myItem = nullptr;
            fFirstProcess = true;
            if ( !displayOnly )
            {
                processInfo.fUnbuffered = true;
                processInfo.fMaximum = QFileInfo( processInfo.fOldName ).size();

                processInfo.fCmd = NPreferences::NCore::CPreferences::instance()->getMKVValidatorEXE();
                if ( processInfo.fCmd.isEmpty() || !QFileInfo( processInfo.fCmd ).isExecutable() )
                {
                    QStandardItem *errorItem = nullptr;
                    if ( processInfo.fCmd.isEmpty() )
                        appendError( processInfo.fItem, tr( "mkvalidator is not set properly" ) );
                    else
                        appendError( processInfo.fItem, tr( "mkvalidator '%1' is not an executable" ).arg( processInfo.fCmd ) );
                    aOK = false;
                }

                aOK = aOK && checkProcessItemExists( processInfo.fOldName, processInfo.fItem );

                processInfo.fArgs = QStringList() 
                    << "--no-warn"
                    << "--quick"
                    //<< "--quiet"
                    << processInfo.fOldName
                    ;
                if ( aOK )
                {
                    fProcessQueue.push_back( processInfo );
                    QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
                }
            }
            myItem = processInfo.fItem;
            return std::make_pair( aOK, myItem );
        }

        QString CValidateMKVModel::getProgressLabel( const SProcessInfo &processInfo ) const
        {
            auto retVal = QString( "Validating MKV<ul><li>%1</li></ul>" ).arg( getDispName( processInfo.fOldName ) );
            return retVal;
        }

        QStringList CValidateMKVModel::headers() const
        {
            return CDirModel::headers() << getMediaHeaders();
            ;
        }

        void CValidateMKVModel::postLoad( QTreeView *treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CValidateMKVModel::preLoad( QTreeView *treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CValidateMKVModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        void CValidateMKVModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
        }

        bool CValidateMKVModel::preFileFunction( const QFileInfo & /*fileInfo*/, std::unordered_set< QString > & /*alreadyAdded*/, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
            return true;
        }

        void CValidateMKVModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {
        }

    }
}
