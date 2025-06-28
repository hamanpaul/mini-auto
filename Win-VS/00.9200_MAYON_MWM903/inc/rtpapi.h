#ifndef __RTP_API_H__
#define __RTP_API_H__

extern void send_PCM_frame(u32 time,	u32	size,	u8* buffer);
extern void send_mpeg4_frame(u32 time,	u32	size,	u8* buffer);
#endif
