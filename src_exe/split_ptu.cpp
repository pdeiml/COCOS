#include "PtuFile.hpp"

double GlobRes = 0.0;
long long iGlobRes = 0;

int main (int argc, char* argv[]){

	std::stringstream vArgument;
	vArgument << argv[1];
	std::string vArgumentRoot = vArgument.str().substr(0,vArgument.str().length()-4);
	// std::cout << vArgumentRoot;
	PtuFile vPtuFile(argv[1]);
    vPtuFile.OpenPtuFile();
    if( vPtuFile.ReadHeader() ){
      GWARNING << "Read ptu header failed.";
    }	

    GlobRes = vPtuFile.GetGlobalResolution();
    iGlobRes = vPtuFile.GetIGlobalResolution();
    long long RecordType = vPtuFile.GetRecordType();
    long long NumRecords = vPtuFile.GetNumberOfRecords();
    FILE* fpin = vPtuFile.GetFilePointer();

    vPtuFile.ClosePtuFile();

    FILE* vFile;
    vFile = fopen(argv[1], "r");

    int vLength = 3392; // 212*16 Laenge des Headers
	char vHeader[vLength];

	int Result1 = fread(&vHeader, 1, sizeof(vHeader), vFile);

	int vNumberOfNewFiles = 0;
	vNumberOfNewFiles = NumRecords/1000000;
	std::cout << vNumberOfNewFiles << std::endl;
	for( int l=0; l<vNumberOfNewFiles; l++){
		std::stringstream vSubFileName;
		// vSubFileName << vArgumentRoot + "_no_" << l << ".ptu";
		std::cout << vSubFileName.str() << std::endl;
		FILE* vSubFile;
		vSubFile = fopen(vSubFileName.str().c_str(), "w");
		char vData[1];
		int vRes_val = fread(&vData, 1, sizeof(vData) ,vFile);
		for( int k= 0; k<3392; k++){
			int writeresult = fputc((char)vHeader[k], vSubFile);
		}
		for(int l=0; l<1; l++){
			int writeresult = fputc((char)vData[l], vSubFile);
		}
		fclose(vSubFile);
	}
	fclose(vFile);
}
