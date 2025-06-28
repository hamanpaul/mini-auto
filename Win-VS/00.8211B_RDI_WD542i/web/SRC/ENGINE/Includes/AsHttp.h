/*
 *	File:		AsHttp.h
 *
 *	Contains:	Internal Embedded Web Server Definitions
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *  Copyright:	© 1995-2003 by Allegro Software Development Corporation
 *  All rights reserved.
 *
 *  This module contains confidential, unpublished, proprietary 
 *  source code of Allegro Software Development Corporation.
 *
 *  The copyright notice above does not evidence any actual or intended
 *  publication of such source code.
 *
 *  License is granted for specific uses only under separate 
 *  written license by Allegro Software Development Corporation.
 *  
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *
 *	Change History (most recent first):
 *
 * * * * Release 4.30  * * *
 *		08/27/03	bva		changed kHttpPatternCookie and kHttpPatternSetCookie
 *							to avoid parsing Cookie2 headers
 *		08/09/03	bva		added strings for Range requests
 * * * * Release 4.21  * * *
 * * * * Release 4.20  * * *
 *		01/31/03	pjr		add strings for Web Client CONNECT support
 *		11/05/02	pjr		add strings for Proxy Authentication
 * * * * Release 4.12  * * *
 * * * * Release 4.11  * * *
 *		07/18/02	pjr		add more pattern recognition strings for caching
 * * * * Release 4.10  * * *
 * * * * Release 4.00  * * *
 *		06/29/01	amp		add kHttpCallback, kHttpPatternSid, kHttpUpnpPropchange
 *		06/29/01	amp		added XML fragments, kCharsetUtf8
 *		05/14/01	pjr		change kContinue
 *		05/02/01	bva		change kLastChunk
 *		12/19/00	amp		remove extra <CR><LF> from kHttpAcceptAll
 *		12/18/00	pjr		initial version, created from RomPager.h
 * * * * Release 3.10  * * *
 * * * * Release 3.0 * * * *
 * * * * Release 2.0 * * * *
 * * * * Release 1.0 * * * *
 *
 *	To Do:
 */

#ifndef	_AS_HTTP_
#define	_AS_HTTP_


/* 
	Chunked Encoding Constants 
*/
#define kChunkedOverhead				12
#define kChunkedOffset					5
#define kLastChunk						"\x30\x0d\x0a\x0d\x0a"



#define kHttpString				"http://"
#define kHttpStringUpper		"HTTP://"
#define kHttpsString			"https://"


/* 
	HTTP Request Components
*/

#define kHttpGet			"GET "
#define kHttpHead			"HEAD "
#define kHttpPost			"POST "
#define kHttpPut			"PUT "
#define kHttpDelete			"DELETE "
#define kHttpOptions		"OPTIONS "
#define kHttpTrace			"TRACE "
#define kHttpConnect		"CONNECT "
#define kHttpMGet			"M-GET "
#define kHttpMPost			"M-POST "
#define kHttpMPut			"M-PUT "
#define kHttpSubscribe		"SUBSCRIBE "
#define kHttpUnsubscribe	"UNSUBSCRIBE "
#define kHttpNotify			"NOTIFY "
#if RomPagerHttpOneDotOne
#define kHttpVersion		"HTTP/1.1"
#else
#define kHttpVersion		"HTTP/1.0"
#endif
#define kRootPath			"/"
#define kForwardSlash		"/"
#define kHttpKeepAlive		"Keep-Alive"
#define kHttpClose			"close"
#define kHttpChunked		"chunked"
#define kBoundary			"boundary"
#define kFilename			"filename"
#define kName				"name"
#define kEmptyLength		"\x30\x0d\x0a"
#define kHttpRequestTLS		"TLS/1.0"
#define kHttpContinue		"100-continue"
#define kHttpUpnpEvent		"upnp:event"
#define kHttpUpnpPropchange	"upnp:propchange"
#define kHttpInfinite		"infinite"
#define kHttpSecond			"second-"
#define kHttpBytes			"bytes"

/*
	Digest Security Components
*/
#define kGetColon			"GET:"
#define kPostColon			"POST:"
#define kHeadColon			"HEAD:"
#define kPutColon			"PUT:"
#define kOptionsColon		"OPTIONS:"
#define kTraceColon			"TRACE:"
#define kDigest				"Digest"
#define kSecurity			"Security"
#define kHttpExtension		"Extension:"
#define kHttpAlgorithm		"algorithm"
#define kHttpAuth			"auth"
#define kHttpDigest			"digest"
#define kHttpOpaque			"opaque"
#define kHttpQop			"qop"
#define kHttpUsername		"username"

/* 
	General Headers 
*/
#define kHttpConnection		"Connection: "
#define kHttpDate			"Date: "


