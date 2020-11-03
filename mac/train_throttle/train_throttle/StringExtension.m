//
//  StringExtension.m
//  CocoaMoney
//
//  Created by Daniel Braun on 29/09/06.
//  Copyright 2006 Daniel Braun http://braun.daniel.free.fr. All rights reserved.
//

#import "StringExtension.h"
#import "NSCalendar+Gmt.h"

//#import "objcExt.h"
@implementation NSString (StringExtension)

- (NSString *) lastChars:(NSUInteger)cnt
{
    NSUInteger l = [self length];
    if (l<cnt) cnt=l;
    NSRange r =  NSMakeRange(l-cnt, cnt);
    return [self substringWithRange:r];
}
- (NSString *) stringBetween:(NSString *)stringBefore and:(NSString *)stringAfter inclusive:(BOOL) incl
{
	NSRange r1, r2;
	NSUInteger l=[self length];
	if (!l) return nil;
	if (stringBefore) {
		r1=[self rangeOfString:stringBefore options:NSCaseInsensitiveSearch];
		if (NSNotFound==r1.location) return nil;
		if (!incl) r1.location+=r1.length;
		r1.length=l-r1.location;
	} else {
		r1.location=0;
		r1.length=l;
	}
	if (stringAfter) {
		r2=[self rangeOfString:stringAfter options:NSCaseInsensitiveSearch range:r1];
		if (NSNotFound==r2.location) return nil;
		r1.length=r2.location-r1.location;
		if (incl) r1.length += r2.length;
	}
	NSString *v=[self substringWithRange:r1];
	if (/* DISABLES CODE */ (0)) NSLog(@"extracted substring %@\n", v);
	return v;
}


- (BOOL) normalizeNumberBuffer:(char *)szTmp size:(size_t)sizbuf ignoreSpace:(BOOL)ignsp
{
    BOOL pf;
	char *pdot, *pvirg;
	char thpoint='\0';
	char decimalpoint='\0';
	const char *str=[self UTF8String];
	if (/* DISABLES CODE */ (0)) NSLog(@"convert %s\n", str);
	pdot=strchr(str, '.');
	pvirg=strchr(str, ',');
	const char *p;
	const char *ldigit=NULL;
	const char *fdigit=NULL;
	pf=NO;
	for (p=str; *p; p++) {
		if (isdigit(*p)) {
			pf=YES;
			ldigit=p;
			if (!fdigit) fdigit=p;
        } else if (pf && !ignsp) {
            if ((*p != '.') && (*p != ',')) {
                break;
            }
        }
	}
	if (/* DISABLES CODE */ (0)) NSLog(@"convert %s ldigit=%p fdigit=%p\n", str, ldigit, fdigit);
	if (!ldigit) return pf;
	const char *tfdigit=fdigit;
	while (tfdigit>=str) {
		if (' '==tfdigit[-1]) {
			tfdigit--;
			continue;
		}
		if ('-'==tfdigit[-1]) {
			fdigit=tfdigit-1;
		}
		break;
	}
	if (pdot>ldigit) pdot=0;
	if (pvirg>ldigit) pvirg=0;
	if (pdot>pvirg) {
		decimalpoint='.';
		thpoint=',';
	} else {
		decimalpoint=',';
		thpoint='.';
	}
	if (ldigit-fdigit>=(int)sizbuf-1) {
		return NO;
	}
	int i;
	for (i=0, p=fdigit; p<=ldigit; p++) {
		if (' '==*p) {
		} else if (thpoint==*p) {
		} else if (isdigit(*p) || (('-'==*p) && !i)) {
			szTmp[i++]=*p;
		} else if (' '==*p) {
		} else if (decimalpoint==*p) {
			szTmp[i++]='.';
		} else break;
	}
	szTmp[i]='\0';
	if (/* DISABLES CODE */ (0)) NSLog(@"converted %s to %s\n", str, szTmp);
	return pf;
}

- (double) doubleSmartValueFound:(BOOL *)pf
{
    NSAssert(pf, @"pf should not be nil");
    char szTmp[1024];
    *pf = [self normalizeNumberBuffer:szTmp size:sizeof(szTmp) ignoreSpace:YES];
    if (*pf) {
        return atof(szTmp);
    }
    return 0.0;
}

