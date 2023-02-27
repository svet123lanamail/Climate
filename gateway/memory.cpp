#include "memory.h"

void FlashMemory::getFile(const char* filename, char* result) {
  if (String(filename).equalsIgnoreCase("table")) {
    strcpy_P(result, _tableFile);
  }
  else if (String(filename).equalsIgnoreCase("tableXml")) {
    strcpy_P(result, _tablePage);
  }
  else if (String(filename).equalsIgnoreCase("tasks")) {
    strcpy_P(result, _taskFile);
  }
  else if (String(filename).equalsIgnoreCase("index")) {
    strcpy_P(result, _indexPage);
  }
  else {
    strcpy_P(result, "err");
  }
}

__FlashStringHelper* FlashMemory::getPage(const char* pagename) {
  if (String(pagename).equalsIgnoreCase("index")) return (__FlashStringHelper*) _indexPage;
  if (String(pagename).equalsIgnoreCase("table")) return (__FlashStringHelper*) _tablePage;
  return nullptr;
}

void FlashMemory::insertStateRecord(const stateRecord& newRecord) {
  _records[_totalRecords % TOTAL_RECORDS] = newRecord;
  DEBUG("newrec: ", _records[_totalRecords % TOTAL_RECORDS].type);
  _totalRecords++;
}

stateRecord* FlashMemory::selectStateRecord(const char* searchPhrase, int& cnt) {
  bool getAll = String(searchPhrase).equals("");
  int counter = 0;
  Serial.print("records Total: "); Serial.println(_totalRecords);
  for (int i = 0; i < _totalRecords; i++) {
    if (String(_records[i].type).equalsIgnoreCase(searchPhrase) || getAll) counter++;
    Serial.print("record "); Serial.print(i);
    Serial.print(". "); Serial.println(_records[i].value);
  }
  cnt = 0;
  if (!counter) return nullptr;
  stateRecord* result = new stateRecord[counter];
  cnt = counter;
  Serial.print("found Total: "); Serial.println(cnt);
  for (int i = 0; i < _totalRecords % TOTAL_RECORDS; i++) {
    if (String(_records[i].type).equalsIgnoreCase(searchPhrase) || getAll) {
      result[--counter] = _records[i];
      Serial.println("search works");
    }
  }
  return result;
}

stateRecord::stateRecord(int id, const char* t, const char* v, unsigned long ti) : stationId(id), time(ti) {
  strcpy(type, t);
  strcpy(value, v);
}
stateRecord::stateRecord(const stateRecord& other) : stationId(other.stationId), time(other.time) {
  strcpy(type, other.type);
  strcpy(value, other.value);
}

stateRecord& stateRecord::operator= (const stateRecord& other) {
  stationId = other.stationId; 
  time = other.time;
  strcpy(type, other.type);
  strcpy(value, other.value);
}
