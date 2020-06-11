/*
 ============================================================================
 Name        : my_lib.c
 Author      : MotoTRBO_Encrypted_DSD_Author
 Version     : 1.0
 Date        : 2017 - February - 11
 Copyright   : None
 Description : Personal library source
 ============================================================================
 */


/* Include ------------------------------------------------------------------*/
#include "dsd.h"


/* Define -------------------------------------------------------------------*/


/* Global variables ---------------------------------------------------------*/


/* Functions ----------------------------------------------------------------*/


/*
 * @brief : This function extract and prints the content of a data frame structure
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void DMRDataFrameProcess(dsd_opts * opts, dsd_state * state)
{
  /* Remove warning compiler */
  UNUSED_VARIABLE(opts);
  UNUSED_VARIABLE(state);
  /* Print the raw content of the DMR frame */
  //printDMRRawDataFrame(opts, state);
} /* End DMRDataFrameProcess() */


/*
 * @brief : This function extract and prints the content of a voice frame
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void DMRVoiceFrameProcess(dsd_opts * opts, dsd_state * state)
{
  DisplaySpecialFormat(opts, state);
  printDMRRawVoiceFrame(opts, state);
  printDMRAmbeVoiceSample(opts, state);
} /* End DMRVoiceFrameProcess() */


/*
 * @brief : This function extract and prints the content of a voice frame
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void dPMRVoiceFrameProcess(dsd_opts * opts, dsd_state * state)
{
  printdPMRAmbeVoiceSample(opts, state);
} /* End DMRVoiceFrameProcess() */


/*
 * @brief : This function prints the content of a voice frame structure
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void printDMRRawVoiceFrame (dsd_opts * opts, dsd_state * state)
{
  int i, j, Frame;
  char Byte;
  TimeSlotVoiceSuperFrame_t * TSVoiceSupFrame = NULL;
  char SuperFrameBuffer[6][264];

  /* Remove warning compiler */
  UNUSED_VARIABLE(opts);

  i = 0;
  j = 0;
  Byte = 0;

  /* Check the current time slot */
  if(state->currentslot == 0)
  {
    TSVoiceSupFrame = &state->TS1SuperFrame;
  }
  else
  {
    TSVoiceSupFrame = &state->TS2SuperFrame;
  }

  /* Fill the 264 bit of the frame */
  for(Frame = 0; Frame < 6; Frame++)
  {
    j = 0;

    /* 1st voice frame (72 bit) */
    for(i = 0; i < 72; i++)
    {
      SuperFrameBuffer[Frame][j++] = TSVoiceSupFrame->TimeSlotRawVoiceFrame[Frame].VoiceFrame[0][i];
    }

    /* 2nd voice frame (first half - 36 bit) */
    for(i = 0; i < 36; i++)
    {
      SuperFrameBuffer[Frame][j++] = TSVoiceSupFrame->TimeSlotRawVoiceFrame[Frame].VoiceFrame[1][i];
    }

    /* SYNC (48 bit) */
    for(i = 0; i < 48; i++)
    {
      SuperFrameBuffer[Frame][j++] = TSVoiceSupFrame->TimeSlotRawVoiceFrame[Frame].Sync[i];
    }

    /* 2nd voice frame (second half - 36 bit) */
    for(i = 36; i < 72; i++)
    {
      SuperFrameBuffer[Frame][j++] = TSVoiceSupFrame->TimeSlotRawVoiceFrame[Frame].VoiceFrame[1][i];
    }

    /* 3rd voice frame (72 bit) */
    for(i = 0; i < 72; i++)
    {
      SuperFrameBuffer[Frame][j++] = TSVoiceSupFrame->TimeSlotRawVoiceFrame[Frame].VoiceFrame[2][i];
    }
  }

  /* 6 superframe to display */
  for(Frame = 0; Frame < 6; Frame++)
  {
    /* Check if the output format is hexadecimal */
    if(state->printDMRRawVoiceFrameHex)
    {
      /* Print the voice frame data number */
      printf("%d/6 Raw Voice Frame = ", Frame + 1);

      /* Print the 264 bit of the voice frame (total 33 bytes) */
      for(i = 0, j = 0; i < (264/8); i++)
      {
        Byte = 0;

        if(i != 0) printf(" - "); // Print a "-" between each byte

        if(SuperFrameBuffer[Frame][j++]) Byte |= 0x80;
        if(SuperFrameBuffer[Frame][j++]) Byte |= 0x40;
        if(SuperFrameBuffer[Frame][j++]) Byte |= 0x20;
        if(SuperFrameBuffer[Frame][j++]) Byte |= 0x10;
        if(SuperFrameBuffer[Frame][j++]) Byte |= 0x08;
        if(SuperFrameBuffer[Frame][j++]) Byte |= 0x04;
        if(SuperFrameBuffer[Frame][j++]) Byte |= 0x02;
        if(SuperFrameBuffer[Frame][j++]) Byte |= 0x01;

        printf("0x%02X", Byte & 0xFF); // Print the byte
      }
      printf("\n");
    }

    /* Check if the output format is binary */
    if(state->printDMRRawVoiceFrameBin)
    {
      /* Print the voice frame data number */
      printf("%d/6 Raw Voice Frame = ", j + 1);

      /* Print all 264 bit of the frame (total 33 bytes) */
      for(i = 0, j = 0; i < (264 / 8); i++)
      {
        if(i != 0) printf(" - "); // Print a "-" between each byte

        if(SuperFrameBuffer[Frame][j++]) printf("1");
        else printf("0");
        if(SuperFrameBuffer[Frame][j++]) printf("1");
        else printf("0");
        if(SuperFrameBuffer[Frame][j++]) printf("1");
        else printf("0");
        if(SuperFrameBuffer[Frame][j++]) printf("1");
        else printf("0");
        if(SuperFrameBuffer[Frame][j++]) printf("1");
        else printf("0");
        if(SuperFrameBuffer[Frame][j++]) printf("1");
        else printf("0");
        if(SuperFrameBuffer[Frame][j++]) printf("1");
        else printf("0");
        if(SuperFrameBuffer[Frame][j++]) printf("1");
        else printf("0");
      }
      printf("\n");
    }
  }
} /* End printDMRRawVoiceFrame() */


