//
//  NSCalendar+gmt.m
//  ZenAccounting http://www.zenaccounting-app.com
//
//  Created by Daniel BRAUN on 19/10/2014.
//  Copyright (c) 2014 Daniel BRAUN. All rights reserved.
//

#import "NSCalendar+Gmt.h"

@implementation NSCalendar (Gmt)

static NSTimeZone *gmtTimeZone = nil;

+ (instancetype) gmtCalendar
{
    static NSCalendar *gmt = nil;
    if (gmt) return gmt;
    
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        gmtTimeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
        NSCalendar *calendar = [[NSCalendar alloc]
                                initWithCalendarIdentifier:NSCalendarIdentifierGregorian];
        [calendar setTimeZone:gmtTimeZone];
        gmt = calendar;
    });
    return gmt;
}

@end


@implementation NSDateFormatter (GmtFormat)

+ (NSDateFormatter *) _gmtFormatter
{
    NSCalendar *gmt = [NSCalendar gmtCalendar];
    NSDateFormatter *df = [[NSDateFormatter alloc]init];
    [df setFormatterBehavior:NSDateFormatterBehavior10_4];
    [df setLenient:NO];
    [df setCalendar:gmt];
    [df setTimeZone:[NSTimeZone timeZoneForSecondsFromGMT:0]];
    return df;
}
+ (NSDateFormatter *) gmtDayFormatter // dd-mm-YYYY
{
    static NSDateFormatter *df = nil;
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        df = [self _gmtFormatter];
        [df setDateFormat:@"dd/MM/yyyy"];
    });
    return df;
}
+ (NSDateFormatter *) gmtDayYFormatter // yyyy-MM-dd
{
    static NSDateFormatter *df = nil;
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        df = [self _gmtFormatter];
        [df setDateFormat:@"yyyy-MM-dd"];
    });
    return df;
}
+ (NSDateFormatter *) gmtDayYfFormatter // dd/MM/yyyy
{
    static NSDateFormatter *df = nil;
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        df = [self _gmtFormatter];
        [df setDateFormat:@"dd/MM/yyyy"];
    });
    return df;
}

+ (NSDateFormatter *) gmtDayYfHFormatter // dd/MM/yyyy HH:MM
{
    static NSDateFormatter *df = nil;
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        df = [self _gmtFormatter];
        [df setDateFormat:@"dd/MM/yyyy HH:mm"];
    });
    return df;
}



+ (NSDateFormatter *) gmtLocalizedDayFormatter
{
    static NSDateFormatter *df = nil;
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        NSUserDefaults *standardDefaults = [NSUserDefaults standardUserDefaults];
        BOOL dh = [[standardDefaults valueForKey:@"dispHour"]boolValue];
        df = [self _gmtFormatter];
        [df setDateStyle:NSDateFormatterShortStyle];
        if (!dh) [df setTimeStyle:NSDateFormatterNoStyle];
        else     [df setTimeStyle:NSDateFormatterMediumStyle]; //for p97
    });
    return df;
}

+ (NSDateFormatter *) gmtLocalizedMonthFormatter
{
    static NSDateFormatter *df = nil;
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        df = [self _gmtFormatter];
        [df setDateFormat:@"MMM yyyy"];
    });
    // TODO: check it's indeed localized
    return df;
}

+ (NSDateFormatter *) gmtLocalizedYearFormatter
{
    static NSDateFormatter *df = nil;
    static dispatch_once_t onceToken = (dispatch_once_t)0;
    dispatch_once(&onceToken, ^{
        df = [self _gmtFormatter];
        [df setDateFormat:@"yyyy"];
    });
    // TODO: check it's indeed localized
    return df;
}


@end

@implementation GmtDayFormatter

- (instancetype)init
{
    NSAssert(0, @"unused");
    id k = [super init]; // remove warning
    if (!k) return nil;
    id s = [NSDateFormatter gmtLocalizedDayFormatter];
    return s;
}

- (instancetype) initWithCoder:(NSCoder *)aDecoder
{
    id unused = [super initWithCoder:aDecoder]; // would crash if we dont call
    if (unused) unused=nil;                     // just for the warning
    id s = [NSDateFormatter gmtLocalizedDayFormatter];
    return s;
}

- (instancetype) initWithDateFormat:(NSString *)format allowNaturalLanguage:(BOOL)flag
{
    NSAssert(0, @"unused");
    return nil;
}
@end

@implementation NSDate (GmtFormat)

