#include "aexpimp.h"


#include "g0/astreaming.h"


static AStreaming logger("aexpimp");

bool GTabLoader::open(std::string fileName)
{
    AFile file;

    if (file.open(fileName) == false)
    {
        logger << "Failed to open file from loader";
        return false;
    }

    char firstBytes[3]={0};
    file.read(firstBytes,1);

    if (firstBytes[0] == 'G')
    {
        //then guitarmy sign to be sure it realy is - but yet skipped

        file.read(&firstBytes[1],1);

        if (firstBytes[1]=='A')
        {
            GmyFile gF;
            if (tab)
            {
                delete tab;
                tab = 0;
            }
            tab = new Tab;

            if (gF.loadFromFile(&file,tab,true))
            {
                tab->connectTracks();
                file.close();
                return true;
            }
        }
    }
    else
    {
       //logger << "Not gmy format "<<firstBytes;
       // std::cout <<"First byte of file - "<<firstBytes[0]<<";"<<(int)firstBytes[0];

        file.r(firstBytes,2);

        std::string headBytes = firstBytes;
        if (headBytes=="IF")
        {
            //""THEN" ;
            char version[2];
            char skip[2];

            for (int i = 0; i < 28; ++i) //1+29-2 + post
            {
                if (i==18) file.read(version,1);
                else
                if (i==20) file.read(&version[1],1);
                else
                    file.read(skip,1);
            }

            byte versionIndex = version[0]-48;
            if (version[1]!='0')
            {
                versionIndex *= 10;
                versionIndex += version[1]-48;
            }

            if ((versionIndex == 5) || (versionIndex == 51)
                    || (versionIndex == 4) || (versionIndex == 3))
            {
                if (tab)
                {
                    delete tab;
                    tab = 0;
                }
                tab = new Tab;
            }

            bool loaded = false;

            if ((versionIndex == 5) || (versionIndex == 51))
            {
                Gp5Import gImp;
                loaded = gImp.import(file,tab,versionIndex);
            }
            if (versionIndex == 4)
            {
                Gp4Import gImp;
                loaded = gImp.import(file,tab,versionIndex);
            }
            if (versionIndex == 3)
            {
                Gp3Import gImp;
                loaded = gImp.import(file,tab,versionIndex);
            }
            if (loaded)
            {
                tab->postGTP();
                tab->connectTracks();

                file.close();
                return true;
            }
        }
        else if (headBytes=="at")
        {
            PtbFile pImp;

            if (tab)
            {
                delete tab;
                tab = 0;
            }
            tab = new Tab;

            bool loaded = pImp.loadFromFile(&file,tab,true);

            if (loaded)
            {
                //tab->postGTP();
                //tab->connectTracks();

                file.close();
                return true;
            }
        }
        else
        {
            logger << "unknown format";
            file.close();
            return false;
        }

    }

    file.close();
    return false;
}
