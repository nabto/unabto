#include <modules/urrdtool/urrdtool.h>
#include <modules/urrdtool/urrdtool_backend_eeprom.h>
#include <modules/urrdtool/urrdtool_backend_cfs.h>
#include "urrd_file_backend.h"

#include <time.h>
#include <stdint.h>
#include <stdio.h>

#include "CuTest.h"

static uint32_t staticTime = 1409230628;

uint32_t urrd_time() {
    return staticTime;
}

void read_from_empty_db(CuTest *tc)
{
    urrd_init(true);
    uint8_t urrdId = 0;
    CuAssert(tc, "", urrd_create(urrdId, 10000, 4, 60, 0, 0, 0) == URRD_OK);
    printf("Got urrd database with id %i\n",urrdId);
    
    uint16_t buf[2];
    uint32_t to;
    
    uint32_t records = urrd_read_by_timestamp(urrdId, staticTime, buf, 4, &to);
    
    CuAssert(tc, "", records == 0);
}


void read_from_db(CuTest *tc)
{
    uint32_t to;
    urrd_init(true);
    uint8_t urrdId = 0;
    CuAssert(tc, "", urrd_create(urrdId, 10000, 2, 60, 0, 0, 0) == URRD_OK);
    printf("Got urrd database with id %i\n",urrdId);
    
    uint16_t buf;

    buf = 42;
 
    CuAssert(tc, "", urrd_write(urrdId, &buf));
    // There is a bug we cant read the current value. Add one to the time.
    staticTime += 0;

    uint32_t records = urrd_read_by_timestamp(urrdId, staticTime, &buf, 2, &to);
    
    CuAssert(tc, "", records == 1);
    CuAssert(tc, "", buf == 42);

    staticTime += 1;
    
    records = urrd_read_by_timestamp(urrdId, staticTime, &buf, 2, &to);
    CuAssert(tc, "", records == 1);
    CuAssert(tc, "", buf == 42);
}

void read_from_db_two_entries(CuTest *tc) {
    uint32_t to;
    urrd_init(true);
    CuAssert(tc, "", urrd_create(0, 10000, 2, 60, 0, 0, 0) == URRD_OK);

    uint16_t buf = 42;
    CuAssert(tc, "", urrd_write(0, &buf));

    staticTime += 60;
    buf = 43;
    CuAssert(tc, "", urrd_write(0, &buf));
    
    uint16_t buffer[2];
    staticTime += 1;
    uint32_t records = urrd_read_by_timestamp(0, staticTime, buffer, 4, &to);
    CuAssert(tc, "", records == 2);

    CuAssert(tc, "", buffer[0] == 42);
    CuAssert(tc, "", buffer[1] == 43);
}


void read_from_db_in_future(CuTest *tc) 
{
    uint32_t to;
    urrd_init(true);   

    uint8_t urrdId = 0;
    urrd_create(urrdId, 10000, 4, 60, 0, 0, 0);
    printf("Got urrd database with id %i\n", urrdId);
    
    uint16_t buf[2];

    buf[0] = 0; buf[1] = 0;
 
    urrd_write(urrdId, buf);
   
    staticTime += 100;

    uint32_t records = urrd_read_by_timestamp(urrdId, staticTime, buf, 4, &to);
    
    CuAssert(tc, "", records == 0);
}

void read_from_middle_of_db(CuTest* tc)
{
    uint32_t to;
    urrd_init(true);
    uint8_t urrdId = 0;
    urrd_create(urrdId, 10000, 4, 60, 0, 0, 0);
    printf("Got urrd database with id %i\n",urrdId);
    
    uint16_t buf[2];

    buf[0] = 0; buf[1] = 0;
    
    int i = 0;
    for (i = 0; i < 1000; i++) {
        staticTime += 60;
        urrd_write(urrdId, buf);        
    }

    uint16_t buf2[2][100];

    uint32_t records = urrd_read_by_timestamp(urrdId, staticTime-60, buf2, 100*4, &to);
    
    CuAssert(tc, "", records == 100);  
}

