#pragma once
//-----------------------------------------------------------------------------------------------
#include <string>
#include <vector>

struct Vec3;

//-----------------------------------------------------------------------------------------------
const std::string Stringf( char const* format, ... );
const std::string Stringf( int maxLength, char const* format, ... );
const std::string Vec3toString(Vec3 const& vector3);

typedef std::vector<std::string> Strings;
Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn, bool cutOutLeadingAndTrailingWhiteSpace = true);
Strings SplitStringOnFirstDelimiter(std::string const& originalString, char delimiterToSplitOn, bool cutOutLeadingAndTrailingWhiteSpace = true);
Strings SplitStringOnDelimiter(std::string const& originalString, char delimiterToSplitOn, char charShelfToSkip, bool cutOutLeadingAndTrailingWhiteSpace = true);
void CutOutLeadingAndTrailingWhiteSpace(Strings& stringsToEdit);
void CutOutLeadingAndTrailingWhiteSpace(std::string& stringToEdit);
void CutOutLeadingAndTrailingCharacters(Strings& stringsToEdit, char charToRemove);
void RemoveAllCharactersOfType(std::string& stringToEdit, char charToRemove);
std::string GetLowercase(std::string const& inputString);
std::string GetUpperCase(std::string const& inputString);
int GetIndexOfLastChar(std::string const& inputString, char charToFind);





