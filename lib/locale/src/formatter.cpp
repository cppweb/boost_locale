namespace boost {
namespace locale {
    namespace impl {

        template<typename CharType>
        class number_format : public formatter<CharType> {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;
            
            virtual string_type format(double value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }
            virtual string_type format(int64_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }

            virtual string_type format(uint64_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(static_cast<int64_t>(value),tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }
            virtual string_type format(int32_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }
            virtual string_type format(uint32_t value,size_t &code_points) const
            {
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }

            virtual bool parse(string_type &str,double &value) const 
            {
                return false;
            }

            virtual bool parse(string_type &str,int64_t &value) const 
            {
                return false;
            }
            virtual bool parse(string_type &str,uint64_t &value) const
            {
                return false;
            }
            virtual bool parse(string_type &str,int32_t &value) const
            {
                return false;
            }
            virtual bool parse(string_type &str,uint32_t &value) const
            {
                return false;
            }

            number_format(std::auto_ptr<icu::NumberFormat> fmt,std::string codepage) :
                cvt_(codepage),
                icu_fmt_(fmt)
            {
            }
 
        private:
            icu_std_converter<CharType> cvt_;
            std::auto_ptr<icu::NumberFormat> icu_fmt_;
        };
        
        
        template<typename CharType>
        class date_format : public formatter<CharType> {
        public:
            typedef CharType char_type;
            typedef std::basic_string<CharType> string_type;
            
            virtual string_type format(double value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(int64_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }

            virtual string_type format(uint64_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(int32_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }
            virtual string_type format(uint32_t value,size_t &code_points) const
            {
                return do_format(value,code_points);
            }

            virtual bool parse(string_type &str,double &value) const 
            {
                return false;
            }

            virtual bool parse(string_type &str,int64_t &value) const 
            {
                return false;
            }
            virtual bool parse(string_type &str,uint64_t &value) const
            {
                return false;
            }
            virtual bool parse(string_type &str,int32_t &value) const
            {
                return false;
            }
            virtual bool parse(string_type &str,uint32_t &value) const
            {
                return false;
            }

            date_format(std::auto_ptr<icu::DateFormat> fmt,std::string codepage) :
                cvt_(codepage),
                icu_fmt_(fmt)
            {
            }
 
        private:
            string_type do_format(double value,size_t &codepoints) const 
            {
                UDate date = value * 1000.0; /// UDate is time_t in miliseconds
                icu::UnicodeString tmp;
                icu_fmt_->format(value,tmp);
                code_points=tmp.countChar32();
                return std(tmp);
            }
            icu_std_converter<CharType> cvt_;
            std::auto_ptr<icu::DateFormat> icu_fmt_;
        };
        
        template<CharType>
        std::auto_ptr<formatter<CharType> > generate(std::ios_base &ios)
        {
            std::auto_ptr<formatter<CharType> > fmt;
            ios_info &info=ios_prop<ios_info>::get(ios);
            if((into.flags() &  flags::display_flags_mask) == posix)
                return fmt;
            switch(into.flags() & flags::display_flags_mask) {
            case number:
                
            }
        }


    } // impl


} // locale
} // boost


// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4



