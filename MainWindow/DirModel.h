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

#include <QFileSystemModel>
#include <QRegularExpression>
#include <QSortFilterProxyModel>
#include <memory>
class QMediaPlaylist;
struct STitleInfo
{
    QString getTitle() const;
    QString getYear() const;
    QString getEpisodeTitle() const;

    QString fTitle;
    QString fReleaseDate;
    QString fTMDBID;
    QString fSeason;
    QString fEpisode;
    QString fEpisodeTitle;
    QString fExtraInfo;

    bool fIsMovie{ true };
};

class CDirModel : public QFileSystemModel
{
    Q_OBJECT
public:
    CDirModel( QObject *parent = nullptr );
    ~CDirModel();
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const override;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    virtual Qt::ItemFlags flags( const QModelIndex &idx ) const override;
    virtual int columnCount( const QModelIndex &parent ) const override;
    virtual int rowCount( const QModelIndex &parent ) const override;

    QModelIndex rootIndex() const;

    std::pair< bool, QStringList > transform( bool displayOnly ) const
    {
        return transform( rootIndex(), displayOnly );
    }
    void saveM3U( QWidget *parent ) const;
    void setTitleInfo( const QModelIndex &idx, std::shared_ptr< STitleInfo > info );
    std::shared_ptr< STitleInfo > getTitleInfo( const QModelIndex &idx ) const;
Q_SIGNALS:
public Q_SLOTS:
    void slotInputPatternChanged( const QString &inPattern );
    void slotOutputDirPatternChanged( const QString &outPattern );
    void slotOutputFilePatternChanged( const QString &outPattern );
    void slotTreatAsMovieChanged( bool treatAsMovie );
private:
    std::pair< bool, QStringList > transform( const QModelIndex &idx, bool displayOnly ) const;
    QString saveM3U( const QModelIndex &parentIndex, const QString &baseName ) const;
    bool isValidDirName( const QString &name ) const;
    void patternChanged();
    void patternChanged( const QModelIndex &idx );
    std::pair< bool, QString > transformItem( const QModelIndex &index ) const;
    [[nodiscard]] QString replaceCapture( const QString &captureName, const QString &returnPattern, const QString &value ) const;


    QString fInPattern;
    QString fOutFilePattern;
    QString fOutDirPattern;
    bool fTreatAsMovie{ false };
    QRegularExpression fInPatternRegExp;
    mutable std::map< QString, std::pair< bool, QString > > fFileMapping;
    mutable std::map< QString, std::pair< bool, QString > > fDirMapping;
    std::map< QString, std::shared_ptr< STitleInfo > > fTitleInfoMapping;
};

class CDirFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    CDirFilterModel( QObject *parent = nullptr );

    virtual bool filterAcceptsRow( int sourceRow, const QModelIndex &parent ) const override;
};



#endif // 
