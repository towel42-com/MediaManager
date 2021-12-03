// The MIT License( MIT )
//
// Copyright( c ) 2020 Scott Aron Bloom
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

#include "DirModel.h"
#include "SearchResult.h"
#include "SearchTMDBInfo.h"
#include "LanguageInfo.h"
#include "Preferences.h"

#include "UI/TransformConfirm.h"

#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/AutoWaitCursor.h"
#include "SABUtils/FileUtils.h"

#include <QDebug>
#include <QUrl>
#include <QInputDialog>
#include <QTextStream>
#include <QCollator>
#include <QDir>
#include <QLocale>
#include <QFileIconProvider>
#include <QTimer>
#include <QTreeView>
#include <QApplication>
#include <QProgressDialog>
#include <QDirIterator>
#include <QMessageBox>

#include <set>
#include <list>

QDebug operator<<( QDebug dbg, const NMediaManager::NCore::STreeNode & node )
{
    dbg << "STreeNode(";
    auto name = node.name();
    dbg << "'" << name << "' ";
    dbg << "Path: " << ( QFileInfo( name ).isFile() ? "Yes" : "No" ) << " "
        << "Is Loaded? " << node.fLoaded << " "
        << "Is File? " << node.fIsFile
        << ")"
        ;
    return dbg;
}

QDebug operator<<( QDebug dbg, const NMediaManager::NCore::TParentTree & parentTree )
{
    dbg << " TParentTree( ";
    bool first = true;
    for ( auto && ii : parentTree )
    {
        if ( !first )
            dbg << ", ";
        first = false;
        dbg << ii;
    }
    dbg << ")";
    return dbg;
}

namespace NMediaManager
{
    namespace NCore
    {
        CDirModel::CDirModel( EModelType modelType, QObject *parent /*= 0*/ ) :
            QStandardItemModel( parent ),
            fModelType( modelType )
        {
            fIconProvider = new QFileIconProvider();
            fTimer = new QTimer( this );
            fTimer->setInterval( 50 );
            fTimer->setSingleShot( true );
            connect( fTimer, &QTimer::timeout, this, &CDirModel::slotLoadRootDirectory );

            fPatternTimer = new QTimer( this );
            fPatternTimer->setInterval( 50 );
            fPatternTimer->setSingleShot( true );
            connect( fPatternTimer, &QTimer::timeout, this, &CDirModel::slotPatternChanged );

            fExcludedDirNames = { "#recycle", "#recycler", "extras" };
            fIgnoredNames = { "sub", "subs" };
        }

        CDirModel::~CDirModel()
        {
            delete fIconProvider;
        }

        void CDirModel::setRootPath( const QString &rootPath, QTreeView *view, QProgressDialog *dlg )
        {
            fRootPath = rootPath;
            reloadModel( view, dlg );
        }

        bool CDirModel::isAutoSetText( const QString &text )
        {
            return ( text == "<NO MATCH>" ) || ( text == "<NO AUTO MATCH>" );
        }

        QString CDirModel::getSearchName( const QModelIndex &idx ) const
        {
            auto nm = index( idx.row(), EColumns::eTransformName, idx.parent() ).data().toString();
            if ( isAutoSetText( nm ) || nm.isEmpty() )
            {
                nm = index( idx.row(), EColumns::eFSName, idx.parent() ).data( ECustomRoles::eFullPathRole ).toString();
                nm = nm.isEmpty() ? QString() : ( QFileInfo( nm ).isDir() ? QFileInfo( nm ).fileName() : QFileInfo( nm ).completeBaseName() );
            }
            return nm;
        }

        void CDirModel::setNameFilters( const QStringList &filters, QTreeView *view, QProgressDialog *dlg )
        {
            fNameFilter = filters;
            reloadModel( view, dlg );
        }

        void CDirModel::reloadModel( QTreeView *view, QProgressDialog *dlg )
        {
            fTreeView = view;
            fProgressDlg = dlg;

            fTimer->stop();
            fTimer->start();
            fPatternTimer->stop(); // if its runinng when this timer stops its realoaded anyway
        }

        void CDirModel::slotLoadRootDirectory()
        {
            CAutoWaitCursor awc;

            clear();
            auto headers = QStringList() << tr( "Name" ) << tr( "Size" ) << tr( "Type" ) << tr( "Date Modified" );
            if ( isTransformModel() )
                headers << tr( "Is TV Show?" ) << tr( "Transformed Name" );
            else if ( isMergeSRTModel() )
                headers << tr( "Language" ) << tr( "Forced?" ) << tr( "Hearing Impaired?" ) << tr( "On by Default?" );

            setHorizontalHeaderLabels( headers );

            QFileInfo rootFI( fRootPath );

            if ( fProgressDlg )
            {
                fProgressDlg->setLabelText( tr( "Computing number of Files under '%1'" ).arg( fRootPath ) );
                qApp->processEvents();
                auto numFiles = computeNumberOfFiles( rootFI ).second;
                if ( fProgressDlg && !fProgressDlg->wasCanceled() )
                    fProgressDlg->setRange( 0, numFiles );
            }
            if ( fProgressDlg && fProgressDlg->wasCanceled() )
            {
                emit sigDirReloaded( true );
                return;
            }

            loadFileInfo( rootFI );

            if ( fTreeView )
            {
                fTreeView->resizeColumnToContents( EColumns::eFSName );
                fTreeView->resizeColumnToContents( EColumns::eFSSize );
                fTreeView->resizeColumnToContents( EColumns::eFSType );
                fTreeView->resizeColumnToContents( EColumns::eFSModDate );
                if ( isTransformModel() )
                {
                    fTreeView->resizeColumnToContents( EColumns::eIsTVShow );
                    fTreeView->resizeColumnToContents( EColumns::eTransformName );
                }
                else if ( isMergeSRTModel() )
                {
                    fTreeView->resizeColumnToContents( EColumns::eLanguage );
                    fTreeView->resizeColumnToContents( EColumns::eForced );
                    fTreeView->resizeColumnToContents( EColumns::eSDH );
                    fTreeView->resizeColumnToContents( EColumns::eOnByDefault );
                }
            }

            emit sigDirReloaded( false );
        }