/*
 * @brief : This function prints the content of a date frame structure
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void printDMRRawDataFrame (dsd_opts * opts, dsd_state * state)
{
  /* Remove warning compiler */
  UNUSED_VARIABLE(opts);
  UNUSED_VARIABLE(state);

  // TODO : To be implement
} /* End printDMRRawDataFrame() */


/*
 * @brief : This function prints the content of a data frame structure
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void printExtractedDMRDataFrame(dsd_opts * opts, dsd_state * state)
{
  /* Remove warning compiler */
  UNUSED_VARIABLE(opts);
  UNUSED_VARIABLE(state);

  // TODO : To be implement
} /* End printExtractedDMRDataFrame() */


/*
 * @brief : This function prints the content of the AMBE sample of
 *          a voice superframe
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void printDMRAmbeVoiceSample(dsd_opts * opts, dsd_state * state)
{
  int i, j, Frame;
  char Byte;
  TimeSlotVoiceSuperFrame_t * TSVoiceSupFrame = NULL;

  /* Remove warning compiler */
  UNUSED_VARIABLE(opts);

  i = 0;
  j = 0;
  Byte = 0;

  /* Check the current time slot */
  if(state->currentslot == 0)
  {
    TSVoiceSupFrame = &state->TS1SuperFrame;
  }
  else
  {
    TSVoiceSupFrame = &state->TS2SuperFrame;
  }

  if((state->printDMRAmbeVoiceSampleHex) || (state->printDMRAmbeVoiceSampleBin))
  {
    printf("\n");

    /* A voice superframe contains 6 voice frames */
    for(Frame = 0; Frame < 6; Frame++)
    {
      /* Check if the output format is hexadecimal */
      if(state->printDMRAmbeVoiceSampleHex)
      {
        /* Print the voice frame data number */
        printf("%d/6 AMBE Voice Frame :\n", Frame + 1);

        /* 3 AMBE Sample of 49 bit each */
        for(i = 0; i < 3; i++)
        {
          printf("E1 = %d, E2 = %d, AMBE Sample #%d = ",
                 TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].errs1[i],
                 TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].errs2[i],
                 i + 1);

          for(j = 0; j < 48;)
          {
            Byte = 0;
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x80;
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x40;
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x20;
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x10;
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x08;
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x04;
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x02;
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x01;
            printf("0x%02X - ", Byte & 0xFF); // Print the byte
          }

          /* Print the 49th bit */
          Byte = 0;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][48]) Byte |= 0x01;
          printf("0x%02X\n", Byte & 0xFF); // Print the byte
        }

        //printf("\n");
      } /* End if(state->printDMRAmbeVoiceSampleHex) */

      /* Check if the output format is binary */
      if(state->printDMRAmbeVoiceSampleBin)
      {
        /* Print the voice frame data number */
        printf("%d/6 AMBE Voice Frame = ", Frame + 1);

        /* 3 AMBE Sample of 49 bit each */
        for(i = 0; i < 3; i++)
        {
          printf("E1 = %d, E2 = %d, AMBE Sample #%d = ",
                 TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].errs1[i],
                 TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].errs2[i],
                 i + 1);

          for(j = 0; j < 48; )
          {
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) printf("1");
            else printf("0");
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) printf("1");
            else printf("0");
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) printf("1");
            else printf("0");
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) printf("1");
            else printf("0");
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) printf("1");
            else printf("0");
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) printf("1");
            else printf("0");
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) printf("1");
            else printf("0");
            if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) printf("1");
            else printf("0");
            printf(" - "); // Print a "-"
          }

          /* Print the 49th bit */
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][48]) printf("1");
          else printf("0");
          printf("\n");
        }

        printf("\n");
      } /* Endif(state->printDMRAmbeVoiceSampleBin) */
    } /* End for(Frame = 0; Frame < 6; Frame++) */
  } /* End if((state->printDMRAmbeVoiceSampleHex) || (state->printDMRAmbeVoiceSampleBin)) */

} /* End printDMRAmbeVoiceSample() */


