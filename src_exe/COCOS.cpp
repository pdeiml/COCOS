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
#include <iomanip>
/*#include "TFile.h"
#include "TSystem.h"
#include "TMacro.h"*/
#include  <stdio.h>
#include  <stddef.h>
#include  <stdlib.h>
#include  <termios.h>
#include  <string.h>

#include "Settings.hpp"
#include "PtuFile.hpp"


using namespace std;

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
//TH1I *timestamphistogram_c0 = new TH1I("Timestamp_c0","Timestamps Channel 0",10,0,10);
//TH1I *timestamphistogram_c1 = new TH1I("Timestamp_c1","Timestamps Channel 1",10,0,10);

// TDateTime (in file) to time_t (standard C) conversion

const int EpochDiff = 25569; // days between 30/12/1899 and 01/01/1970
const int SecsInDay = 86400; // number of seconds in a day

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

//One single vector: (recID) (CHN) (nsync) (truetime)
//std::vector<long long> inputvector[4];

std::vector<long long> inputvector0;
std::vector<long long> inputvector1;
std::vector<long long> inputvector2;
std::vector<long long> inputvector3;

//These values will be increased during filling of the inputvector:
//long long c0_counts = 0; long long c1_counts = 0;
long long ccounts[2];
int noverwlow = 0; long long allcounts = 0;

//For the measurement time
long long startmeastime; bool startmeastimeout = false; long long endmeastime;
long long startlinetotime = 0;//Will find out the start line to the adjusted starttime
long long endlinetotime = 0;//Will find out the end line to the adjusted endtime


//############# Wichtig: GotPhoton-Funktion #############//
//Got Photon
//  TimeTag: Raw TimeTag from Record * Globalresolution = Real Time arrival of Photon
//  DTime: Arrival time of Photon after last Sync event (T3 only) DTime * Resolution = Real time arrival of Photon after last Sync event
//  Channel: Channel the Photon arrived (0 = Sync channel for T2 measurements)
void GotPhoton(long long TimeTag, int Channel, int DTime)
{
  if(IsT2)
  {
    //fprintf(fpout,"%I64u\n%1x\n%I64u\n%8.0lf\n", RecNum, Channel, TimeTag, (TimeTag * GlobRes * 1e12));

  	//Anstatt schreiben in andere Datei, wird nun der Inputvektor mit den Daten gefüllt
    inputvector0.push_back(RecNum);
    inputvector1.push_back(Channel);
    inputvector2.push_back(TimeTag);
    //inputvector3.push_back(TimeTag * GlobRes * 1e12);
    inputvector3.push_back(TimeTag * iGlobRes);

    allcounts ++;


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

  if (RecNum % 10000000 == 0){std::cout << RecNum * 1e-6 << "\t Mio\tTime:\t" << 1e-12 * inputvector3.back() << " s" << std::endl;}//Ausgabe des Fortschritts
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
      //timestamphistogram_c0->Fill(T2Rec.bits.timetag%8);
      }
    }
    else //regular input channel
    {
    truetime = oflcorrection + T2Rec.bits.timetag;
    c = T2Rec.bits.channel + 1;
    GotPhoton(truetime, c, 0);
    //timestamphistogram_c1->Fill(T2Rec.bits.timetag%8);
    }

}


