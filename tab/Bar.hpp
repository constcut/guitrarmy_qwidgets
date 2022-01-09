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

        void printToStream(std::ostream& stream) const;

        void flush() {
            _signatureNum = _signatureDenum = 0;
            _repeat = _repeatTimes = _altRepeat = 0;
            _markerColor = 0;
            _completeStatus = 0;
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

        std::uint8_t _signatureNum;
        std::uint8_t _signatureDenum;

        std::uint8_t _repeat;
        std::uint8_t _repeatTimes;
        std::uint8_t _altRepeat;

        std::uint8_t _tonality;
        std::string _markerText;
        size_t _markerColor; //white byte == 1 if empty

        std::uint8_t _completeStatus;
        short _completeAbs;
        size_t _completeIndex;

    public:

        void setSignNum(std::uint8_t num) { _signatureNum = num; }
        void setSignDenum(std::uint8_t denum) { _signatureDenum = denum; }

        std::uint8_t getSignNum() const { return _signatureNum; }
        std::uint8_t getSignDenum() const { return _signatureDenum; }


        void countUsedSigns(std::uint8_t& numGet, std::uint8_t& denumGet);

        std::uint8_t getCompleteStatus();
        double getCompleteAbs() const;
        size_t getCompleteIndex() const;

        void setRepeat(std::uint8_t rValue, std::uint8_t times=0) {
            if (rValue == 0) _repeat = 0;
            else _repeat |= rValue;
            if(times) _repeatTimes=times;
        }

        std::uint8_t getRepeat() const { return _repeat; }
        std::uint8_t getRepeatTimes() const { return _repeatTimes; }

        void setAltRepeat(std::uint8_t number) { _altRepeat = number; }
        std::uint8_t getAltRepeat() const { return _altRepeat; }

        void setTonality(std::uint8_t tValue) { _tonality = tValue; }
        std::uint8_t getTonality() const { return _tonality; }

        void setMarker(std::string &text, size_t color) { _markerText = text; _markerColor = color; }
        void getMarker(std::string &text, size_t &color) { text = _markerText; color = _markerColor; }

        //pack function
         void clone(Bar *from);
    };


}

#endif // BAR_H
