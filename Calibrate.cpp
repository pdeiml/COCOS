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
using namespace std;


void Calibrate (){


	std::vector<std::string> datanames; datanames.push_back("Evaluate without further calibration\n");

	//datanames.push_back("2018_01_26/CalibrationSun_0toEnd.txt");

	//datanames.push_back("2018_02_02/C-BS-LED-nF.txt");
	//datanames.push_back("2018_02_02/C-BS-LED-F.txt");

	//datanames.push_back("2018_02_08/C-BS-Sun.txt");
	//datanames.push_back("2018_02_08/C-BS-LED-nF.txt");
	//datanames.push_back("2018_02_08/C-BS-LED-F.txt");

	//datanames.push_back("2018_02_09/C-BS-Sun_1h-nF.txt");

	//datanames.push_back("2018_02_13/C-BS-Sun_1h-nF.txt");

    datanames.push_back("2018_02_14/LED-Filter/ganz/Calibration.txt");



	const int nmeas = datanames.size();

	/*std::vector<std::string> shownames;
	for (int i=0; i<nmeas-1; i++)
	{
		shownames.push_back(datanames.at(i).substr(0,datanames.at(i).find("/")));
	}*/
	

	//Define Colors
    int colorarray[20];
    colorarray[0] = 51;//1;
    colorarray[1] = 60;//2;
    colorarray[2] = 65;//3;
    colorarray[3] = 75;//4;
    colorarray[4] = 95;//6;
    colorarray[5] = 100;//7;
    colorarray[6] = 6;//28;
    colorarray[7] = 41;
    colorarray[8] = 51;
    colorarray[9] = 94;
    colorarray[10] = 20;
    colorarray[11] = 30;

    std::string colorstring[6];
    colorstring[0] = "kMagenta+3";
    colorstring[1] = "kMagenta+1";
    colorstring[2] = "kBlue-7";
    colorstring[3] = "kBlue+1";
    colorstring[4] = "kCyan+2";
    colorstring[5] = "kCyan-4";
    
    std::vector<float> vcal[nmeas][2][2];
    std::vector<float> vcalt[nmeas];
    TGraph * g2graph[nmeas][2][2];
    TGraph * g2calib[nmeas][2][2];
    int calTIME;
    double cal00, cal01, cal10, cal11;
    std::string line;
    std::ifstream infile;

    //Plot range paramteres
    double minx, maxx;

    int actmeas = 0;
    int asciicode;
    double calibmod8[nmeas][2][2][8] = {{{{0}}}};

    for (int i=0; i<nmeas; i++)
    {
    	actmeas = i;
        if (actmeas != 0)//Real files
        {
            infile.open(datanames.at(i).c_str());
            while (std::getline(infile,line))
            {
            	asciicode = line.at(0);
            	if (asciicode != 35)//Not "#"
            	{
            		sscanf(line.c_str(),"%i\t%lf\t%lf\t%lf\t%lf", &calTIME, &cal00, &cal01, &cal10, &cal11);
                	if(cal00 < 0.001){cal00 =1;}
                	if(cal01 < 0.001){cal01 =1;}
                	if(cal10 < 0.001){cal10 =1;}
                	if(cal11 < 0.001){cal11 =1;}
                	//std::cout << calTIME << "\t" << cal01 << "\t" << cal10 << std::endl;
                	vcalt[actmeas].push_back(calTIME);
                	vcal[actmeas][0][0].push_back(cal00); vcal[actmeas][0][1].push_back(cal01);
                	vcal[actmeas][1][0].push_back(cal10); vcal[actmeas][1][1].push_back(cal11);
            	}
            }
            infile.close();
        }
        if (actmeas == 0)//1-"File"
        {
            infile.open(datanames.at(i+1).c_str());
            while (std::getline(infile,line))
            {
            	asciicode = line.at(0);
            	if (asciicode != 35)//Not "#"
            	{
            		sscanf(line.c_str(),"%i\t%lf\t%lf\t%lf\t%lf", &calTIME, &cal00, &cal01, &cal10, &cal11);
                	vcalt[actmeas].push_back(calTIME);
                	vcal[actmeas][0][0].push_back(1.); vcal[actmeas][0][1].push_back(1.);
                	vcal[actmeas][1][0].push_back(1.); vcal[actmeas][1][1].push_back(1.);
            	}
            }
            infile.close();
        }    	
    }

    //Set plot range parameters
    minx = vcalt[0].at(0); maxx = vcalt[0].at(vcalt[0].size() -1);


    //Calibration
    std::string calibrationbasistring; int calibrationbasis;
    std::cout << "Calibration possibilities:" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;
    for (int i=0; i<nmeas; i++)
    {
    	//std::cout << i << "\t" << shownames.at(i) << std::endl;
    	std::cout << i << "\t" << datanames.at(i) << std::endl;
    }
    std::cout << "--------------------------------------------------------------------------" << std::endl;
    std::cout << "Set calibration basis:\t";
    cin >> calibrationbasistring;
    calibrationbasis = std::atof(calibrationbasistring.c_str());

    std::cout << "Change plot range [" << 1e-3 * minx << ":" << 1e-3 * maxx << "] ns [y/n] ?\t";
    std::string cprstring; cin >> cprstring;
    if (cprstring == "y")
    {
    	std::cout << "New minimum [ns]:\t";
    	std::string nminstring; cin >> nminstring;
    	minx = 1e3 * std::atof(nminstring.c_str());

    	std::cout << "New maximum [ns]:\t";
    	std::string nmaxstring; cin >> nmaxstring;
    	maxx = 1e3 * std::atof(nmaxstring.c_str());
    }



    //Create uncalibrated graph before doing anything
    for (int imeas=0; imeas<nmeas; imeas++)
    {
    	for (int bas=0; bas<2; bas++)
    	{
    		for (int inv=0; inv<2; inv++)
    		{
    			g2graph[imeas][bas][inv] = new TGraph(vcalt[imeas].size());
    			g2graph[imeas][bas][inv]->SetLineColor(colorarray[imeas]);

    			for (int i=0; i<vcalt[imeas].size(); i++)
    			{
                    //Fill g2 calibrated graphs
    				g2graph[imeas][bas][inv]->SetPoint(i, vcalt[imeas].at(i), vcal[imeas][bas][inv].at(i));
    			}
                //Set range
                g2graph[imeas][bas][inv]->GetXaxis()->SetRangeUser(minx, maxx);
    		}
    	}
    }


    //New calibration: Calibrate with nself times of 8-bin tail results
    int nself = 63;
    for (int imeas=0; imeas<nmeas; imeas++)
    {
    	for (int bas=0; bas<2; bas++)
    	{
    		for (int inv=0; inv<2; inv++)
    		{
    			//Create calibration
    			for (int j=0; j<8*nself ; j++)
    			{
					calibmod8[imeas][bas][inv][j%8] += vcal[imeas][bas][inv][j]/nself;
				}
				//Calibrate
    			for(int ibin=0; ibin<vcalt[imeas].size(); ibin++)
    			{
					vcal[imeas][bas][inv][ibin] /= calibmod8[imeas][bas][inv][ibin%8];
				}
    		}
    	}
	}



    //Fill all graphs and create g2-distribution histograms
    TH1I * g2dist[nmeas][2][2];
    double g2dist_rms[nmeas][2][2], g2dist_mean[nmeas][2][2];
    double g2distmax[2][2]; g2distmax[0][0] = 0; g2distmax[0][1] = 0; g2distmax[1][0] = 0; g2distmax[1][1] = 0;
    for (int imeas=0; imeas<nmeas; imeas++)
    {
    	for (int bas=0; bas<2; bas++)
    	{
    		for (int inv=0; inv<2; inv++)
    		{
    			g2calib[imeas][bas][inv] = new TGraph(vcalt[imeas].size());
    			g2calib[imeas][bas][inv]->SetLineColor(colorarray[imeas]);
    			g2calib[imeas][bas][inv]->SetLineWidth(1);

                g2dist[imeas][bas][inv] = new TH1I("", "g2 distribution histogram", 4000, 0.9, 1.1);
                g2dist[imeas][bas][inv]->SetLineColor(colorarray[imeas]);

    			for (int i=0; i<vcalt[imeas].size(); i++)
    			{
                    //Fill g2 calibrated graphs
    				g2calib[imeas][bas][inv]->SetPoint(i, vcalt[imeas].at(i), vcal[imeas][bas][inv].at(i)/vcal[calibrationbasis][bas][inv].at(i));
                    //Fill histogram
                    if (imeas != calibrationbasis && imeas != 0 && vcalt[0].at(i) >= minx && vcalt[0].at(i) <= maxx)
                    {
                        g2dist[imeas][bas][inv]->Fill(vcal[imeas][bas][inv].at(i)/vcal[calibrationbasis][bas][inv].at(i));
                    }
    			}
                g2dist_rms[imeas][bas][inv] = g2dist[imeas][bas][inv]->GetRMS();
                g2dist_mean[imeas][bas][inv] = g2dist[imeas][bas][inv]->GetMean();
                if (g2dist[imeas][bas][inv]->GetMaximum() > g2distmax[bas][inv] && imeas != calibrationbasis && imeas != 0)
                {
                    g2distmax[bas][inv] = g2dist[imeas][bas][inv]->GetMaximum();
                }

                //Set range
                g2calib[imeas][bas][inv]->GetXaxis()->SetRangeUser(minx, maxx);
    		}
    	}
    }
    std::cout << "--------------------------------------------------------------------------" << std::endl;
    std::cout << "Statistics of calibrated [0][1]-g2 function:" << std::endl;
    std::cout << "Number\tName\tMean\t\tRMS" << std::endl;
    std::cout << "--------------------------------------------------------------------------" << std::endl;
    for (int i=0; i<nmeas; i++)
    {
        //if (i != calibrationbasis){std::cout << i << "\t" << shownames.at(i) << "\t" << std::setprecision(5) << g2dist_mean[i][0][1] << "\t\t" << g2dist_rms[i][0][1] << std::endl;}
        if (i != 0)
        {
            std::cout << i; if (i == calibrationbasis){std::cout << "*";}
            std::cout << "\t" << datanames.at(i) << "\t\t" << std::setprecision(5) << g2dist_mean[i][0][1] << "\t\t" << g2dist_rms[i][0][1] << std::endl;
        }
    }
    std::cout << "--------------------------------------------------------------------------" << std::endl;


    //*************************
    //FFTs of calibrated graphs
    //*************************
    //Look for number of bins in specified range
    int nbinsinrange = 0;
    for (int i=0; i<vcalt[0].size(); i++)
    {
    	if (vcalt[0].at(i) >= minx && vcalt[0].at(i) <= maxx){nbinsinrange ++;}
    }
    TH1F * g2histo[nmeas];
    for (int j=0; j<nmeas; j++)
    {
       //g2histo[j] = new TH1F("","Histogram of g2-Function", vcalt[j].size(), vcalt[j].at(0), vcalt[j].at(vcalt[j].size()-1));
       g2histo[j] = new TH1F("","Histogram of g2-Function", nbinsinrange, minx, maxx);
       int histobin = 0;
       for (int i=0; i<vcalt[j].size(); i++)
       {
           if (vcalt[j].at(i) >= minx && vcalt[j].at(i) <= maxx)
           {
           		histobin ++;
           		g2histo[j]->SetBinContent(histobin, vcal[j][0][1].at(i)/vcal[calibrationbasis][0][1].at(i));
           }
       }
       //g2histo[j]->SetAxisRange(minx, maxx, "X");
    }    
    TH1 * fouriertransform[nmeas];
    for (int imeas=0; imeas<nmeas; imeas++)
    {
        fouriertransform[imeas] = 0;
        TVirtualFFT::SetTransform(0);
        fouriertransform[imeas] = g2histo[imeas]->FFT(fouriertransform[imeas], "MAG");
        fouriertransform[imeas]->SetTitle("FFT of calibrated g2");
        //fouriertransform[imeas]->GetXaxis()->SetLimits(0, vcalt[imeas].size()/(1e-12*(vcalt[imeas].at(vcalt[imeas].size()-1) - vcalt[imeas].at(0))));//Rescale x-axis for real frequencies
        fouriertransform[imeas]->GetXaxis()->SetLimits(0, nbinsinrange/(1e-12*(maxx - minx)));//Rescale x-axis for real frequencies
        fouriertransform[imeas]->GetXaxis()->SetTitle("f [Hz]");
        fouriertransform[imeas]->SetLineColor(colorarray[imeas]);
        fouriertransform[imeas]->SetLineWidth(1);
        fouriertransform[imeas]->GetXaxis()->SetRangeUser(4e6,2000e6);
    }



    //Drawing
    int whichpad[2][2];//Assings histgorams etc to canvas position
    whichpad[0][0] = 1;
    whichpad[0][1] = 2;
    whichpad[1][0] = 3;
    whichpad[1][1] = 4;
    

    TCanvas * FFTcanvas = new TCanvas("fft","FFTC",800,600);
    fouriertransform[1]->Draw();
    for (int imeas=2; imeas<nmeas; imeas++)
    {
        fouriertransform[imeas]->Draw("same");
    }

    TCanvas * plots = new TCanvas("plots","Plots",800,600);
    plots->Divide(2,2);
    for (int bas=0; bas<2; bas++)
    {
    	for (int inv=0; inv<2; inv++)
    	{
    		plots->cd(whichpad[bas][inv]);
    		g2graph[1][bas][inv]->Draw();
    		g2graph[1][bas][inv]->SetTitle("g_{2}-function");
    		g2graph[1][bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
    		g2graph[1][bas][inv]->GetYaxis()->SetTitle("g_{2}(#tau)");

    		for(int imeas=2; imeas<nmeas; imeas++)
    		{
    			g2graph[imeas][bas][inv]->Draw("same");
    		}
    	}
    }

    TCanvas * calibplots = new TCanvas("calibplots","Calibrated plots",800,600);
    calibplots->Divide(2,2);
    for (int bas=0; bas<2; bas++)
    {
    	for (int inv=0; inv<2; inv++)
    	{
    		calibplots->cd(whichpad[bas][inv]);
    		g2calib[1][bas][inv]->Draw();
    		g2calib[1][bas][inv]->SetTitle("g_{2}-function");
    		g2calib[1][bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
    		g2calib[1][bas][inv]->GetYaxis()->SetTitle("g_{2}(#tau)");

    		for(int imeas=2; imeas<nmeas; imeas++)
    		{
    			g2calib[imeas][bas][inv]->Draw("same");
    		}
    	}
    }

    TCanvas * g2distcanvas = new TCanvas("g2distcanvas","g2 distribution canvas",800,600);
    g2distcanvas->Divide(2,2);
    for (int bas=0; bas<2; bas++)
    {
        for (int inv=0; inv<2; inv++)
        {
            g2distcanvas->cd(whichpad[bas][inv]);
            g2dist[1][bas][inv]->GetYaxis()->SetRangeUser(0, g2distmax[bas][inv]+2);
            g2dist[1][bas][inv]->Draw();
            g2dist[1][bas][inv]->SetTitle("g_{2} distribution histogram");
            g2dist[1][bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
            g2dist[1][bas][inv]->GetYaxis()->SetTitle("Number of events");

            for(int imeas=2; imeas<nmeas; imeas++)
            {
                g2dist[imeas][bas][inv]->Draw("same");
            }
        }
    }

    int legheight = 60 * nmeas;
    TCanvas * legcanvas = new TCanvas("legend","Legend", 500,legheight);
    	TLegend* plotlegend = new TLegend(0,0,1,1);
    	for (int i=1; i<nmeas; i++)
    	{
    		//plotlegend->AddEntry(g2graph[i][0][0], shownames.at(i).c_str(),"l");
    		plotlegend->AddEntry(g2graph[i][0][0], datanames.at(i).c_str(),"l");
    	}
    	//plotlegend->SetFillColor(41);
    	plotlegend->SetLineWidth(5);
    	plotlegend->SetTextSize(0.1); //plotlegend->SetTextColor(4);
    	plotlegend->Draw();

    /*TCanvas * test = new TCanvas("ltest","test", 800,600);
    g2histo[1]->Draw();*/



}
