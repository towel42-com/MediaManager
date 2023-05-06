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

#ifndef __UI_BITRATEEXPERIMENT_H
#define __UI_BITRATEEXPERIMENT_H

#include <QDialog>

namespace NSABUtils
{
    struct SResolutionInfo;
}

namespace NMediaManager
{
    namespace NPreferences
    {
        namespace NUi
        {
            namespace Ui
            {
                class CBitrateExperiment;
            };

            class CBitrateExperiment : public QDialog
            {
                Q_OBJECT
            public:
                CBitrateExperiment( QWidget *parent = nullptr );
                virtual ~CBitrateExperiment() override;

                void setGreaterThan4kDivisor( int value );
                int greaterThan4kDivisor() const;

                void setTarget4kBitrate( const QString &value );
                QString getTarget4kBitrate() const;

                void setTargetHDBitrate( const QString &value );
                QString getTargetHDBitrate() const;

                void setTargetSubHDBitrate( const QString &value );
                QString getTargetSubHDBitrate() const;

                void setResolutionThreshold( int value );
                int resolutionThreshold() const;

            public Q_SLOTS:
                void slotResolutionChanged();
                void slotChanged();
                void slotOpenFile();
                void slotSourceChanged();
            Q_SIGNALS:
                void sigChanged();
            private:
                void loadFromFile();
                void load( const NSABUtils::SResolutionInfo &resDef );
                std::shared_ptr< NSABUtils::SResolutionInfo > getResolutionDef() const;

                bool fDisableUpdate{ false };
                std::unique_ptr< Ui::CBitrateExperiment > fImpl;
            };
        }
    }
}
#endif
