#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <time.h>
#include <math.h>
#include <algorithm>
#include <TH1.h>
#include <TF1.h>
#include <TLine.h>
#include <TMath.h>
#include <TGraph.h>
#include <TCanvas.h>
#include <TGraphErrors.h>
#include <TVirtualFFT.h>
#include <TApplication.h>
#include <TMultiGraph.h>
#include <TGaxis.h>
/*#include "TFile.h"
#include "TSystem.h"
#include "TMacro.h"*/

#include  <stdio.h>
#include  <stddef.h>
#include  <stdlib.h>
#include  <termios.h>
#include  <string.h>


using namespace std;
//Compile-Befehl auf mac mit g++:
//g++ -o Intensity Intensity.cpp `root-config --cflags --glibs --ldflags` -L/usr/local/lib/root




double ScalarProduct(std::vector<double> a, std::vector<double> b)
{
  long int product = 0;
  for( int i=0; i< a.size(); i++){
    product += a[i]*b[i];
  }
  return product;
}







//############# Zuerst einiges Zeug aus dem ptu -> txt - Übertragungscode, teilweise abgeändert #############//


// some important Tag Idents (TTagHead.Ident) that we will need to read the most common content of a PTU file
// check the output of this program and consult the tag dictionary if you need more
#define TTTRTagTTTRRecType "TTResultFormat_TTTRRecType"
#define TTTRTagNumRecords  "TTResult_NumberOfRecords"  // Number of TTTR Records in the File;
#define TTTRTagRes         "MeasDesc_Resolution"       // Resolution for the Dtime (T3 Only)
#define TTTRTagGlobRes     "MeasDesc_GlobalResolution" // Global Resolution of TimeTag(T2) /NSync (T3)
#define FileTagEnd         "Header_End"                // Always appended as last tag (BLOCKEND)

// TagTypes  (TTagHead.Typ)
#define tyEmpty8      0xFFFF0008
#define tyBool8       0x00000008
#define tyInt8        0x10000008
#define tyBitSet64    0x11000008
#define tyColor8      0x12000008
#define tyFloat8      0x20000008
#define tyTDateTime   0x21000008
#define tyFloat8Array 0x2001FFFF
#define tyAnsiString  0x4001FFFF
#define tyWideString  0x4002FFFF
#define tyBinaryBlob  0xFFFFFFFF

// RecordTypes
#define rtPicoHarpT3     0x00010303    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $03 (T3), HW: $03 (PicoHarp)
#define rtPicoHarpT2     0x00010203    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $03 (PicoHarp)
#define rtHydraHarpT3    0x00010304    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $03 (T3), HW: $04 (HydraHarp)
#define rtHydraHarpT2    0x00010204    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $04 (HydraHarp)
#define rtHydraHarp2T3   0x01010304    // (SubID = $01 ,RecFmt: $01) (V2), T-Mode: $03 (T3), HW: $04 (HydraHarp)
#define rtHydraHarp2T2   0x01010204    // (SubID = $01 ,RecFmt: $01) (V2), T-Mode: $02 (T2), HW: $04 (HydraHarp)
#define rtTimeHarp260NT3 0x00010305    // (SubID = $00 ,RecFmt: $01) (V2), T-Mode: $03 (T3), HW: $05 (TimeHarp260N)
#define rtTimeHarp260NT2 0x00010205    // (SubID = $00 ,RecFmt: $01) (V2), T-Mode: $02 (T2), HW: $05 (TimeHarp260N)
#define rtTimeHarp260PT3 0x00010306    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T3), HW: $06 (TimeHarp260P)
#define rtTimeHarp260PT2 0x00010206    // (SubID = $00 ,RecFmt: $01) (V1), T-Mode: $02 (T2), HW: $06 (TimeHarp260P)

#pragma pack(8) //structure alignment to 8 byte boundaries
TH1I *timestamphistogram = new TH1I("Timestamp","Timestamp",10,0,10);
// A Tag entry
struct TgHd{
  char Ident[32];     // Identifier of the tag
  int Idx;            // Index for multiple tags or -1
  unsigned int Typ;  // Type of tag ty..... see const section
    long long TagValue; // Value of tag.
} TagHead;


// TDateTime (in file) to time_t (standard C) conversion

const int EpochDiff = 25569; // days between 30/12/1899 and 01/01/1970
const int SecsInDay = 86400; // number of seconds in a day

