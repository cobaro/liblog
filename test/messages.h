// -*- mode: c -*-
#ifndef COBARO_TEST_MESSAGES_H
#define COBARO_TEST_MESSAGES_H
/****************************************************************
COPYRIGHT_BEGIN
Copyright (C) 2015, cobaro.org
All rights reserved.
COPYRIGHT_END
****************************************************************/

/// To add a new message you should it's identifier here and then
/// and content for the language definitions in messages.c

/// Enumeration of message identifiers 
///
/// These are array looked up so must range from 0 to COBARO_MSG_COUNT
enum cobaro_test_message_ids {
    COBARO_TEST_MESSAGE_NULL = 0,
    COBARO_TEST_MESSAGE_TYPES = 1,

    COBARO_TEST_MSG_COUNT
};

extern char *cobaro_messages_en[];
extern char *cobaro_messages_klingon[];

#endif /* COBARO_MESSAGES_H */
