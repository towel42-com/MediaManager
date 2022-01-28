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
#include "SearchTMDBInfo.h"
#include "Preferences.h"
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
#include "TransformResult.h"

namespace NMediaManager
{
    namespace NCore
    {
        CTagsModel::CTagsModel( NUi::CBasePage * page, QObject * parent /*= 0*/ ) :
            CDirModel( page, parent )
        {
        }

        CTagsModel::~CTagsModel()
        {
        }

        QStringList CTagsModel::headers() const
        {
            return CDirModel::headers() << getMediaHeaders();
        }

        std::pair< bool, QStandardItem * > CTagsModel::processItem(const QStandardItem * item, QStandardItem * parentResultItem, bool displayOnly)
        {
            QStandardItem * myItem = nullptr;
            bool aOK = true;
            auto oldName = computeTransformPath(item, true);
            auto newName = computeTransformPath(item, false);

            (void)parentResultItem;
            (void)displayOnly;
            return std::make_pair(aOK, nullptr);
        }

        void CTagsModel::setupNewItem(const SDirNodeItem & nodeItem, const QStandardItem * nameItem, QStandardItem * item)  const
        {
            (void)nodeItem;
            (void)nameItem;
            (void)item;
        }

        std::optional< TItemStatus > CTagsModel::computeItemStatus(const QModelIndex & idx) const
        {
            if (isRootPath(idx))
                return {};

            if ( idx.column() < firstMediaItemColumn() || idx.column() > lastMediaItemColumn() )
                return {};

            TItemStatus retVal = { NCore::EItemStatus::eOK, QString() };
            auto fileInfo = this->fileInfo(idx);

            if (isMediaFile(fileInfo))
            {
                auto path = fileInfo.absoluteFilePath();
                QStringList msgs;
                if (NCore::CPreferences::instance()->getVerifyMediaTags() && canShowMediaInfo() && fileInfo.isFile())
                {
                    auto titleStatus = validateTitle( idx );
                    msgs << titleStatus.second;
                    auto dateStatus = validateDate( idx );
                    msgs << dateStatus.second;
                    retVal.first = std::max( titleStatus.first, dateStatus.first );
                }
                msgs.removeAll( QString() );
                retVal.second = msgs.join("\n");
            }
            return retVal;
        }

        std::list< NMediaManager::NCore::SDirNodeItem > CTagsModel::addAdditionalItems(const QFileInfo & fileInfo) const
        {
            if (canShowMediaInfo())
            {
                return getMediaInfoItems(fileInfo, firstMediaItemColumn());
            }
            return {};
        }

        void CTagsModel::reloadMediaTags(const QModelIndex & idx)
        {
            if ( !canShowMediaInfo() )
                return;

            CDirModel::reloadMediaTags(idx, true);
        }

        int CTagsModel::getMediaTitleLoc() const
        {
            return CDirModel::getMediaTitleLoc();
        }

        int CTagsModel::getMediaLengthLoc() const
        {
            return CDirModel::getMediaLengthLoc();
        }

        int CTagsModel::getMediaDateLoc() const
        {
            return CDirModel::getMediaDateLoc();
        }

        int CTagsModel::getMediaCommentLoc() const
        {
            return CDirModel::getMediaCommentLoc();
        }

        void CTagsModel::postFileFunction(bool /*aOK*/, const QFileInfo & /*fileInfo*/)
        {
        }

        bool CTagsModel::preFileFunction(const QFileInfo & /*fileInfo*/, std::unordered_set<QString> & /*alreadyAdded*/, TParentTree & /*tree*/)
        {
            return true;
        }

        void CTagsModel::attachTreeNodes(QStandardItem * /*nextParent*/, QStandardItem *& /*prevParent*/, const STreeNode & /*treeNode*/)
        {
        }

        TItemStatus CTagsModel::validateTitle( const QModelIndex & idx ) const
        {
            auto fileInfo = this->fileInfo( idx );
            auto tag = index( idx.row(), getMediaTitleLoc(), idx.parent() ).data().toString();
            if ( NCore::CPreferences::instance()->getVerifyMediaTitle() )
            {
                if ( tag.isEmpty() )
                {
                    return TItemStatus( EItemStatus::eError, tr( "File is missing 'Title' Meta Tag" ) );
                }

                auto mediaDate = getMediaDate( fileInfo );
                auto expr = NCore::CPreferences::instance()->getVerifyMediaTitleExpr( fileInfo, mediaDate );
                if ( !expr.match( tag ).hasMatch() )
                {
                    return TItemStatus( EItemStatus::eWarning, tr( "Filename '%1' does not match 'Title' Meta Tag requirement '%3' - '%4'" ).arg( fileInfo.fileName() ).arg( NCore::CPreferences::instance()->getVerifyMediaTitle() ).arg( tag ) );
                }
            }
            return TItemStatus( EItemStatus::eOK, QString() );
        }

        TItemStatus CTagsModel::validateDate( const QModelIndex & idx ) const
        {
            auto fileInfo = this->fileInfo( idx );
            auto tag = index( idx.row(), getMediaDateLoc(), idx.parent() ).data().toString();

            if ( NCore::CPreferences::instance()->getVerifyMediaDate() )
            {
                auto mediaDate = getMediaDate( fileInfo );
                auto expr = NCore::CPreferences::instance()->getVerifyMediaDateExpr( fileInfo, mediaDate );

                if ( !expr.match( tag ).hasMatch() )
                {
                    QLocale locale;
                    return TItemStatus( EItemStatus::eWarning, tr( "File's 'Date' Media Tag '%1' does not match the requiment of '%2' - '%3'" ).arg( tag ).arg( locale.toString( mediaDate ) ) );
                }
            }
            return TItemStatus( EItemStatus::eOK, QString() );
        }

    }
}