time_t TDateTime_TimeT(double Convertee)
{
  time_t Result((long)(((Convertee) - EpochDiff) * SecsInDay));
  return Result;
}

FILE *fpin;//,*fpout;
bool IsT2;
long long RecNum;
long long oflcorrection;
long long truensync, truetime;
int m, c;
double GlobRes = 0.0;
long long iGlobRes = 0;
double Resolution = 0.0;
unsigned int dlen = 0;
unsigned int cnt_0=0, cnt_1=0;


TH1F * Intensityhistogram[3];
//[0]: Channel 0
//[1]: Channel 1
//[2]: Both together
int channelID;
long long eventcounter = 0;
long long writetime;

long long startevaltime;
long long endevaltime;
//bool timelimitation; 


//These values will be increased during filling of the inputvector:
//long long c0_counts = 0; long long c1_counts = 0;
long long ccounts[2];
int noverwlow = 0; long long allcounts = 0;

//For the measurement time
long long startmeastime; bool startmeastimeout = false; long long endmeastime;
bool skipfurther = false;




//############# Wichtig: GotPhoton-Funktion #############//
//Got Photon
//  TimeTag: Raw TimeTag from Record * Globalresolution = Real Time arrival of Photon
//  DTime: Arrival time of Photon after last Sync event (T3 only) DTime * Resolution = Real time arrival of Photon after last Sync event
//  Channel: Channel the Photon arrived (0 = Sync channel for T2 measurements)
void GotPhoton(long long TimeTag, int Channel, int DTime)
{
  if(IsT2)
  {

    channelID = Channel;
    writetime = TimeTag * iGlobRes;

    if (writetime >= startevaltime)
    {
        if(startmeastimeout == false){startmeastime = writetime; startmeastimeout = true;}
        endmeastime = writetime;

        Intensityhistogram[channelID]->Fill(writetime);
        Intensityhistogram[2]->Fill(writetime);
        ccounts[channelID] ++;
    }

    allcounts ++;

    if (writetime > endevaltime)
    {
      skipfurther = true;
      //std::cout << "NOW!" << std::endl;
    }


	if (Channel != 0 && Channel != 1)
    {
        cout << "ERROR: The channel-ID could not be read correctly: Entry " << RecNum << " C-ID: " << Channel << std::endl;
        exit(1); //Call system to stop
    }

  }
  else
  {
    exit(1);   // call system to stop
    //fprintf(fpout,"%I64u CHN %1x %I64u %8.0lf %10u\n", RecNum, Channel, TimeTag, (TimeTag * GlobRes * 1e9), DTime);
  }

  if (RecNum % 10000000 == 0){std::cout << RecNum * 1e-6 << "\t Mio\tTime:\t" << 1e-12 * writetime << " s" << std::endl;}//Ausgabe des Fortschritts
}






//######################################################################################################################################//
//############# Jetzt kommt eine noch eine Menge unbearbeiteter ptu->txt Code (lediglich die fprintf-Befehle rausgelöscht) #############//


//Got Marker
//  TimeTag: Raw TimeTag from Record * Globalresolution = Real Time arrival of Photon
//  Markers: Bitfield of arrived Markers, different markers can arrive at same time (same record)
void GotMarker(long long TimeTag, int Markers)
{
  //fprintf(fpout,"%I64u MAR %2x %I64u\n", RecNum, Markers, TimeTag);
}

//Got Overflow
//  Count: Some TCSPC provide Overflow compression = if no Photons between overflow you get one record for multiple Overflows
void GotOverflow(int Count)
{
  //fprintf(fpout,"%I64u OFL * %2x\n", RecNum, Count);
  noverwlow ++;
}

