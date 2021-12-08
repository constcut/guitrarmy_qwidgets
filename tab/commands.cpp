#include "tab/commands.h"

#include "tab/tab.h" //Из-за reversable отделить их от обычных комманд

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
    os << commandType << static_cast<uint32_t>(command);
    return os;
}

std::ostream& operator<<(std::ostream& os, const TrackCommand& command) {
    uint32_t commandType = static_cast<int>(CommandPack::SingleTrackCommand);
    os << commandType << static_cast<uint32_t>(command);
    return os;
}

std::ostream& operator<<(std::ostream& os, const StringCommand<TabCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::StringTabCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter;
    return os;
}

std::ostream& operator<<(std::ostream& os, const StringCommand<TrackCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::SingleTrackCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter;
    return os;
}

std::ostream& operator<<(std::ostream& os, const IntCommand<TabCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::IntTabCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter;
    return os;
}

std::ostream& operator<<(std::ostream& os, const IntCommand<TrackCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::IntTrackCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter;
    return os;
}

std::ostream& operator<<(std::ostream& os, const TwoIntCommand<TabCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::TwoIntTabCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter1 << command.parameter2;
    return os;
}

std::ostream& operator<<(std::ostream& os, const TwoIntCommand<TrackCommand>& command) {
    uint32_t commandType = static_cast<int>(CommandPack::TwoIntTrackCommand);
    os << commandType << static_cast<uint32_t>(command.type) << command.parameter1 << command.parameter2;
    return os;
}


std::ifstream& operator>>(std::ifstream& is, MacroCommand& macro) {
    uint32_t commandType;
    is >> commandType;
    auto type = static_cast<CommandPack>(commandType);
    uint32_t enumType;
    is >> enumType;

    switch(type) {
        case CommandPack::SingleTabCommand:
            macro = static_cast<TabCommand>(enumType);
        break;
        case CommandPack::SingleTrackCommand:
            macro = static_cast<TrackCommand>(enumType);
        break;
        case CommandPack::IntTabCommand:
        {
            size_t tabInt;;
            is >> tabInt;
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
            is >> int1 >> int2;
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

