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

#ifndef _DIRNODEITEM_H
#define _DIRNODEITEM_H

class QString;
class QVariant;
class QStandardItem;

#include <QIcon>
#include <list>
#include <optional>

namespace NSABUtils
{
    enum class EMediaTags;
}

namespace NMediaManager
{
    namespace NUi
    {
        class CBasePage;
    }

    namespace NCore
    {
        enum class EMediaType;
    }

    namespace NModels
    {
        enum class EType;
        enum EColumns
        {
            eFSName,
            eFSSize,
            eFSType,
            eFSModDate,
            eFirstCustomColumn
        };

        struct SCheckable
        {
            bool fIsCheckable{ false };
            bool fYesNoOnly{ false };
            Qt::CheckState fCheckState{ Qt::CheckState::Unchecked };
        };

        struct SDirNodeItem
        {
            SDirNodeItem();
            SDirNodeItem( const QString &text, int nodeType );

            void setData( const QVariant &value, int role );

            QStandardItem *createStandardItem() const;

            QString fText;
            EColumns fType{ EColumns::eFSName };
            QIcon fIcon;
            std::optional< Qt::Alignment > fAlignment;
            std::list< std::pair< QVariant, int > > fRoles;
            NCore::EMediaType fMediaType;
            std::optional< SCheckable > fCheckable;   // isCHeckable, yesno only, is checked
            std::optional< std::pair< EType, NSABUtils::EMediaTags > > fEditable;
        };
    }
}
#endif
