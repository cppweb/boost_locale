//
//  Copyright (c) 2009 Artyom Beilis (Tonkikh)
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
#define BOOST_LOCALE_SOURCE
#include <boost/locale/date_time.hpp>
#include <boost/locale/formatting.hpp>
#include <unicode/calendar.h>
#include "info_impl.hpp"

namespace boost {
namespace locale {
namespace date_time {

/// UTILITY

#define CALENDAR(ptr) (*reinterpret_cast<icu::Calendar **>(&(ptr)->impl_))
#define calendar_ CALENDAR(this)

static UCalendarDateFields to_icu(field_type f)
{
    switch(f) {
    case ara: return UCAL_ERA;
    case year: return UCAL_YEAR;
    case extended_year: return UCAL_EXTENDED_YEAR;
    case month: return UCAL_MONTH;
    case day: return UCAL_DATE;
    case day_of_year: return UCAL_DAY_OF_YEAR;
    case day_of_week: return UCAL_DAY_OF_WEEK;
    case day_of_week_in_month:  return UCAL_DAY_OF_WEEK_IN_MONTH;
    case day_of_week_local: return UCAL_DOW_LOCAL;
    case hour: return UCAL_HOUR_OF_DAY;
    case hour_12: return UCAL_HOUR;
    case am_pm: return UCAL_AM_PM;
    case minute: return UCAL_MINUTE;
    case second: return UCAL_SECOND;
    case week_of_year: return UCAL_WEEK_OF_YEAR;
    case week_of_month: return UCAL_WEEK_OF_MONTH;
    default:
        throw std::invalid_argument("Invalid date_time field type");
    }
}

static void *create_calendar(std::locale const &loc=std::locale(),time_zone const &zone=time_zone())
{
    std::auto_ptr<icu::Calendar> cal;
    std::auto_ptr<icu::TimeZone> tz(zone.impl()->icu_tz()->clone());
    UErrorCode err=U_ZERO_ERROR:
    if(std::has_facet<info>(loc)) {
        cal.reset(icu::Calendar::createInstance(std::use_facet<info>(loc).impl()->locale,tz.release(),err));
    }
    else {
        cal.reset(icu::Calendar::createInstance(tz.release(),err));
    }
    check_and_throw(err);
    return reinterpret_cast<void *>(cal.release());
}





/////////////////////////
// Calendar
////////////////////////

calendar::calendar(std::locale const &l,time_zone const &zone) :
    locale_(l),
    tz_(zone)
{
    calendar_=create_calendar(locale_,tz_);
}

calendar::calendar(std::locale const &l) :
    locale_(l)
{
    calendar_=create_calendar(locale_,tz_);
}

calendar::calendar(std::ios_base &ios) :
    locale_(ios.getloc()),
    tz_(ext_pattern<char>(ios,flags::time_zone_id))
{
    calendar_=create_calendar(locale_,tz_);
}

calendar::calendar() 
{
    calendar_=create_calendar(locale_,tz_);
}

calendar::~calendar()
{
    delete calendar_;
    calendar_=0;
}

calendar::calendar(calendar const &other) :
    locale_(other.locale_),
    tz_(other.tz_)
{
    calendar_=CALENDAR(&other)->clone();
}

calendar const &operator = (calendar const &other) 
{
    if(this !=&other) {
        locale_ = other.locale_;
        tz_ = other.tz_;
        delete calendar_;
        calendar_=0;
        calendar_=CALENDAR(&other)->clone();
    }
    return *this;
}

boost::locale::time_zone calendar::time_zone() const
{
    return tz_;
}

std::locale calendar::locale() const
{
    return locale_;
}

int calendar::minimum(field_type f) const
{
    return calendar_->getMinimum(to_icu(f));
}

int calendar::greatest_minimum(field_type f) const
{
    return calendar_->getGreatesMinimum(to_icu(f));
}

int calendar::maximum(field_type f) const
{
    return calendar_->getMaximum(to_icu(f));
}

int calendar::least_maximum(field_type f) const
{
    return calendar_->getLeastMaximum(to_icu(f));
}

int calendar::first_day_of_week() const
{
    UErrorCode e=U_ZERO_ERROR;
    int first=static_cast<int>(calendar_->getFirstDayOfWeek(e));
    check_and_throw(e);
    return first;
}

bool calendar::operator==(calendar const &other) const
{
    return calendar_.isEquivalentTo(CALENDAR(&other));
}

bool calendar::operator!=(calendar const &other) const
{
    return !(*this==other);
}

//////////////////////////////////
// date_time
/////////////////

date_time::date_time()
{
    calendar_=create_calendar();
}

date_time::date_time(date_time const &other)
{
    calendar_=CALENDAR(&other)->clone();
}

date_time const &date_time::operator = (date_time const &other)
{
    if(this != &other) {
        delete calendar_;
        calendar_=CALENDAR(&other)->clone();
    }
    return *this;
}

date_time::~date_time()
{
    delete calendar_;
    calendar_ = 0;
}

date_time::date_time(double time)
{
    calendar_=create_calendar();
    UErrorCode e=U_ZERO_ERROR;
    calendar_->setTime(time * 1000.0,e);
    if(U_FAILURE(e)) {
        delete calendar_;
        check_and_throw(e);
    }
}

date_time::date_time(double time,calendar const &cal)
{
    calendar_=CALENDAR(&cal)->clone();
    UErrorCode e=U_ZERO_ERROR;
    calendar_->setTime(time * 1000.0,e);
    if(U_FAILURE(e)) {
        delete calendar_;
        check_and_throw(e);
    }
}

date_time::date_time(date_time_field_set const &s)
{
    calendar_=create_calendar();
    try {
        for(unsigned i=0;i<s.size();i++)
            set(s[i].field,s[i].value);
    }
    catch(...) {
        delete calendar_;
        throw;
    }
}
date_time::date_time(date_time_field_set const &set,calendar const &cal)
{
    calendar_=CALENDAR(&cal)->clone();
    try {
        for(unsigned i=0;i<s.size();i++)
            set(s[i].field,s[i].value);
    }
    catch(...) {
        delete calendar_;
        throw;
    }

}

date_time const &date_time::operator=(date_time_field_set const &s)
{
    for(unsigned i=0;i<s.size();i++)
        set(s[i].field,s[i].value);
    return *this;
}

void date_time::set(field_type f,int v)
{
    calendar_->set(to_icu(f),v);
}

int date_time::get(field_type f) const
{
    UErrorCode e=U_ZERO_ERROR;
    calendar_->set(to_icu(f),e);
    check_and_throw(e);
}

date_time date_time::operator+(date_time_field_set const &v) const
{
    date_time tmp(*this);
    tmp+=v;
    return tmp;
}

date_time date_time::operator-(date_time_field_set const &v) const
{
    date_time tmp(*this);
    tmp-=v;
    return tmp;
}

date_time date_time::operator<<(date_time_field_set const &v) const
{
    date_time tmp(*this);
    tmp<<=v;
    return tmp;
}

date_time date_time::operator>>(date_time_field_set const &v) const
{
    date_time tmp(*this);
    tmp>>=v;
    return tmp;
}

date_time const &date_time::operator+=(date_time_field_set const &v) 
{
    for(unsigned i=0;i<v.size();i++) {
        UErrorCode e;
        calendar_->add(to_icu(v[i].field),v[i].value,e);
        check_and_throw(e);
    }
    return *this;
}

date_time const &date_time::operator-=(date_time_field_set const &v) 
{
    for(unsigned i=0;i<v.size();i++) {
        UErrorCode e;
        calendar_->add(to_icu(v[i].field),-v[i].value,e);
        check_and_throw(e);
    }
    return *this;
}

date_time const &date_time::operator<<=(date_time_field_set const &v) 
{
    for(unsigned i=0;i<v.size();i++) {
        UErrorCode e;
        calendar_->roll(to_icu(v[i].field),v[i].value,e);
        check_and_throw(e);
    }
    return *this;
}

date_time const &date_time::operator>>=(date_time_field_set const &v) 
{
    for(unsigned i=0;i<v.size();i++) {
        UErrorCode e;
        calendar_->roll(to_icu(v[i].field),-v[i].value,e);
        check_and_throw(e);
    }
    return *this;
}

double date_time::time() const
{
    UErrorCode e=U_ZERO_ERROR;
    double v=calendar_->getTime(e);
    check_and_throw(e);
    return v/1000.0;
}

void date_time::time(double v)
{
    UErrorCode e=U_ZERO_ERROR;
    calendar_->setTime(v*1000.0,e);
    check_and_throw(e);
}

bool date_time::operator==(date_time const &other) const
{
    UErrorCode e=U_ZERO_ERROR;
    bool v=calendar_->equals(*CALENDAR(&other),e);
    check_and_throw(e);
    return v;
}

bool date_time::operator!=(date_time const &other) const
{
    return !(*this==other);
}

bool date_time::operator<(date_time const &other) const
{
    UErrorCode e=U_ZERO_ERROR;
    bool v=calendar_->before(*CALENDAR(&other),e);
    check_and_throw(e);
    return v;
}

bool date_time::operator>=(date_time const &other) const
{
    return !(*this<other);
}

bool date_time::operator>(date_time const &other) const
{
    UErrorCode e=U_ZERO_ERROR;
    bool v=calendar_->after(*CALENDAR(&other),e);
    check_and_throw(e);
    return v;
}

bool date_time::operator<=(date_time const &other) const
{
    return !(*this>other);
}

void date_time::swap(date_time &other)
{
    void *tmp=impl_;
    impl_=other.impl_;
    other_.impl_=tmp;
}

int date_time::difference(date_time const &other,field_type f) const
{
    UErrorCode e=U_ZERO_ERROR;
    int d = calendar_->fieldDifference(to_icu(f),1000.0*other.time(),e);
    check_and_throw(e);
    return d;
}

int date_time::maximum(field_type f) const
{
    UErrorCode e=U_ZERO_ERROR;
    int v = calendar_->getActualMaximum(to_icu(f),e);
    check_and_throw(e);
    return v;
}

int date_time::minimum(field_type f) const
{
    UErrorCode e=U_ZERO_ERROR;
    int v = calendar_->getActualMinimum(to_icu(f),e);
    check_and_throw(e);
    return v;
}


} // date_time
} // locale
} // boost



// vim: tabstop=4 expandtab shiftwidth=4 softtabstop=4

