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

#include "TagAnalysisSettings.h"
#include "Preferences/Core/Preferences.h"

#include "ui_TagAnalysisSettings.h"

#include "SABUtils/WidgetEnabler.h"
#include "SABUtils/UtilityModels.h"
#include "SABUtils/QtUtils.h"
#include "SABUtils/MediaInfo.h"

#include <QSettings>
#include <QStringListModel>
#include <QInputDialog>
#include <QFileDialog>
#include <QMessageBox>

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            CTagAnalysisSettings::CTagAnalysisSettings( QWidget *parent ) :
                CBasePrefPage( parent ),
                fImpl( new Ui::CTagAnalysisSettings )
            {
                fImpl->setupUi( this );
                fModel = new NSABUtils::CCheckableStringListModel( this );
                fImpl->tagsToShow->setModel( fModel );
                new NSABUtils::CWidgetEnabler( fImpl->verifyMediaTitle, fImpl->verifyMediaTitleExpr );
                new NSABUtils::CWidgetEnabler( fImpl->verifyMediaDate, fImpl->verifyMediaDateExpr );
                new NSABUtils::CWidgetEnabler( fImpl->verifyMediaComment, fImpl->verifyMediaCommentExpr );
            }

            CTagAnalysisSettings::~CTagAnalysisSettings()
            {
            }

            void CTagAnalysisSettings::load()
            {
                fImpl->verifyMediaTags->setChecked( NPreferences::NCore::CPreferences::instance()->getVerifyMediaTags() );
                fImpl->verifyMediaTitle->setChecked( NPreferences::NCore::CPreferences::instance()->getVerifyMediaTitle() );
                fImpl->verifyMediaTitleExpr->setText( NPreferences::NCore::CPreferences::instance()->getVerifyMediaTitleExpr() );
                fImpl->verifyMediaDate->setChecked( NPreferences::NCore::CPreferences::instance()->getVerifyMediaDate() );
                fImpl->verifyMediaDateExpr->setText( NPreferences::NCore::CPreferences::instance()->getVerifyMediaDateExpr() );
                fImpl->verifyMediaComment->setChecked( NPreferences::NCore::CPreferences::instance()->getVerifyMediaComment() );
                fImpl->verifyMediaCommentExpr->setText( NPreferences::NCore::CPreferences::instance()->getVerifyMediaCommentExpr() );

                auto allTags = NPreferences::NCore::CPreferences::instance()->getAllMediaTags();
                std::list< std::pair< QString, bool > > tmp;
                for ( auto &&ii : allTags )
                {
                    tmp.emplace_back( std::make_pair( NSABUtils::displayName( ii.first ), ii.second ) );
                }

                fModel->setStringList( tmp );
            }

            void CTagAnalysisSettings::save()
            {
                NPreferences::NCore::CPreferences::instance()->setVerifyMediaTags( fImpl->verifyMediaTags->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setVerifyMediaTitle( fImpl->verifyMediaTitle->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setVerifyMediaTitleExpr( fImpl->verifyMediaTitleExpr->text() );
                NPreferences::NCore::CPreferences::instance()->setVerifyMediaDate( fImpl->verifyMediaDate->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setVerifyMediaDateExpr( fImpl->verifyMediaDateExpr->text() );
                NPreferences::NCore::CPreferences::instance()->setVerifyMediaComment( fImpl->verifyMediaComment->isChecked() );
                NPreferences::NCore::CPreferences::instance()->setVerifyMediaCommentExpr( fImpl->verifyMediaCommentExpr->text() );

                std::list< NSABUtils::EMediaTags > enabled;

                auto checked = fModel->getCheckedStrings();
                for ( auto &&ii : checked )
                {
                    enabled.emplace_back( NSABUtils::fromDisplayName( ii ) );
                }

                NPreferences::NCore::CPreferences::instance()->setEnabledTags( enabled );
            }
        }
    }
}