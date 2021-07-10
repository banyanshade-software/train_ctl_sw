//
//  StringExtension.h
//  CocoaMoney
//
//  Created by Daniel Braun on 29/09/06.
//  Copyright 2006 Daniel Braun http://braun.daniel.free.fr. All rights reserved.
//

/*!
    @header StringExtension
    @abstract Various extensions to NSString/NSMutableString
 @discussion  mostly provides some parsing methods
*/


#if  TARGET_OS_IPHONE
#import <UIKit/UIKit.h>
#else
#import <Cocoa/Cocoa.h>
#endif

/*!
    @category
    @abstract    (brief description)
    @discussion  (comprehensive description)
*/

@interface NSString (StringExtension)

- (NSString *) lastChars:(NSUInteger)cnt;

/*!
    @method     
    @abstract   extract string beween s1 and s2
    @discussion s1 and s2 will be included in result string if inc is YES
*/

- (NSString *) stringBetween:(NSString *)s1 and:(NSString *)s2 inclusive:(BOOL)inc;

#pragma mark - extract numbers

/*!
    @method     
    @abstract   smart extraction a double from string.
    @discussion see doubleSmartValueFound:
 */
- (double) doubleSmartValue;

/*!
    @method     
    @abstract   smart extraction a double from string.
    @discussion can handle space/comma separtating 1000's, dot/coma decima point, etc.. If pf is not nil,
 it will be set to YES if a number was found, to NO otherwise. Returns value as a double.
*/
- (double) doubleSmartValueFound:(BOOL *)pf;

- (NSDecimalNumber *) decimalSmartValue;
- (NSDecimalNumber *) decimalSmartValueStopAtSpace;

	/*!
	@method     
	 @abstract   smart extraction a double from string.
	 @discussion see doubleSmartValueFound: numberSmartValue returns value as an NSNumber
	 */
- (NSNumber *) numberSmartValue;



/*!
 @method
 @abstract   true if string starts with number
 @discussion ...
 */

- (BOOL) startsWithNumber;


/*!
    @method     
    @abstract   return substrig up to latest non numeric digit
    @discussion ...
*/
- (NSString *) nonNumPrefix;

/*!
    @method     
    @abstract  return substring after latest digit
    @discussion ...
*/
- (NSString *) nonNumPostfix;

/*!
    @method     
    @abstract   returns range of last number in string, if any
    @discussion handles integer only.<br>returns a range with Length=0 if no numeric substring
    <br><b>CAUTION</b> currently minus prefix not handeld in lastNumberRange
    while beeing handled in firstNumberRange
 */

- (NSRange) lastNumberRange;

/*!
    @method     
    @abstract   returns range of last number in string, if any
    @discussion see lastNumberRange
*/

- (NSRange) firstNumberRange;

/*!
 @method
 @abstract   check for longest substring without number
 @discussion ...
 */

- (NSRange) longestSubstringWithoutNumber;

- (NSRange) longestSubstringWithoutCharSet:(NSCharacterSet *)numset;


/*!
    @method
    @abstract   returns number of digit (0-9) in string
    @discussion ...
 */

- (NSUInteger) numberOfDigits;

/*!
    @method     
    @abstract   check for possible date substring, returns position as an NSRange, and converted substring
    @discussion  looks for dd/mmm/yyyy (the '/' can be any punctuation char, but must
    bet the same in substring, day and month can be 1 or 2 digits, year is optionnal and can
    be 2 or 4 digits. Should handle localisation for day/month ordrer, but does NOT currently)
*/

#pragma mark - extract dates

#if ! TARGET_OS_IPHONE

- (NSDate *) possibleDate:(NSRange *)rg;

/*!
    @method     
    @abstract   approximative words matching
    @discussion compute a Levenshtein  distance http://en.wikipedia.org/wiki/Levenshtein_distance
and returns a score based on this distance : 1.0 for identical words, 0.0 for very differents words

*/
#endif /* TARGET_OS_IPHONE */

