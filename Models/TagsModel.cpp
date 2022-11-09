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

#include "TagsModel.h"
#include "Core/SearchTMDBInfo.h"
#include "Preferences/Core/Preferences.h"
#include "Core/TransformResult.h"

#include "SABUtils/MKVUtils.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/FileUtils.h"
#include "SABUtils/FileCompare.h"
#include "SABUtils/DoubleProgressDlg.h"

#include <QDir>
#include <QTimer>
#include <QTreeView>
#include <QApplication>
#include <QVariant>
#include <QLocale>

#include <QDirIterator>

namespace NMediaManager
{
    namespace NModels
    {
        CTagsModel::CTagsModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CTagsModel::~CTagsModel()
        {
        }

        std::pair< bool, QStandardItem * > CTagsModel::processItem( const QStandardItem * item, bool displayOnly )
        {
            auto fi = fileInfo( item );
            if ( !isMediaFile( fi ) )
                return std::make_pair( true, nullptr );

            QStandardItem * myItem = nullptr;
            bool aOK = true;

            if ( !areMediaTagsSameAsAutoSet( indexFromItem( item ) ) )
            {
                auto oldName = computeTransformPath( item, true );
                myItem = new QStandardItem( getDispName( oldName ) );

                auto idx = indexFromItem( item );
                QString msg;
                if ( !isTitleSameAsAutoSet( idx, &msg ) )
                {
                    myItem->appendRow( new QStandardItem( msg ) );
                }

                if ( !isDateSameAsAutoSet( idx, &msg ) )
                {
                    myItem->appendRow( new QStandardItem( msg ) );
                }

                if ( !isCommentSameAsAutoSet( idx, &msg ) )
                {
                    myItem->appendRow( new QStandardItem( msg ) );
                }
                if ( !displayOnly )
                {
                    if ( progressDlg() )
                    {
                        auto fileName = fileInfo( idx ).absoluteFilePath();
                        progressDlg()->setLabelText( tr( "Processing file '%1'" ).arg( QDir( fRootPath ).relativeFilePath( fileName ) ) );
                    }

                    QString msg;
                    if ( !autoSetMediaTags( idx, &msg ) )
                    {
                        appendError( myItem, msg );
                        aOK = false;
                    }
                }
            }

            if ( progressDlg() )
                progressDlg()->setValue( progressDlg()->value() + 1 );
            return std::make_pair( aOK, myItem );
        }

        void CTagsModel::attachTreeNodes( QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/ )
        {

        }

        void CTagsModel::preLoad( QTreeView * /*treeView*/ )
        {
            fTagsBeingShown = NPreferences::NCore::CPreferences::instance()->getEnabledTags();
            fFirstColumn = -1;
            int colNum = EColumns::eMediaColumnLoc;
            for ( auto && ii : fTagsBeingShown )
            {
                if ( ii == NSABUtils::EMediaTags::eTitle )
                    fTitleColumn = colNum;
                else if ( ii == NSABUtils::EMediaTags::eLength )
                    fLengthColumn = colNum;
                else if ( ii == NSABUtils::EMediaTags::eDate )
                    fDateColumn = colNum;
                else if ( ii == NSABUtils::EMediaTags::eComment )
                    fCommentColumn = colNum;

                if ( fFirstColumn == -1 )
                    fFirstColumn = colNum;
                fLastColumn = colNum;
                colNum++;
            }
        }

        QStringList CTagsModel::headers() const
        {
            return CDirModel::headers() << NPreferences::NCore::CPreferences::instance()->getEnabledTagsForDisplay();
        }

        std::list< SDirNodeItem > CTagsModel::addAdditionalItems( const QFileInfo & fileInfo ) const
        {
            if ( showMediaItems() && canShowMediaInfo() )
                return {};


            bool isMediaFile = this->isMediaFile( fileInfo );

            auto tagsToShow = NPreferences::NCore::CPreferences::instance()->getEnabledTags();
            auto mediaInfo = getMediaTags( fileInfo, tagsToShow );
            int colNum = EColumns::eMediaColumnLoc;

            std::list<SDirNodeItem> retVal;
            for ( auto && ii : tagsToShow )
            {
                QString value;
                auto pos = mediaInfo.find( ii );
                if ( pos != mediaInfo.end() )
                {
                    value = ( *pos ).second;
                }

                retVal.emplace_back( value, colNum++ );
                if ( isMediaFile )
                {
                    if ( NSABUtils::isSettableTag( ii ) )
                        retVal.back().fEditable = std::make_pair( EType::eMediaTag, ii );
                }
            }
            return retVal;
        }