        std::unordered_map< QString, std::vector< QStandardItem * > > CDirModel::getChildSRTFiles( QStandardItem *item ) const
        {
            if ( !item )
                return {};

            std::unordered_map< QString, std::vector< QStandardItem * > > retVal;
            auto childCount = item->rowCount();
            for ( int ii = 0; ii < childCount; ++ii )
            {
                auto child = item->child( ii );
                if ( !child )
                    continue;

                if ( child->data( ECustomRoles::eIsDir ).toBool() )
                    continue;

                if ( !isSubtitleFile( child->data( ECustomRoles::eFullPathRole ).toString() ) )
                    continue;

                auto languageItem = getLanguageItem( child );

                retVal[languageItem->text()].push_back( child );
            }

            for ( auto &&ii : retVal )
            {
                //for( auto && jj : ii.second )
                //{
                //    auto lhsFI = QFileInfo( jj->data( ECustomRoles::eFullPathRole ).toString() );
                //    qDebug() << lhsFI.absoluteFilePath() << lhsFI.size();
                //}
                std::sort( ii.second.begin(), ii.second.end(),
                            []( const QStandardItem *lhs, const QStandardItem *rhs )
                            {
                                auto lhsFI = QFileInfo( lhs->data( ECustomRoles::eFullPathRole ).toString() );
                                auto rhsFI = QFileInfo( rhs->data( ECustomRoles::eFullPathRole ).toString() );
                                //qDebug() << lhsFI.absoluteFilePath() << lhsFI.size();
                                //qDebug() << rhsFI.absoluteFilePath() << rhsFI.size();

                                return lhsFI.size() < rhsFI.size();
                            } );
                //for ( auto &&jj : ii.second )
                //{
                //    auto lhsFI = QFileInfo( jj->data( ECustomRoles::eFullPathRole ).toString() );
                //    qDebug() << lhsFI.absoluteFilePath() << lhsFI.size();
                //}
            }
            return retVal;
        }

        void CDirModel::autoDetermineLanguageAttributes( QStandardItem *dirNode )
        {
            if ( !dirNode || !dirNode->data( ECustomRoles::eIsDir ).toBool() )
                return;

            auto srtFiles = getChildSRTFiles( dirNode ); // map of language to files sorted by filesize

            for ( auto &&ii : srtFiles )
            {
                if ( ii.second.size() == 3 )
                {
                    // smallest is "Forced"
                    // mid is normal
                    // largest is "SDH"
                    setChecked( getItem( ii.second[0], EColumns::eForced ), true );
                    setChecked( getItem( ii.second[2], EColumns::eSDH ), true );
                }
                else if ( ii.second.size() == 2 )
                {
                    // if 2 
                    // smallest is normal
                    // largest is sdh
                    setChecked( getItem( ii.second[1], EColumns::eSDH ), true );
                }
                else if ( ii.second.size() == 1 )
                {
                    // do nothing
                }
            }
        }

        bool CDirModel::setData( const QModelIndex &idx, const QVariant &value, int role )
        {
            if ( role == Qt::CheckStateRole )
            {
                auto isTVShow = value.toInt() == Qt::Checked;
                auto baseItem = getItemFromindex( idx );
                if ( baseItem && idx.column() == EColumns::eIsTVShow )
                    baseItem->setData( isTVShow, eIsTVShowRole );
                auto item = itemFromIndex( idx );
                item->setText( isTVShow ? "Yes" : "No" );
                updateTransformPattern( item );
            }
            return QStandardItemModel::setData( idx, value, role );
        }

        std::unique_ptr< QDirIterator > CDirModel::getDirIteratorForPath( const QFileInfo &fileInfo ) const
        {
            auto retVal = std::make_unique< QDirIterator >( fileInfo.absoluteFilePath(), fNameFilter, QDir::AllDirs | QDir::AllEntries | QDir::NoDotAndDotDot | QDir::Readable );
            return std::move( retVal );
        }

        void CDirModel::iterateEveryFile( const QFileInfo &fileInfo, const SIterateInfo &iterInfo, std::optional< QDateTime > & lastUpdateUI ) const
        {
            if ( !fileInfo.exists() )
                return;

            if ( fProgressDlg && fProgressDlg->wasCanceled() )
                return;

            if ( fileInfo.isDir() )
            {
                bool iteraterDir = true;
                if ( iterInfo.fPreDirFunction )
                    iteraterDir = iterInfo.fPreDirFunction( fileInfo );

                if ( iteraterDir )
                {
                    auto ii = getDirIteratorForPath( fileInfo );
                    if ( !ii )
                    {
                        if ( iterInfo.fPostDirFunction )
                            iterInfo.fPostDirFunction( fileInfo, false );
                        return;
                    }

                    while ( ii->hasNext() )
                    {
                        ii->next();
                        auto fi = ii->fileInfo();
                        iterateEveryFile( fi, iterInfo, lastUpdateUI );
                        if ( fProgressDlg && ( !lastUpdateUI.has_value() || ( lastUpdateUI.value().msecsTo( QDateTime::currentDateTime() ) > 250 ) ) )
                        {
                            qApp->processEvents();
                            lastUpdateUI = QDateTime::currentDateTime();
                        }
                    }
                }

                if ( iterInfo.fPostDirFunction )
                    iterInfo.fPostDirFunction( fileInfo, true );
            }
            else
            {
                bool aOK = true;
                if ( iterInfo.fPreFileFunction )
                    aOK = iterInfo.fPreFileFunction( fileInfo );
                if ( iterInfo.fPostFileFunction )
                    iterInfo.fPostFileFunction( fileInfo, aOK );
            }
        }

        std::pair< uint64_t, uint64_t > CDirModel::computeNumberOfFiles( const QFileInfo &fileInfo ) const
        {
            if ( fProgressDlg )
                fProgressDlg->setRange( 0, 0 );

            uint64_t numDirs = 0;
            uint64_t numFiles = 0;
            SIterateInfo info;
            info.fPreDirFunction = [&numDirs]( const QFileInfo & /*dir*/ ) { numDirs++; return true; };
            info.fPreFileFunction = [&numFiles]( const QFileInfo & /*file*/ ) { numFiles++; return false; };
            std::optional< QDateTime > lastUpdate;
            iterateEveryFile( fileInfo, info, lastUpdate );

            return std::make_pair( numDirs, numFiles );
        }