/*
 * @brief : This function prints the content of the AMBE sample of
 *          a voice superframe
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void printdPMRAmbeVoiceSample(dsd_opts * opts, dsd_state * state)
{
  int i, j;
  char Byte;

  /* Remove warning compiler */
  UNUSED_VARIABLE(opts);

  i = 0;
  j = 0;
  Byte = 0;

  /* Check if the output format is hexadecimal */
  if(state->printdPMRAmbeVoiceSampleHex)
  {
    printf("\n");

    /* 4x49 bit AMBE Sample in a voice frame (16 per superframe) */
    for(i = 0; i < (NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4); i++)
    {
      /* Print the voice frame data number */
      printf("%d/%d AMBE Voice Frame :\n", i + 1, NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4);

      printf("E1 = %d, E2 = %d, AMBE Sample #%d = ",
             state->dPMRVoiceFS2Frame.errs1[i],
             state->dPMRVoiceFS2Frame.errs2[i],
             i + 1);

      for(j = 0; j < 48;)
      {
        Byte = 0;
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) Byte |= 0x80;
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) Byte |= 0x40;
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) Byte |= 0x20;
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) Byte |= 0x10;
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) Byte |= 0x08;
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) Byte |= 0x04;
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) Byte |= 0x02;
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) Byte |= 0x01;
        printf("0x%02X - ", Byte & 0xFF); // Print the byte
      }

      /* Print the 49th bit */
      Byte = 0;
      if(state->dPMRVoiceFS2Frame.AmbeBit[i][48]) Byte |= 0x01;
      printf("0x%02X\n", Byte & 0xFF); // Print the byte
    }
  } /* End if(state->printdPMRAmbeVoiceSampleHex) */

  /* Check if the output format is binary */
  if(state->printdPMRAmbeVoiceSampleBin)
  {
    printf("\n");

    /* 4x49 bit AMBE Sample in a voice frame (16 per superframe) */
    for(i = 0; i < (NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4); i++)
    {
      /* Print the voice frame data number */
      printf("%d/%d AMBE Voice Frame :\n", i + 1, NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4);

      printf("E1 = %d, E2 = %d, AMBE Sample #%d = ",
             state->dPMRVoiceFS2Frame.errs1[i],
             state->dPMRVoiceFS2Frame.errs2[i],
             i + 1);

      for(j = 0; j < 48; )
      {
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.AmbeBit[i][j++]) printf("1");
        else printf("0");
        printf(" - "); // Print a "-"
      }

      /* Print the 49th bit */
      if(state->dPMRVoiceFS2Frame.AmbeBit[i][48]) printf("1");
      else printf("0");
      printf("\n");
    }

    printf("\n");
  }
  //printf("\n");
} /* End printdPMRAmbeVoiceSample() */