- (NSDecimalNumber *) decimalSmartValue
{
    char szTmp[1024];
    BOOL pf = [self normalizeNumberBuffer:szTmp size:sizeof(szTmp) ignoreSpace:YES];
    if (!pf) return nil;
    NSString *s = [NSString stringWithUTF8String:szTmp];
    return [NSDecimalNumber decimalNumberWithString:s];
}

- (NSDecimalNumber *) decimalSmartValueStopAtSpace
{
    char szTmp[1024];
    BOOL pf = [self normalizeNumberBuffer:szTmp size:sizeof(szTmp) ignoreSpace:NO];
    if (!pf) return nil;
    NSString *s = [NSString stringWithUTF8String:szTmp];
    return [NSDecimalNumber decimalNumberWithString:s];
}

- (double) doubleSmartValue
{
	return [self doubleSmartValueFound:NULL];
}

- (NSNumber *) numberSmartValue
{
	BOOL f=NO;
	double v;
	v=[self doubleSmartValueFound:&f];
	if (f) return [NSNumber numberWithDouble:v];
	return nil;
}

- (NSString *) nonNumPrefix
{
	const char *str=[self UTF8String];
	BOOL f=NO;
	NSUInteger i;
	for (i=0; str[i]; i++) {
		if (isdigit(str[i])) {
			f=YES;
			break;
		}
	}
	if (!f) return self;
	if (i>1) {
		if ((str[i-1]==' ') && (str[i-2]=='-')) i-=2;
		else if (str[i-1]=='-') i-=1;
	} else if (i>0) {
		if (str[i-1]=='-') i-=1;
	}
	NSRange r;
	r.location=0;
	r.length=i;
	if (!i) return nil;
	return [self substringWithRange:r];
}


- (NSRange) lastNumberRange
{
	NSRange r;
	r.location=0;
	r.length=0;
	const char *str=[self UTF8String];
	NSUInteger l=[self length];
	if (!l) return r;
	NSUInteger p1=0;
	NSUInteger p2=0;
	NSInteger i;
	for (i=(int)l-1; i>=0; i--) {
		if (isdigit(str[i])) {
			p2=(NSUInteger)i;
			break;
		}
	}
	for (i=(NSInteger)(p2-1); i>=0; i--) {
		if (!isdigit(str[i])) {
			p1=(NSUInteger)(i+1);
			break;
		}
	}
	r.location=p1;
	r.length=p2-p1+1;
	return r;
}

- (NSUInteger) numberOfDigits
{
    const char *str=[self UTF8String];
    const char *p;
    NSUInteger n=0;
    for (p=str; *p; p++) {
        if ((*p >= '0') && (*p <= '9')) {
            n++;
        }
    }
    return n;
}

- (BOOL) startsWithNumber
{
	if (![self length]) return NO;
	NSRange r=[self firstNumberRange];
	if (!r.length) return NO;
	if (r.location==0) return YES;
	return NO;
}
- (NSRange) firstNumberRange
{
	NSRange r;
	r.location=0;
	r.length=0;
	const char *str=[self cStringUsingEncoding:NSISOLatin1StringEncoding];
	NSUInteger l=[self length];
	if (!str || !l) {
		return r;
	}
	NSUInteger p1;
	NSUInteger p2=0;
	NSUInteger i;
	for (i=0; i<l; i++) {
		if (isdigit(str[i])) {
			p2=i;
			break;
		}
	}
	p1=p2;
	for (i=p2+1; i<l; i++) {
		if (!isdigit(str[i])) {
			p1=i;
			break;
		}
	}
	if ((i==l) &&(p1!=p2)) p1=l;
	r.location=p2;
	r.length=p1-p2;
	return r;
}

- (NSRange) longestSubstringWithoutNumber
{
    NSCharacterSet *numset = [NSCharacterSet characterSetWithCharactersInString:@"0123456789"];
    return [self longestSubstringWithoutCharSet:numset];
}

- (NSRange) longestSubstringWithoutCharSet:(NSCharacterSet *)numset
{
    // TODO: suboptimal
    NSRange r;
    r.location = 0;
    r.length = [self length];
    NSArray *a = [self componentsSeparatedByCharactersInSet:numset];
    a = [a sortedArrayUsingComparator:^(NSString *s1, NSString *s2) {
        NSUInteger l1 = [s1 length];
        NSUInteger l2 = [s2 length];
        if (l1>l2) return NSOrderedAscending;
        if (l1==l2) return NSOrderedSame;
        return NSOrderedDescending;
    }];
    NSString *sl = [a firstObject];
    NSRange re = [self rangeOfString:sl];
    
    return re;
}

