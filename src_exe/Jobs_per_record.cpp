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
    std::string filepath = "/mnt/g/TimeHarpData_A/";
    //Folder path of evaluation files
    std::string evaluationpath = "/home/ii/TimeHarpAuswertung/";
    //Folder path of job-files
    std::string jobpath = "/home/ii/ii_jobs/";
    //Path where COCOShpc is loacated
    std::string cocoshpcpath = "/home/ii/Software/cocos/build/";
    //***** Change e-mail!!! *****

    //Standard interval length of one evaluation in s
    int no_subanalysis = 1;

	std::string jobname;
    std::cout << "\033[1;31mName des Jobs (Rumpf):\033[0m\t\t"; cin >> jobname;

    settingsmarker:;
    std::cout << "\n\033[1;31m-----------------------------------------------\033[0m" << std::endl;
    std::cout << "\033[1;31mAnzahl an Sub-Analysen\t[n]\033[0m\t\t" << no_subanalysis << std::endl;
    std::cout << "\033[1;31mContinue\t\t[c]\033[0m\t" << std::endl;
    std::cout << "\033[1;31m-----------------------------------------------\033[0m\n" << std::endl;

    std::cout << "\033[1;31mBefehl eingeben:\033[0m\t";
    std::string instring;
    cin >> instring;

    if (instring == "n")
    {
        std::cout << "Anzahl Sub-Analysen:\t"; cin >> instring;
        no_subanalysis = std::stoi(instring);
    }
    if (instring == "c")
    {
        goto endsettingsmarker;
    }
    goto settingsmarker;
    endsettingsmarker:;

    int nfiles = no_subanalysis;

    std::string datei;
    std::cout << "\n\033[1;31mMögliche Messdateien:\033[0m" << std::endl;
    std::cout << "\033[1;31m---------------------------------------\033[0m" << std::endl;
    std::string lsbefehl = "ls -R " + filepath; system(lsbefehl.c_str());
    std::cout << "\033[1;31m---------------------------------------\033[0m" << std::endl;
    std::cout << "\033[1;31mMessung:\033[0m\t\t" << filepath; cin >> datei;

    std::ostringstream counterstring[100];
    std::string auswertung, cutauswertung, gesamtpfad, befehl, settingsstring, restpath, befehl2;
    ofstream settings, job, mail;
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
            // std::cout << befehl << "  halsdfa" << std::endl;
            if (restpath.find("/") == -1){mkdirfertig = true;}
            restpath = restpath.substr(restpath.find("/")+1);

            //std::string weiter; cin >> weiter;
        }
    
        std::string pfadauswertung = evaluationpath + datei.substr(0, datei.find(".")) + "/" + auswertung;
        befehl2 = "cd " + pfadauswertung + " && mkdir log"; system(befehl2.c_str());
        // std::cout << "pfad: " << pfadauswertung << std::endl;
        //std::string mkdirstring = "mkdir " + pfadauswertung;
        //system(mkdirstring.c_str()); std::cout << "Mache:\t" << mkdirstring << std::endl;
        std::string copystring = "scp " + cocoshpcpath + "COCOS_per_record " + pfadauswertung +  "/";
        system(copystring.c_str()); //std::cout << "Mache:\t" << copystring << std::endl;


        settingsstring = pfadauswertung + "/settings.txt";
        settings.open(settingsstring.c_str());
        settings << "sl 1\n";
        settings << "ts -250000\n";
        settings << "te 250000\n";
        settings << "nb 2000\n";
        settings << "cm 2\n";
        settings << "#end\n";
        settings << "$Calibration_" << jobname << "_" << i << ".txt";
        settings.close();

        std::string makemail = pfadauswertung + "/mail.txt";
        mail.open(makemail.c_str());
        mail << "From: intensityinterferometry@gmail.com\n";
        mail << "To: peter.deiml@fau.de\n";
        mail << "Subject: " << jobname + "_" + counterstring[i].str() + ".sh" << " finished.\n\n";
        mail << "Job\n" << makejob << "\n is finished.\n\nGreetings from pi4097";
	mail.close();

        job.open(makejob.c_str());
        job << "cd " << pfadauswertung << "/\n";
        // job << "mkdir log\n";
        job << "./COCOS_per_record " << filepath << datei << " " << i << "\n";
        job << "ssmtp peter.deiml@fau.de < mail.txt\n";
	job << "rm mail.txt\n";
        job.close();

    }

}


