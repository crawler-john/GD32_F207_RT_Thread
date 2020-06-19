/*****************************************************************************/
/* File      : rtc.h                                                         */
/*****************************************************************************/
/*  History:                                                                 */
/*****************************************************************************/
/*  Date       * Author          * Changes                                   */
/*****************************************************************************/
/*  2017-03-20 * Shengfeng Dong  * Creation of the file                      */
/*             *                 *                                           */
/*****************************************************************************/
#ifndef RTC_H_INCLUDED
#define RTC_H_INCLUDED

/*****************************************************************************/
/*  Function Declarations                                                    */
/*****************************************************************************/
int set_time(const char* time);
int apstime(char* currenttime);
int checktime(char* currenttime);

#endif // RTC_H_INCLUDED