        void CDirModel::loadFileInfo( const QFileInfo &fileInfo )
        {
            if ( !fileInfo.exists() )
                return;

            TParentTree tree;

            SIterateInfo info;
            info.fPreDirFunction = [this, &tree]( const QFileInfo &dirInfo )
            {
                qDebug().noquote().nospace() << "Pre Directory A: " << dirInfo.absoluteFilePath() << tree;

                auto row = getItemRow( dirInfo );
                tree.push_back( std::move( row ) );

                if ( fProgressDlg )
                    fProgressDlg->setLabelText( tr( "Searching Directory '%1'" ).arg( QDir( fRootPath ).relativeFilePath( dirInfo.absoluteFilePath() ) ) );

                if ( isExcludedDirName( dirInfo ) )
                {
                    qDebug().noquote().nospace() << "Pre Directory B: returning false" << dirInfo.absoluteFilePath() << tree;
                    return false;
                }

                qDebug().noquote().nospace() << "Pre Directory B: returning true" << dirInfo.absoluteFilePath() << tree;
                return true;
            };

            std::unordered_set< QString > alreadyAdded;
            info.fPreFileFunction = [this, &tree, &alreadyAdded]( const QFileInfo &fileInfo )
            {
                bool attachFile = true;
                if ( isMergeSRTModel() )
                {
                    auto dir = fileInfo.absoluteDir();
                    auto mkvFiles = dir.entryInfoList( QStringList() << "*.mkv" );
                    for ( auto &&ii : mkvFiles )
                    {
                        if ( alreadyAdded.find( ii.absoluteFilePath() ) != alreadyAdded.end() )
                            continue;
                        alreadyAdded.insert( ii.absoluteFilePath() );
                        tree.push_back( getItemRow( ii ) );
                    }
                    attachFile = !mkvFiles.isEmpty();
                }

                tree.push_back( getItemRow( fileInfo ) );

                qDebug().noquote().nospace() << "Pre File A: " << fileInfo.absoluteFilePath() << tree;

                if ( attachFile )
                {
                    attachTreeNodes( tree );
                }

                qDebug().noquote().nospace() << "Pre File B: " << fileInfo.absoluteFilePath() << tree;

                if ( fProgressDlg )
                    fProgressDlg->setValue( fProgressDlg->value() + 1 );
                return true;
            };

            info.fPostDirFunction = [this, &tree]( const QFileInfo &dirInfo, bool aOK )
            {
                (void)dirInfo;
                qDebug().noquote().nospace() << "Post Dir A: " << dirInfo.absoluteFilePath() << tree << "AOK? " << aOK;  
                if ( isMergeSRTModel() && aOK )
                {
                    auto rootItem = tree.back().rootItem( false );
                    if ( rootItem )
                    {
                        qDebug() << rootItem->text() << rootItem->data( ECustomRoles::eFullPathRole ).toString();
                        autoDetermineLanguageAttributes( rootItem );
                    }
                }
                tree.pop_back(); 
                qDebug().noquote().nospace() << "Post Dir B: " << dirInfo.absoluteFilePath() << tree;
            };

            info.fPostFileFunction = [this, &tree]( const QFileInfo &dirInfo, bool aOK )
            {
                (void)dirInfo;
                qDebug().noquote().nospace() << "Post File A: " << dirInfo.absoluteFilePath() << tree << "AOK? " << aOK;
                while ( tree.back().fIsFile )
                    tree.pop_back();
                qDebug().noquote().nospace() << "Post File B: " << dirInfo.absoluteFilePath() << tree;
            };

            std::optional< QDateTime > lastUpdate;
            iterateEveryFile( fileInfo, info, lastUpdate );
        }

        void CDirModel::appendRow( QStandardItem *parent, QList< QStandardItem * > &items )
        {
            if ( parent )
                parent->appendRow( items );
            else
                QStandardItemModel::appendRow( items );
        }

        QStandardItem * CDirModel::attachTreeNodes( TParentTree &parentTree )
        {
            QStandardItem *prevParent = nullptr;
            for ( auto &&ii : parentTree )
            {
                auto nextParent = ii.rootItem();
                if ( !ii.fLoaded ) // already been loaded
                {
                    if ( prevParent )
                    {
                        if ( isTransformModel() )
                        {
                            bool isTVShow = nextParent->data( eIsTVShowRole ).toBool();
                            bool isParentTVShow = prevParent->data( eIsTVShowRole ).toBool();

                            if ( !isTVShow && ( isTVShow != isParentTVShow ) )
                            {
                                setChecked( ii.item( EColumns::eIsTVShow ), isParentTVShow );
                                nextParent->setData( isParentTVShow, eIsTVShowRole );
                            }
                        }

                        if ( ii.fIsFile )
                        {
                            while ( prevParent && !prevParent->data( ECustomRoles::eIsDir ).toBool() )
                                prevParent = prevParent->parent();
                        }
                        appendRow( prevParent, ii.items() );
                    }
                    else
                    {
                        appendRow( nullptr, ii.items() );
                        nextParent->setData( true, ECustomRoles::eIsRoot );
                    }
                    ii.fLoaded = true;
                }
                prevParent = nextParent;
                fPathMapping[nextParent->data( ECustomRoles::eFullPathRole ).toString()] = nextParent;

                if ( fTreeView && prevParent )
                    fTreeView->setExpanded( prevParent->index(), true );
            }
            return prevParent;
        }

        bool CDirModel::isExcludedDirName( const QFileInfo &ii ) const
        {
            auto fn = ii.fileName().toLower();

            return fn.endsWith( "-ignore" ) || ( fExcludedDirNames.find( fn ) != fExcludedDirNames.end() );
        }

        bool CDirModel::isIgnoredPathName( const QFileInfo &ii ) const
        {
            auto fn = ii.fileName().toLower();
            return fn.contains( "-ignore" ) || ( fIgnoredNames.find( fn ) != fIgnoredNames.end() );
        }

        STreeNode CDirModel::getItemRow( const QFileInfo &fileInfo ) const
        {
            return STreeNode( fileInfo, this );
        }


        QStandardItem *STreeNodeItem::createStandardItem() const
        {
            auto retVal = new QStandardItem( fText );
            retVal->setIcon( fIcon );
            if ( fAlignment.has_value() )
                retVal->setTextAlignment( fAlignment.value() );
            for ( auto &&ii : fRoles )
            {
                retVal->setData( ii.first, ii.second );
            }
            if ( fCheckable.has_value() )
                retVal->setCheckable( fCheckable.value() );
            return retVal;
        }

