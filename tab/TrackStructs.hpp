#ifndef TRACKSTRUCTS_HPP
#define TRACKSTRUCTS_HPP

#include <cstdint>
#include "Bar.hpp"

namespace gtmy {

    class GuitarTuning {
        std::uint8_t stringsAmount;
        std::uint8_t tunes[10]; //TODO vector + at, to expeption and avoid check
        //set it to byte - in fact int would be 128 values of std midi - next could be used as quatones
    public:

        void setStringsAmount(std::uint8_t amount) { stringsAmount = amount; }
        std::uint8_t getStringsAmount() const { return stringsAmount; }

        void setTune(std::uint8_t index, std::uint8_t value) { if (index <= 10) tunes[index] = value; } //(index >= 0) &&
        std::uint8_t getTune(std::uint8_t index) const { if (index <= 10) return tunes[index]; return 0; }
    };


    class ChainedBars : public std::vector<Bar*> {
      public:
        ChainedBars() = default;
        virtual ~ChainedBars() = default;
    };

}


#endif // TRACKSTRUCTS_HPP