- (NSString *) nonNumPostfix;
{
	const char *str=[self UTF8String];
	NSUInteger l=[self length];
	BOOL f=NO;
	NSInteger i;
	for (i=(NSInteger)l-1; i>=0; i--) {
		if (isdigit(str[i])) {
			f=YES;
			break;
		}
	}
	if (!f) return self;
	if (i==(NSInteger)l-1) return @"";
	NSRange r;
	r.location=(NSUInteger)i+1;
	r.length=(NSUInteger)(l-i-1);
	return [self substringWithRange:r];
}

- (float) nearString:(NSString *)s
{
	/* Levenshtein distance
	 * http://en.wikipedia.org/wiki/Levenshtein_distance
	 */
	NSUInteger n1 = [self length];
	NSUInteger n2 = [s length];
	if (!n1) return 0.0f;
	if (!n2) return 0.0f;
	const char *s1=[self cStringUsingEncoding:NSUTF8StringEncoding];
	const char *s2=[s cStringUsingEncoding:NSUTF8StringEncoding];
	const unsigned int cost_del = 1;
	const unsigned int cost_ins = 1;
	const unsigned int cost_sub = 1;
	
	unsigned int *p = (unsigned int *)calloc(sizeof(unsigned int), n2+1);
	unsigned int* q = (unsigned int *) calloc(sizeof(unsigned int), n2+1);
	unsigned int* r;
		
	p[0] = 0;
	unsigned int i,j;
	for (j=1; j<=n2; j++) {
		p[j] = p[j-1] + cost_ins;
	}
	for(i = 1; i <= n1; ++i ) {
		q[0] = p[0] + cost_del;
		for(j=1; j <= n2; ++j ) {
			unsigned int d_del = p[j] + cost_del;
			unsigned int d_ins = q[j-1] + cost_ins;
			unsigned int d_sub = p[j-1] + ( s1[i-1] == s2[j-1] ? 0 : cost_sub );
            unsigned int m1=MIN( d_del, d_ins );
			q[j] = MIN(m1, d_sub );
		}
		r = p;
		p = q;
		q = r;
	}
		
	unsigned int tmp = p[n2];
	free(p);
	free(q);
	// tmp is Levenshtein distance
 	// transform to a matching result 1.0=identical string, 0.0=very different
	int l=(int) MAX(n1, n2);
	float res=1.0f-tmp/(1.0f*l);
	return res;
}

- (BOOL) hasSubString:(NSString *)s
{
	NSRange r;
	r=[self rangeOfString:s];
	if (0==r.length) return NO;
	return YES;
}
- (BOOL) findOneSubstring:(NSArray *)substrings range:(NSRange *)outRange index:(int *)pi
{
	NSRange br={0,0};
	BOOL haveOne=NO;
	NSUInteger i, count = [substrings count];
	for (i = 0; i < count; i++) {
		NSString * sub = [substrings objectAtIndex:i];
		NSString *sb=[sub lowercaseStringRemovingAccentsAndPunct];
		//NSLog(@"testing wit <%@>\n", sb);
		NSRange r=[self rangeOfString:sb];
		if (!r.length) continue;
		if ((!haveOne) ||
		    (r.location<br.location) ||
		    ((r.location==br.location)&&(r.length>br.length)) ) {
			haveOne=YES;
			br=r;
			if (pi) *pi= (int) i;
			continue;
		}
	}
	if (haveOne && outRange) *outRange=br;
	if (haveOne && pi) {
		//NSLog(@"found idx %d (%@) for %@\n",*pi, [substrings objectAtIndex:*pi], self);
	} else if (!haveOne) {
		//NSLog(@"no found for %@\n", self);
	}
	return haveOne;
}

- (BOOL) isIdentifier
{
	const char *s=[self UTF8String];
	if ((*s>='A') && (*s<='Z')) return YES;
	if ((*s>='a') && (*s<='z')) return YES;
	if (*s=='_') return YES;
	return NO;
}

