//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#ifndef BOOST_LOCALE_BOUNDARY_HPP_INCLUDED
#define BOOST_LOCALE_BOUNDARY_HPP_INCLUDED

#include <boost/locale/config.hpp>
#include <boost/cstdint.hpp>

#include <string>
#include <locale>
#include <vector>
#include <iterator>
#include <algorithm>
#include <typeinfo>
#include <iterator>
#include <stdexcept>

namespace boost {
    namespace locale {
        
        namespace boundary {
            ///
            /// \brief The enum that describes possible break types
            ///
            typedef enum {
                character,  ///< Find character boundaries
                word,       ///< Find word boundaries
                sentence,   ///< Find sentence boundaries
                line        ///< Find a positions suitable for line breaks
            } boundary_type;

            ///
            /// Flags used with word boundary analysis -- the type of word found
            ///
            typedef enum {
                word_none       =  0x0000F,   ///< Not a word
                word_number     =  0x000F0,   ///< Word that appear to be a number
                word_letter     =  0x00F00,   ///< Word that contains letters, excluding kana and ideographic characters 
                word_kana       =  0x0F000,   ///< Word that contains kana characters
                word_ideo       =  0xF0000,   ///< Word that contains ideographic characters
                word_any        =  0xFFFF0,   ///< Any word including numbers, 0 is special flag, equivalent to 15
                word_letters    =  0xFFF00,   ///< Any word, excluding numbers but including letters, kana and ideograms.
                word_kana_ideo  =  0xFF000,   ///< Word that includes kana or ideographic characters
                word_mask       =  0xFFFFF    ///< Maximal used mask
            } word_type;
            ///
            /// Flags that describe a type of line break
            ///
            typedef enum {
                line_soft       =  0x0F,   ///< Soft line break: optional but not required
                line_hard       =  0xF0,   ///< Hard line break: like break is required (as per CR/LF)
                line_any        =  0xFF,   ///< Soft or Hard line break
                line_mask       =  0xFF
            } line_break_type;
            
            typedef enum {
                sentence_term   =  0x0F,   ///< The sentence was terminated with a sentence terminator 
                                          ///  like ".", "!" possible followed by hard separator like CR, LF, PS
                sentence_sep    =  0xF0,   ///< The sentence does not contain terminator like ".", "!" but ended with hard separator
                                          ///  like CR, LF, PS or end of input.
                sentence_any    =  0xFF,   ///< Either first or second sentence break type;.
                sentence_mask   =  0xFF
            } sentence_break_type;

            typedef enum {
                character_any   = 0xF,   ///< Not in use, just for consistency
                character_mask  = 0xF,
            } character_break_type;

            inline unsigned boundary_mask(boundary_type t)
            {
                switch(t) {
                case character: return character_mask;
                case word:      return word_mask;
                case sentence:  return sentence_mask;
                case line:      return line_mask;
                default:        return 0;
                }
            }
           
            namespace impl {

                ///
                /// \ brief a structure that describes break position
                /// 
                struct break_info {

                    ///
                    /// Default constructor -- all fields are zero
                    /// 
                    break_info() : 
                        offset(0),
                        mark(0)
                    {
                    }
                    ///
                    /// Create a break info with offset \a v
                    ///
                    break_info(unsigned v) :
                        offset(v),
                        mark(0)
                    {
                    }

                    uint32_t offset;            ///< Offset from the beginning of the text where break occurs
                    uint32_t mark;
                   
                    ///
                    /// Comparison operator allowing usage of break_info in STL algorithms
                    /// 
                    bool operator<(break_info const &other) const
                    {
                        return offset < other.offset;
                    }
                };
               
                ///
                /// The index of all found boundaries. Note: for a string with length \a len, 0 and len are always considered
                /// boundaries
                /// 
                typedef std::vector<break_info> index_type;


                ///
                /// Find boundary positions of type \a t for text in range [begin,end) using locale \a loc
                ///
                template<typename CharType>
                index_type map(boundary_type t,CharType const *begin,CharType const *end,std::locale const &loc=std::locale());
                
                ///
                /// Find boundary positions of type \a t for string \a str using locale \a loc
                ///
                template<typename CharType>
                static index_type map(
                                boundary_type t,
                                std::basic_string<CharType> const &str,
                                std::locale const &loc=std::locale())
                {
                    return boost::locale::boundary::impl::map<CharType>(t,str.data(),str.data()+str.size(),loc);
                }
                
                template<>
                BOOST_LOCALE_DECL index_type 
                map(boundary_type t,char const *begin,char const *end,std::locale const &loc);