/* 
	Request Headers 
*/
#define kHttpAccept				"Accept: "
#define kHttpHost				"Host: "
#define kHttpAcceptHtml			"Accept: text/html\x0d\x0a"
#define kHttpAcceptImage		"Accept: image/pict, image/gif; q = 0.8, "	\
											"image/jpeg; q = 0.8\x0d\x0a"
#define kHttpUserAgent			"User-Agent: "
#define kHttpAcceptLanguage		"Accept-Language: "
#define kHttpAcceptRanges		"Accept-Ranges: bytes\x0d\x0a"
#define kHttpReferer			"Referer: "
#define kHttpMandatory			"Man: "

#define kHttpAuthorizationBasic		"Authorization: Basic "
#define kHttpAuthorizationDigest	"Authorization: Digest username=\""
#define kHttpProxy					"Proxy-"
#define kHttpIfModified				"If-Modified-Since: "
#if RpEtagHeader

#define kHttpIfMatch			"If-Match: "
#define kHttpIfNoneMatch		"If-None-Match: "
#endif


/* 
	Response Headers 
*/
#define kHttpServer						"Server:"
#define kHttpWWWAuthenticate			"WWW-Authenticate: Basic realm=\""
#define kHttpWWWAuthenticateDigest		"WWW-Authenticate: Digest realm=\""
#define kHttpCnonce						", cnonce=\""
#define kHttpNc							", nc="
#define kHttpNonce						"\", nonce=\""
#define kHttpNextNonce					"Authentication-info: nextnonce=\""
#define kHttpQopAuth					", qop=auth"
#define kHttpStaleTrue					"\", stale=true"
#define kHttpProxyAuthenticate			"Proxy-Authenticate:"
#define kHttpRealm						"\", realm=\""
#define kHttpResponse					", response=\""
#define kHttpRetryAfter					"Retry-After:"
#define kHttpRefresh					"Refresh: "
#define kHttpUri						"\", uri=\""
#define kServerHeader					"Server: Allegro-Software-RomPager/" \
												kVersion \
												"\x0d\x0a"
#define kRemoteHostClientHeader			"Allegro-Software-RemoteHost/" \
												kVersion \
												"\x0d\x0a"
#define kHttpNoCache					"Pragma: no-cache\x0d\x0a"

#if RomPagerHttpOneDotOne
#define kHttpOneOneNoCache				"Cache-Control: no-cache\x0d\x0a"
#endif
#if RpEtagHeader
#define kHttpEtag						"Etag: \""
#endif

#define kHttpCookie						"Cookie: "
#define kHttpSetCookie					"Set-Cookie: C"
#define kCookiePath						"; path=/"
#define kHttpContentRange				"Content-Range: bytes "
#define kHttpContentRangeError			"*/"


/* 
	Object Headers 
*/
#define kHttpAllow						"Allow:"
#define kHttpContentLength				"Content-Length: "
#define kHttpContentLengthZero			"Content-Length: 0\x0d\x0a\x0d\x0a"
#define kHttpContentType				"Content-Type: "
#define kHttpContentDisposition			"Content-Disposition: attachment; filename=\""
#define kHttpContentEncodingGzip		"Content-Encoding: gzip\x0d\x0a"
#define kHttpTransferEncodingChunked	"Transfer-Encoding: chunked\x0d\x0a"
#define kHttpContentLanguage			"Content-Language:"
#define kHttpExpires					"Expires: "
#define kHttpLastModified				"Last-Modified: "
#define kHttpURIHeader					"URI: <"
#define kHttpURL						"URL="
#define kHttpLocation					"Location: "
#define kHttpObjectVersion				"Version:"
#define kHttpDerivedFrom				"Derived-From:"
#define kHttpTitle						"Title:"
#define kHttpLink						"Link:"
#define kFieldSeparator					"; "
#define kHttpCallbackHeader				"Callback: <http://"
#define kHttpEndCallbackHeader			">\x0d\x0a"
#define	kHttpNtHeader					"NT: upnp:event\x0d\x0a"
#define kHttpTimeoutHeader				"Timeout: Second-"
#define kHttpSidHeader					"Sid: "


