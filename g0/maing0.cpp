#include "g0/types.h"

/*
bool check_types()
{
    bool allisfine = true;
    allisfine &= (sizeof(long long) == 8);
    allisfine &= (sizeof(long) == 4);
    allisfine &= (sizeof(short) == 2);
    if (allisfine)
        std::cout << "Check Types fine" << std::endl;
    else
        std::cout << "Failed to check types" << std::endl;
    return allisfine;
}
*/


/*/first include for debugging
#include <iostream> 
//as we work from single file first we had to build all of the sources
#include "afile.cpp"
#include "midifile.cpp"
#include "amusic.cpp"
#include "tab.cpp"
//and no need for extra includes - all the project already here - just check everything could be compiled
void checks()
{
	std::cout << "Starting checks function [002]" <<std::endl;
	bool typesFine = check_types();
	AFile afile; //compile check           - no need for output
	MidiFile mfile;//compile check      - because if failed
	AMusic amusic; //compile check - then none would be anyway
	Tab tablature;
}
//end of checks. code follows



int main(int argc, char *argv[])
{
		checks(); //checks first
		
		//now try to read midi file
		std::ifstream ifile;
		//ifile.open("/sdcard/p/test.x"); //large file
		ifile.open("/sdcard/p/g0/t/1.mid");  //small file - runs ok!
		
		if (ifile.is_open())
			std::cout << "File opened" << std::endl;
		else
			std::cout <<"Failed to open file" << std::endl;
			
		MidiFile midiFile;
		AFile abstractFile(ifile);
		midiFile.readStream(abstractFile);
		
		std::cout << "Reading file finished" << std::endl;

		midiFile.printToStream(std::cout);
	
		std::ofstream ofile;
		ofile.open("/sdcard/p/g0/t/out1.mid");
		
		if (ofile.is_open())
			std::cout << "Out file opened."<<std::endl;
		else
			std::cout << "Failed to open out file :("<<std::endl;
				
		AFile outFile(ofile);
		ul outFileSize = midiFile.writeStream(outFile);	

		std::cout << "File wroten. " << outFileSize << " bytes. " << std::endl;

	
		std::ifstream itfile;
		itfile.open("/sdcard/p/g0/t/1.gp4");  //small file - runs ok!
		
		if (itfile.is_open())
			std::cout << "File opened" << std::endl;
		else
			std::cout <<"Failed to open file" << std::endl;


		AFile tabFile(itfile);
		Tab tab;
		Gp4Import importer; //(tabFile,tab);
		
		importer.import(tabFile,tab);

    return 0;
}
*/