- (NSString *) convertToASPath
{
	NSString *mp;
	mp=[self stringByStandardizingPath];
	NSArray *pc=[mp pathComponents];
	NSRange r;
	r.location=1;
	r.length=[pc count]-1;
	pc=[pc subarrayWithRange:r];
	mp=[pc componentsJoinedByString:@":"];
	return mp;
}

- (NSString *) stringByReplacing:(NSString *)s by:(NSString *)s2
{
    if (!s2) return s;
	NSMutableString *r;
	r=[self mutableCopy];
	NSRange rg;
	rg.location=0;
	rg.length=[self length];
	[r replaceOccurrencesOfString:s withString:s2 options:0 range:rg];
	return r;
}



#if 0
- (NSString *) parseWithActions:(NSDictionary *)act for:(id)performer
	   andSimpleReplacement:(NSDictionary *)sr
{
	NSString *r;
	NSString *e;
	unsigned int par=0;
	r=@"";
	NSArray *ar=[self componentsSeparatedByString:@"$"];
	//printf("got %d comp\n", [ar count]);
	unsigned int i, count = [ar count];
	for (i = 0; i < count; i++) {
		e = [ar objectAtIndex:i];
		if ((i%2)==par) {
			r=[r stringByAppendingString:e];
			continue;
		}
		NSString *keyword;
		NSString *paramlist=nil;
		NSMutableDictionary *params=[NSMutableDictionary dictionaryWithCapacity:5];
		NSArray *p1;
		p1=[e componentsSeparatedByString:@"{"];
		keyword=[p1 objectAtIndex:0];
		if ([p1 count]>1) {
			paramlist=[p1 objectAtIndex:1];
			p1=[paramlist componentsSeparatedByString:@"}"];
			paramlist=[p1 objectAtIndex:0];
			p1=[paramlist componentsSeparatedByString:@","];
			unsigned int j, np = [p1 count];
			for (j = 0; j < np; j++) {
				NSString * ps = [p1 objectAtIndex:j];
				NSArray *p2=[ps componentsSeparatedByString:@"="];
				NSString *v;
				if ([p2 count]>1) v=[p2 objectAtIndex:1];
				else v=@"yes";
				[params setObject:v forKey:[p2 objectAtIndex:0]];
			}
		} 
		NSString *rep=nil;

		if ([keyword isEqualToString:@"copyright"]) {
		} else {
			NSString *va=[act objectForKey:keyword];
			if (va) {
				SEL s=selFromString(va);
				if (s) {
					rep=[performer performSelector:s withObject:params];
				}
			}
			if (!rep) {
				rep=[sr objectForKey:keyword];
			}
		}
		if (rep) {
			if (![rep isKindOfClass:[NSString class]]) rep=[rep description];
			r=[r stringByAppendingString:rep];
		}
	}
	return r;
}
#endif
+ (id) stringFromResourceFile:(NSString *)filename
{
	NSBundle *mb=[NSBundle mainBundle];
	NSString *p=[mb pathForResource:filename ofType:nil];
	if (!p) return nil;
	NSError *err=nil;
	NSString *s=[NSString stringWithContentsOfFile:p 
					      encoding:NSISOLatin1StringEncoding
						 error:&err];
	if (!s) NSLog(@"error reading %@ : %@\n", p, err);
	return s;
}

#if !TARGET_OS_IPHONE