- (NSString *) gmtLocalizedDay
{
    NSDateFormatter *df = [NSDateFormatter gmtLocalizedDayFormatter];
    return [df stringFromDate:self];
}
- (NSString *) gmtDay
{
    NSDateFormatter *df = [NSDateFormatter gmtDayFormatter];
    return [df stringFromDate:self];
}
- (NSString *) gmtYDay
{
    NSDateFormatter *df = [NSDateFormatter gmtDayYFormatter];
    return [df stringFromDate:self];
}

@end

#pragma mark -


@implementation NSDate (MiscGmt)

//http://stackoverflow.com/questions/10717574/get-firstdate-lastdate-of-month

- (NSDate *) startOfMonth
{
    return [self startOfMonthWithCalendar:[NSCalendar gmtCalendar]];
}
- (NSDate *) startOfMonthWithCalendar:(NSCalendar *)calendar
{
    NSDateComponents * currentDateComponents = [calendar components: NSCalendarUnitYear | NSCalendarUnitMonth fromDate: self];
    currentDateComponents.day = 1;
    NSDate * startOfMonth = [calendar dateFromComponents: currentDateComponents];
    
    return startOfMonth;
}

- (NSDate *) startOfYear
{
    return [self startOfYearWithCalendar:[NSCalendar gmtCalendar]];
}

- (NSDate *) startOfYearWithCalendar:(NSCalendar *)calendar
{
    NSDateComponents * currentDateComponents = [calendar components: NSCalendarUnitYear fromDate: self];
    currentDateComponents.month = 1;
    currentDateComponents.day = 1;
    NSDate * startOfYear = [calendar dateFromComponents: currentDateComponents];
    
    return startOfYear;
}


- (NSDate *) dateByAddingMonths: (NSInteger) monthsToAdd calendar:(NSCalendar *)calendar
{
    NSDateComponents * months = [[NSDateComponents alloc] init];
    [months setMonth: monthsToAdd];
    
    return [calendar dateByAddingComponents: months toDate: self options: 0];
}

- (NSDate *) endOfMonth
{
    return [self endOfMonthWithCalendar:[NSCalendar gmtCalendar]];
}

- (NSDate *) endOfMonthWithCalendar:(NSCalendar *)calendar
{
    NSDate * plusOneMonthDate = [self dateByAddingMonths: 1 calendar:calendar];
    NSDateComponents * plusOneMonthDateComponents = [calendar components: NSCalendarUnitYear | NSCalendarUnitMonth fromDate: plusOneMonthDate];
    NSDate * endOfMonth = [[calendar dateFromComponents: plusOneMonthDateComponents] dateByAddingTimeInterval: -1]; // One second before the start of next month
    
    return endOfMonth;
}

- (NSDate *) dateByAddingYears: (NSInteger) years  calendar:(NSCalendar *)calendar
{
    NSDateComponents * yc = [[NSDateComponents alloc] init];
    [yc setYear:years];
    
    return [calendar dateByAddingComponents:yc toDate: self options: 0];
}

- (NSDate *) endOfYear
{
    return [self endOfYearWithCalendar:[NSCalendar gmtCalendar]];
}
- (NSDate *) endOfYearWithCalendar:(NSCalendar *)calendar
{
    NSDate * plusOneYearDate = [self dateByAddingYears: 1 calendar:calendar];
    NSDateComponents * plusOneMonthDateComponents = [calendar components: NSCalendarUnitYear fromDate: plusOneYearDate];
    NSDate * endOfYear = [[calendar dateFromComponents: plusOneMonthDateComponents] dateByAddingTimeInterval: -1]; // One second before the start of next month
    
    return endOfYear;
}

- (NSDate *) dateByAddingDays: (NSInteger) daysToAdd calendar:(NSCalendar *)calendar
{
    NSDateComponents * days = [[NSDateComponents alloc] init];
    [days setDay:daysToAdd];
    
    return [calendar dateByAddingComponents: days toDate: self options: 0];
}



- (NSDate *) startOfDay
{
    return [self startOfDayWithCalendar:[NSCalendar gmtCalendar]];
}
- (NSDate *) startOfDayWithCalendar:(NSCalendar *)calendar
{
    NSDateComponents * currentDateComponents = [calendar components: NSCalendarUnitYear | NSCalendarUnitMonth |NSCalendarUnitDay fromDate: self];
    NSDate * startOfDay = [calendar dateFromComponents: currentDateComponents];
    
    return startOfDay;
}