- (BOOL) possibleDay:(int *)pday month:(int *)pmonth range:(NSRange *)rg;

#pragma mark - extract strings

/*!
 @method
 @abstract   Levenshtein distance of 2 strings
 @discussion ...
 */

- (float) nearString:(NSString *)s;


/*!
    @method     
    @abstract   check for substring
    @discussion conveniant wrapper arround rangeOfString:
*/
- (BOOL) hasSubString:(NSString *)s;

/*!
    @method     
    @abstract   check for a substring among an array of possible substring
    @discussion returns YES if one of the substring matched. In this case, it will fill outRange, range
    of matching substring, and *pi, index in substrings.
    Will always returns longuest matching statring at the lowest chr of self
    e.g. in "foofoobar", when looking for fo, foof, oobar : will returns matching for foof
*/

- (BOOL) findOneSubstring:(NSArray *)substrings range:(NSRange *)outRange index:(int *)pi;


/*!
    @method     
    @abstract  check string is a valid identifier
    @discussion check string contains only A-Z, a-z or _ characters
*/

- (BOOL) isIdentifier;

/*!
    @method     
    @abstract   convert posix path to AppleScript path
    @discussion
*/

//- (NSString *) convertToASPath;

/*!
    @method     
    @abstract  search and replace
    @discussion ...
 */

- (NSString *) stringByReplacing:(NSString *)s by:(NSString *)s2;

/*!
    @method     
    @abstract   parse a file, looking for a set of patterns, either replacing it or invoking callbacks
    @discussion  act and/or sr are dictionnary, using the string pattern as key, and either (for sr) a 
    string to replace the pattern, or (in act) the name of a method. The method is then
    sent to performer object.<br>
    All patterns are to be surrounded by dollars signs in string ("$...$"), and may have 
    some parameters eg: $foo{a,b}$<br>
    This is intendeed to process templates, not for a general parser
*/

//- (NSString *) parseWithActions:(NSDictionary *)act for:(id)performer
//	   andSimpleReplacement:(NSDictionary *)sr;

#pragma mark - misc

/*!
    @method     
    @abstract   load a file from main bundle ressources, convert it to NSString
    @discussion uses NSISOLatin1StringEncoding
*/

+ (id) stringFromResourceFile:(NSString *)filename;

@end



/*!
    @category
    @abstract    (brief description)
    @discussion  (comprehensive description)
*/


@interface NSMutableString (StringExtension) 

/*!
    @method     
    @abstract   (brief description)
    @discussion (comprehensive description)
*/

- (void) deleteLastCharacters:(NSUInteger)numtodelete;

@end






/*!
    @category
    @abstract    (brief description)
    @discussion  (comprehensive description)
*/

@interface NSString (removeAccents)
/*!
    @method     
    @abstract   (brief description)
    @discussion (comprehensive description)
*/
- (NSString*) lowercaseStringRemovingAccents;

/*!
    @method     
    @abstract   (brief description)
    @discussion (comprehensive description)
*/
- (NSString*) lowercaseStringRemovingAccentsAndPunct; 
@end


@interface NSString (CommonString)

/*
 * e.g. between "apple juice and bananas" ans "banana juice"
 *  -> "juice", "banana"
 *
 */

- (NSArray<NSString *> *) commonStringsWith:(NSString *)otherString minLength:(int)minl;
- (NSArray<NSString *> *) commonStringsWith:(NSString *)otherString; // minl=3
- (NSArray<NSString *> *) allCommonStringsWith:(NSString *)otherString minLength:(int)minl;
- (NSArray<NSString *> *) allCommonStringsWith:(NSString *)otherString; //minl=3

//+ (NSString *)longestCommonSubstring:(NSString *)substring string:(NSString *)string;
- (NSArray<NSString *> *) longestCommonStringsWith:(NSString *)otherString;

@end

