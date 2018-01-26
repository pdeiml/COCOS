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


void Compare (){


	std::vector<std::string> datanames;

	datanames.push_back("0-10/Calibration.txt");
	datanames.push_back("10-20/Calibration.txt");
	datanames.push_back("20-30/Calibration.txt");
	datanames.push_back("30-40/Calibration.txt");
    //datanames.push_back("40-50/Calibration.txt");
    //datanames.push_back("50-60/Calibration.txt");
	//datanames.push_back("LED/Calibration.txt");

	const int nmeas = datanames.size();

	std::vector<std::string> shownames;
	for (int i=0; i<nmeas; i++)
	{
		shownames.push_back(datanames.at(i).substr(0,datanames.at(i).find("/")));
	}
	

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
    int calTIME;
    double cal00, cal01, cal10, cal11;
    std::string line;
    std::ifstream infile;

    int actmeas = 0;

    for (int i=0; i<nmeas; i++)
    {
    	actmeas = i;
    	infile.open(datanames.at(i).c_str());
    	while (std::getline(infile,line))
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
    	infile.close();
    }


    //Calibration
    std::string calibrationbasistring; int calibrationbasis;
    std::cout << "Calibration possibilities:" << std::endl;
    std::cout << "--------------------------" << std::endl;
    for (int i=0; i<nmeas; i++)
    {
    	std::cout << i << "\t" << shownames.at(i) << std::endl;
    }
    std::cout << "--------------------------" << std::endl;
    std::cout << "Set calibration basis:\t";
    cin >> calibrationbasistring;
    calibrationbasis = std::atof(calibrationbasistring.c_str());
    TGraph * g2calib[nmeas][2][2];


    //Fill all graphs
    for (int imeas=0; imeas<nmeas; imeas++)
    {
    	for (int bas=0; bas<2; bas++)
    	{
    		for (int inv=0; inv<2; inv++)
    		{
    			g2graph[imeas][bas][inv] = new TGraph(vcalt[imeas].size());
    			g2graph[imeas][bas][inv]->SetLineColor(colorarray[imeas]);
    			if(imeas == calibrationbasis){g2graph[imeas][bas][inv]->SetLineWidth(2);}
    			//g2graph[imeas][bas][inv]->SetLineWidth(2);

    			g2calib[imeas][bas][inv] = new TGraph(vcalt[imeas].size());
    			g2calib[imeas][bas][inv]->SetLineColor(colorarray[imeas]);
    			//if(imeas == calibrationbasis){g2calib[imeas][bas][inv]->SetLineWidth(2);}
    			g2calib[imeas][bas][inv]->SetLineWidth(2);

    			for (int i=0; i<vcalt[imeas].size(); i++)
    			{
    				g2graph[imeas][bas][inv]->SetPoint(i, vcalt[imeas].at(i), vcal[imeas][bas][inv].at(i));
    				g2calib[imeas][bas][inv]->SetPoint(i, vcalt[imeas].at(i), vcal[imeas][bas][inv].at(i)/vcal[calibrationbasis][bas][inv].at(i));
    			}
    		}
    	}
    }

    //FFT of one graph
    TH1F * g2histo[nmeas];     
    for (int j=0; j<nmeas; j++)
    {
       g2histo[j] = new TH1F("g2histo","Histogram of g2-Function", vcalt[j].size(), vcalt[j].at(0), vcalt[j].at(vcalt[j].size()-1));
       for (int i=0; i<vcalt[j].size(); i++)
       {
           g2histo[j]->SetBinContent(i+1, vcal[j][0][1].at(i)/vcal[calibrationbasis][0][1].at(i));
       }
    }

    //TCanvas * histoc = new TCanvas("histo","Histogram",800,600);
    //g2histo[1]->Draw();
    
    TH1 * fouriertransform[nmeas];
    for (int imeas=0; imeas<nmeas; imeas++)
    {
        fouriertransform[imeas] = 0;
        TVirtualFFT::SetTransform(0);
        fouriertransform[imeas] = g2histo[imeas]->FFT(fouriertransform[imeas], "MAG");
        fouriertransform[imeas]->SetTitle("FFT of calibrated g2");
        fouriertransform[imeas]->GetXaxis()->SetLimits(0, vcalt[imeas].size()/(1e-12*(vcalt[imeas].at(vcalt[imeas].size()-1) - vcalt[imeas].at(0))));//Rescale x-axis for real frequencies
        fouriertransform[imeas]->GetXaxis()->SetTitle("f [Hz]");
        fouriertransform[imeas]->SetLineColor(colorarray[imeas]);
        fouriertransform[imeas]->SetLineWidth(2);
    }



    //Drawing
    int whichpad[2][2];//Assings histgorams etc to canvas position
    whichpad[0][0] = 1;
    whichpad[0][1] = 2;
    whichpad[1][0] = 3;
    whichpad[1][1] = 4;
    

    TCanvas * FFTcanvas = new TCanvas("fft","FFTC",800,600);
    fouriertransform[0]->Draw();
    for (int imeas=1; imeas<nmeas; imeas++)
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
    		g2graph[0][bas][inv]->Draw();
    		g2graph[0][bas][inv]->SetTitle("g_{2}-function");
    		g2graph[0][bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
    		g2graph[0][bas][inv]->GetYaxis()->SetTitle("g_{2}(#tau)");

    		for(int imeas=1; imeas<nmeas; imeas++)
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
    		g2calib[0][bas][inv]->Draw();
    		g2calib[0][bas][inv]->SetTitle("g_{2}-function");
    		g2calib[0][bas][inv]->GetXaxis()->SetTitle("#tau [ps]");
    		g2calib[0][bas][inv]->GetYaxis()->SetTitle("g_{2}(#tau)");

    		for(int imeas=1; imeas<nmeas; imeas++)
    		{
    			g2calib[imeas][bas][inv]->Draw("same");
    		}
    	}
    }


    int legheight = 80 * nmeas;
    TCanvas * legcanvas = new TCanvas("legend","Legend", 200,legheight);
    	plotlegend = new TLegend(0,0,1,1);
    	for (int i=0; i<nmeas; i++)
    	{
    		plotlegend->AddEntry(g2graph[i][0][0], shownames.at(i).c_str(),"l");
    	}
    	//plotlegend->SetFillColor(41);
    	plotlegend->SetLineWidth(5);
    	plotlegend->SetTextSize(0.15); //plotlegend->SetTextColor(4);
    	plotlegend->Draw();





}