- (NSDate *) endOfDay
{
    return [self endOfDayWithCalendar:[NSCalendar gmtCalendar]];
}
- (NSDate *) endOfDayWithCalendar:(NSCalendar *)calendar
{
    NSDate * plusOneDayDate = [self dateByAddingDays: 1 calendar:calendar];
    NSDateComponents * plusOneDayDateComponents = [calendar components: NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay fromDate: plusOneDayDate];
    NSDate * endOfDay = [[calendar dateFromComponents: plusOneDayDateComponents] dateByAddingTimeInterval: -1]; // One second before the start of next month
    
    return endOfDay;
    
}

- (NSDate *) middleOfDayWithCalendar:(NSCalendar *)calendar
{
    NSDateComponents * currentDateComponents = [calendar components: NSCalendarUnitYear | NSCalendarUnitMonth |NSCalendarUnitDay | NSCalendarUnitHour fromDate: self];
    [currentDateComponents setHour:12];
    NSDate * mod = [calendar dateFromComponents: currentDateComponents];
    
    return mod;
}

+ (NSDate *) currentDayTranslatedToGmt
{
    NSDate *d = [NSDate date];
    NSCalendar *c = [NSCalendar currentCalendar];
    NSCalendar *gmt = [NSCalendar gmtCalendar];
    NSDateComponents * currentDateComponents = [c components: NSCalendarUnitYear | NSCalendarUnitMonth |NSCalendarUnitDay | NSCalendarUnitHour fromDate: d];
    [currentDateComponents setHour:12];
    NSDate * mod = [gmt dateFromComponents: currentDateComponents];
    return mod;
}
- (NSDate *) middleOfDay
{
    return [self middleOfDayWithCalendar:[NSCalendar gmtCalendar]];
}

- (BOOL) isSameDay:(NSDate *)d
{
    return [self isSameDay:d calendar:[NSCalendar gmtCalendar]];
}

- (BOOL) isSameDay:(NSDate *)d calendar:(NSCalendar *)calendar
{
    NSDateComponents * c1 = [calendar components: NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay fromDate:self];
    NSDateComponents * c2 = [calendar components: NSCalendarUnitYear | NSCalendarUnitMonth | NSCalendarUnitDay fromDate:d];
    if (c1.year  != c2.year)  return NO;
    if (c1.month != c2.month) return NO;
    if (c1.day   != c2.day)   return NO;
    // TODO: era is not handled here (nor it is in all other method)
    return YES;
}



@end




@implementation GmtDatePicker

- (instancetype) init
{
    self = [super init];
    if (self) [self setupGmt];
    return self;
}
- (instancetype) initWithCoder:(NSCoder *)c
{
    self = [super initWithCoder:c];
    if (self) [self setupGmt];
    return self;
}
- (void) setupGmt
{
    self.calendar = [NSCalendar gmtCalendar];
    NSAssert(gmtTimeZone, @"gmtTimeZone should be set after calling gmtCalendar");
    self.timeZone = gmtTimeZone;
}
- (NSTimeZone *) timeZone
{
    NSTimeZone *t = [super timeZone];
    return t;
}

@end




@implementation  NSDate (TestFacilities)

// replacement for obsolete dateWithNaturalLanguageString
+ (NSDate *) testDateWithYYYYMMDDString:(NSString *)t
{
    NSDateFormatter *df = [NSDateFormatter gmtDayYFormatter];  // yyyy-MM-dd
    //df.allowsNaturalLanguage
    NSDate *rd = [df dateFromString:t];
    rd = [rd middleOfDay];
    /*
    if ((1)) {
        NSDate *d = [NSDate dateWithNaturalLanguageString:t];
        NSAssert([d isEqual:rd], @"ho");
    }
     */
    return rd;
}
+ (NSDate *) gmtDateWithYear:(int)y month:(int)m day:(int)d hour:(int)hour minute:(int)min second:(int)s
{
    NSCalendar *gmt = [NSCalendar gmtCalendar];
    NSDate *rd = [gmt dateWithEra:1 year:y month:m day:d hour:hour minute:min second:s nanosecond:0];
    /*
    if ((1)) {
        NSTimeZone *gmtTimeZone = [NSTimeZone timeZoneForSecondsFromGMT:0];
        NSDate *r = [NSCalendarDate dateWithYear:y month:m day:d hour:hour minute:min second:s timeZone:gmtTimeZone];
        NSAssert([r isEqual:rd], @"ho");
    }
     */
    return rd;
}

@end
