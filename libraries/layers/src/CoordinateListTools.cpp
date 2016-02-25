////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Project:  [projectName]
//  File:     CoordinateListTools.cpp (common)
//  Authors:  Ofer Dekel
//
//  [copyright]
//
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CoordinateListTools.h"

// utilities
#include "Parser.h"

namespace
{
    void HandleErrors(utilities::Parser::Result result)
    {
        if (result == utilities::Parser::Result::badFormat)
        {
            throw std::runtime_error("bad format in coordinate list definition string");
        }
        else if (result == utilities::Parser::Result::endOfString || result == utilities::Parser::Result::beginComment)
        {
            throw std::runtime_error("premature end of coordinate list definition string");
        }
    }

    // Parses the c-string pStr. Allowed values are (1) non-negative integers not greater than maxIndex, (2) "e", which translates to maxIndex, (3) "e-<uint>" which translates into maxIndex minus the integer value, as long as the outcome is in the range [0,maxValue]
    uint64 ParseIndex(const char*& pStr, uint64 maxIndex)
    {
        uint64 index;
        if (*pStr == 'e')
        {
            index = maxIndex;
            ++pStr;

            if (*pStr == '-')
            {
                ++pStr;
                uint64 diff;
                HandleErrors(utilities::Parser::Parse<uint64>(pStr, /* out */ diff));
                index -= diff;
            }
        }
        else
        {
            HandleErrors(utilities::Parser::Parse<uint64>(pStr, /* out */ index));
        }

        if (index > maxIndex)
        {
            throw std::runtime_error("in coordinate list definition string, index " + std::to_string(index) + " exceeds maximal value " + std::to_string(maxIndex));
        }

        return index;
    }

    // adds an sequence of entries to a coordinateList
    void AddCoordinates(layers::CoordinateList& coordinateList, uint64 layerIndex, uint64 fromElementIndex, uint64 toElementIndex)
    {
        for (uint64 elementIndex = fromElementIndex; elementIndex <= toElementIndex; ++elementIndex)
        {
            coordinateList.emplace_back(layerIndex, elementIndex);
        }
    }
}

namespace layers
{
    layers::CoordinateList GetCoordinateList(const layers::Map& map, const std::string& coordinateListString)
    {
        layers::CoordinateList coordinateList;
        const char* pStr = coordinateListString.c_str();
        const char* pEnd = pStr + coordinateListString.size();

        while (pStr < pEnd)
        {
            // read layer Index
            uint64 layerIndex = ParseIndex(pStr, map.NumLayers() - 1);

            // read element index
            uint64 fromElementIndex = 0;
            uint64 maxElementIndex = map.GetLayer(layerIndex)->Size() - 1;
            uint64 toElementIndex = maxElementIndex;

            // case: no elements specified - take entire layer
            if (*pStr == ';')
            {
                ++pStr;
            }

            // case: elements specified
            else if (*pStr == ',')
            {
                ++pStr;
                fromElementIndex = toElementIndex = ParseIndex(pStr, maxElementIndex);

                // interval of elements
                if (*pStr == ':')
                {
                    ++pStr;
                    toElementIndex = ParseIndex(pStr, maxElementIndex);

                    if (toElementIndex < fromElementIndex)
                    {
                        throw std::runtime_error("bad format in coordinate list definition string");
                    }
                }
            }

            // add the coordinates to the list
            AddCoordinates(coordinateList, layerIndex, fromElementIndex, toElementIndex);
        }

        return coordinateList;
    }

    layers::CoordinateList GetCoordinateList(uint64 layerIndex, uint64 fromElementIndex, uint64 toElementIndex)
    {
        layers::CoordinateList coordinateList;
        AddCoordinates(coordinateList, layerIndex, fromElementIndex, toElementIndex);
        return coordinateList;
    }
}