/*
 * @brief : This function prints the content of a voice frame structure
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void printdPMRRawVoiceFrame (dsd_opts * opts, dsd_state * state)
{
  int i, j, Frame;
  char Byte;

  /* Remove warning compiler */
  UNUSED_VARIABLE(opts);

  i = 0;
  j = 0;
  Byte = 0;

  /* Check if the output format is hexadecimal */
  if(state->printdPMRRawVoiceFrameHex)
  {
    /* 8 or 16 frame to display */
    for(Frame = 0; Frame < (NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4); Frame++)
    {
      /* Print the voice frame data number */
      printf("%d/%d AMBE Voice Frame :\n", i + 1, NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4);

      /* Print the 72 bit of each voice frame (total 9 bytes per AMBE sample) */
      for(i = 0, j = 0; i < (72/8); i++)
      {
        Byte = 0;

        if(i != 0) printf(" - "); // Print a "-" between each byte

        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) Byte |= 0x80;
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) Byte |= 0x40;
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) Byte |= 0x20;
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) Byte |= 0x10;
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) Byte |= 0x08;
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) Byte |= 0x04;
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) Byte |= 0x02;
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) Byte |= 0x01;

        printf("0x%02X", Byte & 0xFF); // Print the byte
      }
      printf("\n");
    }
  } /* End if(state->printdPMRRawVoiceFrameHex) */

  /* Check if the output format is binary */
  if(state->printdPMRRawVoiceFrameBin)
  {
    /* 16 frame to display */
    for(Frame = 0; Frame < (NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4); Frame++)
    {
      /* Print the voice frame data number */
      printf("%d/%d AMBE Voice Frame :\n", i + 1, NB_OF_DPMR_VOICE_FRAME_TO_DECODE * 4);

      /* Print the 72 bit of each voice frame (total 9 bytes per AMBE sample) */
      for(i = 0, j = 0; i < (72 / 8); i++)
      {
        if(i != 0) printf(" - "); // Print a "-" between each byte

        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) printf("1");
        else printf("0");
        if(state->dPMRVoiceFS2Frame.RawVoiceBit[Frame][j++]) printf("1");
        else printf("0");
      }
      printf("\n");
    }
  } /* End if(state->printdPMRRawVoiceFrameBin) */
} /* End printdPMRRawVoiceFrame() */

/*
 * @brief : This function prints the content of the AMBE sample of
 *          a voice superframe in a special formal : No space between
 *          data
 *
 * @param opts : Pointer of option structure parameters
 *
 * @param state : Pointer of state structure parameters
 *
 * @return None
 */