        STreeNode::STreeNode( const QFileInfo &fileInfo, const CDirModel *model ) :
            fModel( model )
        {
            fIsFile = fileInfo.isFile();
            auto nameItem = STreeNodeItem( fileInfo.fileName(), EColumns::eFSName );
            nameItem.fIcon = model->iconProvider()->icon( fileInfo );
            nameItem.setData( fileInfo.absoluteFilePath(), ECustomRoles::eFullPathRole );
            nameItem.setData( fileInfo.isDir(), ECustomRoles::eIsDir );
            fItems.push_back( nameItem );
            fItems.push_back( STreeNodeItem( fileInfo.isFile() ? NFileUtils::fileSizeString( fileInfo ) : QString(), EColumns::eFSSize ) );
            if ( fileInfo.isFile() )
            {
                fItems.back().fAlignment = Qt::AlignRight | Qt::AlignVCenter;
            }
            fItems.push_back( STreeNodeItem( model->iconProvider()->type( fileInfo ), EColumns::eFSType ) );
            fItems.push_back( STreeNodeItem( fileInfo.lastModified().toString( "MM/dd/yyyy hh:mm:ss.zzz" ), EColumns::eFSModDate ) );

            if ( model->isTransformModel() )
            {
                bool isTVShow = SSearchTMDBInfo::looksLikeTVShow( fileInfo.fileName(), nullptr );
                auto isTVShowItem = STreeNodeItem( QString(), EColumns::eIsTVShow );
                isTVShowItem.fIsTVShow = isTVShow;
                isTVShowItem.fCheckable = true;

                fItems.front().setData( isTVShow, ECustomRoles::eIsTVShowRole );
                fItems.push_back( isTVShowItem );

                auto transformInfo = model->transformItem( fileInfo );
                auto transformedItem = STreeNodeItem( transformInfo.second, EColumns::eTransformName );
                fItems.push_back( transformedItem );
            }
            else if ( model->isMergeSRTModel() )
            {
                if ( fileInfo.isFile() )
                {
                    auto language = SLanguageInfo( fileInfo.completeBaseName() );
                    auto languageFileItem = STreeNodeItem( language.displayName(), EColumns::eLanguage );
                    languageFileItem.setData( language.isoCode(), ECustomRoles::eISOCodeRole );
                    fItems.push_back( languageFileItem );

                    auto forcedItem = STreeNodeItem( QString(), EColumns::eForced );
                    forcedItem.fCheckable = true;
                    fItems.push_back( forcedItem );

                    auto sdhItem = STreeNodeItem( QString(), EColumns::eSDH );
                    sdhItem.fCheckable = true;
                    fItems.push_back( sdhItem );

                    auto onByDefaultItem = STreeNodeItem( QString(), EColumns::eOnByDefault );
                    onByDefaultItem.fCheckable = true;
                    fItems.push_back( onByDefaultItem );
                }
                else
                {
                    // do nothing for directories
                }
            }
        }

        QList< QStandardItem * > STreeNode::items( bool createIfNecessary ) const
        {
            if ( createIfNecessary && fRealItems.isEmpty() )
            {
                for ( auto &&ii : fItems )
                {
                    auto currItem = ii.createStandardItem();

                    if ( fModel->isTransformModel() )
                    {
                        if ( ii.fType == EColumns::eIsTVShow )
                            fModel->setChecked( currItem, ii.fIsTVShow );
                        else if ( ii.fText == EColumns::eTransformName )
                        {
                            auto nameItem = fRealItems.front();
                            if ( nameItem && currItem )
                            {
                                fModel->updateTransformPattern( nameItem, currItem );
                            }

                        }
                    }
                    else if ( fModel->isMergeSRTModel() )
                    {
                        if ( ( ii.fType == EColumns::eLanguage ) && ( currItem->text().isEmpty() ) )
                        {
                            currItem->setBackground( Qt::red );
                        }
                    }
                    fRealItems << currItem;
                }
            }
            return fRealItems;
        }

        bool CDirModel::isMergeSRTModel() const
        {
            return fModelType == EModelType::eMergeSRT;
        }

        bool CDirModel::isTransformModel() const
        {
            return fModelType == EModelType::eTransform;
        }

        void CDirModel::setChecked( QStandardItem *item, bool isChecked ) const
        {
            item->setText( isChecked ? "Yes" : "No" );
            item->setCheckState( isChecked ? Qt::Checked : Qt::Unchecked );
        }

        QString patternToRegExp( const QString &captureName, const QString &inPattern, const QString &value, bool removeOptional )
        {
            if ( captureName.isEmpty() || inPattern.isEmpty() )
                return inPattern;

            // see if the capture name exists in the return pattern
            auto capRegEx = QString( "\\\\\\((?<optname>.*)\\\\\\)(\\\\)?\\:\\<%1\\>" ).arg( captureName );
            auto regExp = QRegularExpression( capRegEx );
            auto retVal = inPattern;
            retVal = retVal.replace( regExp, removeOptional ? "\\1" : "(\\1)?" );

            capRegEx = QString( "\\<%1\\>" ).arg( captureName );
            regExp = QRegularExpression( capRegEx );
            retVal = retVal.replace( regExp, value );

            return retVal;
        }

        QString patternToRegExp( const QString &pattern, bool removeOptional )
        {
            QString retVal = pattern;
            retVal.replace( "(", "\\(" );
            retVal.replace( ")", "\\)" );
            retVal.replace( ":", "\\:" );

            retVal = patternToRegExp( "title", retVal, ".*", removeOptional );
            retVal = patternToRegExp( "year", retVal, "(\\d{2}|\\d{4})", removeOptional );
            retVal = patternToRegExp( "tmdbid", retVal, "\\d+", removeOptional );
            retVal = patternToRegExp( "season", retVal, "\\d+", removeOptional );
            retVal = patternToRegExp( "episode", retVal, "\\d+", removeOptional );
            retVal = patternToRegExp( "episode_title", retVal, ".*", removeOptional );
            retVal = patternToRegExp( "extra_info", retVal, ".*", removeOptional );
            return retVal;
        }

        QStandardItem *CDirModel::getItemFromPath( const QFileInfo &fi ) const
        {
            auto path = fi.absoluteFilePath();
            auto pos = fPathMapping.find( path );
            if ( pos != fPathMapping.end() )
                return ( *pos ).second;
            return nullptr;
        }

        bool CDirModel::isValidName( const QFileInfo &fi ) const
        {
            return isValidName( fi.fileName(), fi.isDir(), {} );
        }

        bool CDirModel::isValidName( const QString &path, bool isDir, std::optional< bool > isChecked ) const
        {
            bool defaultAsTVShow = isChecked.has_value() ? isChecked.value() : this->isChecked( path, EColumns::eIsTVShow );
            bool asTVShow = treatAsTVShow( path, defaultAsTVShow );
            if ( ( !asTVShow && fMoviePatterns.isValidName( path, isDir ) )
                 || ( asTVShow && fTVPatterns.isValidName( path, isDir ) ) )
                return true;

            return false;
        }

