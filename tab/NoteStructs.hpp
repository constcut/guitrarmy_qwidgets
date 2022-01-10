#ifndef NOTESTRUCTS_H
#define NOTESTRUCTS_H

#include <vector>
#include <cstdint>


namespace gtmy {


    struct BendPointGPOld //Все структуры и статусы в отдельный заголовок TODO
    {
        size_t absolutePosition;
        size_t heightPosition;
        std::uint8_t vibratoFlag;
    };


    class BendPointsGPOld : public std::vector<BendPointGPOld>
    {
    protected:
        size_t _bendHeight;
        std::uint8_t _bendType;
    public:

        std::uint8_t getType() const { return _bendType; }
        void setType(std::uint8_t newBendType) { _bendType = newBendType; }

        size_t getHeight() const { return _bendHeight; }
        void setHeight(size_t newBendHeight) { _bendHeight=newBendHeight;}

    };


    struct BendPoint
    {
        std::uint8_t vertical; //:4
        std::uint8_t horizontal; //:5
        std::uint8_t vFlag; //:2
    };


    class BendPoints : public std::vector<BendPoint>
    {
    protected:
        std::uint8_t bendType;
    public:
        std::uint8_t getType() const { return bendType; }
        void setType(std::uint8_t newBendType) { bendType = newBendType; }

        void insertNewPoint(BendPoint bendPoint);
    };


    struct GraceNote {
        uint8_t fret;
        uint8_t dynamic;
        uint8_t transition;
        uint8_t duration;
    };

}


#endif // NOTESTRUCTS_H