void DisplaySpecialFormat(dsd_opts * opts, dsd_state * state)
{
  int i, j, Frame;
  char Byte;
  TimeSlotVoiceSuperFrame_t * TSVoiceSupFrame = NULL;

  /* Remove warning compiler */
  UNUSED_VARIABLE(opts);

  i = 0;
  j = 0;
  Byte = 0;

  /* Check the current time slot */
  if(state->currentslot == 0)
  {
    TSVoiceSupFrame = &state->TS1SuperFrame;
  }
  else
  {
    TSVoiceSupFrame = &state->TS2SuperFrame;
  }

  if(state->special_display_format_enable)
  {
    printf("\n");
    printf("AMBE frame in special format\n");

    /* A voice superframe contains 6 voice frames */
    for(Frame = 0; Frame < 6; Frame++)
    {
      /* 3 AMBE Sample of 49 bit each */
      for(i = 0; i < 3; i++)
      {
        /* Display the first 48 bit of an AMBE frame */
        for(j = 0; j < 48;)
        {
          Byte = 0;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x80;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x40;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x20;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x10;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x08;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x04;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x02;
          if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][j++]) Byte |= 0x01;
          printf("%02X", Byte & 0xFF); // Print the byte
        }

        /* Print the last 49th bit */
        Byte = 0;
        if(TSVoiceSupFrame->TimeSlotAmbeVoiceFrame[Frame].AmbeBit[i][48]) Byte |= 0x01;
        printf("%02X", Byte & 0xFF); // Print the byte
      }
    } /* End for(Frame = 0; Frame < 6; Frame++) */
    printf("\n");
  } /* End if(state->special_display_format_enable) */
} /* End DisplaySpecialFormat() */


/* Convert a bitfield into a 32 bytes integer */
/*
 * @brief : This function Convert a bitfield into a 32 bytes integer
 *
 * @param BufferIn : Pointer of the bitfield
 *
 * @param BitLength : Number of bit in the bitfield
 *
 * @return The bitfield converted into a 4 bytes unsigned number
 */
uint32_t ConvertBitIntoBytes(uint8_t * BufferIn, uint32_t BitLength)
{
  uint32_t Output = 0;
  uint32_t i;

  for(i = 0; i < BitLength; i++)
  {
    Output <<= 1;
    Output |= (uint32_t)(BufferIn[i] & 1);
  }

  return Output;
} /* End ConvertBitIntoBytes() */


/*
 * @brief : This function convert two ASCII bytes into one byte
 *          Example : 'A' (0x41) and '9' (0x39) will give an
 *                    output byte value of 0xA9
 *
 * @param AsciiMsbByte : The ASCII byte of the MSByte
 *   @arg Range ['0'..'9'] or ['a'..'f'] or ['A'..'F']
 *
 * @param AsciiLsbByte : The ASCII byte of the LSByte
 *   @arg Range ['0'..'9'] or ['a'..'f'] or ['A'..'F']
 *
 * @param OutputByte : The output pointer of the generated byte
 *
 * @return 0 = Conversion OK
 *         1 = Error => The input numbers are not in the correct range
 */
uint32_t ConvertAsciiToByte(uint8_t AsciiMsbByte, uint8_t AsciiLsbByte, uint8_t * OutputByte)
{
  uint32_t ErrorFlag = 0;
  uint8_t  Temp = 0;

  *OutputByte = 0;

  /* Check if the MSB byte is in the range ['0'..'9'] */
  if((AsciiMsbByte >= '0') && (AsciiMsbByte <= '9'))
  {
    Temp = ((AsciiMsbByte - '0') << 4) & 0xF0;
  }
  /* Check if the MSB byte is in the range ['A'..'F'] */
  else if((AsciiMsbByte >= 'A') && (AsciiMsbByte <= 'F'))
  {
    Temp = ((AsciiMsbByte - 'A' + 0x0A) << 4) & 0xF0;
  }
  /* Check if the MSB byte is in the range ['a'..'f'] */
  else if((AsciiMsbByte >= 'a') && (AsciiMsbByte <= 'f'))
  {
    Temp = ((AsciiMsbByte - 'a'  + 0x0A) << 4) & 0xF0;
  }
  else
  {
    /* Not an ASCII byte in the range ['0'..'9']['a'..'f']['A'..'F'],
     * set the error flag */
    ErrorFlag = 1;
  }

  /* Check if the MSB byte is in the range ['0'..'9'] */
  if((AsciiLsbByte >= '0') && (AsciiLsbByte <= '9'))
  {
    Temp |= ((AsciiLsbByte - '0') & 0x0F);
  }
  /* Check if the MSB byte is in the range ['A'..'F'] */
  else if((AsciiLsbByte >= 'A') && (AsciiLsbByte <= 'F'))
  {
    Temp |= ((AsciiLsbByte - 'A' + 0x0A) & 0x0F);
  }
  /* Check if the MSB byte is in the range ['a'..'f'] */
  else if((AsciiLsbByte >= 'a') && (AsciiLsbByte <= 'f'))
  {
    Temp |= ((AsciiLsbByte - 'a' + 0x0A) & 0x0F);
  }
  else
  {
    /* Not an ASCII byte in the range ['0'..'9']['a'..'f']['A'..'F'],
     * set the error flag */
    ErrorFlag = 1;
  }

  /* Fill the output byte */
  *OutputByte = Temp;

  return ErrorFlag;
} /* End ConvertAsciiToByte() */


