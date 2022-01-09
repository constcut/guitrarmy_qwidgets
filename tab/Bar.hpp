#ifndef BAR_H
#define BAR_H

#include "Types.hpp"
#include "Beat.hpp"


namespace gtmy {


    int translateDenum(std::uint8_t den);
    int translaeDuration(std::uint8_t dur);
    int updateDurationWithDetail(std::uint8_t detail, int base);

    class Track;

    class Bar : public ChainContainer<Beat, Track> {

    public:
        Bar() {
            flush();
        }
        virtual ~Bar() = default;

        void printToStream(std::ostream &stream);

        void flush()
        {
            signatureNum = signatureDenum = 0;
            repeat = repeatTimes = altRepeat = 0;
            markerColor = 0;
            completeStatus = 0;
        }

        Bar &operator=(Bar *another)
        {
            clone(another);
            return *this;
        }

        virtual void push_back(std::unique_ptr<Beat> val) { //TODO так же в TAB
            if (val) {
                val->setParent(this);
                ChainContainer<Beat, Track>::push_back(std::move(val));
            }
        }

        virtual void insertBefore(std::unique_ptr<Beat> val, int index=0) {
            if (val) {
                val->setParent(this);
                ChainContainer<Beat, Track>::insertBefore(std::move(val),index);
            }
        }


    protected:

        std::uint8_t signatureNum;
        std::uint8_t signatureDenum;

        std::uint8_t repeat;//reprease options
        std::uint8_t repeatTimes;
        std::uint8_t altRepeat;

        //GP comp - marker, tonality
        std::uint8_t gpCompTonality;
        std::string markerText;
        size_t markerColor; //white byte == 1 if empty

        std::uint8_t completeStatus;
        short completeAbs;
        size_t completeIndex;

        public:
        //SET GET operations

        void setSignNum(std::uint8_t num) { signatureNum = num; }
        void setSignDenum(std::uint8_t denum) { signatureDenum = denum; }

        std::uint8_t getSignNum() { return signatureNum; }
        std::uint8_t getSignDenum() { return signatureDenum; }

        //!completeStatus !!! that should go private and opt
        void countUsedSigns(std::uint8_t &numGet, std::uint8_t &denumGet);

        std::uint8_t getCompleteStatus();
        double getCompleteAbs();
        size_t   getCompleteIndex();

        void setRepeat(std::uint8_t rValue, std::uint8_t times=0)
        {
            if (rValue == 0) repeat = 0;
            else repeat |= rValue;
            if(times) repeatTimes=times;
        }
        std::uint8_t getRepeat() { return repeat; }
        std::uint8_t getRepeatTimes() { return repeatTimes; }

        void setAltRepeat(std::uint8_t number) { altRepeat = number; }
        std::uint8_t getAltRepeat() { return altRepeat; }

        void setGPCOMPTonality(std::uint8_t tValue) { gpCompTonality = tValue; }
        std::uint8_t getGPCOMPTonality() { return gpCompTonality; }

        void setGPCOMPMarker(std::string &text, size_t color) { markerText = text; markerColor = color; }
        void getGPCOMPMarker(std::string &text, size_t &color) { text = markerText; color = markerColor; }

        //pack function
         void clone(Bar *from);
    };


}

#endif // BAR_H