void create_table_test(CuTest* tc)
{
    uint32_t to;
    urrd_init(true);

    uint8_t urrdId = 0;
    CuAssert(tc, "", urrd_create(urrdId, 10000, 2, 60, 0, 0, 0) == URRD_OK);
    
    uint16_t buf = 42;
    urrd_write(urrdId, &buf);

    staticTime += 1;

    CuAssert(tc, "", urrd_read_by_timestamp(urrdId, staticTime, &buf, 2, &to) == 1);
    CuAssert(tc, "", buf == 42);

    urrd_init(false);
    CuAssert(tc, "", urrd_create(urrdId, 10000, 2, 60, 0, 0, 0) == URRD_OK);
    CuAssert(tc, "", urrd_read_by_timestamp(urrdId, staticTime, &buf, 2, &to) == 1);
    CuAssert(tc, "", buf == 42);
}

void create_many_tables(CuTest* tc) {
    uint32_t to;
    int entries = 40;
    int createUrrd = MAX_NUMBER_OF_URRD;
    int urrdSize = 88;

    urrd_init(true);
    int i,j;
    for(i = 0; i < createUrrd; i++) {
        CuAssert(tc, "", urrd_create(i, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);
    }
    
    for (j = 0; j < entries; j++) {
        staticTime += 60;
        for (i = 0; i < createUrrd; i++) {
            uint16_t entry = i+j;
            CuAssert(tc, "", urrd_write(i, &entry));
        }
    }

    urrd_init(false);
    for(i = 0; i < MAX_NUMBER_OF_URRD; i++) {
        CuAssert(tc, "", urrd_create(i, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);
    }

    staticTime += 1;

    for (j = (entries-1); j >= 0; j--) {
        for (i = 0; i < createUrrd; i++) {
            uint16_t entry = i+j;
            uint16_t readEntry;
            CuAssert(tc, "", urrd_read_by_timestamp(i,staticTime - 60*((entries-1)-j), &readEntry, sizeof(readEntry), &to) == 1);
            CuAssert(tc, "", entry == readEntry);
        }
    }
}

void read_more_than_a_quarter_of_the_data(CuTest* tc) {
    uint32_t to;
    urrd_init(true);
    int urrdSize = 80;
    int entries = 21;
    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);
    
    int i;
    for (i = 0; i < entries; i++) {
        staticTime += 60;
        uint16_t entry = i;
        CuAssert(tc, "", urrd_write(0, &entry));
    }

    staticTime += 1;

    uint16_t readBuffer[entries];
    CuAssert(tc, "", urrd_read_by_timestamp(0,staticTime, readBuffer, sizeof(readBuffer), &to) == entries);
    
    for (i = 0; i < entries; i++) {
        CuAssert(tc, "", readBuffer[i] == i);
    }
}

void read_half_of_the_data_as_undefined_in_the_future(CuTest* tc) {
    uint32_t to;
    urrd_init(true);
    int urrdSize = 80;
    int entries = 2;
    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);
    
    int i;
    for (i = 0; i < entries; i++) {
        staticTime += 60;
        uint16_t entry = i;
        CuAssert(tc, "", urrd_write(0, &entry));
    }

    staticTime += 121;

    uint16_t readBuffer[4];
    CuAssert(tc, "", urrd_read_by_timestamp(0,staticTime, readBuffer, sizeof(readBuffer), &to) == 2);
    
    CuAssert(tc, "", readBuffer[0] == 0);
    CuAssert(tc, "", readBuffer[1] == 1);
}

void test_wrap_around(CuTest* tc) {
    uint32_t to;
    urrd_init(true);
    int urrdSize = 80;
    int entries = 120;
    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);

    int i;
    for (i = 0; i < entries; i++) {
        staticTime += 60;
        uint16_t entry = i;
        CuAssert(tc, "", urrd_write(0, &entry));
    }

    uint16_t readBuffer[1];
    CuAssert(tc, "", urrd_read_by_timestamp(0,staticTime, readBuffer, sizeof(readBuffer), &to) == 1);
    
    CuAssert(tc, "", readBuffer[0] == 119);


    uint8_t readn = 80;
    uint16_t readBuffer2[readn];

    CuAssert(tc, "", urrd_read_by_timestamp(0,staticTime, readBuffer2, sizeof(readBuffer2), &to) == readn);

    for (i = 0; i < readn; i++) {
        CuAssert(tc, "", readBuffer2[i] == 120-readn+i);
    }
}

void test_count_less_than_size_of_database(CuTest* tc) {
    urrd_init(true);
    int urrdSize = 80;
    int entries = 120;
    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);

    int i;
    for (i = 0; i < entries; i++) {
        staticTime += 60;
        uint16_t entry = i;
        CuAssert(tc, "", urrd_write(0, &entry));

        CuAssert(tc, "", get_urrd_count(0) <= 80);
    }
}