// PicoHarp T3 input
void ProcessPHT3(unsigned int TTTRRecord)
{
  const int T3WRAPAROUND = 65536;
  union
  {
    unsigned int allbits;
    struct
    {
    unsigned numsync  :16;
    unsigned dtime    :12;
    unsigned channel  :4;
    } bits;
    struct
    {
    unsigned numsync  :16;
    unsigned markers  :12;
    unsigned channel  :4;
    } special;
  } Record;

  Record.allbits = TTTRRecord;
    if(Record.bits.channel==0xF) //this means we have a special record
  {
    if(Record.special.markers==0) //not a marker means overflow
    {
      GotOverflow(1);
      oflcorrection += T3WRAPAROUND; // unwrap the time tag overflow
    }
    else
    {
      truensync = oflcorrection + Record.bits.numsync;
      m = Record.special.markers;
      GotMarker(truensync, m);
    }
  } else
  {
    if(
        (Record.bits.channel==0) //Should never occur in T3 Mode
      ||(Record.bits.channel>4) //Should not occur with current routers
      )
    {
      printf("\nIllegal Channel: #%1d %1u",dlen,Record.bits.channel);
      //fprintf(fpout,"\nillegal channel ");
    }

    truensync = oflcorrection + Record.bits.numsync;
    m = Record.bits.dtime;
    c = Record.bits.channel;
    GotPhoton(truensync, c, m);
    dlen++;
  }
};


void ProcessPHT2(unsigned int TTTRRecord)
{
  const int T2WRAPAROUND = 210698240;
  union
  {
    unsigned int allbits;
    struct
    {
    unsigned time   :28;
    unsigned channel  :4;
    } bits;

  } Record;
  unsigned int markers;
  Record.allbits = TTTRRecord;
  if(Record.bits.channel==0xF) //this means we have a special record
  {
    //in a special record the lower 4 bits of time are marker bits
    markers=Record.bits.time&0xF;
    if(markers==0) //this means we have an overflow record
    {
      oflcorrection += T2WRAPAROUND; // unwrap the time tag overflow
      GotOverflow(1);
    }
    else //a marker
    {
      //Strictly, in case of a marker, the lower 4 bits of time are invalid
      //because they carry the marker bits. So one could zero them out.
      //However, the marker resolution is only a few tens of nanoseconds anyway,
      //so we can just ignore the few picoseconds of error.
      truetime = oflcorrection + Record.bits.time;
            GotMarker(truetime, markers);
    }
  }
  else
  {
    if((int)Record.bits.channel > 4) //Should not occur
    {
      //printf(" Illegal Chan: #%I64u %1u\n",RecNum,Record.bits.channel);
      //fprintf(fpout," illegal chan.\n");
    }
    else
    {
      if(Record.bits.channel==0) cnt_0++;
      if(Record.bits.channel>=1) cnt_1++;

      truetime = oflcorrection + Record.bits.time;
      m = Record.bits.time;
      c = Record.bits.channel;
      GotPhoton(truetime, c, m);
    }
  }
}

void ProcessHHT2(unsigned int TTTRRecord, int HHVersion)
{
  uint64_t T2WRAPAROUND_V1 = 33552000;
  uint64_t T2WRAPAROUND_V2 = 33554432;
  union{
    unsigned long allbits;
    struct{ unsigned timetag  :25;
        unsigned channel  :6;
        unsigned special  :1; // or sync, if channel==0
        } bits;
    } T2Rec;
  T2Rec.allbits = TTTRRecord;

  if(T2Rec.bits.special==1)
    {
      if(T2Rec.bits.channel==0x3F) //an overflow record
      {
    	if(HHVersion == 1)
    	{
    	  oflcorrection += T2WRAPAROUND_V1;
    	  GotOverflow(1);
    	}
    	else
    	{
      		//number of overflows is stored in timetag
      		if(T2Rec.bits.timetag==0) //if it is zero it is an old style single overflow
      		{
      		    GotOverflow(1);
      		  oflcorrection += T2WRAPAROUND_V2;  //should never happen with new Firmware!
      		}
      		else
      		{ 
      		  oflcorrection += T2WRAPAROUND_V2 * T2Rec.bits.timetag;
      		  GotOverflow(T2Rec.bits.timetag);
      		}
    	}
      }

      if((T2Rec.bits.channel>=1)&&(T2Rec.bits.channel<=15)) //markers
      {
        truetime = oflcorrection + T2Rec.bits.timetag;
        //Note that actual marker tagging accuracy is only some ns.
        m = T2Rec.bits.channel;
        GotMarker(truetime, m);
      }

      if(T2Rec.bits.channel==0) //sync
      {
        truetime = oflcorrection + T2Rec.bits.timetag;
    	GotPhoton(truetime, 0, 0);
      	//timestamphistogram->Fill(T2Rec.bits.timetag%8);
      }
    }
    else //regular input channel
    {
    truetime = oflcorrection + T2Rec.bits.timetag;
    c = T2Rec.bits.channel + 1;
    GotPhoton(truetime, c, 0);
    timestamphistogram->Fill(T2Rec.bits.timetag%8);
    }

}


