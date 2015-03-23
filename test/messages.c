// -*- mode: c -*-
/****************************************************************
COPYRIGHT_BEGIN
Copyright (C) 2015, 0x1 Consulting.
All rights reserved.
COPYRIGHT_END
****************************************************************/

#include "messages.h"

/// The RULES
/// You have eight parameters to play with represented as %1 through %8.
/// To print a '%' (percentage) symbol use %%, otherwise they're dropped.
/// Arguments can be used multiple times in any order you choose so 
/// "%4 %3 %4" is fine.
/// We don't give you options for pretty formatting of leading zeros etc.

/// Catalog of log messages
char *cobaro_messages_en [COBARO_TEST_MSG_COUNT + 1] = {
    // COBARO_TEST_MESSAGE_NULL
    // string literal
    "%1",

    // COBARO_TEST_MESSAGE_TYPES
    // string, int, float, ip
    "s:%1, i:%2, f:%3, ip:%4, percent:%%",


    // so trailing commas always work
    ""
};

char *cobaro_messages_klingon [COBARO_TEST_MSG_COUNT + 1] = {
    // COBARO_TEST_MESSAGE_NULL
    // string literal
    "%1",

    // COBARO_TEST_MESSAGE_TYPES
    // string, int, float, ip
    "i:%2, s:%1, hoch:%1, f:%3, ip:%4, chipath:%%",


    // so trailing commas always work
    ""
};