void test_undefined_value_fill(CuTest* tc) {
    urrd_init(true);
    int urrdSize = 80;
    uint32_t to;
    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);

    uint16_t entry = 42;

    CuAssert(tc, "", urrd_write(0, &entry));

    staticTime += 91;
    
    urrd_init(false);

    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);

    entry = 43;
    CuAssert(tc, "", urrd_write(0, &entry));

    uint16_t readBuffer[3];
    CuAssert(tc, "", urrd_read_by_timestamp(0, staticTime, readBuffer, sizeof(readBuffer), &to) == 3);

    CuAssert(tc, "", readBuffer[0] == 42);
    CuAssert(tc, "", readBuffer[1] == 0xFFFF);
    CuAssert(tc, "", readBuffer[2] == 43);
    
}

void test_time_in_past(CuTest* tc) {
    urrd_init(true);
    int urrdSize = 80;
    uint32_t to;
    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);

    uint16_t entry = 42;

    CuAssert(tc, "", urrd_write(0, &entry));

    staticTime -= 60;
    entry = 43;
    CuAssert(tc, "", urrd_write(0, &entry) == false);

    staticTime += 60;
    uint16_t readBuffer[2];
    CuAssert(tc, "", urrd_read_by_timestamp(0, staticTime, readBuffer, sizeof(readBuffer), &to) == 1);

    CuAssert(tc, "", readBuffer[0] == 42);
}

void test_time_massively_in_future(CuTest* tc) {
    urrd_init(true);
    int urrdSize = 80;
    uint32_t to;
    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);

    uint16_t entry = 42;

    CuAssert(tc, "", urrd_write(0, &entry));

    staticTime += 3444555;
    entry = 43;
    CuAssert(tc, "", urrd_write(0, &entry));

    uint16_t readBuffer[2];
    CuAssert(tc, "", urrd_read_by_timestamp(0, staticTime, readBuffer, sizeof(readBuffer), &to) == 1);

    CuAssert(tc, "", readBuffer[0] == 43);
}

void test_mass_data(CuTest* tc) {
    urrd_init(true);
    int urrdSize = 500000;
    uint32_t to;
    CuAssert(tc, "", urrd_create(0, urrdSize, 2, 60, 0, 0, 0) == URRD_OK);

    uint16_t entry = 42;
    
    int i;
    for (i = 0; i < urrdSize; i++) {
        entry = i % 1000;
        CuAssert(tc, "", urrd_write(0, &entry));
        staticTime += 60;
    }    
    
    staticTime -= 60;

    uint16_t readBuffer[urrdSize];
    CuAssert(tc, "", urrd_read_by_timestamp(0, staticTime, readBuffer, sizeof(readBuffer), &to) == urrdSize);
    for (i = 0; i < urrdSize; i++) {
        CuAssert(tc, "", readBuffer[i] == i % 1000);
    }
}


CuSuite* urrd_util_get_suite() {
    CuSuite* suite = CuSuiteNew();
    SUITE_ADD_TEST(suite, create_table_test);
    SUITE_ADD_TEST(suite, read_from_db_two_entries);
    SUITE_ADD_TEST(suite, read_from_empty_db);
    SUITE_ADD_TEST(suite, read_from_db);
    SUITE_ADD_TEST(suite, read_from_db_in_future);
    SUITE_ADD_TEST(suite, read_from_middle_of_db);
    SUITE_ADD_TEST(suite, create_many_tables);
    SUITE_ADD_TEST(suite, read_more_than_a_quarter_of_the_data);
    SUITE_ADD_TEST(suite, read_half_of_the_data_as_undefined_in_the_future);
    SUITE_ADD_TEST(suite, test_wrap_around);
    SUITE_ADD_TEST(suite, test_count_less_than_size_of_database);
    SUITE_ADD_TEST(suite, test_undefined_value_fill);
    SUITE_ADD_TEST(suite, test_time_in_past);
    SUITE_ADD_TEST(suite, test_time_massively_in_future);
    SUITE_ADD_TEST(suite, test_mass_data);
    return suite;
}


void RunAllTests(void) {
    CuString *output = CuStringNew();
    CuSuite* suite = CuSuiteNew();
    
    CuSuiteAddSuite(suite, urrd_util_get_suite());
    
    CuSuiteRun(suite);
    CuSuiteSummary(suite, output);
    CuSuiteDetails(suite, output);
    printf("%s\n", output->buffer);
}
    
int main(void) {
    RunAllTests();

    return 0;
}