                #ifndef BOOST_NO_STD_WSTRING
                template<>
                BOOST_LOCALE_DECL index_type 
                map(boundary_type t,wchar_t const *begin,wchar_t const *end,std::locale const &loc);
                #endif

                #ifdef BOOST_HAS_CHAR16_T
                template<>
                BOOST_LOCALE_DECL index_type 
                map(boundary_type t,char16_t const *begin,char16_t const *end,std::locale const &loc);
                #endif

                #ifdef BOOST_HAS_CHAR32_T
                template<>
                BOOST_LOCALE_DECL index_type 
                map(boundary_type t,char32_t const *begin,char32_t const *end,std::locale const &loc);
                #endif
            } // impl

            namespace details {

                template<typename IteratorType,typename CategoryType = typename std::iterator_traits<IteratorType>::iterator_category>
                struct mapping_traits {
                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                    static impl::index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        std::basic_string<char_type> str(b,e);
                        return impl::map(t,str,l);
                    }
                };

                template<typename IteratorType>
                struct mapping_traits<IteratorType,std::random_access_iterator_tag> {
                    typedef typename std::iterator_traits<IteratorType>::value_type char_type;

                    static impl::index_type map(boundary_type t,IteratorType b,IteratorType e,std::locale const &l)
                    {
                        impl::index_type result;
                        //
                        // Optimize for most common cases
                        //
                        // C++0x requires that string is continious in memory and all string implementations
                        // do this because of c_str() support. 
                        //

                        if( 
                            (
                                typeid(IteratorType) == typeid(typename std::basic_string<char_type>::iterator)
                                || typeid(IteratorType) == typeid(typename std::basic_string<char_type>::const_iterator)
                                || typeid(IteratorType) == typeid(typename std::vector<char_type>::iterator)
                                || typeid(IteratorType) == typeid(typename std::vector<char_type>::const_iterator)
                                || typeid(IteratorType) == typeid(char_type *)
                                || typeid(IteratorType) == typeid(char_type const *)
                            )
                            &&
                                b!=e
                          )
                        {
                            char_type const *begin = &*b;
                            char_type const *end = begin + (e-b);
                            impl::index_type tmp=impl::map(t,begin,end,l);
                            result.swap(tmp);
                        }
                        else{
                            std::basic_string<char_type> str(b,e);
                            impl::index_type tmp=impl::map(t,str,l);
                            result.swap(tmp);
                        }
                        return result;
                    }
                };

            } // details 

            ///
            /// \brief Class the holds boundary mapping of the text that can be used with iterators
            ///
            /// When the object is created in creates index and provides access to it with iterators.
            /// it is used mostly together with break_iterator and token_iterator
            ///
            /// This class stores iterators to the original text, so you should be careful with iterators
            /// invalidation. The text, iterators pointing to should not change
            ///

            template<class RangeIterator>
            class mapping {
            public:
                typedef RangeIterator iterator;
                typedef typename RangeIterator::base_iterator base_iterator;
                typedef typename std::iterator_traits<base_iterator>::value_type char_type;

                ///
                /// create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc
                ///
                mapping(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,0xFFFFFFFFu);
                }

                mapping(boundary_type type,base_iterator begin,base_iterator end,unsigned mask,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,mask);
                }

