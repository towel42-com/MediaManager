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

#include "PathsToDelete.h"
#include "Core/Preferences.h"

#include "ui_PathsToDelete.h"

#include <QStringListModel>
#include <QInputDialog>
#include "SABUtils/ButtonEnabler.h"

namespace NMediaManager
{
    namespace NUi
    {
        CPathsToDelete::CPathsToDelete( QWidget * parent )
            : CBasePrefPage( parent ),
            fImpl( new Ui::CPathsToDelete )
        {
            fImpl->setupUi( this );

            connect( fImpl->btnAddPathToDelete, &QToolButton::clicked, this, &CPathsToDelete::slotAddPathToDelete );
            connect( fImpl->btnDelPathToDelete, &QToolButton::clicked, this, &CPathsToDelete::slotDelPathToDelete );

            fPathsToDeleteModel = new QStringListModel( this );
            fImpl->pathsToDelete->setModel( fPathsToDeleteModel );

            new NSABUtils::CButtonEnabler( fImpl->pathsToDelete, fImpl->btnDelPathToDelete );
        }

        CPathsToDelete::~CPathsToDelete()
        {
        }

        void CPathsToDelete::slotAddPathToDelete()
        {
            addString( tr( "Add Path (Regular Expressions Allowed) Name to Delete" ), tr( "Name:" ), fPathsToDeleteModel, fImpl->pathsToDelete, false );
        }

        void CPathsToDelete::slotDelPathToDelete()
        {
            delString( fPathsToDeleteModel, fImpl->pathsToDelete );
        }

        void CPathsToDelete::load()
        {
            fPathsToDeleteModel->setStringList( NCore::CPreferences::instance()->getPathsToDelete() );
        }

        void CPathsToDelete::save()
        {
            NCore::CPreferences::instance()->setPathsToDelete( fPathsToDeleteModel->stringList() );
        }
    }
}