- (NSDate *) possibleDate:(NSRange *)rg
{
	const char *s=[self UTF8String];
	// look for d[d]/d[d]/[dd]dd, where d is digit, '/' is ponctuation, same char
	int st=0;
	int sep=-1;
	const char *p;
	char *p1=nil;
	BOOL ok=NO;
	int m=0;
	int d=0;
	int y=0;
	for (p=s; !ok; p++) {
		switch (st) {
			case 0: // skip to digit
				if (isdigit(*p)) {
					st=1;
					p1=(char *) p;
					m=0; y=0;
					d=*p-'0';
				}
				break;
			case 1: // got 1st digit
				if (isdigit(*p)) {
					st=2;
					d=d*10+*p-'0';
				}
				else if (!isalpha(*p)) {
					sep=*p;
					st=3;
				} else st=0;
				break;
			case 2:
				if (!isalpha(*p)) {
					sep=*p;
					st=3;
				} else st=0;
				break;
			case 3: 
				if (isdigit(*p)) {
					st=4;
					m=*p-'0';
				} else st=0;
				break;
			case 4:
				if (sep==*p) {
					sep=*p;
					st=6;
				} else if (isdigit(*p)) {
					st=5;
					m=m*10+*p-'0';
				}
				else st=0;
				break;
			case 5: 
				if (sep==*p) {
					sep=*p;
					st=6;
				} else  st=0;
				break;
			case 6:
				if (isdigit(*p)) {
					st++;
					y=*p-'0';
				}
				else st=0;
				break;
			case 7:
				if (isdigit(*p)) {
					st++;
					y=y*10+*p-'0';
				}
				else st=0;
				break;
			case 8:
				if (isdigit(*p)){
					st++;
					y=y*10+*p-'0';
				}
				else {
					ok=YES;
				}
				break;
			case 9:
				if (isdigit(*p)) {
					st++;
					ok=YES;
					y=y*10+*p-'0';
				}
				else st=0;
				break;
		}
		if (!*p) break;
	}
	if (!ok) return nil;
    if (y<100) {
        if (y>80) y+=1900;
        else y+=2000;

    }
    NSCalendar *calendar = [NSCalendar gmtCalendar];
    NSDateComponents *components = [[NSDateComponents alloc] init];
    [components setYear:y];
    [components setMonth:m];
    [components setDay:d];
    [components setHour:12];
    [components setMinute:0];
    [components setSecond:0];
    NSDate *rd = [calendar dateFromComponents:components];
    
		if (rg) {
		rg->location=(NSUInteger) (p1-s);
		rg->length=(NSUInteger) (p-p1);
	}
	return rd;
}

#endif /* TARGET_OS_IPHONE */


- (BOOL) possibleDay:(int *)pday month:(int *)pmonth range:(NSRange *)rg
{
	const char *s=[self UTF8String];
	// look for d[d]/dd+(non-digit), where d is digit, '/' is ponctuation, same char
	int st=0;
	int sep=-1;
	const char *p;
	char *p1=nil;
	BOOL ok=NO;
	int m=0;
	int d=0;
	int y=0;
	for (p=s; !ok; p++) {
        if (!*p) {
            if (st == 5) ok = YES;
            break;
        }
		switch (st) {
			case 0: // skip to digit
				if (isdigit(*p)) {
					st=1;
					p1=(char *) p;
					m=0; y=0;
					d=*p-'0';
				}
				break;
			case 1: // got 1st digit
				if (isdigit(*p)) {
					st=2;
					d=d*10+*p-'0';
				}
				else if (!isalpha(*p)) {
					sep=*p;
					st=3;
				} else st=0;
				break;
			case 2:
				if (!isalpha(*p)) {
					sep=*p;
					st=3;
				} else st=0;
				break;
			case 3:
				if (isdigit(*p)) {
					st=4;
					m=*p-'0';
				} else st=0;
				break;
			case 4:
				if (/* DISABLES CODE */ (0) && (sep==*p)) {
					sep=*p;
					st=6;
				} else if (isdigit(*p)) {
					st=5;
					m=m*10+*p-'0';
				}
				else st=0;
				break;
			case 5:
                if (isdigit(*p)) {
                    st=0;
                } else {
                    ok = YES;
                }
				break;
		}
		if (!*p) break;
	}
	if (!ok) return NO;
    if (d<1) return NO;
    if (d>31) return NO;
    if (m<1) return NO;
    if (m>12) return NO;
    *pday = d;
    *pmonth = m;
	if (rg) {
		rg->location=(NSUInteger) (p1-s);
		rg->length=(NSUInteger) (p-p1);
	}
	return YES;
}

@end
@implementation NSMutableString (StringExtension) 

- (void) deleteLastCharacters:(NSUInteger)numtodelete
{
	if (!numtodelete) return;
	NSUInteger l=[self length];
	NSRange r;
	if (numtodelete>l) numtodelete = l;
	r.location=l-numtodelete;
	r.length=numtodelete;
	[self deleteCharactersInRange:r];
}

@end



@implementation NSString (removeAccents)
- (NSString*) lowercaseStringRemovingAccents 
{
	return [[NSString alloc] initWithData:[[self lowercaseString] dataUsingEncoding:NSASCIIStringEncoding
								    allowLossyConversion:YES]  
				      encoding:NSASCIIStringEncoding] ;
}
- (NSString*) lowercaseStringRemovingAccentsAndPunct
{
	NSString *s= [[NSString alloc] initWithData:[[self lowercaseString] dataUsingEncoding:NSASCIIStringEncoding 
								    allowLossyConversion:YES]  
				      encoding:NSASCIIStringEncoding];
	const char *str=[s UTF8String];
	NSUInteger l=[s length];
	int i;
	for (i=(int)l-1; i>=0; i--) {
		if (isalnum(str[i])) break;
	}
	return [s substringToIndex:(NSUInteger)i+1];
}	
@end