        bool SPatternInfo::isValidName( const QFileInfo &fi ) const
        {
            return isValidName( fi.fileName(), fi.isDir() );
        }

        bool SPatternInfo::isValidName( const QString &name, bool isDir ) const
        {
            if ( name.isEmpty() )
                return false;
            QStringList patterns;
            if ( isDir )
            {
                patterns
                    << patternToRegExp( fOutDirPattern, false )
                    << "(.*)\\s\\((\\d{2}|\\d{4}\\))\\s(-\\s(.*)\\s)?\\[(tmdbid=\\d+)|(imdbid=tt.*)\\]"
                    ;
            }
            else
            {
                patterns << patternToRegExp( fOutFilePattern, false )
                    ;
            }
            for ( auto &&ii : patterns )
            {
                QRegularExpression regExp( ii );
                if ( !ii.isEmpty() && regExp.match( name ).hasMatch() )
                    return true;
            }
            return false;
        }

        QFileInfo CDirModel::fileInfo( const QStandardItem *item ) const
        {
            if ( !item )
                return QFileInfo();
            return QFileInfo( item->data( ECustomRoles::eFullPathRole ).toString() );
        }

        QStandardItem *CDirModel::getItemFromindex( QModelIndex idx ) const
        {
            if ( idx.column() != EColumns::eFSName )
            {
                idx = idx.model()->index( idx.row(), EColumns::eFSName, idx.parent() );
            }
            return itemFromIndex( idx );
        }

        bool CDirModel::isDir( const QStandardItem *item ) const
        {
            return fileInfo( item ).isDir();
        }

        QString CDirModel::filePath( const QStandardItem *item ) const
        {
            return fileInfo( item ).absoluteFilePath();
        }

        QString CDirModel::filePath( const QModelIndex &idx ) const
        {
            auto item = getItemFromindex( idx );
            return filePath( item );
        }

        QFileInfo CDirModel::fileInfo( const QModelIndex &idx ) const
        {
            auto item = getItemFromindex( idx );
            return fileInfo( item );
        }

        bool CDirModel::isDir( const QModelIndex &idx ) const
        {
            auto item = getItemFromindex( idx );
            return isDir( item );
        }

