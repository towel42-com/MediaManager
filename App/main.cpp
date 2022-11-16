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
#include "UI/MainWindow.h"
#include "SABUtils/ValidateOpenSSL.h"
#include "Version.h"

#include <QApplication>
#include <debugapi.h>
#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QMessageBox>


void myMessageOutput( QtMsgType type, const QMessageLogContext & context, const QString & msg )
{
    static QFile * sOutFile{ nullptr };
    QByteArray localMsg = msg.toLocal8Bit();
    QString realMsg = QString( "%1 (%2:%3, %4)" ).arg( localMsg.constData() ).arg( ( QFileInfo( context.file ).fileName() ) ).arg( context.line ).arg( context.function );

    QString typeString;
    switch ( type )
    {
        case QtDebugMsg:
            typeString = "Debug";
            break;
        case QtInfoMsg:
            typeString = "Info";
            break;
        case QtWarningMsg:
            typeString = "Warning";
            break;
        case QtCriticalMsg:
            typeString = "Critical";
            break;
        case QtFatalMsg:
            typeString = "Fatal";
                //abort();
            break;
    }
    realMsg = QString( "%1: %2" ).arg( typeString ).arg( realMsg ).trimmed();

#ifdef Q_OS_WINDOWS
    OutputDebugString( qUtf16Printable( msg + "\n" ) );
#else
    fprintf( "%s\n", qPrintable( realMsg ) );
#endif
    //if ( !gOutFile )
    //{
    //    gOutFile = new QFile( "log.txt" );
    //    gOutFile->open( QFile::WriteOnly | QFile::Truncate );
    //    QString tmp = QFileInfo( gOutFile->fileName() ).absoluteFilePath();
    //    int xzy = 0;
    //}

    //QTextStream ts( gOutFile );
    //ts << realMsg << "\n";
    //gOutFile->flush();

}

int main( int argc, char ** argv )
{
    Q_INIT_RESOURCE( application );

    QApplication::setAttribute( Qt::AA_EnableHighDpiScaling );
    QApplication::setAttribute( Qt::AA_UseHighDpiPixmaps );
    QApplication appl( argc, argv );
    appl.setApplicationName( NVersion::APP_NAME );
    appl.setApplicationVersion( NVersion::getVersionString( true ) );
    appl.setOrganizationName( NVersion::VENDOR );
    appl.setOrganizationDomain( NVersion::HOMEPAGE );

    qInstallMessageHandler( myMessageOutput );

    auto aOK = NSABUtils::validateOpenSSL( true );
    if ( !aOK.first )
    {
        QMessageBox::critical( nullptr, QObject::tr( "Could not find OpenSSL libraries" ), aOK.second );
        return -1;
    }

    QString bifName;
    for ( int ii = 1; ii < argc; ++ii )
    {
        QString name = argv[ ii ];
        if ( name.toLower().endsWith( ".bif" ) )
        {
            bifName = name;
            break;
        }
    }
    NMediaManager::NUi::CMainWindow mainWindow;
    mainWindow.setWindowTitle( NVersion::getWindowTitle() );
    mainWindow.show();
    if ( !bifName.isEmpty() && !mainWindow.setBIFFileName( bifName ) )
        return -1;
    return appl.exec();
}
