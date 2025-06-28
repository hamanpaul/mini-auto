

#ifndef _SDPH
#define _SDPH
#include <../LwIP/include/lwip/mediainfo.h>
#define SDP_EL "\r\n"
	

	char *get_SDP_user_name(char *);
	char *get_SDP_session_id(char *);
	char *get_SDP_version(char *);
	
	//int get_media_descr(char *url,media_entry *req,media_entry *media,char *descr);
	int get_SDP_descr(char *descr,char *url);

#endif
