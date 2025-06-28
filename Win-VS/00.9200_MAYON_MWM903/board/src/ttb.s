;
;Copyright (c) 2008 Mars Semiconductor Corp.
;
;Module Name:
;
;	ttb.s
;
;Abstract:
;
;   	The TTB region.
;
;Environment:
;
;   	ARM RealView Developer Suite
;
;Revision History:
;	
;	2005/08/26	David Tsai	Create	
;

        PRESERVE8

        AREA    TableBase, DATA, NOINIT

        EXPORT  TTB

; Create dummy variable used to locate TTB in memory

TTB     SPACE   1

        END