/*
 * @brief : This function convert 49 bit AMBE sample
 *          into 7 byte.
 *
 * @param InputBit : 49 bit AMBE sample
 *
 * @param OutputByte : 49 bit AMBE sample converted
 *                     into 7 bytes
 *
 * @return None
 */
void Convert49BitSampleInto7Bytes(char * InputBit, char * OutputByte)
{
  int i, j;
  char Byte;

  j = 0;
  for(i = 0; i < 48;)
  {
    Byte = 0;
    if(InputBit[i++]) Byte |= 0x80;
    if(InputBit[i++]) Byte |= 0x40;
    if(InputBit[i++]) Byte |= 0x20;
    if(InputBit[i++]) Byte |= 0x10;
    if(InputBit[i++]) Byte |= 0x08;
    if(InputBit[i++]) Byte |= 0x04;
    if(InputBit[i++]) Byte |= 0x02;
    if(InputBit[i++]) Byte |= 0x01;
    OutputByte[j++] = Byte;
  }

  /* Print the 49th bit */
  Byte = 0;
  if(InputBit[i++]) Byte |= 0x01;
  OutputByte[j++] = Byte;
} /* End Convert49BitSampleInto7Byte() */


/*
 * @brief : This function convert a 7 byte input buffer into
 *          49 bit of AMBE sample.
 *
 * @param InputByte : The 49 bit AMBE sample into 7 bytes buffer
 *
 * @param OutputBit : The 49 bit AMBE sample converted bit by bit
 *                   (into 49 unsigned char buffer stream).
 *
 * @return None
 */
void Convert7BytesInto49BitSample(char * InputByte, char * OutputBit)
{
  int i, j;

  j = 0;
  for(i = 0; i < 6; i++)
  {
    if(InputByte[i] & 0x80) OutputBit[j++] = 1;
    else OutputBit[j++] = 0;
    if(InputByte[i] & 0x40) OutputBit[j++] = 1;
    else OutputBit[j++] = 0;
    if(InputByte[i] & 0x20) OutputBit[j++] = 1;
    else OutputBit[j++] = 0;
    if(InputByte[i] & 0x10) OutputBit[j++] = 1;
    else OutputBit[j++] = 0;
    if(InputByte[i] & 0x08) OutputBit[j++] = 1;
    else OutputBit[j++] = 0;
    if(InputByte[i] & 0x04) OutputBit[j++] = 1;
    else OutputBit[j++] = 0;
    if(InputByte[i] & 0x02) OutputBit[j++] = 1;
    else OutputBit[j++] = 0;
    if(InputByte[i] & 0x01) OutputBit[j++] = 1;
    else OutputBit[j++] = 0;
  }

  /* Fill the 49th bit */
  if(InputByte[i] & 0x01) OutputBit[j++] = 1;
  else OutputBit[j++] = 0;
} /* End Convert49BitSampleInto7Byte() */


/* End of file */

