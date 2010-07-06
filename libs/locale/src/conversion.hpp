//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#include <boost/locale/conversion.hpp>
#include <boost/locale/info.hpp>
#include <unicode/normlzr.h>
#include <unicode/ustring.h>

#include "info_impl.hpp"
#include "uconv.hpp"


namespace boost {
namespace locale {

    template<typename CharType>
    class converter_impl : public converter<CharType> {
    public:
        typedef CharType char_type;
        typedef std::basic_string<char_type> string_type;

        converter_impl(icu::Locale const &loc,std::string const &encoding) :
            locale_(loc),
            encoding_(encoding)
        {
        }


        virtual string_type convert(converter_base::conversion_type how,char_type const *begin,char_type const *end,int flags = 0) const
        {
            impl::icu_std_converter<char_type> cvt(encoding_);
            icu::UnicodeString str=cvt.icu(begin,end);
            switch(how) {
            case converter_base::normalization:
                do_normalize(str,flags);
                break;
            case converter_base::upper_case:
                str.toUpper(locale_);
                break;
            case converter_base::lower_case:
                str.toLower(locale_);
                break;
            case converter_base::title_case:
                str.toTitle(0,locale_);
                break;
            case converter_base::case_folding:
                str.foldCase();
                break;
            default:
                ;
            }
            return cvt.std(str);
        }
    
    private:

        void do_normalize(icu::UnicodeString &str,int flags) const
        {
            UErrorCode code=U_ZERO_ERROR;
            UNormalizationMode mode=UNORM_DEFAULT;
            switch(flags) {
            case norm_nfd:
                mode=UNORM_NFD;
                break;
            case norm_nfc:
                mode=UNORM_NFC;
                break;
            case norm_nfkd:
                mode=UNORM_NFKD;
                break;
            case norm_nfkc:
                mode=UNORM_NFKC;
                break;
            }
            icu::UnicodeString tmp;
            icu::Normalizer::normalize(str,mode,0,tmp,code);

            impl::check_and_throw_icu_error(code);

            str=tmp;
        }

        icu::Locale locale_;
        std::string encoding_;
    }; // converter_impl

} // locale
} // boost

// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
