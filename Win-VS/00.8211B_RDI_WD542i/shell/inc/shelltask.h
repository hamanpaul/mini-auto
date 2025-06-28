/*	 
 *	shelltask.h
 *	the genie shell task for ucosII
 *	under skyeye
 *
 *	Bugs report:	 Yang Ye  ( yangye@163.net )
 *	Last modified:	 2003-02-19 
 *
 */

#define ERRORCOMMAND    255

#define MaxLenComBuf	100

INT8U CommandAnalys(char *Buf);
void shelltask(void *pParam);
