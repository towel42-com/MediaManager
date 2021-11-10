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

#ifndef _DIRMODEL_H
#define _DIRMODEL_H

#include <QStandardItemModel>
#include <QRegularExpression>
#include <QFileInfo>
#include <memory>
#include <unordered_set>
#include "SABUtils/QtHash.h"

class QTreeView;
class QMediaPlaylist;
class QFileIconProvider;
struct STitleInfo;

using TTreeNode = std::pair< QList< QStandardItem * >, bool >;
using TParentTree = std::list< TTreeNode >;

class CDirModel : public QStandardItemModel
{
    Q_OBJECT
public:
    enum EColumns
    {
        eFSName,
        eFSSize,
        eFSType,
        eFSModDate,
        eTransformName
    };
    enum ECustomRoles
    {
        eFullPathRole = Qt::UserRole + 1,
        eIsDir,
        eIsRoot
    };
    CDirModel( QObject *parent = nullptr );
    ~CDirModel();

    bool isDir( const QModelIndex &idx ) const;
    QFileInfo fileInfo( const QModelIndex &idx ) const;
    QString filePath( const QModelIndex &idx ) const;

    bool isDir( const QStandardItem * item ) const;
    QFileInfo fileInfo( const QStandardItem *item ) const;
    QString filePath( const QStandardItem *item ) const;

    std::pair< bool, QStandardItemModel * > transform( bool displayOnly ) const;
    void saveM3U( QWidget *parent ) const;
    void setTitleInfo( const QModelIndex &idx, std::shared_ptr< STitleInfo > info, bool applyToChilren );
    std::shared_ptr< STitleInfo > getTitleInfo( const QModelIndex &idx ) const;

    void setNameFilters( const QStringList &filters, QTreeView * view = nullptr );

    void reloadModel( QTreeView *view );

    void setRootPath( const QString &path, QTreeView *view = nullptr );

    QString getSearchName( const QModelIndex &idx ) const;
Q_SIGNALS:
    void sigDirReloaded();
public Q_SLOTS:
    void slotInputPatternChanged( const QString &inPattern );
    void slotOutputDirPatternChanged( const QString &outPattern );
    void slotOutputFilePatternChanged( const QString &outPattern );
    void slotTreatAsMovieChanged( bool treatAsMovie );
    void slotLoadRootDirectory();
    void slotPatternChanged();
private:
    void loadFileInfo( const QFileInfo & info, TParentTree &parentTree );
    void attachParentTree( TParentTree & parentTree );
    QStandardItem *getItemFromindex( QModelIndex idx ) const;

    bool isIgnoredPathName( const QFileInfo &ii ) const;
    bool isExcludedDirName( const QFileInfo &ii ) const;

    TTreeNode getItemRow( const QFileInfo &path ) const;

    QString getDispName( const QString &absPath ) const;
    bool transform( const QStandardItem *item, bool displayOnly, QStandardItemModel *resultsModel, QStandardItem *resultsParentItem ) const;

    QStandardItem * getTransformItem( const QStandardItem * parent ) const;

    QString saveM3U( const QStandardItem *parent, const QString &baseName ) const;
    bool isValidName( const QString &name, bool isDir ) const;
    bool isValidName( const QFileInfo &fi ) const;
    void patternChanged();
    void patternChanged( const QStandardItem *parent );

    void updatePattern( const QStandardItem *item ) const;
    void updatePattern( const QStandardItem *transformedItem, QStandardItem *item ) const;

    [[nodiscard]] std::pair< bool, QString > transformItem( const QFileInfo &path ) const;
    [[nodiscard]] QString replaceCapture( const QString &captureName, const QString &returnPattern, const QString &value ) const;
    [[nodiscard]] QString patternToRegExp( const QString &captureName, const QString &inPattern, const QString &value, bool removeOptional ) const;
    [[nodiscard]] QString patternToRegExp( const QString &pattern, bool removeOptional ) const;
    void cleanFileName( QString &inFile ) const;
    [[nodiscard]] QString computeTransformPath( const QStandardItem * item, bool parentsOnly ) const;
    
    bool isLanguageFile( const QModelIndex &idx ) const;

    QString fRootPath;
    QStringList fNameFilter;

    QString fInPattern;
    QString fOutFilePattern;
    QString fOutDirPattern;
    bool fTreatAsMovie{ false };
    QRegularExpression fInPatternRegExp;
    mutable std::map< QString, std::pair< bool, QString > > fFileMapping;
    mutable std::map< QString, std::pair< bool, QString > > fDirMapping;
    std::map< QString, std::shared_ptr< STitleInfo > > fTitleInfoMapping;
    std::unordered_set< QString > fExcludedDirNames;
    std::unordered_set< QString > fIgnoredNames;
    QFileIconProvider *fIconProvider{ nullptr };
    QTimer *fTimer{ nullptr };
    QTimer *fPatternTimer{ nullptr };
    QTreeView *fTreeView{ nullptr };
};

#endif // 
