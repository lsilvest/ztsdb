// This file lifts out of localtime.c the main structures that we need in
// headers so that we can use them in our implementation of time zone
// handling. The main difference with the original code is the use of
// 'chrono::time_point' as the time type. Note the C++-style comments
// are non-original comments.

// This appears in localtime.c and applies to the whole of this file
/*
** This file is in the public domain, so clarified as of
** 1996-06-05 by Arthur David Olson.
*/

/*
** Leap second handling from Bradley White.
** POSIX-style TZ environment variable handling from Guy Harris.
*/



#ifndef TZ_HPP
#define TZ_HPP


#include "tzfile.h"

namespace tz {

  struct ttinfo {                   /* time type information */
    int_fast32_t    tt_gmtoff;      /* UT offset in seconds */
    bool            tt_isdst;       /* used to set tm_isdst */
    int             tt_abbrind;     /* abbreviation list index */
    bool            tt_ttisstd;     /* transition is std time */
    bool            tt_ttisgmt;     /* transition is UT */
  };

  struct lsinfo {                   /* leap second information */
    time_t          ls_trans;       /* transition time */
    int_fast64_t    ls_corr;        /* correction to apply */
  };

#define SMALLEST(a, b)  (((a) < (b)) ? (a) : (b))
#define BIGGEST(a, b)   (((a) > (b)) ? (a) : (b))

#ifdef TZNAME_MAX
#define MY_TZNAME_MAX   TZNAME_MAX
#endif /* defined TZNAME_MAX */
#ifndef TZNAME_MAX
#define MY_TZNAME_MAX   255
#endif /* !defined TZNAME_MAX */

  const char      gmt[] = "GMT";

  struct state {
    int           leapcnt;
    int           timecnt;      /* nb of elts in array of transitions 'ats' and 'ttis' */
    int           typecnt;      /* number of elts in 'ttis' */
    int           charcnt;
    bool          goback;
    bool          goahead;
    time_t        ats[TZ_MAX_TIMES];  /* array of transitions */
    unsigned char types[TZ_MAX_TIMES];/* array pointing to ttis, same length as 'ats' */
    ttinfo        ttis[TZ_MAX_TYPES];
    char          chars[BIGGEST(BIGGEST(TZ_MAX_CHARS + 1, sizeof gmt), (2 * (MY_TZNAME_MAX + 1)))];
    lsinfo        lsis[TZ_MAX_LEAPS];
    int           defaulttype; /* for early times or if no transitions */
  };

  extern "C" int tzload(char const *name, state *sp, bool doextend);

}

#endif
