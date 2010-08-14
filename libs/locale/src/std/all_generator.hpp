//
//  Copyright (c) 2009-2010 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_IMPL_STD_ALL_GENERATOR_HPP
#define BOOST_LOCALE_IMPL_STD_ALL_GENERATOR_HPP

#include <boost/locale/generator.hpp>
#include <vector>

namespace boost {
    namespace locale {
        namespace impl_std {
            typedef enum {
                utf8_none,
                utf8_native,
                utf8_from_wide
            } utf8_support;

            std::locale create_convert( std::locale const &in,
                                        std::string const &locale_name,
                                        character_facet_type type,
                                        utf8_support = utf8_none);

            std::locale create_collate( std::locale const &in,
                                        std::string const &locale_name,
                                        character_facet_type type,
                                        utf8_support utf = utf8_none);

            std::locale create_formatting(  std::locale const &in,
                                            std::string const &locale_name,
                                            character_facet_type type,
                                            utf8_support utf = utf8_none);

            std::locale create_parsing( std::locale const &in,
                                        std::string const &locale_name,
                                        character_facet_type type,
                                        utf8_support utf = utf8_none);

            std::locale create_codecvt( std::locale const &in,
                                        std::locale const &loclae_name,
                                        character_facet_type type,
                                        utf8_support utf = utf8_none); 

            std::locale create_info(std::locale const &,std::string const &name); // ok

        }
    }
}

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
