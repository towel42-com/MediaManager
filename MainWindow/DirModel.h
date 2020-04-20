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
class CDirModel : public QFileSystemModel
{
    Q_OBJECT
public:
    CDirModel(QObject *parent = nullptr);
    ~CDirModel();
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const override;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const override;
    virtual Qt::ItemFlags flags( const QModelIndex & idx ) const override;
    virtual int columnCount( const QModelIndex & parent ) const override;

    std::pair< bool, QStringList > transform( const QModelIndex& idx, bool displayOnly );
    QModelIndex rootIndex() const;
Q_SIGNALS:
public Q_SLOTS:
    void slotInputPatternChanged( const QString & inPattern );
    void slotOutputPatternChanged( const QString& outPattern );
private:
    void patternChanged();
    void patternChanged( const QModelIndex& idx );
    std::pair< bool, QString > transformFile( const QModelIndex& index ) const;

    QString fInPattern;
    QString fOutPattern;
    QRegularExpression fInPatternRegExp;
    mutable std::map< QString, std::pair< bool, QString > > fFileMapping;
};

#endif // 
