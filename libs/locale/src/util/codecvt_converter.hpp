//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_IMPL_UTIL_CODECVT_CONVERTER_HPP
#define BOOST_LOCALE_IMPL_UTIL_CODECVT_CONVERTER_HPP
#include <locale>
#include <boost/cstdint.hpp>
#include <boost/locale/generator.hpp>
#ifdef BOOST_MSVC
#  pragma warning(disable : 4244) // loose data 
#endif

#include <vector>
namespace boost {
namespace locale {
namespace util {

    class base_converter {
    public:

        static const uint32_t illegal=0xFFFFFFFF;
        static const uint32_t incomplete=0xFFFFFFFE;
        
        virtual ~base_converter() 
        {
        }
        virtual int max_len() const = 0;
        virtual bool is_thread_safe() const = 0;
        virtual base_converter *clone() const = 0;
        virtual uint32_t to_unicode(char const *&begin,char const *end) = 0;
        virtual uint32_t from_unicode(uint32_t u,char *begin,char const *end) = 0;
    };

    std::auto_ptr<base_converter> create_utf8_converter();
    std::auto_ptr<base_converter> create_simple_converter(std::string const &encoding);

    std::locale create_codecvt(std::locale const &in,std::auto_ptr<base_converter> cvt,character_facet_type type);

} // impl_icu
} // locale 
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
