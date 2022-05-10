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

#include "DirNodeItem.h"
#include "DirModel.h"
#include "Core/TransformResult.h"
#include "Preferences/Core/Preferences.h"

#include <QVariant>
#include <QStandardItem>

namespace NMediaManager
{
    namespace NModels
    {
        SDirNodeItem::SDirNodeItem() :
            fMediaType( NCore::EMediaType::eUnknownType )
        {

        }

        SDirNodeItem::SDirNodeItem( const QString & text, int nodeType ) :
            fText( text ),
            fType( static_cast<EColumns>(nodeType) ),
            fMediaType( NCore::EMediaType::eUnknownType )
        {

        }

        void SDirNodeItem::setData( const QVariant & value, int role )
        {
            fRoles.emplace_back( value, role);
        }

        QStandardItem * SDirNodeItem::createStandardItem() const
        {
            QStandardItem * retVal = nullptr;
            if ( !fEditable.has_value() )
            {
                retVal = new QStandardItem( fText );
                retVal->setEditable( false );
            }
            else
            {
                retVal = new CDirModelItem( fText, fEditable.value().first );
                bool editable = true;
                if ( fEditable.value().first == EType::eMediaTag )
                {
                    retVal->setData( static_cast<int>( fEditable.value().second ), NModels::ECustomRoles::eMediaTagTypeRole );
                    auto mkvPropEdit = NPreferences::NCore::CPreferences::instance()->getMKVPropEditEXE();
                    if ( mkvPropEdit.isEmpty() )
                        editable = false;
                }
                retVal->setEditable( editable );
            }

            retVal->setIcon( fIcon );
            if ( fAlignment.has_value() )
                retVal->setTextAlignment( fAlignment.value() );
            for ( auto && ii : fRoles )
            {
                retVal->setData( ii.first, ii.second );
            }
            if ( fCheckable.has_value() )
            {
                retVal->setCheckable( std::get< 0 >( fCheckable.value() ) );
                retVal->setData( std::get< 1 >( fCheckable.value() ), NModels::ECustomRoles::eYesNoCheckableOnly );
                retVal->setCheckState( std::get< 2 >( fCheckable.value() ) ? Qt::CheckState::Checked : Qt::CheckState::Unchecked );
            }
            return retVal;
        }
    }
}