void ProcessHHT3(unsigned int TTTRRecord, int HHVersion)
{
  const int T3WRAPAROUND = 1024;
  union {
    unsigned long allbits;
    struct  {
      unsigned nsync    :10;  // numer of sync period
      unsigned dtime    :15;  // delay from last sync in units of chosen resolution
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
    std::cout << "\n\nSTART COCOS" << "\n";

    PtuFile vPtuFile(argv[1]);
    vPtuFile.OpenPtuFile();
    vPtuFile.ReadHeader();


    Settings vSetting;
    vSetting.PrintSettings();
    
    std::string calibfilename = vSetting.GetCalibrationFileName();

	//############# Ende manuelles Einstellen-Zeug #############//
	//##########################################################//


    const int nbins = vSetting.GetNumberOfBins();
    double binwidth = 1.*(vSetting.GetTauEnd() - vSetting.GetTauBegin())/(1.*nbins);
    if (binwidth < 250){std::cout << "Warning: Binwidth (" << binwidth << " ps) is below the mimimum time-step of the electronics (250 ps)" << std::endl;}
    
    //Adjust start and end time, so that the bins are centered around their value
    int taubeg = vSetting.GetTauBegin() - (binwidth/2);
    int tauend = vSetting.GetTauEnd() - (binwidth/2);

    bool evaluating = false;//Check if we are evaluation at the moment

    //Get calibration data
    std::vector<float> vcal[2][2];
    std::vector<float> vcalt;
    int calTIME;
    double cal00, cal01, cal10, cal11;
    std::string line;
    std::ifstream infile;
    if (vSetting.GetCalibrationMode() == 1)
    {
        infile.open(calibfilename.c_str());
        if (!infile) 
        {
            std::cout << "Unable to open file Kalibration file. Please make sure it exists!" << std::endl;
            exit(1);   // call system to stop
        }
        while (std::getline(infile,line))
        {
            sscanf(line.c_str(),"%i\t%lf\t%lf\t%lf\t%lf", &calTIME, &cal00, &cal01, &cal10, &cal11);
            if(cal00 < 0.001){cal00 =1;}
            if(cal01 < 0.001){cal01 =1;}
            if(cal10 < 0.001){cal10 =1;}
            if(cal11 < 0.001){cal11 =1;}
            //std::cout << calTIME << "\t" << cal01 << "\t" << cal10 << std::endl;
            vcalt.push_back(calTIME);
            vcal[0][0].push_back(cal00); vcal[0][1].push_back(cal01);
            vcal[1][0].push_back(cal10); vcal[1][1].push_back(cal11);
        }
        infile.close();
    }
    else//Fill vectors with 1 so that the vector size is correct
    {
        for (int i=0; i<nbins; i++)
        {
          vcalt.push_back(1);
          vcal[0][0].push_back(1); vcal[1][0].push_back(1);
          vcal[1][0].push_back(1); vcal[1][1].push_back(1);
        }
    }

    //Start-timer for calculating the total computation time
    time_t timer;
    time(&timer);
    double begintime = timer;
    //std::cout << "Start time: " << timer << std::endl;
    
    std::vector<string> input; input.clear();


    TH1* allchannelhistogram[2][2];//Histogram array for [basicpoint-CHN][invesitgationpoint-CHN]
	  allchannelhistogram[0][0] = new TH1I("00-Histogram", "Events Channel 0", nbins, taubeg, tauend);
	  allchannelhistogram[0][1] = new TH1I("01-Histogram", "Cross-Correlation to Channel 0", nbins, taubeg, tauend);
	  allchannelhistogram[1][0] = new TH1I("10-Histogram", "Cross-Correlation to Channel 1", nbins, taubeg, tauend);
	  allchannelhistogram[1][1] = new TH1I("11-Histogram", "Events Channel 1", nbins, taubeg, tauend);
	  allchannelhistogram[0][0]->GetXaxis()->SetTitle("#tau [ps]");
	  allchannelhistogram[0][1]->GetXaxis()->SetTitle("#tau [ps]");
	  allchannelhistogram[1][0]->GetXaxis()->SetTitle("#tau [ps]");
	  allchannelhistogram[1][1]->GetXaxis()->SetTitle("#tau [ps]");

	TH1I *timestamphistogram[2];
	  timestamphistogram[0] = new TH1I ("Timestamp_c0","Timestamps Channel 0",10,0,10);
	  timestamphistogram[1] = new TH1I ("Timestamp_c1","Timestamps Channel 1",10,0,10);

    

	  double correlationarray[nbins][5][2][2];
	  //First  [] : Time (referred to bins)
	  //Second [] : What : (Time)  (Events)  (g2)  (smoothed g2)  (dg2)
	  //Third  [] : Which Channel is basic
	  //Fourth [] : Which channel is investigation
	
	 
	  int BchannelID; int IchannelID; //int totalfills = 0; int channelerrors = 
	  long long basicpoint; long long investigationpoint; long long deltat;
	  int isbin; double timerec; 
	  long long eventcounter = 0;

    //Fill time axis of correlation array
    for (int i=0; i<nbins; i++)
	  {
    	 correlationarray[i][0][0][0] = taubeg + binwidth/2 + i * binwidth;
    	 correlationarray[i][0][0][1] = taubeg + binwidth/2 + i * binwidth;
    	 correlationarray[i][0][1][0] = taubeg + binwidth/2 + i * binwidth;
    	 correlationarray[i][0][1][1] = taubeg + binwidth/2 + i * binwidth;
	  }


//##################################################################################//
//############# Jetzt wieder jede Menge Code aus dem ptu -> txt - file #############//

    
    GlobRes = vPtuFile.GetGlobalResolution();
    iGlobRes = vPtuFile.GetIGlobalResolution();
    long long RecordType = vPtuFile.GetRecordType();
    long long NumRecords = vPtuFile.GetNumberOfRecords();
    FILE* fpin = vPtuFile.GetFilePointer();
    
    int Result;
    long long maxinput = -1;//For later limitation of inputlines

    unsigned int TTTRRecord;
    std::cout << "Total number of records of the file:\t" << NumRecords << "\n";

    //############# Ende wieder jede Menge Code aus dem ptu -> txt - file #############\\
    //##################################################################################\\

  	//Ab jetzt: Vektor füllen, bis 10^9 Events, anschließend auswerten und das ganze nochmal machen...

  	//Jetzt kommt das eigentliche Auslesen aus der ptu-Datei:
  	//for(RecNum=0; RecNum<maxinput; RecNum++)
  	for(RecNum=0;RecNum<NumRecords;RecNum++)
  	{
  		eventcounter ++;
  		// GDEBUG << std::hex << "RecordType: " << RecordType <<  std::dec << std::endl;
  		Result = fread(&TTTRRecord, 1, sizeof(TTTRRecord) ,fpin);
    	if (Result!= sizeof(TTTRRecord))
    	  {
    	    std::cerr << "\nUnexpected end of input file!" << "\n";
    	    break;
    	  }

    	if (RecordType != rtTimeHarp260NT2)
    	{
    		std::cerr << "Wrong Record Type!" << "\n";
    		break;
    	}

    	IsT2 = true;
    	ProcessHHT2(TTTRRecord, 2);

    	if (eventcounter >= 1e8 || RecNum >= (NumRecords-1))//Jezt kommt die ganze Auswertung einer 10^8-Reihe!
    	{
    		std::cout << "EVC: " << eventcounter << "  RecNum " << RecNum << std::endl;

        //Evaluate only if last time is not before start evaluation time
        if (inputvector3.back() >= vSetting.GetStartEvalTime())
        {
                      //###############################################//
                     std::cout << "Space-Time-Evaluation:" << std::endl;
                     //###############################################//
                     long long partinputs = inputvector0.size();
              
                     if (tauend > 0)//Fill positive time-range
                     {
                       std::cout << ">Positive time range..." << std::endl;
                       for (long long i=0; i<partinputs-1; i++)
                       {
                         //Pass ranges that are not within the adjusted evaluation time range:
                         if (vSetting.GetTimeLimitation() == true && inputvector3.at(i) < vSetting.GetStartEvalTime()){startlinetotime ++; goto skipevalp;}
                         else if (startmeastimeout == false){startmeastime = inputvector3.at(i); startmeastimeout = true;}
                         ccounts[inputvector1.at(i)] ++;
                         //End evaluation complete if current time is larger than the upper adjusted timelimit:
                         if (vSetting.GetTimeLimitation() == true && inputvector3.at(i) > vSetting.GetEndEvalTime()){endlinetotime = i; if(taubeg>0){goto close;}; goto closep;}
              
                           //if (i % 10000000 == 0){std::cout << "\t" << 100.*i/(1.*partinputs) << " %" << std::endl;}
                         
                           basicpoint = i;
                           investigationpoint = i+1;
                           deltat = inputvector3.at(investigationpoint) - inputvector3.at(basicpoint);
                           BchannelID = inputvector1.at(basicpoint);
                           //Fill timestamphistogram (only during positive time range)
                           timestamphistogram[BchannelID]->Fill((inputvector3.at(basicpoint)/250)%8);
                         
                           while (deltat < taubeg && investigationpoint < partinputs-1)//Find start photon, only for taubeg > 0
                           {   
                               investigationpoint ++;
                                 deltat = inputvector3.at(investigationpoint) - inputvector3.at(basicpoint);
                           }
                         
                           while (deltat < tauend && investigationpoint < partinputs-1)
                           {
                                 //BchannelID = inputvector1.at(basicpoint);
                                 IchannelID = inputvector1.at(investigationpoint);
                               allchannelhistogram[BchannelID][IchannelID]->Fill(deltat);
              
                               investigationpoint ++;
                                 deltat = inputvector3.at(investigationpoint) - inputvector3.at(basicpoint);
                           }
              
                           endlinetotime = i;//If it did not stop before now the endline is set to the last entry.
                           endmeastime = inputvector3.at(i);//Can be always done, as it always increases
                           skipevalp:;//Marker for not evaluating
                       }
                     }
                     closep:;
              
              
                     if (taubeg < 0)//Fill negative time-range - do everything backwards
                     {
                       std::cout << ">Negative time range..." << std::endl;
                       for (long long i=1; i<partinputs; i++)//Start with i=1 as i-1=0
                       {
                         //Pass ranges that are not within the adjusted evaluation time range:
                         if (vSetting.GetTimeLimitation() == true && inputvector3.at(i) < vSetting.GetStartEvalTime()){goto skipevaln;}
                         if (tauend <= 0){ccounts[inputvector1.at(i)] ++;}//Count events as well when only negative time range
                         //End evaluation completely if actual time is larger than the upper adjusted timelimit:
                         if (vSetting.GetTimeLimitation() == true && inputvector3.at(i) > vSetting.GetEndEvalTime()){goto close;}
              
              
                           //if (i % 10000000 == 0){std::cout << "\t" << 100.*i/(1.*partinputs) << " %" << std::endl;}
                             
                             basicpoint = i;
                             investigationpoint = i-1;   
                             deltat = inputvector3.at(investigationpoint) - inputvector3.at(basicpoint);
                             BchannelID = inputvector1.at(basicpoint);
                  
                             while (deltat > tauend && investigationpoint > 0)//Find start Photon, only for tauend < 0
                             {
                                 investigationpoint --;
                                 deltat = inputvector3.at(investigationpoint) - inputvector3.at(basicpoint);
                             }
                  
                             while (deltat > taubeg && investigationpoint > 0)
                             {
                                 //BchannelID = inputvector1.at(basicpoint);
                                 IchannelID = inputvector1.at(investigationpoint);   
                                 allchannelhistogram[BchannelID][IchannelID]->Fill(deltat);
                  
                                 investigationpoint --;
                                 deltat = inputvector3.at(investigationpoint) - inputvector3.at(basicpoint);
                             }
                             skipevaln:;//Marker for not evaluating
                       }
                     }
                     closen:;
        }

    		

    		//Read out the startmeastime as first element of the inputvector, but only once!
    		//if (startmeastimeout == false){std::cout << "PPPPL: " << startlinetotime << std::endl; startmeastime = inputvector3.at(startlinetotime); startmeastimeout = true;}

    		//Reset important values for the next round
    		eventcounter = 0; startlinetotime = 0;
    		inputvector0.clear(); inputvector1.clear(); inputvector2.clear(); inputvector3.clear();
    	}
  	}

	  close:
	  fclose(fpin);
	  //fclose(fpout);
	



	//############# Ende Code aus der ptu -> txt - Datei #############//
	//################################################################//


    
    //const int arraysize = inputvector0.size();
    long long inputs = allcounts;//Doesn't consider a startoffset of lines/time
    std::cout << "Inputs : " << inputs << std::endl;
    std::cout << "Overflow events: " << noverwlow << std::endl;

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
    std::cout << "0\t" << ccounts[0] << "\t" << c0_rate << std::endl;
    std::cout << "1\t" << ccounts[1] << "\t" << c1_rate << std::endl;
	  std::cout << "Ratio 0:1 : " << rateratio << "\t1:0 : " << invrateratio << std::endl;
	  std::cout << "--------------------------------------------" << std::endl;    

   

	std::cout << ">Further analysis..." << std::endl;

    //Fill the Histogram-Events into array
    for (int i=0; i<nbins; i++)
    {
        correlationarray[i][1][0][0] = allchannelhistogram[0][0]->GetBinContent(i+1);
        correlationarray[i][1][0][1] = allchannelhistogram[0][1]->GetBinContent(i+1);
        correlationarray[i][1][1][0] = allchannelhistogram[1][0]->GetBinContent(i+1);
        correlationarray[i][1][1][1] = allchannelhistogram[1][1]->GetBinContent(i+1);
    }


    

    //Normalisation: Pol-0 fit
    int maxbin; int minbin; int bindiff; double maxx;//Used to give some start points of fit

    TF1 *pol0fit[2][2];
    double pol0off[2][2];//Will be fit-parameter
    double dpol0off[2][2];//Will be fit error
    double manualerror[2][2]; double manualvariance[2][2];

    TGraphErrors * pol0fitgraph[2][2];
    ostringstream graphtitleos[2][2];
    std::string graphtitle[2][2];

    TLine * pol0fitline[2][2];


    for (int bas=0; bas<2; bas++)
    {
        for (int inv=0; inv<2; inv++)
        {
            maxbin = allchannelhistogram[bas][inv]->GetMaximum();
            minbin = allchannelhistogram[bas][inv]->GetMinimum();
            bindiff = maxbin - minbin;
            maxx = allchannelhistogram[bas][inv]->GetMaximumBin();

            //pol0fit[bas][inv] = new TF1("Pol-0-Fit","pol0", taubeg, tauend);
            pol0fit[bas][inv] = new TF1("Pol-0-Fit","pol0", taubeg + 3*(tauend-taubeg)/4, tauend);
            pol0fit[bas][inv]->SetParName(0,"offfset"); //pol0fit[bas][inv]->SetParameter(0, allchannelhistogram[bas][inv]->GetBinContent(0));

            allchannelhistogram[bas][inv]->Fit(pol0fit[bas][inv], "0QR");
            pol0fit[bas][inv]->SetLineColor(2);
            pol0fit[bas][inv]->SetFillColor(2); pol0fit[bas][inv]->SetFillStyle(3001);
            //gausfit[bas][inv]->GetYaxis()->SetRangeUser(minbin - 0.05 * bindiff, maxbin + 0.05* bindiff);

            //Fill fit-parameters
            pol0off[bas][inv] = pol0fit[bas][inv]->GetParameter(0); dpol0off[bas][inv] = pol0fit[bas][inv]->GetParError(0);

            pol0fitline[bas][inv] = new TLine(taubeg, pol0off[bas][inv], tauend, pol0off[bas][inv]);
            pol0fitline[bas][inv]->SetLineColor(5); pol0fitline[bas][inv]->SetLineWidth(2);
        }
    }

    


    //Create g2-function: Divide events by the normalisation by pol0-fit
    double g2_max[2][2], g2_min[2][2];//Will become maximum and minimum values of arraycolumn
    int g2_max_x[2][2];//X-Bin-Value of maximum value
    TGraph *g2[2][2];
    TGraphErrors *g2witherrors[2][2];
    for (int bas=0; bas<2; bas++)
    {
        for (int inv=0; inv<2; inv++)
        {
            g2[bas][inv] = new TGraph(nbins);
            g2[bas][inv]->SetFillColor(33);
            g2[bas][inv]->SetLineColor(2);
            g2[bas][inv]->SetLineWidth(2);
            g2[bas][inv]->SetMarkerColor(4);
            g2[bas][inv]->SetMarkerStyle(1);
            g2[bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
            g2[bas][inv]->GetYaxis()->SetTitle("g_{2}(#tau)");

            g2_max[bas][inv] = 0.;
            g2_max_x[bas][inv] = 0;
            for (int i=0; i<nbins; i++)
            {
                correlationarray[i][2][bas][inv] = 1.*(correlationarray[i][1][bas][inv])/(pol0off[bas][inv]);
                if (vSetting.GetCalibrationMode() == 1){correlationarray[i][2][bas][inv] /= vcal[bas][inv].at(i);}
                g2[bas][inv]->SetPoint(i, correlationarray[i][0][bas][inv], correlationarray[i][2][bas][inv]);
                if (correlationarray[i][2][bas][inv] > g2_max[bas][inv])
                {
                	g2_max[bas][inv] = correlationarray[i][2][bas][inv];
                	g2_max_x[bas][inv] = i;
                }
            }
        }
    }
    g2[0][0]->SetTitle("g_{2}-function channel 0 to channel 0");
    g2[0][1]->SetTitle("g_{2}-function channel 0 to channel 1");
    g2[1][0]->SetTitle("g_{2}-function channel 1 to channel 0");
    g2[1][1]->SetTitle("g_{2}-function channel 1 to channel 1");

    //g2-Distribution histogramm
    double g2ymax, g2ymin;
    TH1D * g2disthisto[2][2];
    for (int bas=0; bas<2; bas++)
    {
    	for (int inv=0; inv<2; inv++)
    	{
    		g2ymax = correlationarray[0][2][bas][inv]; g2ymin = correlationarray[0][2][bas][inv];
    		for (int i=0; i<nbins; i++)
    		{
    			if (correlationarray[i][2][bas][inv] > g2ymax){g2ymax = correlationarray[i][2][bas][inv];}
    			if (correlationarray[i][2][bas][inv] < g2ymin){g2ymin = correlationarray[i][2][bas][inv];}
    		}
    		g2disthisto[bas][inv] = new TH1D("g2dist","g2-Distribution", 4000, 0.9, 1.1);
    		//std::cout << "nbins: " << (g2ymax - g2ymin)/0.001 << std::endl;
    		for (int i=0; i<nbins; i++)
    		{
    			g2disthisto[bas][inv]->Fill(correlationarray[i][2][bas][inv]);
    		}
    	}
    }
    g2disthisto[0][0]->SetTitle("g_{2}-distribution channel 0 to channel 0");
    g2disthisto[0][1]->SetTitle("g_{2}-distribution channel 0 to channel 1");
    g2disthisto[1][0]->SetTitle("g_{2}-distribution channel 1 to channel 0");
    g2disthisto[1][1]->SetTitle("g_{2}-distribution channel 1 to channel 1");


    //FFT of (g2-)histogram
    TH1F * g2histo[2][2];
    for (int bas=0; bas<2; bas++)
    {
      for (int inv=0; inv<2; inv++)
      {
        g2histo[bas][inv] = new TH1F("g2histo","Histogram of g2-Function", nbins, taubeg, tauend);
        for (int i=0; i<nbins; i++)
        {
          g2histo[bas][inv]->SetBinContent(i+1, correlationarray[i][2][bas][inv]);
        }
      }
    }

    TH1 * fouriertransform[2][2];
    ostringstream fftstring[2][2]; std::string ffttitle;

    for (int bas=0; bas<2; bas++)
    {
        for (int inv=0; inv<2; inv++)
        {
            fftstring[bas][inv] << "Magnitude of the first transform  Channel " << bas << " <> Channel " << inv;
            ffttitle = fftstring[bas][inv].str();
 
            fouriertransform[bas][inv] = 0;
            TVirtualFFT::SetTransform(0);
            //fouriertransform[bas][inv] = allchannelhistogram[bas][inv]->FFT(fouriertransform[bas][inv], "MAG");
            fouriertransform[bas][inv] = g2histo[bas][inv]->FFT(fouriertransform[bas][inv], "MAG");
            fouriertransform[bas][inv]->SetTitle(ffttitle.c_str());
            fouriertransform[bas][inv]->GetXaxis()->SetLimits(0, nbins/(1e-12*(tauend - taubeg)));//Rescale x-axis for real frequencies
            fouriertransform[bas][inv]->GetXaxis()->SetTitle("f [Hz]");
        }
    }



    //Smooth curve - array[][3] by averaging
    int nsmooth = 5;//Number of averaging points (should be odd)
    
    int startendsmooth = (nsmooth + 1)/2;
    int jstart = -1* (startendsmooth -1);
    int jend   =  1* (startendsmooth -1);
    
    double g2_smoothed_max[2][2]; int g2_smoothed_max_x[2][2];
    double smoothtotal;
    TGraph *g2_smoothed[2][2];
    for (int bas=0; bas<2; bas++)
    {
        for (int inv=0; inv<2; inv++)
        {
            g2_smoothed[bas][inv] = new TGraph(nbins-5 -2*jend);
            g2_smoothed[bas][inv]->SetFillColor(33);
            g2_smoothed[bas][inv]->SetLineColor(5);
            g2_smoothed[bas][inv]->SetLineWidth(2);
            g2_smoothed[bas][inv]->SetMarkerColor(4);
            g2_smoothed[bas][inv]->SetMarkerStyle(1);
            g2_smoothed[bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
            g2_smoothed[bas][inv]->GetYaxis()->SetTitle("g_{2}(#tau)");

            g2_smoothed_max[bas][inv] = 0;
            for (int i=startendsmooth; i<(nbins - startendsmooth); i++)
            {
                smoothtotal = 0;
                for (int j=jstart; j<(jend +1); j++)
                {
                    smoothtotal += correlationarray[i+j][2][bas][inv];
                }
                correlationarray[i][3][bas][inv] = smoothtotal/(1.*nsmooth);
                g2_smoothed[bas][inv]->SetPoint(i-startendsmooth, correlationarray[i][0][bas][inv], correlationarray[i][3][bas][inv]);
                //double hmx, hmy; g2_smoothed[bas][inv]->GetPoint(i-startendsmooth, hmx, hmy);
                //std::cout << "SMO:  i: " << i-startendsmooth << "\tx: " << hmx << "\ty: " << hmy << std::endl;
                if (correlationarray[i][3][bas][inv] > g2_smoothed_max[bas][inv])
                {
                		g2_smoothed_max[bas][inv] = correlationarray[i][3][bas][inv];
                		g2_smoothed_max_x[bas][inv] = i-startendsmooth;
                }
            }
        }
    }
    g2_smoothed[0][0]->SetTitle("Smoothed g_{2}-function channel 0 to channel 0");
    g2_smoothed[0][1]->SetTitle("Smoothed g_{2}-function channel 0 to channel 1");
    g2_smoothed[1][0]->SetTitle("Smoothed g_{2}-function channel 1 to channel 0");
    g2_smoothed[1][1]->SetTitle("Smoothed g_{2}-function channel 1 to channel 1");
    

      

    time(&timer);
    double finishtime = timer;
    //std::cout << "End time: " << timer << std::endl;
    double timediff = finishtime - begintime;

    
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Total computation time: " << timediff << " s" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;

  system("mkdir results");
  //Infofile
  ofstream info;
  info.open("results/infos.txt");
   //fileout << fixed;
   info << "File: " << argv[1] << "\n";
   info << "Computation time:\t" << timediff << " s\n";
   info << "---------------------------------------------------\n";
   info << "Starttime: " << 1e-12 * startmeastime << " s\t\tEndtime: " << 1e-12 * endmeastime << " s\n";
   info << "Effective Measurement time : " << 1e-12 * meastime << " s\n";
   info << "Channel\tCounts\t\tRate (MHz)\n";
   info << "0\t" << ccounts[0] << "\t" << c0_rate << "\n";
   info << "1\t" << ccounts[1] << "\t" << c1_rate << "\n";
   info << "Total counts:\t" << ccounts[0] + ccounts[1] << "\n";
   info << "Ratio 0:1 : " << rateratio << "\t1:0 : " << invrateratio << "\n";
  info.close();


    
    //Calibration file
    if (vSetting.GetCalibrationMode() == 2)
    {
      ofstream fileout;
      fileout << setprecision(15);
      fileout.open(calibfilename.c_str());
      //fileout << fixed;
      fileout << "#---------------------------------------------------\n";
      fileout << "# File: " << argv[1] << "\n";
      fileout << "# is " << 1e-12 * vSetting.GetStartEvalTime() << ";\tie " << 1e-12 * vSetting.GetEndEvalTime() << ";\tsl " << vSetting.GetTimeLimitation() << "\n";
      fileout << "# ts " << taubeg + (binwidth/2) << ";\tte " << tauend + (binwidth/2) << ";\tnb " << nbins << " ->Width: " << binwidth << "\n";
      fileout << "# Real start time, end time and measurement time [s]:\n";
      fileout << "# " << 1e-12 * startmeastime << "\t" << 1e-12 * endmeastime << "\t" << 1e-12 * meastime << "\n";
      fileout << "# Rates: [MHz]\n";
      fileout << "# " << c0_rate << "\t" << c1_rate << "\n";
      fileout << "#---------------------------------------------------\n";
      for (int b=0; b<nbins; b++)
      {
        //Write g2:
        //fileout << correlationarray[b][0][0][1] << "\t" << correlationarray[b][2][0][0] << "\t" << correlationarray[b][2][0][1] << "\t" << correlationarray[b][2][1][0] << "\t" << correlationarray[b][2][1][1] << "\n";
        //Write Events:
        fileout << correlationarray[b][0][0][1] << "\t" << correlationarray[b][1][0][0] << "\t" << correlationarray[b][1][0][1] << "\t" << correlationarray[b][1][1][0] << "\t" << correlationarray[b][1][1][1] << "\n";
      }
      fileout.close();
    }
    

    //Information for you that the evaluation has finished  
    //system("afplay /System/Library/Sounds/Glass.aiff");
    //system("afplay /System/Library/Sounds/Glass.aiff");  

    std::cout << "Enter anything to continue  ";
    std::string unusedstring;
    cin >>  unusedstring;

    int whichpad[2][2];//Assings histgorams etc to canvas position
    whichpad[0][0] = 1;
    whichpad[0][1] = 2;
    whichpad[1][0] = 3;
    whichpad[1][1] = 4;


    char argc2[8]; char argv2[8];


    //##############################################################\\
    //################ APPLICATION FOR THE CANVASES ################\\

    TApplication TheCorrelationCanvas("TCC",0,0);

          /*TCanvas *timestampcanvas = new TCanvas("timestamp","timestamp",700,500);
          timestampcanvas->SetFillColor(29); timestampcanvas->SetGrid(); timestampcanvas->Divide(2,1);
          timestampcanvas->cd(1); timestamphistogram[0]->Draw();
          timestampcanvas->cd(2); timestamphistogram[1]->Draw();
          timestampcanvas->Modified(); timestampcanvas->Update();
          timestampcanvas->SaveAs("results/ts.root");*/
      
          TCanvas * CorrelationCanvas = new TCanvas("CorrelationCanvas", "Correlations", 700, 500);
          CorrelationCanvas->SetFillColor(29); CorrelationCanvas->SetGrid(); CorrelationCanvas->Divide(2,2);
          for (int bas=0; bas<2; bas++)
          {
              for (int inv=0; inv<2; inv++)
              {
              	CorrelationCanvas->cd(whichpad[bas][inv]);
              	allchannelhistogram[bas][inv]->Draw();
              	//pol0fit[bas][inv]->Draw("same");
                if (bas != inv){pol0fitline[bas][inv]->Draw();}
              }
          }
          CorrelationCanvas->Modified(); CorrelationCanvas->Update();
          CorrelationCanvas->SaveAs("results/Events.root");
      
          TCanvas * g2Canvas = new TCanvas("g_{2}-Canvas", "g_{2}-functions", 700, 500);
          g2Canvas->SetFillColor(29); g2Canvas->SetGrid(); g2Canvas->Divide(2,2);
          for (int bas=0; bas<2; bas++)
          {
              for (int inv=0; inv<2; inv++)
              {
                  g2Canvas->cd(whichpad[bas][inv]);
                  g2[bas][inv]->Draw();
                  g2_smoothed[bas][inv]->Draw("same");
              }
          }
          g2Canvas->Modified(); g2Canvas->Update();
          g2Canvas->SaveAs("results/g2.root");

          TCanvas * g2distCanvas = new TCanvas("g_{2}-dist-Canvas", "g_{2}-Distributions", 700, 500);
          g2distCanvas->SetFillColor(29); g2distCanvas->SetGrid(); g2distCanvas->Divide(2,2);
          for (int bas=0; bas<2; bas++)
          {
              for (int inv=0; inv<2; inv++)
              {
                  g2distCanvas->cd(whichpad[bas][inv]);
                  g2disthisto[bas][inv]->Draw();
              }
          }
          g2distCanvas->Modified(); g2distCanvas->Update();
          g2distCanvas->SaveAs("results/g2dist.root");
      
          TCanvas * FFTCanvas = new TCanvas("fftCanvas", "FFT-Canvas", 700, 500);
          FFTCanvas->SetFillColor(29); FFTCanvas->SetGrid(); FFTCanvas->Divide(2,2);
          for (int bas=0; bas<2; bas++)
          {
                for (int inv=0; inv<2; inv++)
                {
                    FFTCanvas->cd(whichpad[bas][inv]);
                    fouriertransform[bas][inv]->Draw();
                }
          }
          FFTCanvas->Modified(); FFTCanvas->Update();
          FFTCanvas->SaveAs("results/FFT.root");

    TheCorrelationCanvas.Run();        
    //################ Application for the canvases ################\\
    //##############################################################\\

  

}//End of main



//######################## START NOT IMPORTANT NOW ###############################
//Some old functions that are not used any more but maybe need to be revived some day
    /*


      double normstart = 0.7; //Define relative startpoint (referred to the whole histogram area) for normalisation of the time-correlation
      int timelimit = 0;//Spatial-Evaluation (Channel 0 <> Channel 1): Set timelimit for events
      std::string documentline;
      FOR WINDOWS PC IN LAB
      documentline = "C:\\Users\\II\\Documents\\1MHz_High_Lines_2test.txt";
      documentline = "C:\\Users\\II\\Documents\\1MHz_High_Lines.txt";    
      FOR MAC IN OFFICE
      documentline = "/Users/azmija/Documents/Masterarbeit/COCOS/Daten/20MHz_High_Lines.txt";
      documentline = "/Users/azmija/Documents/Masterarbeit/COCOS/Messungen/2017_09_07/Daten/txt/Messung2.txt";
      FOR linux in lab
      documentline = "/home/iceact/Documents/NewReader/test.txt";



      //Possibility to create a resultfile with the values of the g2-Function etc
      std::string readoutfileline;
      readoutfileline = "/Users/azmija/Documents/Masterarbeit/COCOS/Messungen/2017_09_22/Auswertungen/Verschiebung/Messung1.txt";
      bool writefile = false;//true == write output-file



    

    
    //#################################################
    std::cout << ">Channel-separation..." << std::endl;
    //#################################################
    
    time(&timer);
    double startseperatetime = timer;
    
    //One array for each channel: (recID) (CHN) (nsync) (truetime)
    const int c0_array_size = c0_counts; const int c1_array_size = c1_counts;
    double c0_dataarray[c0_array_size][4]; double c1_dataarray[c1_array_size][4];
    int c0_position = 0; int c1_position = 0;//For channel-arrays

    for (int i=0; i<inputs; i++)
    {
    
        if(inputarray[i][1] == 0.)
        {
            c0_dataarray[c0_position][0] = inputarray[i][0];
            c0_dataarray[c0_position][1] = inputarray[i][1];
            c0_dataarray[c0_position][2] = inputarray[i][2];
            c0_dataarray[c0_position][3] = inputarray[i][3];
            
            c0_position ++;
        }
        if (inputarray[i][1] == 1.)
        {
            c1_dataarray[c1_position][0] = inputarray[i][0];
            c1_dataarray[c1_position][1] = inputarray[i][1];
            c1_dataarray[c1_position][2] = inputarray[i][2];
            c1_dataarray[c1_position][3] = inputarray[i][3];
            
            c1_position ++;
        }
    }

    time(&timer);
    double endseperatetime = timer;
    double seperatetime = endseperatetime - startseperatetime;
   
    


    //Test the channel-spearated arrays
    
     std::cout << "Channel 0:" << std::endl;
     for(int i=0; i< c0_counts; i++)
     {
     std::cout << "#REC: " << c0_dataarray[i][0] << " CHN: " << c0_dataarray[i][1] << " nsync: " << c0_dataarray[i][2] << " Time: " << c0_dataarray[i][3] << std::endl;
     }
     std::cout << "Channel 1:" << std::endl;
     for(int i=0; i< c1_counts; i++)
     {
     std::cout << "#REC: " << c1_dataarray[i][0] << " CHN: " << c1_dataarray[i][1] << " nsync: " << c1_dataarray[i][2] << " Time: " << c1_dataarray[i][3] << std::endl;
     }


     //################
    //G2-TIME-ANALYSIS
    //################





     
    double e = 2.71828;
    
    //Normalisation Method: Gauß-Fit (Later: Pol0-Fit)
    int maxbin; int minbin; int bindiff; double maxx;//Used to give some start points of fit

    TF1 *gausfit[2][2];
    double gausamp[2][2]; double gausmed[2][2]; double gaussig[2][2]; double gausoff[2][2];//Will be fit-parameters
    double dgausamp[2][2]; double dgausmed[2][2]; double dgaussig[2][2]; double dgausoff[2][2];//Will be fit errors
    double oneovere[2][2];//Will be threshold for determining correlation time
    double abstau_c[2][2];//Absolute correlation time (will be reduced by medium of gaus-fit)
    double reltau_c[2][2];//Reduced correlation time after substracting medium of gaus-fit
    TLine *oneovereline[2][2]; TLine *tau_cline[2][2];
    double manualerror[2][2]; double manualvariance[2][2];

    TGraphErrors * gausfitgraph[2][2];
    ostringstream graphtitleos[2][2];
    std::string graphtitle[2][2];


    for (int bas=0; bas<2; bas++)
    {
        for (int inv=0; inv<2; inv++)
        {
            maxbin = allchannelhistogram[bas][inv]->GetMaximum();
            minbin = allchannelhistogram[bas][inv]->GetMinimum();
            bindiff = maxbin - minbin;
            maxx = allchannelhistogram[bas][inv]->GetMaximumBin();

            gausfit[bas][inv] = new TF1("Gausfit","[0] * exp(- (x-[1])^2 / (2*[2]^2) ) + [3]", taubeg, tauend);
            gausfit[bas][inv]->SetParName(0,"amplitude"); gausfit[bas][inv]->SetParameter(0, bindiff); //std::cout << "Startampl.: " << bindiff << std::endl;
            gausfit[bas][inv]->SetParName(1,"medium"); gausfit[bas][inv]->SetParameter(1, maxx);
            gausfit[bas][inv]->SetParName(2,"sigma"); gausfit[bas][inv]->SetParameter(2, 1000000);
            gausfit[bas][inv]->SetParName(3,"offset"); gausfit[bas][inv]->SetParameter(3, minbin); //std::cout << "Startoffset: " << minbin << std::endl;

            allchannelhistogram[bas][inv]->Fit(gausfit[bas][inv], "0QR");
            gausfit[bas][inv]->SetLineColor(2);
            gausfit[bas][inv]->SetFillColor(2); gausfit[bas][inv]->SetFillStyle(3001);
            //gausfit[bas][inv]->SetMarkerSize(1); gausfit[bas][inv]->SetMarkerStyle(34); gausfit[bas][inv]->SetMarkerColor(2);
            gausfit[bas][inv]->GetYaxis()->SetRangeUser(minbin - 0.05 * bindiff, maxbin + 0.05* bindiff);
            //Fill fit-parameters
            gausamp[bas][inv] = gausfit[bas][inv]->GetParameter(0); dgausamp[bas][inv] = gausfit[bas][inv]->GetParError(0);
            gausmed[bas][inv] = gausfit[bas][inv]->GetParameter(1); dgausmed[bas][inv] = gausfit[bas][inv]->GetParError(1);
            gaussig[bas][inv] = gausfit[bas][inv]->GetParameter(2); dgaussig[bas][inv] = gausfit[bas][inv]->GetParError(2);
            gausoff[bas][inv] = gausfit[bas][inv]->GetParameter(3); dgausoff[bas][inv] = gausfit[bas][inv]->GetParError(3);
            //Determine threshold for correlation time
            oneovere[bas][inv] = gausoff[bas][inv] + (gausamp[bas][inv]/e);
            //Find absolute correlation time from the fit
            abstau_c[bas][inv] = gausfit[bas][inv]->GetX(oneovere[bas][inv]);
            //Reduced correlation time
            reltau_c[bas][inv] = abs(abstau_c[bas][inv] - gausmed[bas][inv]);

            //Lines
            oneovereline[bas][inv] = new TLine(gausmed[bas][inv], oneovere[bas][inv], abstau_c[bas][inv], oneovere[bas][inv]);
            oneovereline[bas][inv]->SetLineColor(5); oneovereline[bas][inv]->SetLineWidth(3);

            tau_cline[bas][inv] = new TLine(abstau_c[bas][inv], gausoff[bas][inv], abstau_c[bas][inv], oneovere[bas][inv]);
            tau_cline[bas][inv]->SetLineColor(5); tau_cline[bas][inv]->SetLineWidth(3);




            //SO HALB SINNVOLL:
            //Calculate standard error manually
            double squaresum = 0.; double yfit; double ymeas;
            //std::cout << "\n\n" << std::endl;
            for (int i=0; i<nbins; i++)
            {
              ymeas = correlationarray[i][1][bas][inv]; //std::cout << "Ymeas: " << ymeas;
              yfit  = gausfit[bas][inv]->Eval( taubeg + i * binwidth ); //std::cout << "   Yfit: " << yfit << std::endl;
              squaresum += pow( (ymeas - yfit) , 2); 
            }
            manualvariance[bas][inv] = squaresum/(nbins-1); //std::cout << "Variance:  " << manualvariance[bas][inv] << std::endl;
            manualerror[bas][inv] = sqrt(manualvariance[bas][inv]); //std::cout << "Error:  " << manualerror[bas][inv] << std::endl;

            gausfitgraph[bas][inv] = new TGraphErrors(nbins);
            for (int i=0; i<nbins; i++)
            {
              gausfitgraph[bas][inv]->SetPoint(i, correlationarray[i][0][bas][inv], gausfit[bas][inv]->Eval(correlationarray[i][0][bas][inv]));
              gausfitgraph[bas][inv]->SetPointError(i, 0., manualerror[bas][inv]);
            }
            gausfitgraph[bas][inv]->SetLineColor(2);
            gausfitgraph[bas][inv]->SetFillColor(2); gausfitgraph[bas][inv]->SetFillStyle(3001);
            gausfitgraph[bas][inv]->GetYaxis()->SetRangeUser(minbin - 0.05 * bindiff, maxbin + 0.05* bindiff);
            graphtitleos[bas][inv] << "Channel " << bas << " <> Channel" << inv;
            graphtitle[bas][inv] = graphtitleos[bas][inv].str();
            gausfitgraph[bas][inv]->SetTitle(graphtitle[bas][inv].c_str());
        }
    }

    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "CORRELATION TIMES BY GAUS-FIT" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Calculated Correlation Times by Gaus-fit [Reduced]:" << std::endl;
    std::cout << "[0][0] : " << reltau_c[0][0] << " ps\t\t[0][1] : " << reltau_c[0][1] << " ps" << std::endl;
    std::cout << "[1][0] : " << reltau_c[1][0] << " ps\t\t[1][1] : " << reltau_c[1][1] << " ps" << std::endl;
    std::cout << "Calculated Correlation Times by Gaus-fit [Absolute]:" << std::endl;
    std::cout << "[0][0] : " << abstau_c[0][0] << " ps\t\t[0][1] : " << abstau_c[0][1] << " ps" << std::endl;
    std::cout << "[1][0] : " << abstau_c[1][0] << " ps\t\t[1][1] : " << abstau_c[1][1] << " ps" << std::endl;
    std::cout << "Medium Values" << std::endl;
    std::cout << "[0][0] : " << gausmed[0][0] << " ps\t\t[0][1] : " << gausmed[0][1] << " ps" << std::endl;
    std::cout << "[1][0] : " << gausmed[1][0] << " ps\t\t[1][1] : " << gausmed[1][1] << " ps" << std::endl;
    std::cout << "--------------------------------------------------------------------------------" << std::endl;





    //Create g0-Functions as correlationarray[i][2][bas][inv]

    
    //Normalisation by Gaußfit
    double g2_max[2][2];//Will become maximum value of arraycolumn
    int g2_max_x[2][2];//X-Bin-Value of maximum value
    TGraph *g2[2][2];
    TGraphErrors *g2witherrors[2][2];
    for (int bas=0; bas<2; bas++)
    {
        for (int inv=0; inv<2; inv++)
        {
            g2[bas][inv] = new TGraph(nbins);
            g2[bas][inv]->SetFillColor(33);
            g2[bas][inv]->SetLineColor(2);
            g2[bas][inv]->SetLineWidth(2);
            g2[bas][inv]->SetMarkerColor(4);
            g2[bas][inv]->SetMarkerStyle(1);
            g2[bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
            g2[bas][inv]->GetYaxis()->SetTitle("g_{2}(#tau)");

            g2_max[bas][inv] = 0.;
            g2_max_x[bas][inv] = 0;
            for (int i=0; i<nbins; i++)
            {
                correlationarray[i][2][bas][inv] = 1.*(correlationarray[i][1][bas][inv])/(gausoff[bas][inv]);
                g2[bas][inv]->SetPoint(i, correlationarray[i][0][bas][inv], correlationarray[i][2][bas][inv]);
                if (correlationarray[i][2][bas][inv] > g2_max[bas][inv])
                {
                  g2_max[bas][inv] = correlationarray[i][2][bas][inv];
                  g2_max_x[bas][inv] = i;
                }
            }




            //Error propagation from manual calculated errors
            g2witherrors[bas][inv] = new TGraphErrors(nbins);
            double B  = gausoff[bas][inv];
            double dB = manualerror[bas][inv];//Error on normalisation
            double C;//Countrate
            double dg2;//Error on g2-Funktion
            for (int i=0; i<nbins; i++)
            {
              C = correlationarray[i][1][bas][inv];
              dg2 = sqrt(C * (1+C*pow(dB/B, 2)))/B;//Formula derived via error-propagation

              correlationarray[i][4][bas][inv] = dg2;
              g2witherrors[bas][inv]->SetPoint(i, taubeg + i * binwidth, C/B);
              g2witherrors[bas][inv]->SetPointError(i, 0., dg2);
            }
            g2witherrors[bas][inv]->SetFillColor(33);
            g2witherrors[bas][inv]->SetLineColor(2);
            g2witherrors[bas][inv]->SetLineWidth(2);
            g2witherrors[bas][inv]->SetMarkerColor(4);
            g2witherrors[bas][inv]->SetMarkerStyle(1);
            g2witherrors[bas][inv]->SetTitle(graphtitle[bas][inv].c_str());
            g2witherrors[bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
            g2witherrors[bas][inv]->GetYaxis()->SetTitle("g_{2}(#tau)");
        }
    }
    

    
    
  
    
    
    //#################################################
    std::cout << ">Spatial evaluation..." << std::endl;
    //#################################################
    time(&timer);
    double spacialstartevaluationtime = timer;
    
    int spacialcounts = 0;
    for (int i=0; i<inputs-1; i++)
    {
        basicpoint = i;
        investigationpoint = i+1;
        deltat = inputarray[investigationpoint][3] - inputarray[basicpoint][3];
        
        while (deltat <= (timelimit + range) && investigationpoint < inputs-1)
        {
            if (inputarray[investigationpoint][1] != inputarray[basicpoint][1])//Compare events from DIFFERENT channels
            {
                spacialcounts ++;
                //std::cout << "Hit: i= " << i << "; Channel " << channelID << "; RECID: " << recordIDvec[i] << "; Time: " << truetimevec[i] << " *** Channel " << channelIDvec[investigationpoint] << "; RECID: " << recordIDvec[investigationpoint] << "; Time: " << truetimevec[investigationpoint] << "; Diff: " << deltat << std::endl;
            }
            investigationpoint ++;
            deltat = inputarray[investigationpoint][3] - inputarray[basicpoint][3];
        }
    }
    
    time(&timer);
    double spacialendevaluationtime = timer;
    double spacialevaluationtime = spacialendevaluationtime - spacialstartevaluationtime;
    
    std::cout << "Events within adjusted timelimit (" << timelimit << " + " << range << " ps):  " << spacialcounts << std::endl;


    
    //Find tau_c: 2 Methods:
    //---------------------
    //1.: Assume, g2 starts at g2(0)=2 -> Determine 1/e -value
    double taucy2 = 1. + 1./e;  TLine *taucy2Line[2][2];
    double taucx2[2][2];    TLine *taucx2Line[2][2];
    double taucx2red[2][2];//Reduced by medium value
    //2.: Take the maximum value of g2(tau) to determine 1/e - value
    double taucymax[2][2];
    double oneoveremax[2][2]; TLine *taucymaxLine[2][2];
    double taucxmax[2][2];    TLine *taucxmaxLine[2][2];
    double taucxmaxred[2][2];//Reduced by medium value

    for (int bas=0; bas<2; bas++)
    {
      for (int inv=0; inv<2; inv++)
      {
        int searchvaluex; double searchvaluey; double valuex;
        //(1.)
        if (g2_smoothed_max[bas][inv] < taucy2)
        {
          std::cout << "Information: 2-Method could not be executed as maximum value of g2 is below limit: [" << bas << "][" << inv << "]" << std::endl;
          taucx2[bas][inv] = gausmed[bas][inv];//By convention
        }
        else
        {
          searchvaluex = g2_max_x[bas][inv]; g2_smoothed[bas][inv]->GetPoint(searchvaluex, valuex, searchvaluey);
          while (searchvaluey > taucy2 && searchvaluex < nbins)
          {
            searchvaluex ++;
            g2_smoothed[bas][inv]->GetPoint(searchvaluex, valuex, searchvaluey);
            //std::cout << "["<<bas<<"]["<<bas<<"]\tx-Bin: " << searchvaluex << "\ty-Value: " << searchvaluey << "\ttaucy: " << taucy2 << std::endl;
          }
          taucx2[bas][inv] = taubeg + searchvaluex * binwidth;
        }       
        taucx2red[bas][inv] = abs(taucx2[bas][inv] - gausmed[bas][inv]);
        //(2.)
        taucymax[bas][inv] = g2_max[bas][inv];
        oneoveremax[bas][inv] = 1. + ((taucymax[bas][inv]-1)/e);
        searchvaluex = g2_max_x[bas][inv]; g2_smoothed[bas][inv]->GetPoint(searchvaluex, valuex, searchvaluey);
        while (searchvaluey > oneoveremax[bas][inv] && searchvaluex < nbins)
        {
          searchvaluex ++;
          g2_smoothed[bas][inv]->GetPoint(searchvaluex, valuex, searchvaluey);
          //std::cout <<"zz: searchvaluex: " << searchvaluex << std::endl;
        }
        taucxmax[bas][inv] = taubeg + searchvaluex * binwidth;
        taucxmaxred[bas][inv] = abs(taucxmax[bas][inv] - gausmed[bas][inv]);
        //Create Lines for Plots
        //(1.)
        taucy2Line[bas][inv] = new TLine(gausmed[bas][inv], taucy2, taucx2[bas][inv], taucy2);
        taucy2Line[bas][inv]->SetLineColor(9); taucy2Line[bas][inv]->SetLineWidth(3);
        taucx2Line[bas][inv] = new TLine(taucx2[bas][inv], 1, taucx2[bas][inv], taucy2);
        taucx2Line[bas][inv]->SetLineColor(9); taucx2Line[bas][inv]->SetLineWidth(3);
        //(2.)
        taucymaxLine[bas][inv] = new TLine(gausmed[bas][inv], oneoveremax[bas][inv], taucxmax[bas][inv], oneoveremax[bas][inv]);
        taucymaxLine[bas][inv]->SetLineColor(7); taucymaxLine[bas][inv]->SetLineWidth(3);
        taucxmaxLine[bas][inv] = new TLine(taucxmax[bas][inv], 1, taucxmax[bas][inv], oneoveremax[bas][inv]);
        taucxmaxLine[bas][inv]->SetLineColor(7); taucxmaxLine[bas][inv]->SetLineWidth(3);
      }
    }


    //Create Resultfile (if writefile == 1)
    if (writefile == true)
    {
        //#################################################
        std::cout << ">Create Resultfile..." << std::endl;
        //#################################################
        ofstream fileout;

        double fitpar[4][2][2];//[parameter][bas][inv]
        for (int bas=0; bas<2; bas++)
        {
          for (int inv=0; inv<2; inv++)
          {
            fitpar[0][bas][inv] = gausamp[bas][inv];
            fitpar[1][bas][inv] = gausmed[bas][inv];
            fitpar[2][bas][inv] = gaussig[bas][inv];
            fitpar[3][bas][inv] = gausoff[bas][inv];
          }
        }

        fileout.open(readoutfileline.c_str());
        fileout << fixed;

        fileout << "Analysis from document: " << documentline << "\n";
        fileout << "Taubeg: " << taubeg << "; Tauend: " << tauend << "; nbins: " << nbins << "; inputs: " << inputs << "\n\n";
        fileout << "Effective Measurement time (ps)\n";
        fileout << endtime << "\n\n";

        fileout << "General Results: Correlation times from Gaus-Fit (ps)\n";
        fileout << "[0][0]\t[0][1]\t[1][0]\t[1][1]\n";
        fileout << reltau_c[0][0] << "\t" << reltau_c[0][1] << "\t" << reltau_c[1][0] << "\t" << reltau_c[1][1] << "\n";
        fileout << "Absolute values:\n";
        fileout << abstau_c[0][0] << "\t" << abstau_c[0][1] << "\t" << abstau_c[1][0] << "\t" << abstau_c[1][1] << "\n\n";

        fileout << "Gauß-Parameter: Lines follow (1.)[0][0]; (2.)[0][1]; (3.)[1][0]; (4.)[1][1]\n";
        fileout << "Amplitude\tMedium\tSigma\tOffset\n";
        for (int bas=0; bas<2; bas++)
        {
          for (int inv=0; inv<2; inv++)
          {
            fileout << fitpar[0][bas][inv] << "\t" << fitpar[1][bas][inv] << "\t" << fitpar[2][bas][inv] << "\t" << fitpar[3][bas][inv] << "\n";
          }
        }

        fileout << "\n\n\nTABLE [0][0]\n";
        fileout << "Tau\tEvents[0][0]\tg2[0][0]\tdg2[0][0]\n";
        for (int i=0; i<nbins; i++)
        {
            fileout << correlationarray[i][0][0][0] << "\t" << correlationarray[i][1][0][0] << "\t" << correlationarray[i][2][0][0] << "\t" << correlationarray[i][4][0][0] << "\n";
        }
        fileout << "\n\n";

        fileout << "\n\n\nTABLE [0][1]\n";
        fileout << "Tau\tEvents[0][1]\tg2[0][1]\tdg2[0][1]\n";
        for (int i=0; i<nbins; i++)
        {
            fileout << correlationarray[i][0][0][1] << "\t" << correlationarray[i][1][0][1] << "\t" << correlationarray[i][2][0][1] << "\t" << correlationarray[i][4][0][1] << "\n";
        }
        fileout << "\n\n";

        fileout << "\n\n\nTABLE [1][0]\n";
        fileout << "Tau\tEvents[1][0]\tg2[1][0]\tdg2[1][0]\n";
        for (int i=0; i<nbins; i++)
        {
            fileout << correlationarray[i][0][1][0] << "\t" << correlationarray[i][1][1][0] << "\t" << correlationarray[i][2][1][0] << "\t" << correlationarray[i][4][1][0] << "\n";
        }
        fileout << "\n\n";

        fileout << "\n\n\nTABLE [1][1]\n";
        fileout << "Tau\tEvents[1][1]\tg2[1][1]\tdg2[1][1]\n";
        for (int i=0; i<nbins; i++)
        {
            fileout << correlationarray[i][0][1][1] << "\t" << correlationarray[i][1][1][1] << "\t" << correlationarray[i][2][1][1] << "\t" << correlationarray[i][4][1][1] << "\n";
        }
        fileout << "\n\n";


        fileout.close();
    }
    
    

        //EXTRAS: Many possibilities to return in terminal or Canvas:
    //labelextras:;//Marker for showing extra informations
    //std::string showextras = "";


    std::cout << "\n--------------------------------------------------------------------------------" << std::endl;
    std::cout << "Choose extra results by typing in the letters:" << std::endl;
    std::cout << "Show g-informations: [i]" << std::endl;
    std::cout << "Show resulttable Channel X-Channel Y: [rXY])\tExit root: [q]" << std::endl;
    std::cout << "Type in the letters:  ";
    cin >> showextras;

    std::string showi = "i";
    std::string showr00  = "r00";
    std::string showr01  = "r01";
    std::string showr10  = "r10";
    std::string showr11  = "r11";
    std::string exitroot  = "q";

    if (showextras.find(showi) != std::string::npos)
    {
      std::cout << "\n--------------------------------------------------------------------------------" << std::endl;
      std::cout << "CORRELATION TIMES WITH G2-METHODS" << std::endl;
      std::cout << "--------------------------------------------------------------------------------" << std::endl;
      std::cout << "Calculated Correlation Times by 2-Method [Reduced]:" << std::endl;
      std::cout << "[0][0] : " << taucxmaxred[0][0] << " ps\t\t[0][1] : " << taucxmaxred[0][1] << " ps" << std::endl;
      std::cout << "[1][0] : " << taucxmaxred[1][0] << " ps\t\t[1][1] : " << taucxmaxred[1][1] << " ps" << std::endl;
      std::cout << "Calculated Correlation Times by 2-Method [Absolute]:" << std::endl;
      std::cout << "[0][0] : " << taucxmax[0][0] << " ps\t\t[0][1] : " << taucxmax[0][1] << " ps" << std::endl;
      std::cout << "[1][0] : " << taucxmax[1][0] << " ps\t\t[1][1] : " << taucxmax[1][1] << " ps\n" << std::endl;
      std::cout << "Calculated Correlation Times by Max-Method [Reduced]:" << std::endl;
      std::cout << "[0][0] : " << taucxmaxred[0][0] << " ps\t\t[0][1] : " << taucxmaxred[0][1] << " ps" << std::endl;
      std::cout << "[1][0] : " << taucxmaxred[1][0] << " ps\t\t[1][1] : " << taucxmaxred[1][1] << " ps" << std::endl;
      std::cout << "Calculated Correlation Times by Max-Method [Absolute]:" << std::endl;
      std::cout << "[0][0] : " << taucxmax[0][0] << " ps\t\t[0][1] : " << taucxmax[0][1] << " ps" << std::endl;
      std::cout << "[1][0] : " << taucxmax[1][0] << " ps\t\t[1][1] : " << taucxmax[1][1] << " ps" << std::endl;
      std::cout << "--------------------------------------------------------------------------------\n" << std::endl;
    }

    if (showextras.find(showr00) != std::string::npos)
    {

      std::cout << "\n--------------------------------------------------------------------------------" << std::endl;
      std::cout << "RESULTS CHANNEL 0 TO CHANNEL 0" << std::endl;
      std::cout << "--------------------------------------------------------------------------------" << std::endl;
      std::cout << "Tau[ps]\t\tEvents\t\tg2" << std::endl;
      for (int i=0; i<nbins; i++)
      {
        std::cout << correlationarray[i][0][0][0] << "\t" << correlationarray[i][1][0][0] << "\t" << correlationarray[i][2][0][0] << std::endl;
      }
      std::cout << "--------------------------------------------------------------------------------\n" << std::endl;
    }

    if (showextras.find(showr01) != std::string::npos)
    {

      std::cout << "\n--------------------------------------------------------------------------------" << std::endl;
      std::cout << "RESULTS CHANNEL 0 TO CHANNEL 1" << std::endl;
      std::cout << "--------------------------------------------------------------------------------" << std::endl;
      std::cout << "Tau[ps]\t\tEvents\t\tg2" << std::endl;
      for (int i=0; i<nbins; i++)
      {
        std::cout << correlationarray[i][0][0][1] << "\t" << correlationarray[i][1][0][1] << "\t" << correlationarray[i][2][0][1] << std::endl;
      }
      std::cout << "--------------------------------------------------------------------------------\n" << std::endl;
    }

    if (showextras.find(showr10) != std::string::npos)
    {

      std::cout << "\n--------------------------------------------------------------------------------" << std::endl;
      std::cout << "RESULTS CHANNEL 1 TO CHANNEL 0" << std::endl;
      std::cout << "--------------------------------------------------------------------------------" << std::endl;
      std::cout << "Tau[ps]\t\tEvents\t\tg2" << std::endl;
      for (int i=0; i<nbins; i++)
      {
        std::cout << correlationarray[i][0][1][0] << "\t" << correlationarray[i][1][1][0] << "\t" << correlationarray[i][2][1][0] << std::endl;
      }
      std::cout << "--------------------------------------------------------------------------------\n" << std::endl;
    }

    if (showextras.find(showr11) != std::string::npos)
    {

      std::cout << "\n--------------------------------------------------------------------------------" << std::endl;
      std::cout << "RESULTS CHANNEL 1 TO CHANNEL 1" << std::endl;
      std::cout << "--------------------------------------------------------------------------------" << std::endl;
      std::cout << "Tau[ps]\t\tEvents\t\tg2" << std::endl;
      for (int i=0; i<nbins; i++)
      {
        std::cout << correlationarray[i][0][1][1] << "\t" << correlationarray[i][1][1][1] << "\t" << correlationarray[i][2][1][1] << std::endl;
      }
      std::cout << "--------------------------------------------------------------------------------\n" << std::endl;
    }


    if (showextras.find(exitroot) != std::string::npos)
    {
      std::cout << "END COCOS\n" << std::endl;
      exit(0);
    }

    if (showextras.find(work) == std::string::npos){goto labelextras;}


    
    
    */

    //########################END NOT IMPORTANT NOW###############################






    
    
  

