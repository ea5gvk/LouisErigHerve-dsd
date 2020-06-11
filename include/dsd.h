#ifndef DSD_H
#define DSD_H

#ifdef __cplusplus
extern "C" {
#endif


/*
 * Copyright (C) 2010 DSD Author
 * GPG Key ID: 0x3F1D7FD0 (74EF 430D F7F2 0A48 FCE6  F630 FAA2 635D 3F1D 7FD0)
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES WITH
 * REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY
 * AND FITNESS.  IN NO EVENT SHALL ISC BE LIABLE FOR ANY SPECIAL, DIRECT,
 * INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM
 * LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE
 * OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 * PERFORMANCE OF THIS SOFTWARE.
 */

#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <signal.h>
#include <string.h>
#include <assert.h>
#define __USE_XOPEN
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#ifdef SOLARIS
#include <sys/audioio.h>
#endif
#if defined(BSD) && !defined(__APPLE__)
#include <sys/soundcard.h>
#endif
#include <math.h>
#include <mbelib.h>
#include <sndfile.h>

#include "p25p1_heuristics.h"


/* Define -------------------------------------------------------------------*/
#define UNUSED_VARIABLE(x) (x = x)

#define SAMPLE_RATE_IN 48000
#define SAMPLE_RATE_OUT 8000

#define true  1u
#define false 0u

typedef unsigned char bool;

/* Could only be 2 or 4 */
#define NB_OF_DPMR_VOICE_FRAME_TO_DECODE 2


#define BUILD_DSD_WITH_FRAME_CONTENT_DISPLAY


#ifdef USE_PORTAUDIO
#include "portaudio.h"
#define PA_FRAMES_PER_BUFFER 64
//Buffer needs to be large enough to prevent input buffer overruns while DSD is doing other struff (like outputting voice)
//else you get skipped samples which result in incomplete/erronous decodes and a mountain of error messages.
#define PA_LATENCY_IN 0.500
//Buffer needs to be large enough to prevent output buffer underruns while DSD is doing other stuff (like decoding input)
//else you get choppy audio and in 'extreme' cases errors.
//Buffer also needs to be as small as possible so we don't have a lot of audio delay.
#define PA_LATENCY_OUT 0.100
#endif


/*
 * global variables
 */
int exitflag;


/*
 * Enum
 */
enum
{
  MODE_UNENCRYPTED,
  MODE_BASIC_PRIVACY,
  MODE_ENHANCED_PRIVACY_ARC4,
  MODE_ENHANCED_PRIVACY_AES256,
  MODE_HYTERA_BASIC_40_BIT,
  MODE_HYTERA_BASIC_128_BIT,
  MODE_HYTERA_BASIC_256_BIT
};


/*
 * typedef
 */

/* Read Solomon (12,9) constant */
#define RS_12_9_DATASIZE        9
#define RS_12_9_CHECKSUMSIZE    3

/* Read Solomon (12,9) struct */
typedef struct
{
  uint8_t data[RS_12_9_DATASIZE+RS_12_9_CHECKSUMSIZE];
} rs_12_9_codeword_t;

// Maximum degree of various polynomials.
#define RS_12_9_POLY_MAXDEG (RS_12_9_CHECKSUMSIZE*2)

typedef struct
{
  uint8_t data[RS_12_9_POLY_MAXDEG];
} rs_12_9_poly_t;

#define RS_12_9_CORRECT_ERRORS_RESULT_NO_ERRORS_FOUND           0
#define RS_12_9_CORRECT_ERRORS_RESULT_ERRORS_CORRECTED          1
#define RS_12_9_CORRECT_ERRORS_RESULT_ERRORS_CANT_BE_CORRECTED  2
typedef uint8_t rs_12_9_correct_errors_result_t;

typedef struct
{
  uint8_t bytes[3];
} rs_12_9_checksum_t;


typedef struct
{
  char VoiceFrame[3][72];  /* 3 x 72 bit of voice per time slot frame    */
  char Sync[48];           /* 48 bit of data or SYNC per time slot frame */
}TimeSlotRawVoiceFrame_t;


typedef struct
{
  char DeInterleavedVoiceSample[3][4][24];  /* 3 x (4 x 24) deinterleaved bit of voice per time slot frame */
}TimeSlotDeinterleavedVoiceFrame_t;


typedef struct
{
  int  errs1[3];  /* 3 x errors #1 computed when demodulate the AMBE voice bit of the frame */
  int  errs2[3];  /* 3 x errors #2 computed when demodulate the AMBE voice bit of the frame */
  char AmbeBit[3][49];  /* 3 x 49 bit of AMBE voice of the frame */
}TimeSlotAmbeVoiceFrame_t;


typedef struct
{
  unsigned int  ProtectFlag;
  unsigned int  Reserved;
  unsigned int  FullLinkControlOpcode;
  unsigned int  FeatureSetID;
  unsigned int  ServiceOptions;
  unsigned int  GroupAddress;      /* Destination ID or TG (in Air Interface format) */
  unsigned int  SourceAddress;     /* Source ID (in Air Interface format) */
  unsigned int  DataValidity;      /* 0 = All Full LC data are incorrect ; 1 = Full LC data are correct (CRC checked OK) */
}FullLinkControlPDU_t;


typedef struct
{
  TimeSlotRawVoiceFrame_t TimeSlotRawVoiceFrame[6]; /* A voice superframe contains 6 timeslot voice frame */
  TimeSlotDeinterleavedVoiceFrame_t TimeSlotDeinterleavedVoiceFrame[6];
  TimeSlotAmbeVoiceFrame_t TimeSlotAmbeVoiceFrame[6];
  FullLinkControlPDU_t FullLC;
} TimeSlotVoiceSuperFrame_t;


typedef struct
{
  unsigned char RawVoiceBit[NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4][72];
  unsigned int  errs1[NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4];  /* 8 x errors #1 computed when demodulate the AMBE voice bit of the frame */
  unsigned int  errs2[NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4];  /* 8 x errors #2 computed when demodulate the AMBE voice bit of the frame */
  unsigned char AmbeBit[NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4][49];  /* 8 x 49 bit of AMBE voice of the frame */
  unsigned char CCHData[NB_OF_DPMR_VOICE_FRAME_TO_DECODE][48];
  unsigned int  CCHDataHammingOk[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned char CCHDataCRC[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned int  CCHDataCrcOk[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned char CalledID[8];
  unsigned int  CalledIDOk;
  unsigned char CallingID[8];
  unsigned int  CallingIDOk;
  unsigned int  FrameNumbering[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned int  CommunicationMode[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned int  Version[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned int  CommsFormat[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned int  EmergencyPriority[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned int  Reserved[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned char SlowData[NB_OF_DPMR_VOICE_FRAME_TO_DECODE];
  unsigned int  ColorCode[NB_OF_DPMR_VOICE_FRAME_TO_DECODE / 2];
} dPMRVoiceFS2Frame_t;


typedef struct __attribute__((packed, aligned(1)))
{
  char  FileTypeBlockID[4];  /* "RIFF" constant (0x52,0x49,0x46,0x46) */
  int   FileSize;            /* Wave file size in bytes minus 8 bytes */
  char  FileFormatID[4];     /* Format "WAVE" (0x57,0x41,0x56,0x45) */

  /* Block describing the audio format */
  char  FormatBlockID[4];    /* "fmt" constant 0x66,0x6D, 0x74,0x20) */
  int   BlockSize;           /* Number of byte of block : here 16 bytes (0x10) */
  short AudioFormat;         /* 1 = PCM */
  short NbCanaux;            /* Mono = 1 ; Stereo = 2 ; may be up to 6 */
  int   Frequency;           /* 11025, 22050, 44100, 48000 or 96000 */
  int   BytePerSec;          /* Frequency * BytePerBlock */
  short BytePerBlock;        /* NbCanaux * BitsPerSample / 8 */
  short BitsPerSample;       /* Number of bit per audio sample : 8, 16 or 24 bits */

  /* Data block */
  char  DataBlockID[4];      /* "data" constant (0x64,0x61,0x74,0x61) */
  int   DataSize;            /* Number of audio sample in bytes */
} wav_t;


typedef struct
{
  uint8_t StructureField;
  uint8_t RAN;
  uint8_t Data[18];
  uint8_t CrcIsGood;
} NxdnSacchRawPart_t;


typedef struct
{
  uint8_t  MessageType;

  /* VCALL parameters */
  uint8_t  CCOption;
  uint8_t  CallType;
  uint8_t  VoiceCallOption;
  uint16_t SourceUnitID;
  uint16_t DestinationID;  /* May be a Group ID or a Unit ID */
  uint8_t  CipherType;
  uint8_t  KeyID;
  uint8_t  VcallCrcIsGood;

  /* VCALL_IV parameters */
  uint8_t  IV[8];
  uint8_t  VcallIvCrcIsGood;

  /*****************************/
  /***** Custom parameters *****/
  /*****************************/

  /* Specifies if the "CipherType" and the "KeyID" parameter are valid
   * 1 = Valid ; 0 = CRC error */
  uint8_t  CipherParameterValidity;

  /* Used on DES and AES encrypted frames */
  uint8_t  PartOfCurrentEncryptedFrame;  /* Could be 1 or 2 */
  uint8_t  PartOfNextEncryptedFrame;  /* Could be 1 or 2 */
  uint8_t  CurrentIVComputed[8];
  uint8_t  NextIVComputed[8];
} NxdnSacchFull_t;


typedef struct
{
  int onesymbol;
  char mbe_in_file[1024];
  FILE *mbe_in_f;
  int errorbars;
  int datascope;
  int symboltiming;
  int verbose;
  int p25enc;
  int p25lc;
  int p25status;
  int p25tg;
  int scoperate;
  char audio_in_dev[1024];
  int audio_in_fd;
  SNDFILE *audio_in_file;
  SF_INFO *audio_in_file_info;
#ifdef USE_PORTAUDIO
  PaStream* audio_in_pa_stream;
#endif
  int audio_in_type; // 0 for device, 1 for file, 2 for portaudio
  char audio_out_dev[1024];
  int audio_out_fd;
  SNDFILE *audio_out_file;
  SF_INFO *audio_out_file_info;
#ifdef USE_PORTAUDIO
  PaStream* audio_out_pa_stream;
#endif
  int audio_out_type; // 0 for device, 1 for file, 2 for portaudio
  int split;
  int playoffset;
  char mbe_out_dir[1024];
  char mbe_out_file[1024];
  char mbe_out_path[1024];
  FILE *mbe_out_f;
  float audio_gain;
  int audio_out;
  char wav_out_file[1024];
  SNDFILE *wav_out_f;
  //int wav_out_fd;
  int serial_baud;
  char serial_dev[1024];
  int serial_fd;
  int resume;
  int frame_dstar;
  int frame_x2tdma;
  int frame_p25p1;
  int frame_nxdn48;
  int frame_nxdn96;
  int frame_dmr;
  int frame_provoice;
  int frame_dpmr;
  int mod_c4fm;
  int mod_qpsk;
  int mod_gfsk;
  int uvquality;
  int inverted_x2tdma;
  int inverted_dmr;
  int inverted_dpmr;
  int mod_threshold;
  int ssize;
  int msize;
  int playfiles;
  int delay;
  int use_cosine_filter;
  int unmute_encrypted_p25;

  unsigned int dPMR_curr_frame_is_encrypted;
  int dPMR_next_part_of_superframe;

  int EncryptionMode;

} dsd_opts;

typedef struct
{
  int *dibit_buf;
  int *dibit_buf_p;
  int repeat;
  short *audio_out_buf;
  short *audio_out_buf_p;
  float *audio_out_float_buf;
  float *audio_out_float_buf_p;
  float audio_out_temp_buf[160];
  float *audio_out_temp_buf_p;
  int audio_out_idx;
  int audio_out_idx2;
  //int wav_out_bytes;
  int center;
  int jitter;
  int synctype;
  int min;
  int max;
  int lmid;
  int umid;
  int minref;
  int maxref;
  int lastsample;
  int sbuf[128];
  int sidx;
  int maxbuf[1024];
  int minbuf[1024];
  int midx;
  char err_str[64];
  char fsubtype[16];
  char ftype[16];
  unsigned int color_code;
  unsigned int color_code_ok;
  unsigned int PI;
  unsigned int PI_ok;
  unsigned int LCSS;
  unsigned int LCSS_ok;
  int symbolcnt;
  int rf_mod; /* 0=C4FM 1=QPSK 2=GFSK */
  int numflips;
  int lastsynctype;
  int lastp25type;
  int offset;
  int carrier;
  char tg[25][16];
  int tgcount;
  int lasttg;
  int lastsrc;
  int nac;
  int errs;
  int errs2;
  char ambe_ciphered[49];
  char ambe_deciphered[49];
  int mbe_file_type;
  int optind;
  int numtdulc;
  int firstframe;
  char slot1light[8];
  char slot2light[8];
  float aout_gain;
  float aout_max_buf[200];
  float *aout_max_buf_p;
  int aout_max_buf_idx;
  int samplesPerSymbol;
  int symbolCenter;     /* DMR=4 - NXDN96=2 - dPMR/NXDN48=10 */
  char algid[9];
  char keyid[17];
  int currentslot; /* 0 = Slot 1 ; 1 = Slot 2 */
  int directmode;
  mbe_parms *cur_mp;
  mbe_parms *prev_mp;
  mbe_parms *prev_mp_enhanced;
  int p25kid;

  unsigned int debug_audio_errors;
  unsigned int debug_header_errors;
  unsigned int debug_header_critical_errors;

  // Last dibit read
  int last_dibit;

  // Heuristics state data for +P5 signals
  P25Heuristics p25_heuristics;

  // Heuristics state data for -P5 signals
  P25Heuristics inv_p25_heuristics;

#ifdef TRACE_DSD
  char debug_prefix;
  char debug_prefix_2;
  unsigned int debug_sample_index;
  unsigned int debug_sample_left_edge;
  unsigned int debug_sample_right_edge;
  FILE* debug_label_file;
  FILE* debug_label_dibit_file;
  FILE* debug_label_imbe_file;
#endif

  int TimeYear;
  int TimeMonth;
  int TimeDay;
  int TimeHour;
  int TimeMinute;
  int TimeSecond;

  TimeSlotVoiceSuperFrame_t TS1SuperFrame;
  TimeSlotVoiceSuperFrame_t TS2SuperFrame;

  dPMRVoiceFS2Frame_t dPMRVoiceFS2Frame;

  /* These flags are used to known the DMR frame
   * printing format (hex or binary) */
  int printDMRRawVoiceFrameHex;
  int printDMRRawVoiceFrameBin;
  int printDMRRawDataFrameHex;
  int printDMRRawDataFrameBin;
  int printDMRAmbeVoiceSampleHex;
  int printDMRAmbeVoiceSampleBin;

  /* These flags are used to known the dPMR frame
   * printing format (hex or binary) */
  int printdPMRRawVoiceFrameHex;
  int printdPMRRawVoiceFrameBin;
  int printdPMRRawDataFrameHex;
  int printdPMRRawDataFrameBin;
  int printdPMRAmbeVoiceSampleHex;
  int printdPMRAmbeVoiceSampleBin;

  int special_display_format_enable;  // Format used to decrypt EP keys
  int display_raw_data;  // Data displayed unencrypted

  NxdnSacchRawPart_t NxdnSacchRawPart[4];
  NxdnSacchFull_t NxdnSacchFull;

  int printNXDNAmbeVoiceSampleHex;
} dsd_state;


/* Only for debug
 * Used to find frame sync pattern used on
 * Icom IDAS NXDN 4800 baud system
 */
#define SIZE_OF_SYNC_PATTERN_TO_SEARCH  18


/*
 * Frame sync patterns
 */
#define INV_P25P1_SYNC "333331331133111131311111"
#define P25P1_SYNC     "111113113311333313133333"

#define X2TDMA_BS_VOICE_SYNC "113131333331313331113311"
#define X2TDMA_BS_DATA_SYNC  "331313111113131113331133"
#define X2TDMA_MS_DATA_SYNC  "313113333111111133333313"
#define X2TDMA_MS_VOICE_SYNC "131331111333333311111131"

#define DSTAR_HD       "131313131333133113131111"
#define INV_DSTAR_HD   "313131313111311331313333"
#define DSTAR_SYNC     "313131313133131113313111"
#define INV_DSTAR_SYNC "131313131311313331131333"

#define NXDN_MS_DATA_SYNC         "313133113131111333"
#define INV_NXDN_MS_DATA_SYNC     "131311331313333111"
#define NXDN_MS_VOICE_SYNC        "313133113131113133"
#define INV_NXDN_MS_VOICE_SYNC    "131311331313331311"
#define INV_NXDN_BS_DATA_SYNC     "131311331313333131"
#define NXDN_BS_DATA_SYNC         "313133113131111313"
#define INV_NXDN_BS_VOICE_SYNC    "131311331313331331"
#define NXDN_BS_VOICE_SYNC        "313133113131113113"
#define NXDN_SYNC                 "3131331131"  // TODO : Only for test
#define INV_NXDN_SYNC             "1313113313"  // TODO : Only for test
#define NXDN_SYNC2                "1313113331"  // TODO : Only for test
#define INV_NXDN_SYNC2            "3131331113"  // TODO : Only for test
#define NXDN_SYNC3                "3313113331"  // TODO : Only for test
#define INV_NXDN_SYNC3            "1131331113"  // TODO : Only for test

#define DMR_BS_DATA_SYNC  "313333111331131131331131"
#define DMR_BS_VOICE_SYNC "131111333113313313113313"
#define DMR_MS_DATA_SYNC  "311131133313133331131113"
#define DMR_MS_VOICE_SYNC "133313311131311113313331"
#define DMR_DIRECT_MODE_TS1_DATA_SYNC  "331333313111313133311111"
#define DMR_DIRECT_MODE_TS1_VOICE_SYNC "113111131333131311333333"
#define DMR_DIRECT_MODE_TS2_DATA_SYNC  "311311111333113333133311"
#define DMR_DIRECT_MODE_TS2_VOICE_SYNC "133133333111331111311133"

#define INV_PROVOICE_SYNC    "31313111333133133311331133113311"
#define PROVOICE_SYNC        "13131333111311311133113311331133"
#define INV_PROVOICE_EA_SYNC "13313133113113333311313133133311"
#define PROVOICE_EA_SYNC     "31131311331331111133131311311133"

/* dPMR Frame Sync 1 - 48 bits sequence
 * HEX    : 57 FF 5F 75 D5 77
 * Binary : 0101 0111 1111 1111 0101 1111 0111 0101 1101 0101 0111 0111
 * Dibit  :  1 1  1 3  3 3  3 3  1 1  3 3  1 3  1 1  3 1  1 1  1 3  1 3 */
#define DPMR_FRAME_SYNC_1     "111333331133131131111313"

/* dPMR Frame Sync 2 - 24 bits sequence
 * HEX    : 5F F7 7D
 * Binary : 0101 1111 1111 0111 0111 1101
 * Dibit  :  1 1  3 3  3 3  1 3  1 3  3 1 */
#define DPMR_FRAME_SYNC_2     "113333131331"

/* dPMR Frame Sync 3 - 24 bits sequence
 * HEX    : 7D DF F5
 * Binary : 0111 1101 1101 1111 1111 0101
 * Dibit  :  1 3  3 1  3 1  3 3  3 3  1 1 */
#define DPMR_FRAME_SYNC_3     "133131333311"

/* dPMR Frame Sync 4 - 48 bits sequence
 * HEX    : FD 55 F5 DF 7F DD
 * Binary : 1111 1101 0101 0101 1111 0101 1101 1111 0111 1111 1101 1101
 * Dibit  :  3 3  3 1  1 1  1 1  3 3  1 1  3 1  3 3  1 3  3 3  3 1  3 1 */
#define DPMR_FRAME_SYNC_4     "333111113311313313333131"

/* dPMR Frame Sync 1 to 4 - Inverted */
#define INV_DPMR_FRAME_SYNC_1 "333111113311313313333131"
#define INV_DPMR_FRAME_SYNC_2 "331111313113"
#define INV_DPMR_FRAME_SYNC_3 "311313111133"
#define INV_DPMR_FRAME_SYNC_4 "111333331133131131111313"


/* Sample per symbol constants */
#define SAMPLE_PER_SYMBOL_DMR     (SAMPLE_RATE_IN / 4800)
#define SAMPLE_PER_SYMBOL_P25     SAMPLE_PER_SYMBOL_DMR
#define SAMPLE_PER_SYMBOL_NXDN48  (SAMPLE_RATE_IN / 2400)
#define SAMPLE_PER_SYMBOL_NXDN96  (SAMPLE_RATE_IN / 9600)
#define SAMPLE_PER_SYMBOL_DPMR    SAMPLE_PER_SYMBOL_NXDN48


enum
{
  C4FM_MODE = 0,
  QPSK_MODE = 1,
  GFSK_MODE = 2,
  UNKNOWN_MODE
};


/*
 * function prototypes
 */
void processDMRdata (dsd_opts * opts, dsd_state * state);
void processDMRvoice (dsd_opts * opts, dsd_state * state);
void processdPMRvoice (dsd_opts * opts, dsd_state * state);
void processAudio (dsd_opts * opts, dsd_state * state);
void writeSynthesizedVoice (dsd_opts * opts, dsd_state * state);
void playSynthesizedVoice (dsd_opts * opts, dsd_state * state);
void openAudioOutDevice (dsd_opts * opts, int speed);
void openAudioInDevice (dsd_opts * opts);
void getCurrentTime (dsd_opts * opts, dsd_state * state);

int getDibit (dsd_opts * opts, dsd_state * state);
int get_dibit_and_analog_signal (dsd_opts * opts, dsd_state * state, int * out_analog_signal);

void skipDibit (dsd_opts * opts, dsd_state * state, int count);
void saveImbe4400Data (dsd_opts * opts, dsd_state * state, char *imbe_d);
void saveAmbe2450Data (dsd_opts * opts, dsd_state * state, char *ambe_d);
int readImbe4400Data (dsd_opts * opts, dsd_state * state, char *imbe_d);
int readAmbe2450Data (dsd_opts * opts, dsd_state * state, char *ambe_d);
void openMbeInFile (dsd_opts * opts, dsd_state * state);
void closeMbeOutFile (dsd_opts * opts, dsd_state * state);
void openMbeOutFile (dsd_opts * opts, dsd_state * state);
void openWavOutFile (dsd_opts * opts, dsd_state * state);
void closeWavOutFile (dsd_opts * opts, dsd_state * state);
void printFrameInfo (dsd_opts * opts, dsd_state * state);
void processFrame (dsd_opts * opts, dsd_state * state);
void printFrameSync (dsd_opts * opts, dsd_state * state, char *frametype, int offset, char *modulation);
int getFrameSync (dsd_opts * opts, dsd_state * state);
int comp (const void *a, const void *b);
void noCarrier (dsd_opts * opts, dsd_state * state);
void initOpts (dsd_opts * opts);
void initState (dsd_state * state);
void usage ();
void liveScanner (dsd_opts * opts, dsd_state * state);
void freeAllocatedMemory(dsd_opts * opts, dsd_state * state);
void cleanupAndExit (dsd_opts * opts, dsd_state * state);
void sigfun (int sig);
int main (int argc, char **argv);
void playMbeFiles (dsd_opts * opts, dsd_state * state, int argc, char **argv);
void processMbeFrame (dsd_opts * opts, dsd_state * state, char imbe_fr[8][23], char ambe_fr[4][24], char imbe7100_fr[7][24]);
void processMbeFrameEncrypted (dsd_opts * opts, dsd_state * state, char imbe_fr[8][23], char ambe_fr[4][24], char imbe7100_fr[7][24], char ambe_keystream[49], char imbe_keystream[88]);
void openSerial (dsd_opts * opts, dsd_state * state);
void resumeScan (dsd_opts * opts, dsd_state * state);
int getSymbol (dsd_opts * opts, dsd_state * state, int have_sync);
void upsample (dsd_state * state, float invalue);
void processDSTAR (dsd_opts * opts, dsd_state * state);
void processNXDNVoice (dsd_opts * opts, dsd_state * state);
void processNXDNData (dsd_opts * opts, dsd_state * state);
void processP25lcw (dsd_opts * opts, dsd_state * state, char *lcformat, char *mfid, char *lcinfo);
void processHDU (dsd_opts * opts, dsd_state * state);
void processLDU1 (dsd_opts * opts, dsd_state * state);
void processLDU2 (dsd_opts * opts, dsd_state * state);
void processTDU (dsd_opts * opts, dsd_state * state);
void processTDULC (dsd_opts * opts, dsd_state * state);
void processProVoice (dsd_opts * opts, dsd_state * state);
void processX2TDMAdata (dsd_opts * opts, dsd_state * state);
void processX2TDMAvoice (dsd_opts * opts, dsd_state * state);
void processDSTAR_HD (dsd_opts * opts, dsd_state * state);
short dmr_filter(short sample);
short nxdn_filter(short sample);

/* DMR print frame functions */
void printDMRRawVoiceFrame (dsd_opts * opts, dsd_state * state);
void printDMRRawDataFrame (dsd_opts * opts, dsd_state * state);
void printExtractedDMRDataFrame(dsd_opts * opts, dsd_state * state);
void printDMRAmbeVoiceSample(dsd_opts * opts, dsd_state * state);

/* dPMR print frame function */
void printdPMRAmbeVoiceSample(dsd_opts * opts, dsd_state * state);
void printdPMRRawVoiceFrame (dsd_opts * opts, dsd_state * state);

void DMRDataFrameProcess(dsd_opts * opts, dsd_state * state);
void DMRVoiceFrameProcess(dsd_opts * opts, dsd_state * state);
void dPMRVoiceFrameProcess(dsd_opts * opts, dsd_state * state);

void DisplaySpecialFormat(dsd_opts * opts, dsd_state * state);


/* dPMR functions */
void ScrambledPMRBit(uint32_t * LfsrValue, uint8_t * BufferIn, uint8_t * BufferOut, uint32_t NbOfBitToScramble);
void DeInterleave6x12DPmrBit(uint8_t * BufferIn, uint8_t * BufferOut);
uint8_t CRC7BitdPMR(uint8_t * BufferIn, uint32_t BitLength);
uint8_t CRC8BitdPMR(uint8_t * BufferIn, uint32_t BitLength);
void ConvertAirInterfaceID(uint32_t AI_ID, uint8_t ID[8]);
int32_t GetdPmrColorCode(uint8_t ChannelCodeBit[24]);


/* NXDN functions */
void CNXDNConvolution_start(void);
void CNXDNConvolution_decode(uint8_t s0, uint8_t s1);
void CNXDNConvolution_chainback(unsigned char* out, unsigned int nBits);
void CNXDNConvolution_encode(const unsigned char* in, unsigned char* out, unsigned int nBits);
uint8_t NXDN_SACCH_raw_part_decode(uint8_t * Input, uint8_t * Output);
void NXDN_SACCH_Full_decode(dsd_opts * opts, dsd_state * state);
void NXDN_decode_VCALL(dsd_opts * opts, dsd_state * state, uint8_t * Message);
void NXDN_decode_VCALL_IV(dsd_opts * opts, dsd_state * state, uint8_t * Message);
char * NXDN_Call_Type_To_Str(uint8_t CallType);
void NXDN_Voice_Call_Option_To_Str(uint8_t VoiceCallOption, uint8_t * Duplex, uint8_t * TransmissionMode);
char * NXDN_Cipher_Type_To_Str(uint8_t CipherType);
uint8_t CRC6BitdNXDN(uint8_t * BufferIn, uint32_t BitLength);
void ScrambledNXDNVoiceBit(int * LfsrValue, char * BufferIn, char * BufferOut, int NbOfBitToScramble);
void NxdnEncryptionStreamGeneration (dsd_opts* opts, dsd_state* state, uint8_t KeyStream[1664]);



/* DMR encryption functions */
void ProcessDMREncryption (dsd_opts * opts, dsd_state * state);


/* Global functions */
uint32_t ConvertBitIntoBytes(uint8_t * BufferIn, uint32_t BitLength);
uint32_t ConvertAsciiToByte(uint8_t AsciiMsbByte, uint8_t AsciiLsbByte, uint8_t * OutputByte);
void Convert49BitSampleInto7Bytes(char * InputBit, char * OutputByte);
void Convert7BytesInto49BitSample(char * InputByte, char * OutputBit);


/* FEC correction */
extern unsigned char Hamming_7_4_m_corr[8];             //!< single bit error correction by syndrome index
extern const unsigned char Hamming_7_4_m_G[7*4]; //!< Generator matrix of bits
extern const unsigned char Hamming_7_4_m_H[7*3]; //!< Parity check matrix of bits

extern unsigned char Hamming_12_8_m_corr[16];             //!< single bit error correction by syndrome index
extern const unsigned char Hamming_12_8_m_G[12*8]; //!< Generator matrix of bits
extern const unsigned char Hamming_12_8_m_H[12*4]; //!< Parity check matrix of bits

extern unsigned char Hamming_13_9_m_corr[16];             //!< single bit error correction by syndrome index
extern const unsigned char Hamming_13_9_m_G[13*9]; //!< Generator matrix of bits
extern const unsigned char Hamming_13_9_m_H[13*4]; //!< Parity check matrix of bits

extern unsigned char Hamming_15_11_m_corr[16];              //!< single bit error correction by syndrome index
extern const unsigned char Hamming_15_11_m_G[15*11]; //!< Generator matrix of bits
extern const unsigned char Hamming_15_11_m_H[15*4];  //!< Parity check matrix of bits

extern unsigned char Hamming_16_11_4_m_corr[32];              //!< single bit error correction by syndrome index
extern const unsigned char Hamming_16_11_4_m_G[16*11]; //!< Generator matrix of bits
extern const unsigned char Hamming_16_11_4_m_H[16*5];  //!< Parity check matrix of bits

extern unsigned char Golay_20_8_m_corr[4096][3];         //!< up to 3 bit error correction by syndrome index
extern const unsigned char Golay_20_8_m_G[20*8];  //!< Generator matrix of bits
extern const unsigned char Golay_20_8_m_H[20*12]; //!< Parity check matrix of bits

extern unsigned char Golay_23_12_m_corr[2048][3];         //!< up to 3 bit error correction by syndrome index
extern const unsigned char Golay_23_12_m_G[23*12]; //!< Generator matrix of bits
extern const unsigned char Golay_23_12_m_H[23*11]; //!< Parity check matrix of bits

extern unsigned char Golay_24_12_m_corr[4096][3];         //!< up to 3 bit error correction by syndrome index
extern const unsigned char Golay_24_12_m_G[24*12]; //!< Generator matrix of bits
extern const unsigned char Golay_24_12_m_H[24*12]; //!< Parity check matrix of bits

extern unsigned char QR_16_7_6_m_corr[512][2];          //!< up to 2 bit error correction by syndrome index
extern const unsigned char QR_16_7_6_m_G[16*7];  //!< Generator matrix of bits
extern const unsigned char QR_16_7_6_m_H[16*9];  //!< Parity check matrix of bits


void Hamming_7_4_init();
void Hamming_7_4_encode(unsigned char *origBits, unsigned char *encodedBits);
bool Hamming_7_4_decode(unsigned char *rxBits);

void Hamming_12_8_init();
void Hamming_12_8_encode(unsigned char *origBits, unsigned char *encodedBits);
bool Hamming_12_8_decode(unsigned char *rxBits, unsigned char *decodedBits, int nbCodewords);

void Hamming_13_9_init();
void Hamming_13_9_encode(unsigned char *origBits, unsigned char *encodedBits);
bool Hamming_13_9_decode(unsigned char *rxBits, unsigned char *decodedBits, int nbCodewords);

void Hamming_15_11_init();
void Hamming_15_11_encode(unsigned char *origBits, unsigned char *encodedBits);
bool Hamming_15_11_decode(unsigned char *rxBits, unsigned char *decodedBits, int nbCodewords);

void Hamming_16_11_4_init();
void Hamming_16_11_4_encode(unsigned char *origBits, unsigned char *encodedBits);
bool Hamming_16_11_4_decode(unsigned char *rxBits, unsigned char *decodedBits, int nbCodewords);

void Golay_20_8_init();
void Golay_20_8_encode(unsigned char *origBits, unsigned char *encodedBits);
bool Golay_20_8_decode(unsigned char *rxBits);

void Golay_23_12_init();
void Golay_23_12_encode(unsigned char *origBits, unsigned char *encodedBits);
bool Golay_23_12_decode(unsigned char *rxBits);

void Golay_24_12_init();
void Golay_24_12_encode(unsigned char *origBits, unsigned char *encodedBits);
bool Golay_24_12_decode(unsigned char *rxBits);

void QR_16_7_6_init();
void QR_16_7_6_encode(unsigned char *origBits, unsigned char *encodedBits);
bool QR_16_7_6_decode(unsigned char *rxBits);

void InitAllFecFunction(void);


/* BPTC (Block Product Turbo Code) variables */
extern const uint8_t BPTCInterleavingIndex[196];
extern const uint8_t BPTCDeInterleavingIndex[196];


/* BPTC (Block Product Turbo Code) functions */
void BPTCDeInterleaveDMRData(uint8_t * Input, uint8_t * Output);
uint32_t BPTC_196x96_Extract_Data(uint8_t InputDeInteleavedData[196], uint8_t DMRDataExtracted[96], uint8_t R[3]);
uint32_t BPTC_128x77_Extract_Data(uint8_t InputDataMatrix[8][16], uint8_t DMRDataExtracted[77]);
uint32_t BPTC_16x2_Extract_Data(uint8_t InputInterleavedData[32], uint8_t DMRDataExtracted[32]);


/* SYNC DMR data extraction functions */
void ProcessDmrVoiceLcHeader(dsd_opts * opts, dsd_state * state, uint8_t info[196], uint8_t syncdata[48], uint8_t SlotType[20]);
void ProcessDmrTerminaisonLC(dsd_opts * opts, dsd_state * state, uint8_t info[196], uint8_t syncdata[48], uint8_t SlotType[20]);
void ProcessVoiceBurstSync(dsd_opts * opts, dsd_state * state);
uint16_t ComputeCrcCCITT(uint8_t * DMRData);
uint32_t ComputeAndCorrectFullLinkControlCrc(uint8_t * FullLinkControlDataBytes, uint32_t * CRCComputed, uint32_t CRCMask);
uint8_t ComputeCrc5Bit(uint8_t * DMRData);
uint8_t * DmrAlgIdToStr(uint8_t AlgID);
uint8_t * DmrAlgPrivacyModeToStr(uint32_t PrivacyMode);


/* Read Solomon (12,9) functions */
void rs_12_9_calc_syndrome(rs_12_9_codeword_t *codeword, rs_12_9_poly_t *syndrome);
uint8_t rs_12_9_check_syndrome(rs_12_9_poly_t *syndrome);
rs_12_9_correct_errors_result_t rs_12_9_correct_errors(rs_12_9_codeword_t *codeword, rs_12_9_poly_t *syndrome, uint8_t *errors_found);
rs_12_9_checksum_t *rs_12_9_calc_checksum(rs_12_9_codeword_t *codeword);


#ifdef __cplusplus
}
#endif

#endif /* DSD_H */
