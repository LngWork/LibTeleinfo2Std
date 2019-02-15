// **********************************************************************************
// Driver definition for French Teleinfo
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// For any explanation about teleinfo ou use , see my blog
// http://hallard.me/category/tinfo
//
// Code based on following datasheet
// http://www.erdf.fr/sites/default/files/ERDF-NOI-CPT_02E.pdf
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// Edit : Tab size set to 2 but I converted tab to sapces
//
// **********************************************************************************

#include "LibTeleinfo2Std.h"

/* ======================================================================
Class   : TInfo
Purpose : Constructor
Input   : -
Output  : -
Comments: -
====================================================================== */
TInfo::TInfo()
{
  //Init state
  _state = TINFO_INIT;
  //Nothing received
  _recv_idx = 0;
  // callbacks
  _fn_dataStd = NULL;
  _fn_full_frame = NULL;
}

/* ======================================================================
Function: init
Purpose : 
Input   : -
Output  : -
Comments: -
====================================================================== */
void TInfo::init(ModeTIC_t m)
{
  // clear our receive buffer
  clearBuffer();

  // We're in INIT in term of receive data
  _state = TINFO_INIT;
  
  //Sauvegarde du mode
  _mode = m;
  if (_mode == TINFO_MODE_HISTO) {
    _data_sep = TINFO_DATA_HISTO;
  } else {
    _data_sep = TINFO_DATA_STD;
  }
}

/* ======================================================================
Function: attachDataStd
Purpose : attach a callback when we detected a new/changed value
Input   : callback function
Output  : -
Comments: -
====================================================================== */
void TInfo::attachDataStd(void (*fn_dataStd)(char * pLab, char * pHor, char * pVal))
{
  // indicate the user callback
  _fn_dataStd = fn_dataStd;
}

/* ======================================================================
Function: attachFullFrameStd
Purpose : attach a callback when we received a full frame
Input   : callback function
Output  : -
Comments: -
====================================================================== */
void TInfo::attachFullFrameStd(void (*fn_full_frame)())
{
  // indicate the user callback
  _fn_full_frame = fn_full_frame;
}

/* ======================================================================
Function: clearBuffer
Purpose : clear and init the buffer
Input   : -
Output  : -
Comments: -
====================================================================== */
uint8_t TInfo::clearBuffer()
{
  // Clear our buffer, set index to 0
  // memset(_recv_buff, 0, TINFO_BUFSIZE);
  _recv_idx = 0;
  _recv_buff[_recv_idx] = 0;

  pLabel = NULL;        // no value, point to NULL
  pHoro = NULL;         // no value, point to NULL
  pValue = NULL;        // no value, point to NULL
  checksum = 0;         // checksum est toujours le dernier carractere

  return 0;
}

/* ======================================================================
Function: process
Purpose : teleinfo serial char received processing, should be called
          my main loop, this will take care of managing all the other
          
          Par rapport a l'original on traite le checksum et la detection des donnees au fur et
          a mesure de la reception. Ca evite un traitement lourd (CPU) en fin de Groupe. C est plus fluide
          surtout avec un CPU limite et une TIC en Standard à 9600 Baud.
Input   : un carractere recu
Output  : teleinfo global state
====================================================================== */
_State_e TInfo::process(char c)
{
  // be sure 7 bits only
  c &= 0x7F;

  // What we received ?
  switch (c) {
    // Case start of transmission
    case  TINFO_STX:
      TI_Debugln("TINFO_STX before clear buffer");
      // Clear buffer, begin to store in it
      clearBuffer();

      //Now ready to receive Groups
      _state = TINFO_READING;
      break;

    // Case End of transmission
    case  TINFO_ETX:
      // Normal working mode ?
      if (_state == TINFO_READING) {
        TI_Debugln("TINFO_ETX and I am READING");
        // Call user callback if any - Transmission complete
        if (_fn_full_frame)
          _fn_full_frame();
      }
      //Now we wait for a new transmission
      _state = TINFO_WAIT_STX ;
      break;

    // Case Start of group (\n)
    case  TINFO_SGR:
      TI_Debugln("TINFO_SGR");
      //Just clear buffer
      clearBuffer();
      //Normalement on ne devrait pas changer d'etat ici (uniquement sur STX)
      //Mais on le laisse quand meme. Ca permet de demarrer la reception de groupe sans attendre STX
      _state = TINFO_READING;
      break;

    // Case End of group (\r)
    case  TINFO_EGR:
      // Are we in a good state?
      if (_state == TINFO_READING) {
        TI_Debugln("TINFO_EGR before checkLine");
        // Controle de la ligne

        //On ajuste le controle du checksum car on a compte trop de carracteres
        //On commence par retirer le dernier carractere qui est le checksum lui meme
        checksum -= _recv_buff[_recv_idx - 1];
        //Si on est en mode HISTORIQUE on doit aussi enlever le dernier separateur
        if (_mode == TINFO_MODE_HISTO) {
          checksum -= TINFO_DATA_HISTO;
        }
        //On termine le calcul du checksum
        checksum = (checksum & 0x3F) + 0x20;
        //Si il n'est pas identique au checksum recu, on ignore le groupe
        if (checksum != _recv_buff[_recv_idx - 1]) {
          //On re-initialise le buffer
          TI_Debugln("TINFO_EGR checksum KO");
          TI_Debugln(checksum);
          TI_Debugln(_recv_buff[_recv_idx - 1]);
          clearBuffer();
        } else {
          //On a un bon checksum
          // Si on n'a pas de donnee c'est qu'il n y a pas d horodatage (mode standard)
          if (pValue == (_recv_buff + _recv_idx - 1) ) {
            pValue = pHoro;
            pHoro = NULL;
          }
          //On appelle le callback fin de Groupe valide
          if (_fn_dataStd) {
            _fn_dataStd(pLabel, pHoro, pValue);
          }

        }
      }
      break;

    // Case other char
    default:
      // Only in a reading state
      if (_state == TINFO_READING) {
        TI_Debug(c);
        // If buffer is not full
        if ( _recv_idx < TINFO_BUFSIZE){
          //Store DATA
          // d abord la nouvelle fin de chaine
          _recv_buff[_recv_idx+1] = 0;
          // ensuite on ecrase l ancienne fin de chaine avec le nouveau char
          _recv_buff[_recv_idx] = c;
          _recv_idx++;
          
          //Si premier carracere on a un debut de label
          if (_recv_idx == 1) {
            pLabel = _recv_buff;
          }
          // Si on a un séparateur de champs
          if (c == _data_sep) {
            // Isolate item creating a \0 terminated char*
            _recv_buff[_recv_idx - 1] = '\0';
            //Si c est le 1er separateur
            if (pHoro == NULL) {
              //Horodatage start just after (on traite plus tard le cas sans horodatage)
              pHoro = _recv_buff + _recv_idx;
            //Deuxieme separateur
            } else if (pValue == NULL) {
              //Value start just after (on traite plus tard le cas sans horodatage)
              pValue = _recv_buff + _recv_idx;
            }
            //Rien à faire pour le troisieme separateur
          }
          
          //Cumul du checksum
          checksum += c;
        }
      }
      break;
    //End of cases
  }
  return _state;
}