
#ifndef _MEDIAINFOH
#define _MEDIAINFOH

//#include <sys/types.h>
//#include <sys/stat.h>
//#include <unistd.h>
//#include <fenice/bufferpool.h>
#ifndef MAX_DESCR_LENGTH
#define MAX_DESCR_LENGTH 4096
#endif
#define DIM_VIDEO_BUFFER 5 
#define DIM_AUDIO_BUFFER 5

/* Formati di descrizione supportati */	

typedef enum 
{
    df_SDP_format=0
} description_format;

typedef enum {stored=0,live} media_source;
typedef enum {audio=0,video,application,data,control} media_type;
typedef enum {undefined=-1,frame=0,sample=1} media_coding;

typedef enum 
{
    ME_FILENAME=1,
    /*    disposable   ME=2,  */
    ME_DESCR_FORMAT=4,
    ME_AGGREGATE=8,
    ME_RESERVED=16,
    ME_FD=32
} me_flags;

typedef enum 
{
    SD_FL_TWIN=1,
    SD_FL_MULTICAST=2,
    SD_FL_MULTICAST_PORT=4
} sd_descr_flags;

typedef enum
{
    MED_MTYPE=1,
    MED_MSOURCE=2,
    MED_PAYLOAD_TYPE=4,
    MED_CLOCK_RATE=8,
    MED_ENCODING_NAME=16,
    MED_AUDIO_CHANNELS=32,
    MED_SAMPLE_RATE=64, 
    MED_BIT_PER_SAMPLE=128,
    MED_CODING_TYPE=256,
    MED_FRAME_LEN=512,
    MED_BITRATE=1024,
    MED_PKT_LEN=2048,
    MED_PRIORITY=4096,
    MED_TYPE=8192,
    MED_FRAME_RATE=16384,
    MED_BYTE_PER_PCKT=32768,
    
    /*start CC*/
    MED_LICENSE=65536,
    MED_RDF_PAGE=131072,
    MED_TITLE=262144,
    MED_CREATOR=524288,
    MED_ID3=1048576,
    /*end CC*/

    MED_FORCE_FRAME_RATE=2097152

    /*DYN_PAYLOAD_TOKEN    	
    PACKETTIZED=
    PAYLOAD=
    FORMAT=
    CHANNELS=
    COLOR_DEPTH=
    SCREEN_WIDTH=
    SCREEN_HEIGHT=*/
} me_descr_flags;

typedef struct _media_entry 
{
    me_flags flags;
    int fd;
    void *stat;
    struct _media_fn *media_handler;
    unsigned int data_chunk;

    /*Buffering with bufferpool module*/
    unsigned char buff_data[4]; /* shawill: needed for audio-mp3 live-by-named-pipe*/       /*TODO: to move in mp3.h*/
    unsigned int buff_size;            /* shawill: needed for audio-mp3 live-by-named-pipe */          /*TODO: to move in mp3.h*/
 //   OMSBuffer *pkt_buffer; 

    /*these time vars have transferred here*/
    double mtime;
    double mstart;
    double mstart_offset;
    double play_offset;                          /*chicco. Usefull for random access*/
    double prev_mstart_offset;             /*usefull for random access*/
    int rtp_multicast_port; 

    /*started has transferred itself here*/
    int reserved;
    char filename[255];
    char aggregate[50];
    description_format descr_format;

    struct 
    {
        /*start CC*/
        char commons_dead[255]; 
        char rdf_page[255];
        char title[80];
        char author[80];
        int tag_dim;    
        /*end CC*/
        
        me_descr_flags flags;
        media_source msource;
        media_type mtype;
        int payload_type;
        int clock_rate;
        int sample_rate;
        short audio_channels;
        int bit_per_sample;
        char encoding_name[11];
        
        media_coding coding_type;
        int frame_len;
        int bitrate;
        int priority;
        float pkt_len;                       /*packet length*/
        float delta_mtime;
        int frame_rate;
        int byte_per_pckt;

        
    } description;   
    struct _media_entry *next;
} media_entry;

/* 
* \struct media_fn
*/
/*¨ç¼Æ«ü°w*/
typedef struct _media_fn
{
/*
* \fn load_media
*\brief Function pointer to load media.
*\param media_entry *me .
*
*/
int (*load_media)(media_entry *);

/*
* \fn read_media
* brief Function pointer to read bitstream.
* param media_entry *me .
* param uint8 *buffer .
* param uint32 *buffer_size .
* param double *mtime .
* param int *recallme .
* param uint8 *marker; 
*
*/

/*
\fn free_media
brief Function pointer to free static_X structure used in read_X.
param void *stat .
warnings Don't free structures allocated by load_X because it is recalled only if .sd changes.
*/
int (*free_media)(void *);
} media_fn;


#endif