        void CTagsModel::reloadMediaTags( const QModelIndex & idx )
        {
            if ( !canShowMediaInfo() )
                return;

            CDirModel::reloadMediaTags( idx, true );
        }

        void CTagsModel::postFileFunction( bool /*aOK*/, const QFileInfo & /*fileInfo*/, TParentTree & /*tree*/ )
        {
        }

        bool CTagsModel::preFileFunction( const QFileInfo & /*fileInfo*/, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/ )
        {
            return true;
        }

        std::optional< TItemStatus > CTagsModel::computeItemStatus( const QModelIndex & idx ) const
        {
            if ( isRootPath( idx ) )
                return {};

            if ( !NPreferences::NCore::CPreferences::instance()->getVerifyMediaTags() )
                return {};

            if ( !canShowMediaInfo() )
                return {};

            auto fileInfo = this->fileInfo( idx );
            if ( !isMediaFile( fileInfo ) )
                return {};

            auto mediaDate = getMediaDate( fileInfo );
            //qDebug() << mediaDate;

            QRegularExpression regExp;
            bool validate = false;
            QString tagName;
            QString expr;
            if ( idx.column() == getMediaTitleLoc() )
            {
                validate = NPreferences::NCore::CPreferences::instance()->getVerifyMediaTitle() && !NPreferences::NCore::CPreferences::instance()->getVerifyMediaTitleExpr().isEmpty();
                expr = NPreferences::NCore::CPreferences::instance()->getVerifyMediaTitleExpr();
                regExp = NPreferences::NCore::CPreferences::instance()->getVerifyMediaTitleExpr( fileInfo, mediaDate );
                tagName = tr( "Title" );
            }
            else if ( idx.column() == getMediaDateLoc() )
            {
                validate = NPreferences::NCore::CPreferences::instance()->getVerifyMediaDate() && !NPreferences::NCore::CPreferences::instance()->getVerifyMediaDateExpr().isEmpty();
                expr = NPreferences::NCore::CPreferences::instance()->getVerifyMediaDateExpr();
                regExp = NPreferences::NCore::CPreferences::instance()->getVerifyMediaDateExpr( fileInfo, mediaDate );
                tagName = tr( "Date" );
            }
            else if ( idx.column() == getMediaCommentLoc() )
            {
                validate = NPreferences::NCore::CPreferences::instance()->getVerifyMediaComment() && !NPreferences::NCore::CPreferences::instance()->getVerifyMediaCommentExpr().isEmpty();
                expr = NPreferences::NCore::CPreferences::instance()->getVerifyMediaCommentExpr();
                regExp = NPreferences::NCore::CPreferences::instance()->getVerifyMediaCommentExpr( fileInfo, mediaDate );
                tagName = tr( "Comment" );
            }

            if ( validate )
            {
                auto tag = idx.data().toString();
                if ( !regExp.match( tag ).hasMatch() )
                {
                    if ( tag.isEmpty() )
                        tag = QString( "<EMPTY>" ).toHtmlEscaped();

                    auto msg = tr( "<p style='white-space:pre'>File <b>'%1'</b> does not meet <b>'%2'</b> Meta Tag requirement '%3' - Currently <b>'%4'</b></p>" )
                        .arg( fileInfo.fileName() )
                        .arg( tagName )
                        .arg( expr.toHtmlEscaped() )
                        .arg( tag );

                    return TItemStatus( NPreferences::EItemStatus::eWarning, msg );
                }
            }
            return {};
        }

        CTagsFilterModel::CTagsFilterModel( QObject * parent ) :
            QSortFilterProxyModel( parent )
        {
            setRecursiveFilteringEnabled( true );
        }

        void CTagsFilterModel::slotSetFilter( const QString & text )
        {
            if ( text != fFilter )
            {
                auto regEx = QRegularExpression( text, QRegularExpression::CaseInsensitiveOption );
                if ( fFilter.isEmpty() || regEx.isValid() )
                {
                    fFilter = text;
                    fFilterRegEx = regEx;
                }
                invalidateFilter();
            }
        }

        bool CTagsFilterModel::filterAcceptsRow( int source_row, const QModelIndex & source_parent ) const
        {
            if ( fFilter.isEmpty() || !fFilterRegEx.isValid() )
                return true;

            auto myIndex = sourceModel()->index( source_row, 0, source_parent );
            auto data = myIndex.data().toString();
            auto match = fFilterRegEx.match( data );
            return match.hasMatch();
        }

    }
}