        void CDirModel::slotTVOutputFilePatternChanged( const QString &outPattern )
        {
            fTVPatterns.fOutFilePattern = outPattern;
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CDirModel::slotTVOutputDirPatternChanged( const QString &outPattern )
        {
            fTVPatterns.fOutDirPattern = outPattern;
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CDirModel::slotMovieOutputFilePatternChanged( const QString &outPattern )
        {
            fMoviePatterns.fOutFilePattern = outPattern;
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CDirModel::slotMovieOutputDirPatternChanged( const QString &outPattern )
        {
            fMoviePatterns.fOutDirPattern = outPattern;
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        void CDirModel::slotTreatAsTVByDefaultChanged( bool treatAsTVShowByDefault )
        {
            fTreatAsTVShowByDefault = treatAsTVShowByDefault;
            // need to emit datachanged on column 4 for all known indexes
            transformPatternChanged();
        }

        // do not include <> in the capture name
        QString replaceCapture( const QString &captureName, const QString &returnPattern, const QString &value )
        {
            if ( captureName.isEmpty() )
                return returnPattern;

            // see if the capture name exists in the return pattern
            auto capRegEx = QString( "\\<%1\\>" ).arg( captureName );
            auto regExp = QRegularExpression( capRegEx );

            int start = -1;
            int replLength = -1;

            auto match = regExp.match( returnPattern );
            if ( !match.hasMatch() )
                return returnPattern;
            else
            {
                start = match.capturedStart( 0 );
                replLength = match.capturedLength( 0 );
            }

            // its in there..now lets see if its optional
            auto optRegExStr = QString( "\\((?<replText>[^()]+)\\)\\:%1" ).arg( capRegEx );
            regExp = QRegularExpression( optRegExStr );
            match = regExp.match( returnPattern );
            bool optional = match.hasMatch();
            QString replText = value;
            if ( optional )
            {
                start = match.capturedStart( 0 );
                replLength = match.capturedLength( 0 );

                replText = match.captured( "replText" );
                if ( value.isEmpty() )
                    replText.clear();
                else
                {
                    replText = replaceCapture( captureName, replText, value );;
                }
            }
            auto retVal = returnPattern;
            retVal.replace( start, replLength, replText );
            return retVal;
        }

        void cleanFileName( QString &inFile, bool isDir )
        {
            if ( inFile.isEmpty() )
                return;

            inFile.replace( QRegularExpression( "^(([A-Za-z]\\:)|(\\/)|(\\\\))+" ), "" );

            QString text = "\\s*\\:\\s*";
            inFile.replace( QRegularExpression( text ), " - " );
            text = "[\\:\\<\\>\\\"\\|\\?\\*";
            if ( !isDir )
                text += "\\/\\\\";
            text += "]";
            inFile.replace( QRegularExpression( text ), "" );
        }

        std::pair< bool, QString > CDirModel::transformItem( const QFileInfo &fileInfo ) const
        {
            if ( treatAsTVShow( fileInfo, isChecked( fileInfo, EColumns::eIsTVShow ) ) )
                return transformItem( fileInfo, fTVPatterns );
            else
                return transformItem( fileInfo, fMoviePatterns );
        }

        std::pair< bool, QString > CDirModel::transformItem( const QFileInfo &fileInfo, const SPatternInfo &info ) const
        {
            auto filePath = fileInfo.absoluteFilePath();

            //if ( !info.fInPatternRegExp.isValid() )
            //    return std::make_pair( false, QObject::tr( "<INVALID INPUT REGEX>" ) );

            auto pos = fileInfo.isDir() ? fDirMapping.find( filePath ) : fFileMapping.find( filePath );
            auto retVal = std::make_pair( false, QString() );

            if ( pos == ( fileInfo.isDir() ? fDirMapping.end() : fFileMapping.end() ) )
            {
                QString fn = fileInfo.fileName();
                QString ext = QString();
                if ( !fileInfo.isDir() )
                {
                    fn = fileInfo.fileName();
                    ext = fileInfo.suffix();
                }

                auto pos = fSearchResultMap.find( filePath );
                if ( pos == fSearchResultMap.end() )
                {
                    if ( isValidName( fileInfo ) || isIgnoredPathName( fileInfo ) )
                        retVal.second = QString();
                }
                else
                {
                    auto title = ( *pos ).second->getTitle();
                    auto year = ( *pos ).second->getYear();
                    auto tmdbid = ( *pos ).second->fTMDBID;
                    auto season = ( *pos ).second->fSeason;
                    auto episode = ( *pos ).second->fEpisode;
                    auto extraInfo = ( *pos ).second->fExtraInfo;
                    auto episodeTitle = ( *pos ).second->fEpisodeTitle;

                    retVal.second = fileInfo.isDir() ? info.fOutDirPattern : info.fOutFilePattern;
                    retVal.second = replaceCapture( "title", retVal.second, title );
                    retVal.second = replaceCapture( "year", retVal.second, year );
                    retVal.second = replaceCapture( "tmdbid", retVal.second, tmdbid );
                    retVal.second = replaceCapture( "season", retVal.second, QString( "%1" ).arg( season, fileInfo.isDir() ? 1 : 2, QChar( '0' ) ) );
                    retVal.second = replaceCapture( "episode", retVal.second, QString( "%1" ).arg( episode, fileInfo.isDir() ? 1 : 2, QChar( '0' ) ) );
                    retVal.second = replaceCapture( "episode_title", retVal.second, episodeTitle );
                    retVal.second = replaceCapture( "extra_info", retVal.second, extraInfo );

                    cleanFileName( retVal.second, fileInfo.isDir() );
                    if ( !fileInfo.isDir() )
                        retVal.second += "." + ext;
                    retVal.first = true;
                }

                if ( fileInfo.isDir() )
                    fDirMapping[filePath] = retVal;
                else
                    fFileMapping[filePath] = retVal;
            }
            else
                retVal = ( *pos ).second;

            return retVal;
        }

        bool CDirModel::treatAsTVShow( const QFileInfo &fileInfo, bool defaultValue ) const
        {
            bool asTVShow = defaultValue;
            auto pos = fSearchResultMap.find( fileInfo.absoluteFilePath() );
            if ( pos != fSearchResultMap.end() )
                asTVShow = ( *pos ).second->isTVShow();
            return asTVShow;
        }

        void CDirModel::setSearchResult( QStandardItem *item, std::shared_ptr< SSearchResult > searchResult, bool applyToChildren )
        {
            auto idx = indexFromItem( item );
            setSearchResult( idx, searchResult, applyToChildren );
        }

        void CDirModel::setSearchResult( const QModelIndex &idx, std::shared_ptr< SSearchResult > searchResult, bool applyToChildren )
        {
            if ( !idx.isValid() )
                return;

            if ( !isTransformModel() )
                return;

            if ( searchResult && searchResult->getTitle().isEmpty() )
                searchResult.reset();

            auto fi = fileInfo( idx );
            if ( !isIgnoredPathName( fi ) )
            {
                if ( !searchResult )
                    fSearchResultMap.erase( fi.absoluteFilePath() );
                else
                    fSearchResultMap[fi.absoluteFilePath()] = searchResult;
                if ( isDir( idx ) )
                    fDirMapping.erase( fi.absoluteFilePath() );
                else
                    fFileMapping.erase( fi.absoluteFilePath() );

                updateTransformPattern( getItemFromindex( idx ) );
            }
            if ( applyToChildren )
            {
                auto childCount = rowCount( idx );
                for ( int ii = 0; ii < childCount; ++ii )
                {
                    auto childIdx = index( ii, EColumns::eFSName, idx );

                    // exception for "SRT" files that are of the form X_XXXX.ext dont transform
                    //auto txt = childIdx.data( ECustomRoles::eFullPathRole ).toString();
                    if ( !isSubtitleFile( childIdx ) )
                    {
                        auto childInfo = getSearchResultInfo( childIdx );
                        if ( !childInfo )
                        {
                            setSearchResult( childIdx, searchResult, applyToChildren );
                        }
                    }
                }
            }
        }

        // only return true for X_Lang.srt files or subs directories
        bool CDirModel::isSubtitleFile( const QFileInfo &fi, bool * isLangFileFormat ) const
        {
            if ( isLangFileFormat )
                *isLangFileFormat = false;

            static std::optional< std::unordered_set< QString > > extensions;
            if ( !extensions.has_value() )
            {
                auto exts = CPreferences::instance()->getSubtitleExtensions();
                for ( auto &&ii : exts )
                {
                    auto pos = ii.lastIndexOf( '.' );
                    ii = ii.mid( pos + 1 );
                }
                extensions = { exts.begin(), exts.end() };
            }

            auto suffix = fi.suffix();
            if ( extensions.value().find( suffix ) == extensions.value().end() )
                return false;

            if ( isLangFileFormat )
            {
                auto fn = fi.completeBaseName();
                auto regExp = QRegularExpression( "\\d+_\\S+" );
                *isLangFileFormat = ( regExp.match( fn ).hasMatch() );
            }
            return true;
        }

        bool CDirModel::isSubtitleFile( const QModelIndex &idx, bool *isLangFileFormat ) const
        {
            auto path = idx.data( ECustomRoles::eFullPathRole ).toString();
            if ( path.isEmpty() )
                return false;

            return isSubtitleFile( QFileInfo( path ), isLangFileFormat );
        }


        bool CDirModel::canAutoSearch( const QModelIndex &index ) const
        {
            auto path = index.data( ECustomRoles::eFullPathRole ).toString();
            if ( path.isEmpty() )
                return false;
            return canAutoSearch( QFileInfo( path ) );
        }

        bool CDirModel::canAutoSearch( const QFileInfo &fileInfo ) const
        {
            bool isLangFormat;
            if ( isSubtitleFile( fileInfo, &isLangFormat ) && !isLangFormat )
                return false;
            if ( !fileInfo.isDir() )
                return true;

            auto files = QDir( fileInfo.absoluteFilePath() ).entryInfoList( QDir::Files );
            bool hasFiles = false;
            for ( auto &&ii : files )
            {
                if ( canAutoSearch( ii ) )
                {
                    hasFiles = true;
                    break;
                }
            }

            return hasFiles;
        }

        std::shared_ptr< SSearchResult > CDirModel::getSearchResultInfo( const QModelIndex &idx ) const
        {
            if ( !idx.isValid() )
                return {};

            auto fi = fileInfo( idx );
            auto pos = fSearchResultMap.find( fi.absoluteFilePath() );
            if ( pos == fSearchResultMap.end() )
                return {};
            return ( *pos ).second;
        }

        QString CDirModel::computeTransformPath( const QStandardItem *item, bool transformParentsOnly ) const
        {
            if ( !item || ( item == invisibleRootItem() ) )
                return QString();
            if ( item->data( ECustomRoles::eIsRoot ).toBool() )
                return item->data( ECustomRoles::eFullPathRole ).toString();

            auto parentDir = computeTransformPath( item->parent(), false );

            auto myName = item->text();
            if ( isTransformModel() )
            {
                auto transformItem = transformParentsOnly ? nullptr : getTransformItem( item );
                myName = transformItem ? transformItem->text() : QString();
                if ( myName.isEmpty() || isAutoSetText( myName ) )
                {
                    myName = item->text();
                }
            }
            else if ( isMergeSRTModel() )
            {
                if ( !transformParentsOnly )
                {
                }
            }

            if ( myName.isEmpty() || parentDir.isEmpty() )
                return QString();

            auto retVal = QDir( parentDir ).absoluteFilePath( myName );
            return retVal;
        }

        QString CDirModel::getDispName( const QString &absPath ) const
        {
            auto item = invisibleRootItem();
            if ( !invisibleRootItem() )
                return QString();
            auto root = invisibleRootItem()->child( 0, 0 );
            if ( !root->data( ECustomRoles::eIsRoot ).toBool() )
                return QString();

            auto rootDir = QDir( root->data( ECustomRoles::eFullPathRole ).toString() );
            return rootDir.relativeFilePath( absPath );
        }

        bool CDirModel::process( const QStandardItem *item, bool displayOnly, QStandardItemModel *resultModel, QStandardItem *parentItem ) const
        {
            if ( !item )
                return false;

            bool aOK = true;
            QStandardItem *returnItem = nullptr;
            if ( item != invisibleRootItem() )
            {
                auto oldName = computeTransformPath( item, true );
                auto newName = computeTransformPath( item, false );

                if ( oldName != newName )
                {
                    returnItem = new QStandardItem( QString( "'%1' => '%2'" ).arg( getDispName( oldName ) ).arg( getDispName( newName ) ) );

                    returnItem->setData( oldName, Qt::UserRole + 1 );
                    returnItem->setData( newName, Qt::UserRole + 2 );
                    if ( parentItem )
                        parentItem->appendRow( returnItem );
                    else
                        resultModel->appendRow( returnItem );
                    if ( !displayOnly )
                    {
                        bool removeIt = newName == "<DELETE THIS>";
                        if ( fProgressDlg )
                        {
                            if ( removeIt )
                                fProgressDlg->setLabelText( tr( "Removing '%1'" ).arg( getDispName( oldName ) ) );
                            else
                                fProgressDlg->setLabelText( tr( "Renaming '%1' => '%2'" ).arg( getDispName( oldName ) ).arg( getDispName( newName ) ) );
                        }
                        QFileInfo fi( oldName );
                        if ( !fi.exists() && !removeIt )
                        {
                            auto errorItem = new QStandardItem( QString( "ERROR: '%1' - No Longer Exists" ).arg( oldName ) );
                            returnItem->appendRow( errorItem );

                            QIcon icon;
                            icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                            errorItem->setIcon( icon );
                        }
                        else if ( fi.exists() && removeIt )
                        {
                            auto fi = QDir( oldName );
                            aOK = fi.removeRecursively();
                            if ( !aOK )
                            {
                                auto errorItem = new QStandardItem( QString( "ERROR: Failed to Remove '%1'" ).arg( oldName ) );
                                returnItem->appendRow( errorItem );
                            }
                        }
                        else
                        {
                            auto timeStamps = NFileUtils::timeStamps( oldName );
                            if ( fProgressDlg )
                            {
                                fProgressDlg->setValue( fProgressDlg->value() + 1 );
                                qApp->processEvents();
                            }

                            auto transFormItem = getTransformItem( item );
                            bool parentPathOK = true;
                            if ( transFormItem )
                            {
                                auto mySubPath = transFormItem->text();
                                if ( ( mySubPath.indexOf( "/" ) != -1 ) || ( mySubPath.indexOf( "\\" ) != -1 ) )
                                {
                                    auto pos = mySubPath.lastIndexOf( QRegularExpression( "[\\/\\\\]" ) );
                                    auto myParentPath = mySubPath.left( pos );

                                    auto parentPath = computeTransformPath( item->parent(), false );
                                    parentPathOK = QDir( parentPath ).mkpath( myParentPath );
                                    if ( !parentPathOK )
                                    {
                                        auto errorItem = new QStandardItem( QString( "ERROR: '%1' => '%2' : FAILED TO MAKE PARENT DIRECTORY PATH" ).arg( oldName ).arg( newName ) );
                                        returnItem->appendRow( errorItem );

                                        QIcon icon;
                                        icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                                        errorItem->setIcon( icon );
                                    }
                                }
                            }
                            if ( fProgressDlg )
                            {
                                fProgressDlg->setValue( fProgressDlg->value() + 1 );
                                qApp->processEvents();
                            }

                            QString errorMsg;
                            if ( parentPathOK )
                            {
                                if ( QFileInfo( newName ).exists() )
                                {
                                    aOK = QFile( oldName ).remove();
                                    if ( !aOK )
                                        errorMsg = QString( "Destination file Exists - Old Size: %1 New Size: %2" ).arg( NFileUtils::fileSizeString( oldName, false ) ).arg( NFileUtils::fileSizeString( newName, false ) );
                                }
                                else
                                {
                                    auto fi = QFile( oldName );
                                    aOK = fi.rename( newName );
                                    if ( !aOK )
                                        errorMsg = fi.errorString();
                                }
                            }
                            else
                                aOK = false;
                            if ( fProgressDlg )
                            {
                                fProgressDlg->setValue( fProgressDlg->value() + 1 );
                                qApp->processEvents();
                            }

                            if ( parentPathOK && !aOK )
                            {
                                auto errorItem = new QStandardItem( QString( "ERROR: '%1' => '%2' : FAILED TO RENAME - %3" ).arg( oldName ).arg( newName ).arg( errorMsg ) );
                                returnItem->appendRow( errorItem );

                                QIcon icon;
                                icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                                errorItem->setIcon( icon );
                            }
                            else if ( parentPathOK )
                            {
                                QString msg;
                                auto aOK = NFileUtils::setTimeStamps( newName, timeStamps );
                                if ( !aOK )
                                {
                                    auto errorItem = new QStandardItem( QString( "ERROR: %1: FAILED TO MODIFY TIMESTAMP: %2" ).arg( newName ).arg( msg ) );
                                    returnItem->appendRow( errorItem );

                                    QIcon icon;
                                    icon.addFile( QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                                    errorItem->setIcon( icon );
                                }
                            }
                            else
                                aOK = parentPathOK;

                            QIcon icon;
                            icon.addFile( aOK ? QString::fromUtf8( ":/resources/ok.png" ) : QString::fromUtf8( ":/resources/error.png" ), QSize(), QIcon::Normal, QIcon::Off );
                            returnItem->setIcon( icon );
                            if ( fProgressDlg )
                            {
                                fProgressDlg->setValue( fProgressDlg->value() + 1 );
                                qApp->processEvents();
                            }
                        }
                    }
                }
            }

            auto numRows = item->rowCount();
            for ( int ii = 0; ii < numRows; ++ii )
            {
                auto child = item->child( ii );
                if ( !child )
                    continue;

                if ( fProgressDlg && fProgressDlg->wasCanceled() )
                    break;

                aOK = process( child, displayOnly, resultModel, returnItem ) && aOK;
            }
            return aOK;
        }

        std::pair< bool, QStandardItemModel * > CDirModel::process( bool displayOnly ) const
        {
            CAutoWaitCursor awc;
            auto model = new QStandardItemModel;
            auto retVal = std::make_pair( process( invisibleRootItem(), displayOnly, model, nullptr ), model );
            if ( fProgressDlg )
                fProgressDlg->setValue( retVal.second->rowCount() );
            return retVal;
        }


        bool CDirModel::process( const std::function< QProgressDialog *( int count ) > &startProgress, const std::function< void( QProgressDialog * ) > &endProgress, QWidget *parent ) const
        {
            fProgressDlg = nullptr;
            auto transformations = process( true );
            if ( transformations.second->rowCount() == 0 )
            {
                QMessageBox::information( parent, tr( "Nothing to change" ), tr( "No files or directories could be transformed" ) );
                return false;
            }
            NUi::CTransformConfirm dlg( tr( "Transformations:" ), tr( "Proceed?" ), parent );
            auto count = NQtUtils::itemCount( transformations.second, true );
            dlg.setModel( transformations.second );
            dlg.setIconLabel( QMessageBox::Information );
            dlg.setButtons( QDialogButtonBox::Yes | QDialogButtonBox::No );
            if ( dlg.exec() != QDialog::Accepted )
                return false;

            fProgressDlg = startProgress( count * eventsPerPath() );
            transformations = process( false );
            if ( !transformations.first )
            {
                NUi::CTransformConfirm dlg( tr( "Error While Transforming:" ), tr( "Issues:" ), parent );
                dlg.setModel( transformations.second );
                dlg.setIconLabel( QMessageBox::Critical );
                dlg.setButtons( QDialogButtonBox::Ok );
                dlg.exec();
            }
            endProgress( fProgressDlg );
            return true;
        }

        QStandardItem *CDirModel::getItem( const QStandardItem *item, EColumns column ) const
        {
            auto idx = indexFromItem( item );
            if ( !idx.isValid() )
                return nullptr;

            auto retVal = itemFromIndex( index( idx.row(), column, idx.parent() ) );
            return retVal;
        }

        QStandardItem *CDirModel::getLanguageItem( const QStandardItem *item ) const
        {
            return getItem( item, EColumns::eLanguage );
        }

        QStandardItem *CDirModel::getTransformItem( const QStandardItem *item ) const
        {
            return getItem( item, EColumns::eTransformName );
        }

        bool CDirModel::isChecked( const QFileInfo &fileInfo, EColumns column ) const
        {
            return isChecked( fileInfo.absoluteFilePath(), column );
        }

        bool CDirModel::isChecked( const QString &path, EColumns column ) const
        {
            auto item = getItemFromPath( path );
            auto colItem = getItem( item, column );
            if ( !colItem )
                return false;
            return item->checkState() != Qt::Unchecked;
        }

        void CDirModel::transformPatternChanged()
        {
            fPatternTimer->stop();
            fPatternTimer->start();
        }

        void CDirModel::slotPatternChanged()
        {
            fFileMapping.clear();
            fDirMapping.clear();
            transformPatternChanged( invisibleRootItem() );
        }

        void CDirModel::transformPatternChanged( const QStandardItem *item )
        {
            if ( !item )
                return;

            if ( !isTransformModel() )
                return;

            updateTransformPattern( item );

            auto numRows = item->rowCount();
            for ( int ii = 0; ii < numRows; ++ii )
            {
                auto child = item->child( ii );
                if ( child )
                {
                    //emit dataChanged( child, child );
                    transformPatternChanged( child );
                }
            }
        }

        void CDirModel::updateTransformPattern( const QStandardItem *item )const
        {
            if ( !isTransformModel() )
                return;

            if ( item != invisibleRootItem() )
            {
                auto idx = item->index();

                auto baseIdx = ( idx.column() == EColumns::eFSName ) ? idx : idx.model()->index( idx.row(), EColumns::eFSName, idx.parent() );
                auto baseItem = this->itemFromIndex( baseIdx );

                auto transformedItem = getTransformItem( baseItem );
                updateTransformPattern( baseItem, transformedItem );
            }
        }

        void CDirModel::updateTransformPattern( const QStandardItem *item, QStandardItem *transformedItem ) const
        {
            if ( !item || !transformedItem )
                return;

            transformedItem->setBackground( Qt::white );

            auto path = item->data( ECustomRoles::eFullPathRole ).toString();
            auto fileInfo = QFileInfo( path );

            if ( isIgnoredPathName( fileInfo ) && !isSubtitleFile( fileInfo ) )
                return;

            auto transformInfo = transformItem( fileInfo );
            if ( transformedItem->text() != transformInfo.second )
                transformedItem->setText( transformInfo.second );

            auto isTVShow = treatAsTVShow( fileInfo, this->isChecked( path, EColumns::eIsTVShow ) );
            if ( canAutoSearch( fileInfo ) && ( CDirModel::isAutoSetText( transformInfo.second ) || ( !isValidName( transformInfo.second, fileInfo.isDir(), isTVShow ) && !isValidName( fileInfo ) ) ) )
                transformedItem->setBackground( Qt::red );
        }
    }
}
