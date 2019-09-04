#include "../include/MT4ServerAPI.h"

#define BUILD_DATE_YEAR_CH0 (__DATE__[ 7])
#define BUILD_DATE_YEAR_CH1 (__DATE__[ 8])
#define BUILD_DATE_YEAR_CH2 (__DATE__[ 9])
#define BUILD_DATE_YEAR_CH3 (__DATE__[10])

#define BUILD_DATE_MONTH_IS_JAN (__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define BUILD_DATE_MONTH_IS_FEB (__DATE__[0] == 'F')
#define BUILD_DATE_MONTH_IS_MAR (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define BUILD_DATE_MONTH_IS_APR (__DATE__[0] == 'A' && __DATE__[1] == 'p')
#define BUILD_DATE_MONTH_IS_MAY (__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define BUILD_DATE_MONTH_IS_JUN (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define BUILD_DATE_MONTH_IS_JUL (__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define BUILD_DATE_MONTH_IS_AUG (__DATE__[0] == 'A' && __DATE__[1] == 'u')
#define BUILD_DATE_MONTH_IS_SEP (__DATE__[0] == 'S')
#define BUILD_DATE_MONTH_IS_OCT (__DATE__[0] == 'O')
#define BUILD_DATE_MONTH_IS_NOV (__DATE__[0] == 'N')
#define BUILD_DATE_MONTH_IS_DEC (__DATE__[0] == 'D')


#define BUILD_DATE_MONTH_CH0 \
((BUILD_DATE_MONTH_IS_OCT || BUILD_DATE_MONTH_IS_NOV || BUILD_DATE_MONTH_IS_DEC) ? '1' : '0')

#define BUILD_DATE_MONTH_CH1 \
( \
(BUILD_DATE_MONTH_IS_JAN) ? '1' : \
(BUILD_DATE_MONTH_IS_FEB) ? '2' : \
(BUILD_DATE_MONTH_IS_MAR) ? '3' : \
(BUILD_DATE_MONTH_IS_APR) ? '4' : \
(BUILD_DATE_MONTH_IS_MAY) ? '5' : \
(BUILD_DATE_MONTH_IS_JUN) ? '6' : \
(BUILD_DATE_MONTH_IS_JUL) ? '7' : \
(BUILD_DATE_MONTH_IS_AUG) ? '8' : \
(BUILD_DATE_MONTH_IS_SEP) ? '9' : \
(BUILD_DATE_MONTH_IS_OCT) ? '0' : \
(BUILD_DATE_MONTH_IS_NOV) ? '1' : \
(BUILD_DATE_MONTH_IS_DEC) ? '2' : \
/* error default */    '?' \
)

#define BUILD_DATE_DAY_CH0 ((__DATE__[4] >= '0') ? (__DATE__[4]) : '0')
#define BUILD_DATE_DAY_CH1 (__DATE__[ 5])

#define BUILD_TIME_HOUR_CH0 (__TIME__[0])
#define BUILD_TIME_HOUR_CH1 (__TIME__[1])

#define BUILD_TIME_MIN_CH0 (__TIME__[3])
#define BUILD_TIME_MIN_CH1 (__TIME__[4])

#define BUILD_TIME_SEC_CH0 (__TIME__[6])
#define BUILD_TIME_SEC_CH1 (__TIME__[7])

const char _PLUGIN_VERSION_STRING[] = { '{', 'B', 'u', 'i', 'l', 'd', ':', ' ',
          BUILD_DATE_YEAR_CH0,
          BUILD_DATE_YEAR_CH1,
          BUILD_DATE_YEAR_CH2,
          BUILD_DATE_YEAR_CH3,
          '-',
          BUILD_DATE_MONTH_CH0,
          BUILD_DATE_MONTH_CH1,
          '-',
          BUILD_DATE_DAY_CH0,
          BUILD_DATE_DAY_CH1,
          ' ',
          BUILD_TIME_HOUR_CH0,
          BUILD_TIME_HOUR_CH1,
          ':',
          BUILD_TIME_MIN_CH0,
          BUILD_TIME_MIN_CH1,
          ':',
          BUILD_TIME_SEC_CH0,
          BUILD_TIME_SEC_CH1,
          '}','\0' };

const char *PLUGIN_VERSION_STRING = _PLUGIN_VERSION_STRING;