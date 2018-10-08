#include <fstream>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <sstream>
#include <math.h>

using namespace std;

int main()
{


    //Folder path of ptu-files
    std::string filepath = "/home/vault/caph/mppi019h/TimeHarpData/";
    //Folder path of evaluation files
    std::string evaluationpath = "/home/hpc/caph/mppi019h/TimeHarpAuswertungen/";
    //Folder path of job-files
    std::string jobpath = "/home/vault/caph/mppi019h/TimeHarpData/jobs/";
    //Path where COCOShpc is loacated
    std::string cocoshpcpath = "/home/woody/caph/mppi019h/cocos/build/";
    //***** Change e-mail!!! *****

    //Standard interval length of one evaluation in s
    int evaluationlength_h = 1;
    int evaluationlength_m = 0;
    int evaluationlength_s = 0;
    //Standard total length of evaluation
    int totalevaluationlength_h = 1;
    int totalevaluationlength_m = 0;
    int totalevaluationlength_s = 0;
    //Standard walltime
    std::string walltime = "10:00:00";
    



	std::string jobname;
    std::cout << "\033[1;31mName des Jobs (Rumpf):\033[0m\t\t"; cin >> jobname;


    //Settings
    settingsmarker:;
    std::cout << "\n\033[1;31m-----------------------------------------------\033[0m" << std::endl;
    std::cout << "\033[1;31mLänge der Auswertung\t[a]\033[0m\t" << totalevaluationlength_h << ":" << totalevaluationlength_m << ":" << totalevaluationlength_s << std::endl;
    std::cout << "\033[1;31mLänge eines Pakets\t[p]\033[0m\t" << evaluationlength_h << ":" << evaluationlength_m << ":" << evaluationlength_s << std::endl;
    std::cout << "\033[1;31mWalltime\t\t[w]\033[0m\t" << walltime << std::endl;
    std::cout << "\033[1;31mContinue\t\t[c]\033[0m" << std::endl;
    std::cout << "\033[1;31m-----------------------------------------------\033[0m\n" << std::endl;

    std::cout << "\033[1;31mBefehl eingeben:\033[0m\t";
    std::string instring; cin >> instring;
    if (instring == "a")
    {
        std::cout << "Länger der Auswertung - Stunden:\t"; cin >> instring;
        totalevaluationlength_h = std::stoi(instring);
        std::cout << "Länger der Auswertung - Minuten:\t"; cin >> instring;
        totalevaluationlength_m = std::stoi(instring);
        std::cout << "Länger der Auswertung - Sekunden:\t"; cin >> instring;
        totalevaluationlength_s = std::stoi(instring);
    }
    if (instring == "p")
    {
        std::cout << "Länger eines Pakets - Stunden:\t"; cin >> instring;
        evaluationlength_h = std::stoi(instring);
        std::cout << "Länger eines Pakets - Minuten:\t"; cin >> instring;
        evaluationlength_m = std::stoi(instring);
        std::cout << "Länger eines Pakets - Sekunden:\t"; cin >> instring;
        evaluationlength_s = std::stoi(instring);
    }
    if (instring == "w")
    {
        std::cout << "\033[1;31mWalltime [hh:mm:ss]:\033[0m\t\t"; cin >> walltime;
    }
    if (instring == "c")
    {
        goto endsettingsmarker;
    }
    goto settingsmarker;
    endsettingsmarker:;

    int totalev = 3600 * totalevaluationlength_h + 60 * totalevaluationlength_m + totalevaluationlength_s;
    int paketev = 3600 * evaluationlength_h + 60 * evaluationlength_m + evaluationlength_s;
    int nfiles = ceil(totalev/paketev);

    

    std::string datei;
    std::cout << "\n\033[1;31mMögliche Messdateien:\033[0m" << std::endl;
    std::cout << "\033[1;31m---------------------------------------\033[0m" << std::endl;
    std::string lsbefehl = "tree " + filepath; system(lsbefehl.c_str());
    std::cout << "\033[1;31m---------------------------------------\033[0m" << std::endl;
    std::cout << "\033[1;31mMessung:\033[0m\t\t" << filepath; cin >> datei;

    std::ostringstream counterstring[100];
    std::string auswertung, cutauswertung, gesamtpfad, befehl, settingsstring, restpath;
    ofstream settings, job;
    for (int i=0; i<nfiles; i++)
    {       
        //counterstring[i] << "_" << i << "-" << i+1 << "h";
        counterstring[i] << i;
        std::string makejob = jobpath + jobname + "_" + counterstring[i].str() + ".sh";
        auswertung = jobname + "_" + counterstring[i].str();
        cutauswertung = evaluationpath + datei.substr(0, datei.find(".")) + "/" + auswertung; std::string makeordner;
        restpath = datei.substr(0, datei.find(".")) + "/" + auswertung;
        //std::cout << "\033[1;31mOrdner Auswertung:\033[0m\t" << cutauswertung << std::endl;

        gesamtpfad = "";
        bool mkdirfertig = false;
        while (mkdirfertig == false)
        {
            //std::cout << "cutauswertung:\t" << cutauswertung << std::endl;
            //std::cout << "restpath:\t" << restpath << std::endl;

            int wo = restpath.find("/");

            //std::cout << "wo:\t" << wo << std::endl;

            if (wo > 0){makeordner = restpath.substr(0,restpath.find("/")+1);}
            else {makeordner = restpath;}

            //std::cout << "makeordner:\t" << makeordner << std::endl;

            gesamtpfad = gesamtpfad + makeordner;

            //std::cout << "gesamtpfad:\t" << gesamtpfad << std::endl;

            befehl = "mkdir " + evaluationpath + gesamtpfad; system(befehl.c_str()); //std::cout << "Mache:\t" << befehl << std::endl;
            if (restpath.find("/") == -1){mkdirfertig = true;}
            restpath = restpath.substr(restpath.find("/")+1);

            //std::string weiter; cin >> weiter;
        }
    
        std::string pfadauswertung = evaluationpath + datei.substr(0, datei.find(".")) + "/" + auswertung;
        //std::string mkdirstring = "mkdir " + pfadauswertung;
        //system(mkdirstring.c_str()); std::cout << "Mache:\t" << mkdirstring << std::endl;
        std::string copystring = "scp " + cocoshpcpath + "COCOShpc " + pfadauswertung +  "/";
        system(copystring.c_str()); //std::cout << "Mache:\t" << copystring << std::endl;


        settingsstring = pfadauswertung + "/settings.txt";
        settings.open(settingsstring.c_str());
        settings << "is " << paketev * i << "\n";
        settings << "ie " << paketev * (i+1) << "\n";
        settings << "sl 1\n";
        settings << "ts -250000\n";
        settings << "te 250000\n";
        settings << "nb 2000\n";
        settings << "cm 2\n";
        settings << "#end\n";
        settings << "$Calibration_" << jobname << "_" << i << ".txt";
        settings.close();

        job.open(makejob.c_str());
        job << "#!/bin/bash -l\n#\n";
        job << "# Set Walltime\n#PBS -l nodes=1:ppn=4,walltime=" << walltime << "\n#\n";
        job << "# job name\n#PBS -N " << jobname  << "_" << counterstring[i].str() << "\n";
        //job << "# stdout und stderr Dateien\n#PBS -o log/out.out -e log/err.err\n#\n";
        job << "# stdout und stderr Dateien\n#PBS -o " << pfadauswertung << "/log/out.out -e " << pfadauswertung << "/log/err.err\n#\n";
        job << "#PBS -M peter.deiml@fau.de -m abe\n# first non-empty non-comment line ends PBS options\n\n";
        job << "# jobs always start in $HOME\n";
        job << "cd " << pfadauswertung << "/\n";
        job << "mkdir log\n";
        job << "module load root/root_new\n";
        job << "./COCOShpc " << filepath << datei;
        job.close();

    }

}