@implementation NSString (CommonString)

/*
 * e.g. between "apple juice and bananas" ans "banana juice"
 *  -> "juice", "banana"
 *
 * order of result is unspecified
 */

// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Longest_common_substring#Objective-C


/*
+ (NSString *)longestCommonSubstring:(NSString *)substring string:(NSString *)string {
    if (substring == nil || substring.length == 0 || string == nil || string.length == 0) {
        return nil;
        
    }
    NSMutableDictionary *map = [NSMutableDictionary dictionary];
    int maxlen = 0;
    int lastSubsBegin = 0;
    NSMutableString *sequenceBuilder = [NSMutableString string];
    
    for (int i = 0; i < substring.length; i++) {
        for (int j = 0; j < string.length; j++) {
            unichar substringC = [[substring lowercaseString] characterAtIndex:i];
            unichar stringC = [[string lowercaseString] characterAtIndex:j];
            
            if (substringC != stringC) {
                //NSLog(@"diff sequenceBuilder='%@' i,j=%i,%i %C/%C\n", sequenceBuilder, i, j, stringC, substringC);
                [map setObject:[NSNumber numberWithInt:0] forKey:[NSString stringWithFormat:@"%i_%i",i,j]];
                //sequenceBuilder= [@"" mutableCopy];
            } else {
                if ((i == 0) || (j == 0)) {
                    [map setObject:[NSNumber numberWithInt:1] forKey:[NSString stringWithFormat:@"%i_%i",i,j]];
                }
                else {
                    int prevVal = [[map objectForKey:[NSString stringWithFormat:@"%i_%i",i-1,j-1]] intValue];
                    [map setObject:[NSNumber numberWithInt:1+prevVal] forKey:[NSString stringWithFormat:@"%i_%i",i,j]];
                }
                int currVal = [[map objectForKey:[NSString stringWithFormat:@"%i_%i",i,j]] intValue];
                if (currVal > maxlen) {
                    maxlen = currVal;
                    int thisSubsBegin = i - currVal + 1;
                    if (lastSubsBegin == thisSubsBegin)
                    {//if the current LCS is the same as the last time this block ran
                        NSString *append = [NSString stringWithFormat:@"%C",substringC];
                        [sequenceBuilder appendString:append];
                    } else {//this block resets the string builder if a different LCS is found
                        lastSubsBegin = thisSubsBegin;
                        NSLog(@"reset sequenceBuilder='%@'\n", sequenceBuilder);
                        NSString *resetStr = [substring substringWithRange:NSMakeRange(lastSubsBegin, (i + 1) - lastSubsBegin)];
                        sequenceBuilder = [NSMutableString stringWithFormat:@"%@",resetStr];
                    }
                }
            }
            //NSLog(@"    end i=%d j=%d, %@\n", i, j, sequenceBuilder);
        }
        //NSLog(@"end i=%d, %@\n", i, sequenceBuilder);
    }
    return sequenceBuilder;
}
 */

static NSArray<NSString*> * _lcs(NSString *s1, NSInteger l1, NSString *s2, NSInteger l2, BOOL lcs, int minl, BOOL reuse);


- (NSArray<NSString *> *) commonStringsWith:(NSString *)otherString
{
    return _lcs(self, [self length], otherString, [otherString length], NO, 2, NO);
}
- (NSArray<NSString *> *) commonStringsWith:(NSString *)otherString minLength:(int)minl
{
    return _lcs(self, [self length], otherString, [otherString length], NO, minl, NO);
}

- (NSArray<NSString *> *) allCommonStringsWith:(NSString *)otherString
{
    return _lcs(self, [self length], otherString, [otherString length], NO, 2, YES);
}
- (NSArray<NSString *> *) allCommonStringsWith:(NSString *)otherString minLength:(int)minl
{
    return _lcs(self, [self length], otherString, [otherString length], NO, minl, YES);
}

