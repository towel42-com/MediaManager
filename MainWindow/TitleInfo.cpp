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

#include "TitleInfo.h"
#include "SABUtils/StringUtils.h"
#include "SABUtils/QtUtils.h"

QString STitleInfo::getTitle() const
{
    return NStringUtils::transformTitle( fTitle );
}

QString STitleInfo::getYear() const
{
    auto dt = NQtUtils::findDate( fReleaseDate );
    if ( !dt.isValid() )
        return QString();
    return QString::number( dt.year() );
}

QString STitleInfo::getEpisodeTitle() const
{
    return NStringUtils::transformTitle( fEpisodeTitle );
}

QString STitleInfo::getText( ETitleInfo which, bool forceTop /*= false */ ) const
{
    auto parentPtr = fParent.lock();
    if ( forceTop )
    {
        if ( parentPtr )
            return parentPtr->getText( which, forceTop );
        else
            return QString();
    }

    auto text = getMyText( which );
    if ( text.isEmpty() && parentPtr )
    {
        text = parentPtr->getText( which, forceTop );
    }
    return text;
}

QString STitleInfo::getMyText( ETitleInfo which ) const
{
    switch( which )
    {
        case ETitleInfo::eTitle: return getTitle();
        case ETitleInfo::eReleaseDate: return getYear();
        case ETitleInfo::eTMDBID: return fTMDBID;
        case ETitleInfo::eSeason: return fSeason;
        case ETitleInfo::eEpisode: return fEpisode;
        case ETitleInfo::eEpisodeTitle: return fEpisodeTitle;
        case ETitleInfo::eExtraInfo: return fExtraInfo;
        case ETitleInfo::eDescription: return fDescription;
    }
    return QString();
}