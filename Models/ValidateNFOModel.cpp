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

#include "ValidateNFOModel.h"
#include "Preferences/Core/Preferences.h"
#include "T42-Utils/FileUtils.h"
#include "T42-Utils/DoubleProgressDlg.h"

#include <QDir>
#include <QTimer>
#include <QXmlStreamReader>

namespace NMediaManager
{
    namespace NModels
    {
        CValidateNFOModel::CValidateNFOModel( NUi::CBasePage *page, QObject *parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CValidateNFOModel::~CValidateNFOModel()
        {
        }

        QStringList CValidateNFOModel::dirModelFilter() const
        {
            return QStringList() << "*.nfo";
        }

        std::pair< bool, std::list< QStandardItem * > > CValidateNFOModel::processItem( const QStandardItem *item, bool displayOnly )
        {
            auto retVal = std::make_pair( true, std::list< QStandardItem * >() );

            if ( item->data( ECustomRoles::eIsDir ).toBool() )
                return retVal;

            qDebug() << item->data( ECustomRoles::eAbsFilePath ).toString();

            auto status = getItemStatus( item );
            if ( !status.has_value() || ( status.value().first == NPreferences::EItemStatus::eOK ) )
                return retVal;

            auto processInfo = std::make_shared< SProcessInfo >();
            processInfo->fOldName = item->data( ECustomRoles::eAbsFilePath ).toString();
            processInfo->fNewNames << processInfo->fOldName + ".bak";

            processInfo->fItem = new QStandardItem( QString( "Delete '%1'?" ).arg( getDispName( processInfo->fOldName ) ) );
            processInfo->fItem->setData( processInfo->fOldName, ECustomRoles::eOldName );
            processInfo->fBackupOrig = true;
            processInfo->fModifyTimestampsOnNewFiles = false;

            bool aOK = true;
            if ( !displayOnly )
            {
                if ( progressDlg() )
                {
                    progressDlg()->setLabelText( tr( "Renaming '%1' => '%2'" ).arg( getDispName( processInfo->fOldName ) ).arg( getDispName( processInfo->fOldName + ".bak" ) ) );
                }

                fProcessQueue.push_back( processInfo );
                QTimer::singleShot( 0, this, &CDirModel::slotRunNextProcessInQueue );
            }
            return std::make_pair( aOK, std::list< QStandardItem * >( { processInfo->fItem } ) );
        }

        QString CValidateNFOModel::getProgressLabel( std::shared_ptr< SProcessInfo > processInfo ) const
        {
            auto retVal = QString( "Removing NFO<ul><li>%1</li></ul>" ).arg( getDispName( processInfo->fOldName ) );
            return retVal;
        }

        QStringList CValidateNFOModel::headers() const
        {
            return CDirModel::headers();
        }

        void CValidateNFOModel::postLoad( QTreeView *treeView )
        {
            CDirModel::postLoad( treeView );
        }

        void CValidateNFOModel::preLoad( QTreeView *treeView )
        {
            CDirModel::preLoad( treeView );
        }

        void CValidateNFOModel::postProcess( bool /*displayOnly*/ )
        {
            if ( progressDlg() )
                progressDlg()->setValue( 0 );
        }

        void CValidateNFOModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/, bool /*countOnly*/ )
        {
        }

        bool CValidateNFOModel::preFileFunction( const QFileInfo &fileInfo, std::unordered_set< QString > & /*alreadyAdded*/, TParentTree & /*tree*/, bool countOnly )
        {
            if ( countOnly )
                return true;

            if ( fileInfo.isFile() )
            {
                auto match = tmdbidMatches( fileInfo );
                return !match.has_value() || !match.value().matches();
            }

            return true;
        }

        void CValidateNFOModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {
        }

        std::optional< std::pair< uint64_t, std::optional< uint64_t > > > CValidateNFOModel::getCurrentProgress( const QString &string )
        {
            auto msgRegEx = QRegularExpression( R"((?<msg>(ERR|WRN)[A-Fa-f0-9]{3}.*))" );
            auto ii = msgRegEx.globalMatch( string );

            while ( ii.hasNext() )
            {
                auto match = ii.next();
                auto msg = match.captured( "msg" ).trimmed();
                addMessageForFile( msg );
            }

            auto regEx = QRegularExpression( R"(Stage: (?<stageNum>\d+) -)" );

            QRegularExpressionMatch match;
            auto pos = string.lastIndexOf( regEx, -1, &match );
            if ( pos == -1 || !match.hasMatch() )
                return {};

            auto stage = match.captured( "stageNum" );
            int stageNum = 0;
            if ( !stage.isEmpty() )
            {
                bool aOK;
                int curr = stage.toInt( &aOK );
                if ( aOK )
                    stageNum = curr;
            }

            return std::pair< uint64_t, std::optional< uint64_t > >( stageNum, {} );
        }

        std::optional< NModels::TItemStatus > CValidateNFOModel::computeItemStatus( const QModelIndex &idx ) const
        {
            if ( isRootPath( idx ) )
                return {};

            auto fileInfo = this->fileInfo( idx );

            auto matches = tmdbidMatches( fileInfo );
            if ( !matches.has_value() )
                return {};

            auto retVal = std::make_pair( NPreferences::EItemStatus::eOK, QString() );
            if ( !matches.value().matches() )
            {
                retVal = std::make_pair( NPreferences::EItemStatus::eError, tr( "TMDB value in NFO is '%1' should be '%2'" ).arg( matches.value().fNFOTMDBID ).arg( matches.value().fPathTMDBID ) );
            }

            return retVal;
        }

        std::optional< CValidateNFOModel::STMDBInfo > CValidateNFOModel::tmdbidMatches( const QFileInfo &fileInfo ) const
        {
            if ( fileInfo.isDir() )
                return {};
            auto path = fileInfo.absoluteFilePath();

            QRegularExpression regEx( R"__(\[tmdbid=(\d+)\])__" );
            auto match = regEx.match( path );
            if ( !match.hasMatch() )
                return {};

            STMDBInfo retVal;
            retVal.fPathTMDBID = match.captured( 1 );

            QFile file( path );
            if ( !file.open( QFile::ReadOnly | QFile::Text ) )
                return {};
            QXmlStreamReader reader( &file );

            QStringList elementStack;
            bool inMovie = false;
            while ( !reader.atEnd() && !reader.hasError() )
            {
                reader.readNext();
                if ( reader.isStartElement() )
                {
                    auto &&name = reader.name().toString();
                    elementStack.push_back( name );
                    if ( name == "movie" )
                    {
                        inMovie = true;
                    }
                    else if ( inMovie && ( elementStack.size() == 2 ) && ( name == "tmdbid" ) )
                    {
                        retVal.fNFOTMDBID = reader.readElementText();
                        break;
                    }
                }
                else if ( reader.isEndElement() )
                {
                    auto &&name = reader.name().toString();
                    if ( elementStack.empty() )
                        continue;

                    Q_ASSERT( !elementStack.empty() );
                    Q_ASSERT( elementStack.back() == name );
                    if ( name == "movie" )
                    {
                        inMovie = false;
                    }
                    elementStack.pop_back();
                }
            }

            file.close();
            if ( retVal.fNFOTMDBID.isEmpty() )
                return {};
            return retVal;
        }

    }
}
