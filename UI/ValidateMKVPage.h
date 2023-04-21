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

#ifndef _VALIDATEMKVPAGE_H
#define _VALIDATEMKVPAGE_H

#include "BasePage.h"

namespace NMediaManager
{
    namespace NUi
    {
        class CValidateMKVPage : public CBasePage
        {
            Q_OBJECT
        public:
            CValidateMKVPage( QWidget *parent = nullptr );
            virtual ~CValidateMKVPage() override;

            virtual bool useSecondaryProgressBar() const override { return true; }
            virtual QString secondaryProgressLabel() const override;

            virtual QString loadTitleName() const override;
            virtual QString loadCancelName() const override;

            virtual QString actionTitleName() const override;
            virtual QString actionCancelName() const override;
            virtual QString actionErrorName() const override;

            virtual NModels::CDirModel *createDirModel() override;

        Q_SIGNALS:
        public Q_SLOTS:
        protected Q_SLOTS:
        protected:
        };
    }
}
#endif
