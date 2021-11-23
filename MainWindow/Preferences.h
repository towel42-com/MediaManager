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

#ifndef _PREFERENCES_H
#define _PREFERENCES_H

#include <QDialog>
class QStringListModel;
namespace Ui {class CPreferences;};

class CPreferences : public QDialog
{
    Q_OBJECT
public:
    CPreferences(QWidget* parent = 0);
    ~CPreferences();

    static void setMediaDirectory( const QString &dir );
    static QString getMediaDirectory();

    static void setTreatAsTVShowByDefault( bool value );
    static bool getTreatAsTVShowByDefault();

    static void setExactMatchesOnly( bool value );
    static bool getExactMatchesOnly();

    static void setTVOutFilePattern( const QString &value );
    static QString getTVOutFilePattern();

    static void setTVOutDirPattern( const QString &value );
    static QString getTVOutDirPattern();

    static void setMovieOutFilePattern( const QString &value );
    static QString getMovieOutFilePattern();

    static void setMovieOutDirPattern( const QString &value );
    static QString getMovieOutDirPattern();

    static void setSearchExtensions( const QString &value );
    static void setSearchExtensions( const QStringList &value );
    static QStringList getSearchExtensions();

    static void setKnownStrings( const QStringList &value );
    static QStringList getKnownStrings();
public Q_SLOTS:
    void slotAddString();
    void slotDelString();
private:
    //static QString getDefaultInPattern( bool forTV ) const;
    static QString getDefaultOutDirPattern( bool forTV ); 
    static QString getDefaultOutFilePattern( bool forTV );
    
    void loadSettings();
    void loadPatterns();
    void saveSettings();
    void loadDirectory();

    QStringListModel * fStringModel;
    std::unique_ptr< Ui::CPreferences > fImpl;
};

#endif 
