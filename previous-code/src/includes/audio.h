/*
  Previous - audio.h

  This file is distributed under the GNU General Public License, version 2
  or at your option any later version. Read the file gpl.txt for details.
*/

#ifndef PREV_AUDIO_H
#define PREV_AUDIO_H

extern void Audio_Output_Enable(bool bEnable);
extern void Audio_Output_Init(void);
extern void Audio_Output_UnInit(void);
extern void Audio_Output_Queue_Put(uint8_t* data, int len);
extern void Audio_Output_Queue_Clear(void);
extern int  Audio_Output_Queue_Size(void);

extern void Audio_Input_Enable(bool bEnable);
extern void Audio_Input_Init(void);
extern void Audio_Input_UnInit(void);
extern int  Audio_Input_Buffer_Get(int16_t* sample);
extern int  Audio_Input_Buffer_Size(void);
extern void Audio_Input_Lock(void);
extern void Audio_Input_Unlock(void);

#endif /* PREV_AUDIO_H */
