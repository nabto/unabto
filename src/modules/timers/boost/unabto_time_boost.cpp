/*
 * Copyright (C) 2008-2013 Nabto - All Rights Reserved.
 */
#include "unabto_time_boost.h"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/thread/thread_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <unabto/unabto_external_environment.h>

bool nabtoIsStampPassed(nabto_stamp_t *stamp)
{
    boost::posix_time::ptime t(boost::gregorian::date(1970,boost::gregorian::Jan,1));
    
    t += boost::posix_time::microseconds(*stamp);
     
    return boost::get_system_time() >= t;
}

/* declared im unabto_external_environment.h */
void setTimeFromGSP(uint32_t stamp)
{
    if (stamp) {}
    // TBD
}

/**
 * Get Current time stamp
 * @return current time stamp
 * nabto_stamp_t nabtoGetStamp();
 */
nabto_stamp_t nabtoGetStamp() {
    boost::posix_time::ptime epoch(boost::gregorian::date(1970,boost::gregorian::Jan,1));
    boost::posix_time::ptime now = boost::get_system_time();
    
    
    return (now-epoch).total_microseconds();
}

int nabtoStampDiff2ms(nabto_stamp_diff_t diff) {
    // we are working with microseconds.
    return (int)(diff / 1000);
}



nabto_stamp_diff_t nabtoStampDiff(nabto_stamp_t* newest, nabto_stamp_t* oldest) {
    return ((*(newest)) - (*(oldest)));
}

