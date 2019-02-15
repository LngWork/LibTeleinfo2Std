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
// LNG adaptation pour la TIC Standard
//  Suppression du stockage des donnees. Trop de memoire consommee avec des messages qui ont considerablement grossi.
//  Les donnees TIC sont lues, verifiees.
//  La procedure de call back est appellee mais les donnees ne sont pas stockees.
//
// **********************************************************************************

#ifndef LibTeleinfo2_h
#define LibTeleinfo2_h

#ifdef __arm__
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#define boolean bool
#endif

#ifdef ARDUINO
#include <Arduino.h>
#endif

// Using ESP8266 ?
#ifdef ESP8266
//#include "stdlib_noniso.h"
#include <ESP8266WiFi.h>
#endif

// Define this if you want library to be verbose
//#define TI_DEBUG

// I prefix debug macro to be sure to use specific for THIS library
// debugging, this should not interfere with main sketch or other
// libraries
#ifdef TI_DEBUG
#ifdef ESP8266
#define TI_Debug(x)    Serial.print(x)
#define TI_Debugln(x)  Serial.println(x)
#define TI_Debugf(...) Serial.printf(__VA_ARGS__)
#define TI_Debugflush  Serial.flush
#else
#define TI_Debug(x)    Serial.print(x)
#define TI_Debugln(x)  Serial.println(x)
#define TI_Debugf(...) Serial.printf(__VA_ARGS__)
#define TI_Debugflush  Serial.flush
#endif
#else
#define TI_Debug(x)
#define TI_Debugln(x)
#define TI_Debugf(...)
#define TI_Debugflush
#endif

//LNG longueurs max
#define TINFO_LABEL_MAXLEN		8		// Nombre de carracteres max sur un label (inferieur ou egal à 8 dans la doc ENEDIS)
#define TINFO_HORO_MAXLEN		13		// Nombre de carracteres d un horodatage (13 fixe dans la doc ENEDIS)
#define TINFO_VALUE_MAXLEN		98		// Nombre de carracteres max sur une valeur (pas fixe par la spec ENEDIS mais pas trouve de valeur plus longue que 98)


// Local buffer for one line of teleinfo
// la ligne la plus longue c est : Profil du prochain jour calendrier fournisseur : label PJOURF+1 (8 car + 1 séparateur), message de 98 carracteres + 1 séparateur + 1 checksum. Total 109.
#define TINFO_BUFSIZE  109

// Linked list structure containing all values received
// Finalement pas utilise
/*
struct TicData
{
  char  label [TINFO_LABEL_MAXLEN + 1]; // label
  char  horo [TINFO_HORO_MAXLEN + 1]; // label
  char  value[TINFO_VALUE_MAXLEN + 1]; // value
};
*/

// Library state machine
enum _State_e {
  TINFO_INIT,        // We're in init
  TINFO_WAIT_STX,    // We're waiting for STX - Start TX = Start full frame
  TINFO_READING,     // We had STX AND ETX, So we're READING data
  TINFO_WAIT_ETX     // We had STX, We're waiting for ETX - End TX = End full frame
};

// Teleinfo MODE
typedef enum {
  #define TINFO_MODE_HISTO  1
  #define TINFO_MODE_STD    2
} ModeTIC_t;

// Teleinfo start and end of frame characters
#define TINFO_STX 0x02
#define TINFO_ETX 0x03
#define TINFO_SGR 0x0A // Start of group  
#define TINFO_EGR 0x0D // End of group    
#define TINFO_DATA_HISTO  0x20 // separateur DATA en mode Historique
#define TINFO_DATA_STD    0x09 // separateur DATA en mode Standard

class TInfo
{
  public:
    TInfo();
    void        init(ModeTIC_t m);
    _State_e    process (char c);
    //Changement du prototype de la fonction, on change aussi son nom
    void        attachDataStd(void (*fn_dataStd)(char * pLab, char * pHor, char * pVal));
    void        attachFullFrameStd(void (*_fn_full_frame)());
    uint8_t     clearBuffer();

  private:
    //void        checkLine(char * pline) ;

    _State_e  _state;                 // Teleinfo machine state
    ModeTIC_t _mode;                  // Teleinfo mode (TINFO_MODE_HISTO ou TINFO_MODE_STD)
    char      _data_sep;              // separateur de data (TINFO_DATA-HISTO ou TINFO_DATA-STD selon le mode)
    char      _recv_buff[TINFO_BUFSIZE + 1]; // Buffer pour le stockage d'un Groupe d'info TIC (= une ligne)
    uint8_t   _recv_idx;              // index dans receive buffer
    void      (*_fn_dataStd)(char * pLab, char * pHor, char * pVal);
    void      (*_fn_full_frame)();

    //Pour l analyse des trames
    char * pLabel;  //Le debut du label
    char * pHoro;   //La valeur de l horodatage s il existe. Null sinon.
    char * pValue;  //La valeur
    char   checksum;//Checksum courrant
};

#endif