                ///
                /// create a mapping of type \a type of the text in range [\a begin, \a end) using locale \a loc
                ///
                void map(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,0xFFFFFFFFu);
                }

                void map(boundary_type type,base_iterator begin,base_iterator end,unsigned mask,std::locale const &loc = std::locale())
                {
                    create_mapping(type,begin,end,loc,mask);
                }

                mapping()
                {
                    mask_=0xFFFFFFFF;
                }

                template<typename ORangeIterator>
                mapping(mapping<ORangeIterator> const &other) :
                    index_(other.index_),
                    begin_(other.begin_),
                    end_(other.end_),
                    mask_(other.mask_)
                {
                }
                
                template<typename ORangeIterator>
                void swap(mapping<ORangeIterator> &other)
                {
                    index_.swap(other.index_),
                    std::swap(begin_,other.begin_);
                    std::swap(end_,other.end_);
                    std::swap(mask_,other.mask_);
                }

                template<typename ORangeIterator>
                mapping const &operator=(mapping<ORangeIterator> const &other)
                {
                    index_=other.index_;
                    begin_=other.begin_;
                    end_=other.end_;
                    mask_=other.mask_;
                }

                unsigned mask() const
                {
                    return mask_;
                }
                void mask(unsigned u)
                {
                    mask_ = u;
                }

                ///
                /// Get \a begin iterator used when object was created
                ///
                RangeIterator begin()
                {
                    return RangeIterator(*this,true,mask_);
                }
                ///
                /// Get \a end iterator used when object was created
                ///
                RangeIterator end()
                {
                    return RangeIterator(*this,false,mask_);
                }

            private:
                void create_mapping(boundary_type type,base_iterator begin,base_iterator end,std::locale const &loc,unsigned mask)
                {
                    impl::index_type idx=details::mapping_traits<base_iterator>::map(type,begin,end,loc);
                    index_.swap(idx);
                    begin_ = begin;
                    end_ = end;
                    mask_=mask;
                }
                template<typename I,typename M> 
                friend class base_boundary_iterator; 
                template<typename I> 
                friend class break_iterator; 
                template<typename I,typename V>
                friend class token_iterator; 
                template<typename I>
                friend class mapping;

                base_iterator begin_,end_;
                impl::index_type index_;
                unsigned mask_;
            };


            ///
            /// \brief token iterator is an iterator that returns text chunks between boundary positions
            ///
            /// It is similar to break iterator, but it rather "points" to string then iterator. It is used when we are more
            /// interested in text chunks themselves then boundary points.
            ///
            
            template<
                typename IteratorType,
                typename ValueType = std::basic_string<typename std::iterator_traits<IteratorType>::value_type> 
            >
            class token_iterator : public std::iterator<std::bidirectional_iterator_tag,ValueType> 
            {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                typedef IteratorType base_iterator;
                typedef mapping<token_iterator<IteratorType,ValueType> > mapping_type;
                                
                token_iterator() : 
                    map_(0),
                    offset_(0),
                    mask_(0xFFFFFFFF),
                    full_select_(false)
                {
                }

                token_iterator const &operator=(IteratorType p)
                {
                    unsigned dist=std::distance(map_->begin_,p);
                    impl::index_type::const_iterator b=map_->index_.begin(),e=map_->index_.end();
                    impl::index_type::const_iterator 
                        bound=std::upper_bound(b,e,impl::break_info(dist));
                    while(bound != e && (bound->mark & mask_)==0)
                        bound++;
                    offset_ = bound - b;
                    return *this;
                }

                token_iterator(mapping_type const &map,bool begin,unsigned mask) :
                    map_(&map),
                    mask_(mask),
                    full_select_(false)
                {
                    if(begin) {
                        offset_ = 0;
                        next();
                    }
                    else
                        offset_=map_->index_.size();
                }
                token_iterator(token_iterator const &other) :
                    map_(other.map_),
                    offset_(other.offset_),
                    mask_(other.mask_),
                    full_select_(other.full_select_)
                {
                }

                token_iterator const &operator=(token_iterator const &other)
                {
                    if(this!=&other) {
                        map_ = other.map_;
                        offset_ = other.offset_;
                        mask_=other.mask_;
                        full_select_ = other.full_select_;
                    }
                    return *this;
                }

                ValueType operator*() const
                {
                    if(offset_ < 1 || offset_ >= map_->index_.size())
                        throw std::out_of_range("Invalid token iterator location");
                    unsigned pos=offset_-1;
                    if(full_select_)
                        while(!valid_offset(pos))
                            pos--;
                    base_iterator b=map_->begin_;
                    unsigned b_off = map_->index_[pos].offset;
                    std::advance(b,b_off);
                    base_iterator e=b;
                    unsigned e_off = map_->index_[offset_].offset;
                    std::advance(e,e_off-b_off);
                    return ValueType(b,e);
                }

                token_iterator &operator++() 
                {
                    next();
                    return *this;
                }
                
                token_iterator &operator--() 
                {
                    prev();
                    return *this;
                }
                
                token_iterator operator++(int unused) 
                {
                    token_iterator tmp(*this);
                    next();
                    return tmp;
                }

                token_iterator operator--(int unused) 
                {
                    token_iterator tmp(*this);
                    prev();
                    return tmp;
                }

                bool full_select() const
                {
                    return full_select_;
                }
                void full_select(bool fs)
                {
                    full_select_ = fs;
                }
                
                bool operator==(token_iterator const &other) const
                {
                    return  map_ == other.map_ 
                            && offset_==other.offset_ 
                            && mask_ == other.mask_;
                }

                bool operator!=(token_iterator const &other) const
                {
                    return !(*this==other);
                }

                unsigned mark() const
                {
                    return map_->index_.at(offset_).mark;
                }

            private:
                
                bool valid_offset(unsigned offset) const
                {
                    return  offset == 0 
                            || offset == map_->index_.size()
                            || (map_->index_[offset].mark & mask_)!=0;
                }

                bool at_end() const
                {
                    return !map_ || offset_>=map_->index_.size();
                }
                
                void next()
                {
                    while(offset_ < map_->index_.size()) {
                        offset_++;
                        if(valid_offset(offset_))
                            break;
                    }
                }
                
                void prev()
                {
                    while(offset_ > 0) {
                        offset_ --;
                        if(valid_offset(offset_))
                            break;
                    }
                }
                
                mapping_type const * map_;
                size_t offset_;
                unsigned mask_;
                uint32_t full_select_ : 1;
                uint32_t reserved_ : 31;
            };


            ///
            /// \brief break_iterator is bidirectional iterator that returns text boundary positions
            ///
            /// This iterator is used when boundary points are more interesting then text chunks themselves.
            ///
            template<typename IteratorType>
            class break_iterator : public std::iterator<std::bidirectional_iterator_tag,IteratorType> 
            {
            public:
                typedef typename std::iterator_traits<IteratorType>::value_type char_type;
                typedef IteratorType base_iterator;
                typedef mapping<break_iterator<IteratorType> > mapping_type;
                
                break_iterator() : 
                    map_(0),
                    offset_(0),
                    mask_(0xFFFFFFFF)
                {
                }

                break_iterator(break_iterator const &other):
                    map_(other.map_),
                    offset_(other.offset_),
                    mask_(other.mask_)
                {
                }
                
                break_iterator const &operator=(break_iterator const &other)
                {
                    if(this!=&other) {
                        map_ = other.map_;
                        offset_ = other.offset_;
                        mask_=other.mask_;
                    }
                    return *this;
                }

                break_iterator(mapping_type const &map,bool begin,unsigned mask) :
                    map_(&map),
                    mask_(mask)
                {
                    if(begin)
                        offset_ = 0;
                    else
                        offset_=map_->index_.size();
                }

                bool operator==(break_iterator const &other) const
                {
                    return  map_ == other.map_ 
                            && offset_==other.offset_
                            && mask_==other.mask_;
                }

                bool operator!=(break_iterator const &other) const
                {
                    return !(*this==other);
                }

                unsigned mark() const
                {
                    return map_->index_.at(offset_).mark;
                }
 
                break_iterator const &operator=(base_iterator p)
                {
                    at_least(p);
                    return *this;
                }
                
                base_iterator operator*() const
                {
                    if(offset_ >=map_->index_.size())
                        throw std::out_of_range("Invalid position of break iterator");
                    base_iterator p = map_->begin_;
                    std::advance(p, map_->index_[offset_].offset);
                    return p;
                }

                break_iterator &operator++() 
                {
                    next();
                    return *this;
                }
                
                break_iterator &operator--() 
                {
                    prev();
                    return *this;
                }
                
                break_iterator operator++(int unused) 
                {
                    break_iterator tmp(*this);
                    next();
                    return tmp;
                }

                break_iterator operator--(int unused) 
                {
                    break_iterator tmp(*this);
                    prev();
                    return tmp;
                }

            private:
                bool valid_offset(unsigned offset) const
                {
                    return  offset == 0 
                            || offset + 1 >= map_->index_.size() // last and first are always valid regardless of mark
                            || (map_->index_[offset].mark & mask_)!=0;
                }

                bool at_end() const
                {
                    return !map_ || offset_>=map_->index_.size();
                }
                
                void next()
                {
                    while(offset_ < map_->index_.size()) {
                        offset_++;
                        if(valid_offset(offset_))
                            break;
                    }
                }
                void prev()
                {
                    while(offset_ > 0) {
                        offset_ --;
                        if(valid_offset(offset_))
                            break;
                    }
                }
                
                void at_least(IteratorType p)
                {
                    unsigned diff =  std::distance(map_->begin_,p);

                    impl::index_type::const_iterator b=map_->index_.begin();
                    impl::index_type::const_iterator e=map_->index_.end();
                    impl::index_type::const_iterator ptr = std::lower_bound(b,e,impl::break_info(diff));

                    if(ptr==map_->index_.end())
                        offset_=map_->index_.size()-1;
                    else
                        offset_=ptr - map_->index_.begin();
                    
                    while(!valid_offset(offset_))
                        offset_ ++;
                }

                mapping_type const * map_;
                size_t offset_;
                unsigned mask_;
                uint32_t reserved_;
            };



            
            
        } // boundary
    } // locale
} // boost

#endif
// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4
