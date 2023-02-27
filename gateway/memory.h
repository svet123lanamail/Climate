#pragma once
#include <Arduino.h>
#include "files.h"

#define TOTAL_RECORDS 16

#define DEBUG(a, b) \
  Serial.print(a); Serial.println(b);

struct stateRecord {
  int stationId;
  char type[16];
  char value[8];
  unsigned long time;

  stateRecord() {};
  stateRecord(int id, const char* t, const char* v, unsigned long ti);
  stateRecord(const stateRecord& other);
  stateRecord& operator= (const stateRecord& other);
};

class IMemory {
    public:
    virtual void insertStateRecord(const stateRecord& newRecord) = 0;
    virtual stateRecord* selectStateRecord(const char*, int&) = 0;
    virtual void getFile(const char*, char*) = 0;
};

class FlashMemory : public IMemory {
    private:
    const char* _indexPage;
    const char* _tablePage;
    const char* _tableFile;
    const char* _taskFile;
    stateRecord _records[TOTAL_RECORDS];
    int _totalRecords;
    public:
    FlashMemory() : _indexPage(indexPage), _tableFile(tableFile), _taskFile(taskFile), _tablePage(tablePage), _totalRecords(0) {};
    virtual __FlashStringHelper* getPage(const char* pagename);
    virtual void getFile(const char*, char*) override;
    virtual void insertStateRecord(const stateRecord& newRecord) override;
    virtual stateRecord* selectStateRecord(const char* searchPhrase, int& count) override;
    virtual ~FlashMemory() {};
};