void ProcessHHT3(unsigned int TTTRRecord, int HHVersion)
{
  const int T3WRAPAROUND = 1024;
  union {
    unsigned long allbits;
    struct  {
      unsigned nsync    :10;  // numer of sync period
      unsigned dtime    :15;    // delay from last sync in units of chosen resolution
      unsigned channel  :6;
      unsigned special  :1;
    } bits;
  } T3Rec;
  T3Rec.allbits = TTTRRecord;
  if(T3Rec.bits.special==1)
  {
    if(T3Rec.bits.channel==0x3F) //overflow
    {
      //number of overflows is stored in nsync
      if((T3Rec.bits.nsync==0) || (HHVersion==1)) //if it is zero or old version it is an old style single oferflow
      {
        oflcorrection += (uint64_t)T3WRAPAROUND;
        GotOverflow(1); //should never happen with new Firmware!
      }
      else
      {
        oflcorrection += (uint64_t)T3WRAPAROUND * T3Rec.bits.nsync;
        GotOverflow(T3Rec.bits.nsync);
      }
    }
    if((T3Rec.bits.channel>=1)&&(T3Rec.bits.channel<=15)) //markers
    {
      truensync = oflcorrection + T3Rec.bits.nsync;
      //the time unit depends on sync period which can be obtained from the file header
      c = T3Rec.bits.channel;
      GotMarker(truensync, c);
    }
  }
  else //regular input channel
    {
      truensync = oflcorrection + T3Rec.bits.nsync;
      //the nsync time unit depends on sync period which can be obtained from the file header
      //the dtime unit depends on the resolution and can also be obtained from the file header
      c = T3Rec.bits.channel;
      m = T3Rec.bits.dtime;
      GotPhoton(truensync, c, m);
    }
}


//############# Ende unbearbeiteter Code, jetzt geht's richtig los #############//
//##############################################################################//