/* 
	HTTP Pattern Headers 
*/
#define kHttpPatternAccept				"accept:"
#define kHttpPatternAcceptLanguage		"accept-language"
#define kHttpPatternAuthenticate		"www-authenticate"
#define kHttpPatternAuthInfo			"authentication-info"
#define kHttpPatternAuthorization		"authorization"
#define kHttpPatternBasic				"basic"
#define kHttpPatternCacheControl		"cache-control"
#define kHttpPatternCnonce				"cnonce"
#define kHttpPatternConnection			"connection"
#define kHttpPatternContentDisposition	"content-disposition"
#define kHttpPatternContentLength		"content-length"
#define kHttpPatternContentType			"content-type"
#define kHttpPatternCookie				"cookie:"
#define kHttpPatternDate				"date"
#define kHttpPatternDigest				"digest"
#define kHttpPatternDomain				"domain"
#define kHttpPatternExpect				"expect"
#define kHttpPatternExpires				"expires"
#define kHttpPatternExtension			"extension"
#define kHttpPatternHost				"host"
#define kHttpPatternIfModified			"if-modified-since"
#define kHttpPatternIfNoneMatch			"if-none-match"
#define kHttpPatternLastModified		"last-modified"
#define kHttpPatternLocation			"location"
#define kHttpPatternMandatory			"man"
#define kHttpPatternNc					"nc"
#define kHttpPatternNextNonce			"nextnonce"
#define kHttpPatternNoCache				"no-cache"
#define kHttpPatternNoStore				"no-store"
#define kHttpPatternNonce				"nonce"
#define kHttpPatternPath				"path"
#define kHttpPatternPragma				"pragma"
#define kHttpPatternPrivate				"private"
#define kHttpPatternProxyAuthenticate	"proxy-authenticate"
#define kHttpPatternQop					"qop"
#define kHttpPatternRange				"range"
#define kHttpPatternRealm				"realm"
#define kHttpPatternReferer				"referer"
#define kHttpPatternRefresh				"refresh"
#define kHttpPatternResponse			"response"
#define kHttpPatternSecure				"secure"
#define kHttpPatternServer				"server"
#define kHttpPatternSetCookie			"set-cookie:"
#define kHttpPatternTransferEncoding	"transfer-encoding"
#define kHttpPatternUpdate				"update"
#define kHttpPatternUserAgent			"user-agent"
#define kHttpPatternUsername			"username"
#define kHttpPatternUri					"uri"
#define kHttpPatternSid					"sid"


/*
	Response components 
*/

#define kEnglish						" en\x0d\x0a"
#define kFrench							" fr\x0d\x0a"
#define kGerman							" ge\x0d\x0a"
#define kSpanish						" sp\x0d\x0a"
#define kJapanese						" jp\x0d\x0a"
#define kContinue						"HTTP/1.1 100 Continue\x0d\x0a\x0d\x0a"
#define kPageFound						" 200 OK\x0d\x0a"
#define kCreated						" 201 Created\x0d\x0a"
#define kPartial						" 206 Partial Content\x0d\x0a"
#define kMultipleChoices				" 300 Multiple Choices\x0d\x0a"
#if RomPagerHttpOneDotOne
#define kMoved							" 302 Found\x0d\x0a"
#else
#define kMoved							" 302 Moved Temporarily\x0d\x0a"
#endif
#define kSeeOther						" 303 See Other\x0d\x0a"
#define kNotModified					" 304 Not Modified\x0d\x0a"
#define kBadRequest						" 400 Bad Request\x0d\x0a"
#define kUnauthorized					" 401 Unauthorized\x0d\x0a"
#define kForbidden						" 403 Forbidden\x0d\x0a"
#define kNoPageFound					" 404 Not Found\x0d\x0a"
#define kMethodNotAllowed				" 405 Method Not Allowed\x0d\x0a"
#define kNoneAcceptable					" 406 Not Acceptable\x0d\x0a"
#define kPreconditionFailed				" 412 Precondition Failed\x0d\x0a"
#define kRequestTooLarge				" 413 Request Entity Too Large\x0d\x0a"
#define kBadMediaType					" 415 Unsupported Media Type\x0d\x0a"
#define kBadRange						" 416 Requested Range Not Satisfiable\x0d\x0a"
#define kExpectFailed					" 417 Expectation Failed\x0d\x0a"
#define kServerError					" 500 Internal Server Error\x0d\x0a"
#define kNotImplemented					" 501 Not Implemented\x0d\x0a"
#define kNotExtended					" 510 Not Extended\x0d\x0a"
#define kCommaSpace						", "
#define kSpaceGMT						" GMT"
#define kTwoDashes						"--"
#define kHttp10_OK						"HTTP/1.0 200 OK"
#define kHttp11_OK						"HTTP/1.1 200 OK"
#define kHttpEndRemoteUrl				" HTTP/1.0\x0d\x0a"
#define kHttpAcceptAll					"Accept: */*\x0d\x0a"
#define kCharsetUtf8					"; charset=\"utf-8\"\x0d\x0a"

#if 0
#define kTCNTest						"TCN: list\x0d\x0a"
#define kTCNTest1						"Vary: *\x0d\x0a"
#define kTCNTest2						"Alternates: {\"Allegro.1\" 0.9 {type text/html} {language en}},\x0d\x0a"
#define kTCNTest3						"{\"Allegro.2\" 0.7 {type text/html} {language fr}}\x0d\x0a"
#endif

/*
	XML fragments
*/
#define	kStartCloseXmlTag				"</"


#endif	/* _AS_HTTP_ */