- (NSArray<NSString *> *) longestCommonStringsWith:(NSString *)otherString
{
    return _lcs(self, [self length], otherString, [otherString length], YES, 0, NO);
}



//http://stackoverflow.com/questions/34805488/finding-all-the-common-substrings-of-given-two-strings
// https://codedump.io/share/q7Q8PPkz4K71/1/finding-all-the-common-substrings-of-given-two-strings

#define _dbg_lcs 0

static NSArray<NSString*> * _lcs(NSString *s1, NSInteger l1, NSString *s2, NSInteger l2, BOOL lcs, int minl, BOOL reuse)
{
    int  table[l1][l2];
    memset(table, 0, sizeof(table));
    //int s = sizeof(table);
    //int st2 = sizeof(tbl2);
    int longest = 1;
    int *tbl = &table[0][0];
#define _T(_i,_j) tbl[l2*(_i)+(_j)]
    void (^dumptbl)(NSString *) = NULL;
    if ((_dbg_lcs)) dumptbl = ^(NSString *head){
        NSMutableString *res = [[NSMutableString alloc]initWithCapacity:1000];
        [res appendString:head];
        [res appendString:@"\n"];
        [res appendString:@"    "];
        for (int j=0; j<l2; j++) {
            unichar c = [s2 characterAtIndex:j];
            [res appendFormat:@"  %C ", c];
        }
        [res appendString:@"\n"];
        for (int i=0; i<l1; i++) {
            unichar c = [s1 characterAtIndex:i];
            [res appendFormat:@"%C : ", c];
            for (int j=0; j<l2; j++) {
                NSString *s = [NSString stringWithFormat:@" %2d ", _T(i, j)];
                [res appendString:s];
            }
            [res appendString:@"\n"];
        }
        NSLog(@"table: %@\n", res);
    };
    
    NSMutableArray *results = [NSMutableArray arrayWithCapacity:20];
    for (int i = 0; i < l1; i++) {
        for (int j = 0; j < l2; j++) {
            unichar c1 = [s1 characterAtIndex:i];
            unichar c2 = [s2 characterAtIndex:j];
            if (c1 != c2) {
                continue;
            }
            
            table[i][j] = (i == 0 || j == 0) ? 1 : 1 + table[i - 1][j - 1];
            if (table[i][j] > longest) {
                longest = table[i][j];
                if (lcs) [results removeAllObjects];
            }
            if (lcs && (table[i][j] == longest)) {
                NSString *seq = [s1 substringWithRange:NSMakeRange(i-longest+1, longest)];
                [results addObject:seq];
            }
        }
    }
    if (lcs) return results;
    if ((_dbg_lcs)) dumptbl(@"after LCS");
    if (minl<0) minl=0;
    for (int tlong = longest; tlong>minl; tlong--) {
        // get entries in table with value tlong
        for (int i=0; i<l1; i++) {
            for (int j=0; j<l2; j++) {
                if (table[i][j] != tlong) continue;
                // get the string
                NSString *s = [s1 substringWithRange:NSMakeRange(i-tlong+1, tlong)];
                [results addObject:s];
                // clear table square
                if (reuse) {
                    for (int ti = i-tlong+1; ti<=i; ti++) {
                        for (int tj = j-tlong+1; tj<=j; tj++) {
                            table[ti][tj] = -1;//0;
                        }
                    }

                } else {
                    for (int ti = i-tlong+1; ti<=i; ti++) {
                        for (int tj=0; tj<l2; tj++) {
                            table[ti][tj] = -1;//0;
                        }
                    }
                    for (int tj = j-tlong+1; tj<=j; tj++) {
                        for (int ti=0; ti<l1; ti++) {
                            table[ti][tj] = -1;//0;
                        }
                    }
                }
            }
        }
        if ((_dbg_lcs)) {
            NSString *l = [NSString stringWithFormat:@"tlong=%d", tlong];
            dumptbl(l);
        }
        //NSLog(@"tlong %d results %@\n", tlong, results);
    }
    return results;
}



// http://stackoverflow.com/questions/17054989/all-common-substrings-between-two-strings
// https://code.google.com/archive/p/all-common-subsequences/source
// https://www.aaai.org/Papers/IJCAI/2007/IJCAI07-101.pdf
// https://codedump.io/share/q7Q8PPkz4K71/1/finding-all-the-common-substrings-of-given-two-strings

@end