int main (int argc, char* argv[])
{

    std::cout << "\n\nSTART Intensity Evaluation" << std::endl;


    //Set Parameters, all of them can be changed later on manually in the executable
    startevaltime = 0;
    endevaltime = 30e12; 
    
    long long binlength = 1e9;
    long long binnumber;

    long long goalmeastime = endevaltime - startevaltime;
    binnumber = std::ceil(goalmeastime/binlength);

    //#############################################################################//
    //############# Jetzt kommen jede Menge manuelles-Einstellen-Zeug #############//
    std::cout << "\nStandard-Settings:" << std::endl;
    settingsmarker:;

    goalmeastime = endevaltime - startevaltime;
    binnumber = std::ceil(goalmeastime/binlength);

    std::cout << "\tStart evaluation time:\t" << 1e-12 * startevaltime << " s\n\tEnd evaluation time:\t" << 1e-12 * endevaltime << " s" << std::endl;
    std::cout << "\n\tBinwidth:\t\t" << 1e-9 * binlength << " ms" << std::endl;
    std::cout << "\t-> Number of bins\t" << binnumber << std::endl;
    
	  std::cout << "\n\nWhat do you want to do/change?" << std::endl;
        std::cout << "\nStart evaluation time\t[is]\nEnd evaluation time\t[ie]\nSet input limitation\t[sl]\nWidth of bins\t\t[wb]\nContinue\t\t[c]\nType it here:\t";
        std::string whatchangestring; cin >> whatchangestring;
        std::cout << "\n\n\n\n\n" << std::endl;

        if (whatchangestring != "is" && whatchangestring != "ie" && whatchangestring != "wb" && whatchangestring !="c")
        {
            std::cout << "\n\n\n\nWhat do you mean? Please enter again." << std::endl;
            goto settingsmarker;
        }
        if (whatchangestring == "is")
        {
            std::cout << "Enter new Start of evaluation time [s]:\t";
            std::string isstring; cin >> isstring;
            startevaltime = 1e12 * std::atof(isstring.c_str());
            goalmeastime = endevaltime - startevaltime;
        }
        if (whatchangestring == "ie")
        {
            std::cout << "Enter new end of evaluation time [s]:\t";
            std::string instring; cin >> instring;
            endevaltime = 1e12 * std::atof(instring.c_str());
            goalmeastime = endevaltime - startevaltime;
        }
        if (whatchangestring == "wb")
        {
            std::cout << "Enter new bin width (integration time) [ms]:\t";
            std::string nbstring; cin >> nbstring;
            binlength = 1e9 * std::atof(nbstring.c_str());
            binnumber = std::ceil(goalmeastime/binlength);
        }
        if (whatchangestring == "c")
        {
        	goto settingsdone;
        }

        goto settingsmarker;

	settingsdone:;

	//############# Ende manuelles Einstellen-Zeug #############//
	//##########################################################//


  const int nbins = binnumber;
  if (binlength < 250){std::cout << "Warning: Integration time (" << binlength << " ps) is below the mimimum time-step of the electronics (250 ps)" << std::endl;}

  
  Intensityhistogram[0] = new TH1F("ihi0","Intensityhistogram Channel 0", nbins, startevaltime, endevaltime);
    Intensityhistogram[0]->GetXaxis()->SetTitle("t [ps]");
    Intensityhistogram[0]->GetYaxis()->SetTitle("f [MHz]");
    Intensityhistogram[0]->GetYaxis()->SetTitleOffset(1.4);
  Intensityhistogram[1] = new TH1F("ihi1","Intensityhistogram Channel 1", nbins, startevaltime, endevaltime);
    Intensityhistogram[1]->GetXaxis()->SetTitle("t [ps]");
    Intensityhistogram[1]->GetYaxis()->SetTitle("f [MHz]");
    Intensityhistogram[1]->GetYaxis()->SetTitleOffset(1.4);
  Intensityhistogram[2] = new TH1F("ihib","Intensityhistogram both Channels", nbins, startevaltime, endevaltime);
    Intensityhistogram[2]->GetXaxis()->SetTitle("t [ps]");
    Intensityhistogram[2]->GetYaxis()->SetTitle("f [MHz]");
    Intensityhistogram[2]->GetYaxis()->SetTitleOffset(1.4);

    ccounts[0] = 0;
    ccounts[1] = 1;



    //##################################################################################//
    //############# Jetzt wieder jede Menge Code aus dem ptu -> txt - file #############//

  	char Magic[8];
  	char Version[8];
  	char Buffer[40];
  	char* AnsiBuffer;
  	char* WideBuffer;
  	int Result;

  	long long NumRecords = -1;
  	long long RecordType = 0;
  	long long maxinput = -1;//For later limitation of inputlines


  	printf("\nPicoQuant Unified TTTR (PTU) Mode File Demo");
  	printf("\n~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");

  	/*if((argc<3)||(argc>3))
  	{
  	 printf("usage: ht2demo infile oufile\n");
  	 printf("infile is a Unified TTTR ptu file (binary)\n");
  	 printf("outfile is ASCII\n");
  	 getchar();
  	 exit(-1);
  	}*/
  	if((fpin=fopen(argv[1],"rb"))==NULL)
      	{printf("\n ERROR! Input file cannot be opened, aborting.\n"); goto close;}



  	printf("\n Loading data from %s \n", argv[1]);
  	//printf("\n Writing output to %s \n", argv[2]);
	
  	Result = fread( &Magic, 1, sizeof(Magic) ,fpin);
  	if (Result!= sizeof(Magic))
  	{
  	  printf("\nerror reading header, aborted.");
  	    goto close;
  	}
  	Result = fread(&Version, 1, sizeof(Version) ,fpin);
  	if (Result!= sizeof(Version))
  	  {
  	  printf("\nerror reading header, aborted.");
  	    goto close;
  	}
  	if (strncmp(Magic, "PQTTTR", 6))
  	{
  	  printf("\nWrong Magic, this is not a PTU file.");
  	  goto close;
  	}
  	//fprintf(fpout, "Tag Version: %s \n", Version);
	
  	// read tagged header
  	do
  	{
  	  // This loop is very generic. It reads all header items and displays the identifier and the
  	  // associated value, quite independent of what they mean in detail.
  	  // Only some selected items are explicitly retrieved and kept in memory because they are 
  	  // needed to subsequently interpret the TTTR record data.
	
  	  Result = fread( &TagHead, 1, sizeof(TagHead) ,fpin);
  	  if (Result!= sizeof(TagHead))
  	  {
  	      printf("\nIncomplete File.");
  	        goto close;
  	  }
	
  	  strcpy(Buffer, TagHead.Ident);
  	  if (TagHead.Idx > -1)
  	  {
  	    sprintf(Buffer, "%s(%d)", TagHead.Ident,TagHead.Idx);
  	  }
  	  //fprintf(fpout, "\n%-40s", Buffer);
  	  switch (TagHead.Typ)
  	  {
  	      case tyEmpty8:
  	      //fprintf(fpout, "<empty Tag>");
  	      break;
  	    case tyBool8:
  	      //fprintf(fpout, "%s", bool(TagHead.TagValue)?"True":"False");
  	      break;
  	    case tyInt8:
  	      //fprintf(fpout, "%lld", TagHead.TagValue);
  	      // get some Values we need to analyse records
  	      if (strcmp(TagHead.Ident, TTTRTagNumRecords)==0) // Number of records
  	                  NumRecords = TagHead.TagValue;
  	      if (strcmp(TagHead.Ident, TTTRTagTTTRRecType)==0) // TTTR RecordType
  	                  RecordType = TagHead.TagValue;
  	      break;
  	    case tyBitSet64:
  	      //fprintf(fpout, "0x%16.16X", TagHead.TagValue);
  	      break;
  	    case tyColor8:
  	      //fprintf(fpout, "0x%16.16X", TagHead.TagValue);
  	      break;
  	    case tyFloat8:
  	      //fprintf(fpout, "%E", *(double*)&(TagHead.TagValue));
  	      if (strcmp(TagHead.Ident, TTTRTagRes)==0) // Resolution for TCSPC-Decay
  	                  Resolution = *(double*)&(TagHead.TagValue);
  	      if (strcmp(TagHead.Ident, TTTRTagGlobRes)==0) // Global resolution for timetag
  	                  GlobRes = *(double*)&(TagHead.TagValue); // in ns
                      iGlobRes = 1e12 * GlobRes;
  	      break;
  	    case tyFloat8Array:
  	      //fprintf(fpout, "<Float Array with %d Entries>", TagHead.TagValue / sizeof(double));
  	      // only seek the Data, if one needs the data, it can be loaded here
  	      fseek(fpin, (long)TagHead.TagValue, SEEK_CUR);
  	      break;
  	    case tyTDateTime:
  	      time_t CreateTime;
  	      CreateTime = TDateTime_TimeT(*((double*)&(TagHead.TagValue)));
  	      //fprintf(fpout, "%s", asctime(gmtime(&CreateTime)), "\0");
  	      break;
  	    case tyAnsiString:
  	      AnsiBuffer = (char*)calloc((size_t)TagHead.TagValue,1);
  	              Result = fread(AnsiBuffer, 1, (size_t)TagHead.TagValue, fpin);
  	            if (Result!= TagHead.TagValue)
  	      {
  	        printf("\nIncomplete File.");
  	        free(AnsiBuffer);
  	                goto close;
  	      }
  	      //fprintf(fpout, "%s", AnsiBuffer);
  	      free(AnsiBuffer);
  	      break;
  	          case tyWideString:
  	      WideBuffer = (char*)calloc((size_t)TagHead.TagValue,1);
  	              Result = fread(WideBuffer, 1, (size_t)TagHead.TagValue, fpin);
  	            if (Result!= TagHead.TagValue)
  	      {
  	        printf("\nIncomplete File.");
  	        free(WideBuffer);
  	                goto close;
  	      }
  	      //fwprintf(fpout, L"%s", WideBuffer);
  	      free(WideBuffer);
  	      break;
  	          case tyBinaryBlob:
  	      //fprintf(fpout, "<Binary Blob contains %d Bytes>", TagHead.TagValue);
  	      // only seek the Data, if one needs the data, it can be loaded here
  	      fseek(fpin, (long)TagHead.TagValue, SEEK_CUR);
  	      break;
  	    default:
  	      printf("Illegal Type identifier found! Broken file?");
  	      goto close;
  	  }
  	}
  	while((strncmp(TagHead.Ident, FileTagEnd, sizeof(FileTagEnd))));
  	//fprintf(fpout, "\n-----------------------\n");
	// End Header loading

 	 // TTTR Record type
 	 switch (RecordType)
 	 {
 	   case rtPicoHarpT2:
 	     //fprintf(fpout, "PicoHarp T2 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ps\n");
 	     break;
 	   case rtPicoHarpT3:
 	     //fprintf(fpout, "PicoHarp T3 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ns dtime\n");
 	     break;
 	   case rtHydraHarpT2:
 	     //fprintf(fpout, "HydraHarp V1 T2 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ps\n");
 	     break;
 	   case rtHydraHarpT3:
 	     //fprintf(fpout, "HydraHarp V1 T3 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ns dtime\n");
 	     break;
 	   case rtHydraHarp2T2:
 	     //fprintf(fpout, "HydraHarp V2 T2 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ps\n");
 	     break;
 	   case rtHydraHarp2T3:
 	     //fprintf(fpout, "HydraHarp V2 T3 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ns dtime\n");
 	     break;
 	   case rtTimeHarp260NT3:
 	     //fprintf(fpout, "TimeHarp260N T3 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ns dtime\n");
 	     break;
 	   case rtTimeHarp260NT2:
 	     //fprintf(fpout, "TimeHarp260N T2 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ps\n");
 	     break;
 	   case rtTimeHarp260PT3:
 	     //fprintf(fpout, "TimeHarp260P T3 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ns dtime\n");
 	     break;
 	   case rtTimeHarp260PT2:
 	     //fprintf(fpout, "TimeHarp260P T2 data\n");
 	     //fprintf(fpout,"\nrecord# chan   nsync truetime/ps\n");
 	     break;
 	 default:
 	   //fprintf(fpout, "Unknown record type: 0x%X\n 0x%X\n ", RecordType);
 	   goto close;
  	}

  	unsigned int TTTRRecord;

  
  	std::cout << "Total number of records of the file:\t" << NumRecords << std::endl;



  	//Jetzt kommt das eigentliche Auslesen aus der ptu-Datei:
  	//for(RecNum=0; RecNum<maxinput; RecNum++)
  	for(RecNum=0;RecNum<NumRecords;RecNum++)
  	{
  		eventcounter ++;
  		//std::cout << RecNum << std::endl;
  		Result = fread(&TTTRRecord, 1, sizeof(TTTRRecord) ,fpin);
    	if (Result!= sizeof(TTTRRecord))
    	  {
    	    printf("\nUnexpected end of input file!");
    	    break;
    	  }
    	switch (RecordType)
    	{
    	case rtPicoHarpT2:
    	  IsT2 = true;
    	  //if (linelimit == false || RecNum >= startinput){ProcessPHT2(TTTRRecord);}
    	  ProcessPHT2(TTTRRecord);
    	  break;
    	case rtPicoHarpT3:
    	  IsT2 = false;
    	  //if (linelimit == false || RecNum >= startinput){ProcessPHT3(TTTRRecord);}
    	  ProcessPHT3(TTTRRecord);
    	  break;
    	case rtHydraHarpT2:
    	  IsT2 = true;
    	  //if (linelimit == false || RecNum >= startinput){ProcessHHT2(TTTRRecord, 1);}
    	  ProcessHHT2(TTTRRecord, 1);
        if (skipfurther == true){goto close;}
    	  break;
    	case rtHydraHarpT3:
    	  IsT2 = false;
    	  //if (linelimit == false || RecNum >= startinput){ProcessHHT3(TTTRRecord, 1);}
    	  ProcessHHT3(TTTRRecord, 1);
    	  break;
    	case rtHydraHarp2T2:
    	case rtTimeHarp260NT2:
    	case rtTimeHarp260PT2:
    	  IsT2 = true;
    	  //if (linelimit == false || RecNum >= startinput){ProcessHHT2(TTTRRecord, 2);}
    	  ProcessHHT2(TTTRRecord, 2);
        if (skipfurther == true){goto close;}
    	  break;
    	case rtHydraHarp2T3:
    	case rtTimeHarp260NT3:
    	case rtTimeHarp260PT3:
    	  IsT2 = false;
    	  //if (linelimit == false || RecNum >= startinput){ProcessHHT3(TTTRRecord, 2);}
    	  ProcessHHT3(TTTRRecord, 2);
    	  break;
    	default:
	
    	  goto close;
    	}
    }


	  close:
	  fclose(fpin);
	  //fclose(fpout);
	
	//############# Ende Code aus der ptu -> txt - Datei #############//
	//################################################################//
    
    //const int arraysize = inputvector0.size();
    long long inputs = allcounts;//Doesn't consider a startoffset of lines/time
    std::cout << "Inputs in meastime:\t" << inputs << std::endl;
    std::cout << "Overflow events in meastime:\t" << noverwlow << std::endl;

    //long long starttime = inputvector3.at(0);//Works only for startinput = 0
    //long long endtime = inputvector3.at(inputvector3.size()-1);
    long long meastime = endmeastime - startmeastime;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "Starttime: " << 1e-12 * startmeastime << " s\t\tEndtime: " << 1e-12 * endmeastime << " s" << std::endl;
    std::cout << "Effective Measurement time : " << 1e-12 * meastime << " s" << std::endl;
    
    double c0_rate = 1e6 * ccounts[0]/meastime;//in MHz
    double c1_rate = 1e6 * ccounts[1]/meastime;//in MHz
    double rateratio = c0_rate/c1_rate; double invrateratio = c1_rate/c0_rate;

    std::cout << "Channel\tCounts\t\tRate (MHz)" << std::endl;
    std::cout << "0\t" << ccounts[0] << "\t\t" << c0_rate << std::endl;
    std::cout << "1\t" << ccounts[1] << "\t\t" << c1_rate << std::endl;
	  std::cout << "Ratio 0:1 : " << rateratio << "\t1:0 : " << invrateratio << std::endl;
	  std::cout << "--------------------------------------------" << std::endl;

    std::vector <double> r0vec, r1vec;
    double interval_time = binlength*1e-12; // Binlaenge in Sekunden
    std::cout << interval_time << std::endl;
    for (int i=1; i<Intensityhistogram[0]->GetSize()-1; i++)
    {
      r0vec.push_back((Intensityhistogram[0]->GetBinContent(i))/interval_time); // in Hz
      r1vec.push_back((Intensityhistogram[1]->GetBinContent(i))/interval_time); // in Hz

       //std::cout << r0vec.back() << "\t" << r1vec.back() << std::endl;
    }

    //std::cout << "Enter anything to continue  ";
    //std::cout << binlength << "[ps]" << std::endl;
	//std::cout << ScalarProduct(r0vec, r1vec) << std::endl;
    double random_coinc_events = ScalarProduct(r0vec, r1vec)*interval_time*250e-12;
    // std::cout << meastime << std::endl;
	 std::cout << "Sigma from the averaged rates:\t\t\t" << 1/sqrt(c0_rate*c1_rate*250*meastime*1e-12) << std::endl;
    std::cout << "Exact sigma with individual intervals:\t\t" << 1/sqrt(random_coinc_events) << std::endl;

    // std::ofstream ratefilestream;
    // ratefilestream.open("rates.txt");
    // ratefilestream << "#Rate0 \t Rate1 (in intervals of " << interval_time << "s)\n";
    // for( int i=0; i<r0vec.size(); i++ ){
    //   ratefilestream << r0vec[i] << "\t" << r1vec[i] << "\n";
    // }

    TApplication TheApp("app",0 ,0);

    TGraph* graph0 = new TGraph();
    for( int k=0; k<r0vec.size(); k++ ){
      graph0->SetPoint(k, k*interval_time, r0vec[k]*1e-3);
    }
    graph0 -> SetTitle("PMT Rates");
    graph0 -> SetLineColor(2);
    // graph0 -> GetHistogram() -> GetXaxis() -> SetTitle("Time [s]");
    // graph0 -> GetHistogram() -> GetYaxis() -> SetTitle("Rate [kHz]");

    TGraph* graph1 = new TGraph();

    for( int k=0; k<r1vec.size(); k++ ){
      graph1->SetPoint(k, k*interval_time, r1vec[k]*1e-3);
    }
    // graph1 -> SetTitle("PMT Rates");
    graph1 -> SetLineColor(1);

    TCanvas* can0 = new TCanvas("Rate");
    TMultiGraph* multigraph = new TMultiGraph();
    multigraph -> SetTitle("PMT Rates; Time [s]; Rate [kHz]");
    multigraph -> Add(graph0, "ACP");
    multigraph -> Add(graph1, "ACP");
    multigraph -> Draw("AP"); 

    TheApp.Run();    
}


