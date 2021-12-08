#include "tab/commands.h"

#include "tab/tab.h" //Из-за reversable отделить их от обычных комманд

#include <QDebug>

enum CommandPack {
    SingleTabCommand=0,
    SingleTrackCommand,
    IntTabCommand,
    IntTrackCommand,
    StringTabCommand,
    StringTrackCommand,
    TwoIntTabCommand,
    TwoIntTrackCommand
};


std::ostream& operator<<(std::ostream& os, const TabCommand& command) {
    uint32_t commandType = static_cast<int>(CommandPack::SingleTabCommand);
    uint32_t commandValue = static_cast<uint32_t>(command);

    os.write((const char*)& commandType, 4);
    os.write((const char*)& commandValue, 4);

    qDebug() << "Written TabCommand: " << commandType <<  " " << static_cast<uint32_t>(command);
    return os;
}

std::ostream& operator<<(std::ostream& os, const TrackCommand& command) {
    uint32_t commandType = static_cast<int>(CommandPack::SingleTrackCommand);
    os << commandType << static_cast<uint32_t>(command);
    return os;
}

std::ostream& operator<<(std::ostream& os, const StringCommand<TabCommand>& command) {

    //uint32_t commandType = static_cast<int>(CommandPack::StringTabCommand);
    //os << commandType << static_cast<uint32_t>(command.type) << command.parameter;

    uint32_t commandType = static_cast<int>(CommandPack::StringTabCommand);
    uint32_t commandValue = static_cast<uint32_t>(command.type);
    os.write((const char*)& commandType, 4);
    os.write((const char*)& commandValue, 4);
    os << command.parameter;
    //os.write((const char*)&command.parameter, 4);

    return os;
}

std::ostream& operator<<(std::ostream& os, const StringCommand<TrackCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::SingleTrackCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter;
    return os;
}

std::ostream& operator<<(std::ostream& os, const IntCommand<TabCommand>& command) {

    uint32_t commandType = static_cast<int>(CommandPack::IntTabCommand);
    uint32_t commandValue = static_cast<uint32_t>(command.type);
    os.write((const char*)& commandType, 4);
    os.write((const char*)& commandValue, 4);
    os.write((const char*)&command.parameter, 4); //TODO может изменить с size_t, тк он вроде 64
    return os;
}

std::ostream& operator<<(std::ostream& os, const IntCommand<TrackCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::IntTrackCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter;
    return os;
}

std::ostream& operator<<(std::ostream& os, const TwoIntCommand<TabCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::TwoIntTabCommand);
    uint32_t commandValue = static_cast<uint32_t>(command.type);
    os.write((const char*)& commandType, 4);
    os.write((const char*)& commandValue, 4);
    os.write((const char*)&command.parameter1, 4);
    os.write((const char*)&command.parameter2, 4);
    return os;
}

std::ostream& operator<<(std::ostream& os, const TwoIntCommand<TrackCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::TwoIntTrackCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter1 << command.parameter2;
    return os;
}


std::ifstream& operator>>(std::ifstream& is, MacroCommand& macro) {
    uint32_t commandType;
    //is >> commandType;
    is.read((char*)&commandType, 4); //TODO 1
    auto type = static_cast<CommandPack>(commandType);
    uint32_t enumType;
    is.read((char*)&enumType, 4);

    if (is.eof())
        return is;

    qDebug() << "Reading: " << commandType << " " << enumType;

    switch(type) {
        case CommandPack::SingleTabCommand:
            macro = static_cast<TabCommand>(enumType);
        break;
        case CommandPack::SingleTrackCommand:
            macro = static_cast<TrackCommand>(enumType);
        break;
        case CommandPack::IntTabCommand:
        {
            size_t tabInt;
            is.read((char*)&tabInt, 4);
            qDebug() << "Read int " << tabInt;
            macro = IntCommand<TabCommand>{static_cast<TabCommand>(enumType), tabInt };
        }
        break;
        case CommandPack::IntTrackCommand:
        {
            size_t trackInt;
            is >> trackInt;
            macro = IntCommand<TabCommand>{static_cast<TabCommand>(enumType), trackInt };
        }
        break;
        case CommandPack::StringTabCommand:
        {
            std::string tabString;
            is >> tabString;
            qDebug() << "String read: " << tabString.c_str();
            macro = StringCommand<TabCommand>{static_cast<TabCommand>(enumType), tabString };
        }
        break;
        case CommandPack::StringTrackCommand: {
            std::string trackString;
            is >> trackString;
            macro = StringCommand<TrackCommand>{static_cast<TrackCommand>(enumType), trackString };
        }
        break;
        case CommandPack::TwoIntTabCommand:
        {
            size_t int1, int2;
            is.read((char*)&int1, 4);
            is.read((char*)&int2, 4);
            macro = TwoIntCommand<TabCommand>{static_cast<TabCommand>(enumType), int1, int2 };
        }
        break;
        case CommandPack::TwoIntTrackCommand:
        {
            size_t int1, int2;
            is >> int1 >> int2;
            macro = TwoIntCommand<TrackCommand>{static_cast<TrackCommand>(enumType), int1, int2 };
        }
        break;
    default:
        qDebug() << "ERROR: unknown command type";
    }

    return is;
}


void saveMacroComannds(const std::vector<MacroCommand>& commands, std::ofstream& os) {

    for (const auto& command: commands)
        std::visit([&os](const auto& command){ os << command; }, command);
}


std::vector<MacroCommand> loadMacroCommands(std::ifstream& is) {
    std::vector<MacroCommand> commands;
    while (is.eof() == false) {
        MacroCommand macro;
        is >> macro;
        commands.push_back(macro);
    }
    return commands;
}

