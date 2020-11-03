//
//  NSCalendar+Gmt.h
//  ZenAccounting http://www.zenaccounting-app.com
//
//  Created by Daniel BRAUN on 19/10/2014.
//  Copyright (c) 2014 Daniel BRAUN. All rights reserved.
//

#import <Cocoa/Cocoa.h>

@interface NSCalendar (Gmt)

+ (instancetype) gmtCalendar;

@end

@interface NSDateFormatter (GmtFormat)
+ (NSDateFormatter *) gmtDayFormatter; // dd-mm-YYYY
+ (NSDateFormatter *) gmtDayYFormatter; // yyyy-MM-dd
+ (NSDateFormatter *) gmtDayYfFormatter; // dd/MM/yyyy
+ (NSDateFormatter *) gmtDayYfHFormatter;
+ (NSDateFormatter *) gmtLocalizedDayFormatter;
+ (NSDateFormatter *) gmtLocalizedMonthFormatter;
+ (NSDateFormatter *) gmtLocalizedYearFormatter;

@end

@interface NSDate (GmtFormat)

- (NSString *) gmtLocalizedDay;
- (NSString *) gmtDay;  // dd-mm-YYYY
- (NSString *) gmtYDay; // yyyy-MM-dd

@end


@interface GmtDayFormatter : NSDateFormatter   // dd-mm-YYYY


@end


@interface NSDate (MiscGmt)

- (NSDate *) startOfMonthWithCalendar:(NSCalendar *)c;
- (NSDate *) startOfMonth;
- (NSDate *) endOfMonthWithCalendar:(NSCalendar *)c;
- (NSDate *) endOfMonth;


- (NSDate *) startOfYearWithCalendar:(NSCalendar *)c;
- (NSDate *) startOfYear;
- (NSDate *) endOfYearWithCalendar:(NSCalendar *)c;
- (NSDate *) endOfYear;


- (NSDate *) startOfDayWithCalendar:(NSCalendar *)c;
- (NSDate *) startOfDay;
- (NSDate *) endOfDayWithCalendar:(NSCalendar *)c;
- (NSDate *) endOfDay;
- (NSDate *) middleOfDayWithCalendar:(NSCalendar *)c;
- (NSDate *) middleOfDay;



- (BOOL) isSameDay:(NSDate *)d calendar:(NSCalendar *)c;;
- (BOOL) isSameDay:(NSDate *)d;

/*
 * return current day 12:00 GMT, for the current day in local calendar
 */

+ (NSDate *) currentDayTranslatedToGmt;


@end


@interface GmtDatePicker : NSDatePicker
@end


@interface NSDate (TestFacilities)

// replacement for obsolete dateWithNaturalLanguageString
+ (NSDate *) testDateWithYYYYMMDDString:(NSString *)t;

//NSTimeZone *gmtTimeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
//r = [NSCalendarDate dateWithYear:2014 month:05 day:12 hour:12 minute:0 second:0 timeZone:gmtTimeZone];
+ (NSDate *) gmtDateWithYear:(int)y month:(int)m day:(int)d hour:(int)hour minute:(int)min second:(int)s;
@end
